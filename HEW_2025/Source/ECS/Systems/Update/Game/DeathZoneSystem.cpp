/*********************************************************************/
/* @file   DeathZoneSystem.cpp
 * @brief  死亡ゾーン（DeathZone）システム実装
 * * @author 浅野勇生
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
#include "System/Defines.h"
#include "System/DebugSettings.h"
#include "System/TimeAttackManager.h"

#include <Windows.h> // MessageBox
#include <Scene/GameScene.h>
#include <Scene/ResultScene.h>
#include <Scene/StageSelectScene.h>
#include <Scene/TestStageScene.h>

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
    const bool god = DebugSettings::Get().godMode;

    // Deathゾーンの上昇処理
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e, TransformComponent& tr, Collider2DComponent& col)
        {
            if (col.layer == Physics::LAYER_DESU_ZONE)
            {
                if (magmaOn)
                {
                    // 変更点: 定数ではなくメンバ変数の m_riseSpeed を使用
                    tr.position.y += dt * m_riseSpeed * speedScale; // 上昇速度（倍率）
                }
            }
        });

    /// Deathゾーンに被弾したかチェック
    if (!magmaOn || m_triggered || !m_colSys) return;
    CollisionEventBuffer* eventBuffer = m_colSys->GetEventBuffer();
    if (!eventBuffer) return;

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
                DebugSettings::Get().gameDead = true;
                DebugSettings::Get().gameTimerRunning = false; // 停止
                TimeAttackManager::Get().NotifyDeath();

                MessageBoxA(nullptr, "Deathゾーンに接触しました!", "Game Over", MB_OK | MB_ICONEXCLAMATION);
                world.View<GameOverMenu, Sprite2DComponent>(
                    [&](EntityId e, const GameOverMenu& gom, Sprite2DComponent& sprite)
                    {
                        sprite.visible = true;
                    });
                break;
            }
        }
    }
}
void DeathZoneSystem::GameOverUpdate(World& world)
{
   

	//現在のシーンを取得
    GameScene* currentScene = dynamic_cast<GameScene*>(CurrentScene());
    // GameSceneのゲッターを使ってステージ番号と難易度を取得
  
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
            // ダウンキャスト失敗時など、StageSelectSceneへ遷移（安全策）
            ChangeScene<StageSelectScene>();
        }

    }
    else if (IS_LEFT || IS_DOWN)
	{//ステージセレクトへ戻る
               // シーン遷移処理
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
        if (m_sceneSwitch == true)
        {
            // 現在のステージ・難易度でGameSceneを再生成（リロード）
            ChangeScene<GameScene>(stageNo, difficulty);
        }
        else
        {
            ChangeScene<StageSelectScene>();
        }
       
    }
}
