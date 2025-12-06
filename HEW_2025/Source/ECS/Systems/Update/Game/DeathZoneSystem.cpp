/*********************************************************************/
/* @file   DeathZoneSystem.cpp
 * @brief  死亡ゾーン（DeathZone）システム実装
 * *author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "DeathZoneSystem.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Tag/Tag.h"
#include "ECS/Prefabs/PrefabGameOver.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "System/AssetManager.h"
#include "ECS/Components/Render/DeathTextureOverrideComponent.h"

#include "System/Defines.h"
#include "System/DebugSettings.h"
#include "System/TimeAttackManager.h"

#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/StageSelectScene.h"
#include "Scene/TestStageScene.h"

#include <Windows.h> // MessageBox

 // 便利なマクロ
#define IS_DECIDE (IsKeyTrigger(VK_SPACE) || IsKeyTrigger(VK_RETURN) || IsPadTrigger(0, XINPUT_GAMEPAD_A))
#define IS_CANCEL (IsKeyTrigger(VK_BACK)  || IsPadTrigger(0, XINPUT_GAMEPAD_B))
#define IS_RIGHT  (IsKeyTrigger(VK_RIGHT) || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_RIGHT) || GetPadLX(0) > 0.5f)
#define IS_LEFT   (IsKeyTrigger(VK_LEFT)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_LEFT)  || GetPadLX(0) < -0.5f)
#define IS_UP     (IsKeyTrigger(VK_UP)    || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_UP)    || GetPadLY(0) > 0.5f)
#define IS_DOWN   (IsKeyTrigger(VK_DOWN)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)  || GetPadLY(0) < -0.5f)


void DeathZoneSystem::Update(World& world, float dt)
{
    const bool magmaOn = DebugSettings::Get().magmaEnabled;
    const float speedScale = DebugSettings::Get().magmaSpeedScale;

    // Deathゾーンの上昇処理（通常時）
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e, TransformComponent& tr, Collider2DComponent& col)
        {
            if (col.layer == Physics::LAYER_DESU_ZONE)
            {
                if (magmaOn && !m_triggered)
                {
                    tr.position.y += dt * m_riseSpeed * speedScale; // 上昇速度（倍率）
                }
            }
        });

    // デス後：まず遅延時間中は上昇を停止し、演出可能時間を確保
    if (m_triggered)
    {
        if (m_inDelay)
        {
            m_delayRemainSec -= dt;
            if (m_delayRemainSec <= 0.0f)
            {
                m_inDelay = false;
                m_delayRemainSec = 0.0f;
            }
        }
        
        // 遅延が終わったら、一定時間で画面上端まで移動（カメラが動いても追従）
        if (!m_inDelay)
        {
            // 毎フレーム現在のカメラから目標上端Yを更新
            EntityId camEntity = kInvalidEntity;
            TransformComponent* camTr = nullptr;
            world.View<ActiveCameraTag>([&](EntityId e, const ActiveCameraTag&) { camEntity = e; });
            if (camEntity != kInvalidEntity && world.Has<TransformComponent>(camEntity))
            {
                camTr = &world.Get<TransformComponent>(camEntity);
            }
            if (camTr)
            {
                m_targetScreenTopY = camTr->position.y + m_cameraTopOffset;
            }

            // 残り時間があれば補間、無ければ目標座標に揃える
            world.View<TransformComponent, Collider2DComponent>(
                [&](EntityId e, TransformComponent& tr, Collider2DComponent& col)
                {
                    if (col.layer != Physics::LAYER_DESU_ZONE) return;

                    // 上端 = 本体Y + 半高さ（形状別）
                    float halfY = 0.0f;
                    switch (col.shape)
                    {
                    case ColliderShapeType::AABB2D: halfY = col.aabb.halfY; break;
                    case ColliderShapeType::CIRCLE2D: halfY = col.circle.radius; break;
                    case ColliderShapeType::CAPSULE2D: halfY = col.capsule.halfHeight + col.capsule.radius; break;
                    }

                    const float targetBaseY = m_targetScreenTopY - halfY;
                    if (m_fillRemainSec > 0.0f)
                    {
                        const float ratio = (dt >= m_fillRemainSec) ? 1.0f : (dt / m_fillRemainSec);
                        const float delta = targetBaseY - tr.position.y;
                        tr.position.y += delta * ratio; // 時間に依存する線形補間
                    }
                    else
                    {
                        tr.position.y = targetBaseY; // 到達
                    }
                });

            if (m_fillRemainSec > 0.0f)
            {
                m_fillRemainSec -= dt;
                if (m_fillRemainSec < 0.0f) m_fillRemainSec = 0.0f;
            }
        }
    }

    /// Deathゾーンに被弾したかチェック（被弾後は早期リターン）
    if (!magmaOn || m_triggered || !m_colSys) return;
    CollisionEventBuffer* eventBuffer = m_colSys->GetEventBuffer();
    if (!eventBuffer) return;

    const bool god = DebugSettings::Get().godMode;

    // プレイヤーエンティティ取得
    EntityId player1 = 0, player2 = 0;
    world.View<PlayerInputComponent>(
        [&](EntityId e, const PlayerInputComponent& pic)
        {
            if (player1 == 0) player1 = e;
            else if (player2 == 0) player2 = e;
        });

    // 衝突イベントバッファからプレイヤーとDeathゾーンの接触を検出
    for (const auto& ev : eventBuffer->events)
    {
        if ((ev.self == player1 || ev.self == player2) && ev.trigger)
        {
            auto* colOther = world.TryGet<Collider2DComponent>(ev.other);
            if (colOther && colOther->layer == Physics::LAYER_DESU_ZONE)
            {
                if (god) { continue; }
                m_triggered = true;
                m_moveToScreenTop = true; // 画面上端への移動開始
                DebugSettings::Get().gameDead = true;
                DebugSettings::Get().gameTimerRunning = false; // 停止
                TimeAttackManager::Get().NotifyDeath();

                // MessageBoxA(nullptr, "Deathゾーンに接触しました!", "Game Over", MB_OK | MB_ICONEXCLAMATION);

                // カメラエンティティと位置取得（ActiveCameraTag）
                EntityId camEntity2 = kInvalidEntity;
                TransformComponent* camTr2 = nullptr;
                world.View<ActiveCameraTag>([&](EntityId e, const ActiveCameraTag&) { camEntity2 = e; });
                if (camEntity2 != kInvalidEntity && world.Has<TransformComponent>(camEntity2))
                {
                    camTr2 = &world.Get<TransformComponent>(camEntity2);
                }

                // デス時点の画面上端Yを確定し、埋まり時間カウントを開始
                if (camTr2)
                {
                    m_targetScreenTopY = camTr2->position.y + m_cameraTopOffset;
                }
                m_fillRemainSec = m_fillDurationSec;

                // 上昇開始遅延をセット（この間に生存キャラの演出が可能）
                m_inDelay = true;
                m_delayRemainSec = m_postDeathDelaySec;

                // ゲームオーバーUIを表示し、死亡時点のカメラ位置に合わせる
                world.View<GameOverMenu, Sprite2DComponent, TransformComponent>(
                    [&](EntityId e, const GameOverMenu& gom, Sprite2DComponent& sprite, TransformComponent& tr)
                    {
                        sprite.visible = true;
                        if (camTr2)
                        {
                            tr.position.x = camTr2->position.x;
                            tr.position.y = camTr2->position.y;
                            // UIは前面にするため僅かに手前へ（必要なら固定Z）
                            tr.position.z = camTr2->position.z + 0.0f;
                        }
                    });

                // 以降も追従させたい場合はフォロワーを付与
                world.View<GameOverMenu>(
                    [&](EntityId e, const GameOverMenu&)
                    {
                        if (camEntity2 != kInvalidEntity)
                        {
                            if (!world.Has<FollowerComponent>(e))
                            {
                                auto& fol = world.Add<FollowerComponent>(e);
                                fol.targetId = camEntity2;
                                fol.offset = { 0.0f, 0.0f };
                            }
                            else
                            {
                                world.Get<FollowerComponent>(e).targetId = camEntity2;
                            }
                        }
                    });

                // 生存プレイヤーに演出用テクスチャとアニメーションを設定
                {
                    EntityId survivor = (ev.self == player1) ? player2 : player1;
                    EntityId deadOne = (ev.self == player1) ? player1 : player2;

                    AssetHandle<Texture> tex = AssetManager::GetTexture("tex_aousagi_gameover");

                    if (survivor != 0 && survivor != kInvalidEntity)
                    {
                        // モデルの強制テクスチャ上書きをコンポーネントで指示
                        if (!world.Has<DeathTextureOverrideComponent>(survivor))
                        {
                            auto& dto = world.Add<DeathTextureOverrideComponent>(survivor);
                            dto.texture = tex;
                            dto.enabled = true;
                        }
                        else
                        {
                            auto& dto = world.Get<DeathTextureOverrideComponent>(survivor);
                            dto.texture = tex;
                            dto.enabled = true;
                        }

                        if (world.Has<ModelAnimationStateComponent>(survivor))
                        {
                            world.Get<ModelAnimationStateComponent>(survivor).requested = ModelAnimState::Death;
                        }
                    }

                    // 死亡側は上書き解除しておく（必要なら）
                    if (deadOne != 0 && deadOne != kInvalidEntity)
                    {
                        if (world.Has<DeathTextureOverrideComponent>(deadOne))
                        {
                            world.Get<DeathTextureOverrideComponent>(deadOne).enabled = false;
                        }
                        if (world.Has<ModelAnimationStateComponent>(deadOne))
                        {
                            world.Get<ModelAnimationStateComponent>(deadOne).requested = ModelAnimState::Death;
                        }
                    }
                }
                break;
            }
        }
    }
}

void DeathZoneSystem::GameOverUpdate(World& world)
{
	//現在のシーンを取得
    GameScene* currentScene = dynamic_cast<GameScene*>(CurrentScene());
    
    int stageNo = currentScene->GetStageNo();
    Difficulty difficulty = currentScene->GetDifficulty();
        
	//continueかStageSelectに戻るか選択させる
    if (IS_RIGHT || IS_UP)
    {//コンティニュー
        if (currentScene)
        {
            m_sceneSwitch = true;
        }
        else
        {
            ChangeScene<StageSelectScene>();
        }

    }
    else if (IS_LEFT || IS_DOWN)        //ステージセレクトへ戻る
	{           
        m_sceneSwitch = false;
        
    }

    //プレイヤーとDeathゾーンの間の距離を検出
    std::vector<EntityId> players;
    world.View<PlayerInputComponent>(
        [&](EntityId e, const PlayerInputComponent& pic)
        {
            players.push_back(e);
        });

    std::vector<EntityId> deathZones;
    world.View<Collider2DComponent>(
        [&](EntityId e, const Collider2DComponent& col)
        {
            if (col.layer == Physics::LAYER_DESU_ZONE)
                deathZones.push_back(e);
        });

    for (EntityId player : players)
    {
        auto* trPlayer = world.TryGet<TransformComponent>(player);
        if (!trPlayer) continue;

        for (EntityId deathZone : deathZones)
        {
            auto* trDeathZone = world.TryGet<TransformComponent>(deathZone);
            if (!trDeathZone) continue;

            //縦の方向のデスゾーンとプレイヤーの間の距離計算
            float dy = trPlayer->position.y - trDeathZone->position.y;
            m_dis = dy * dy;

            if (m_dis <= m_warningDistance * m_warningDistance)
            {
                m_warningActive = true;
                break;
            }
            else
            {
                m_warningActive = false;
                m_oldDis = m_warningDistance * m_warningDistance;
                break;
            }
        }
    }

    // 警告演出
    if (m_warningActive)
    {
        if (m_oldDis > m_dis)
        {   
            m_oldDis = m_dis;
        }
    }

    if (IS_DECIDE)
    {
        // 遅延シーン遷移リクエストに切り替え（この場でChangeSceneしない）
        m_requestSceneChange = true;
        m_pendingStageNo = stageNo;
        m_pendingDifficulty = difficulty;
        // m_sceneSwitch は既存ロジックで右/上がtrue, 左/下がfalseに設定済み
    }
}
