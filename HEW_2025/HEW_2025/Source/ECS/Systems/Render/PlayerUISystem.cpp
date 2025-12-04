/*********************************************************************/
/* @file   PlayerUISystem.cpp
 * @brief  プレイヤー入力(強制ジャンプ/ブリンク)に応じたUI表示制御 (同時表示対応 / 個別タイマ)
 *
 * 各UIエンティティは alias ("ui_jump" / "ui_blink") で種類識別し、
 * PlayerUIComponet でジャンプ/ブリンクを個別のタイマとフラグで管理。
 * 
 * @author 土本 蒼翔
 * @date   2025/11/17
 *********************************************************************/
#include "PlayerUISystem.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"

// 表示保持時間
static constexpr float UI_SHOW_TIME = 1.0f;
// オフセット（カメラのオーソサイズに合わせ小さめ)
static constexpr float OFFSET_JUMP_Y  = 3.03f;       // 無理やり合わせてるから本番は統一
static constexpr float OFFSET_BLINK_Y = 2.999f;

void PlayerUISystem::Update(World& world, float dt)
{
    // UIエンティティ更新: Follower + PlayerUI + Sprite
    world.View<FollowerComponent, PlayerUIComponet, Sprite2DComponent>(
        [&](EntityId, FollowerComponent& follower, PlayerUIComponet& ui, Sprite2DComponent& sp)
        {
            PlayerInputComponent* inputFollow = world.TryGet<PlayerInputComponent>(follower.targetId);
            if (!inputFollow) { sp.visible = false; return; }
            int followIdx = inputFollow->playerIndex;
            if (followIdx < 0 || followIdx > 1) { sp.visible = false; return; }

            // ジャンプUI（追従先の入力でワンショット表示）
            if (sp.alias == "ui_jump")
            {
                if (inputFollow->isJumpRequested)
                {
                    ui.jumpActive = true;
                    ui.jumpTimer  = UI_SHOW_TIME;
                    sp.visible = true;
                    follower.offset.y = OFFSET_JUMP_Y;
                    inputFollow->isJumpRequested = false;
                }
                if (ui.jumpActive)
                {
                    ui.jumpTimer -= dt;
                    if (ui.jumpTimer <= 0.0f)
                    {
                        ui.jumpActive = false;
                        ui.jumpTimer = 0.0f;
                        sp.visible = false;
                    }
                }
                ui.playerHI = ui.jumpActive;
                ui.playerUItimer = ui.jumpTimer;
                return;
            }

            // ブリンクUI: Lボタンを押した側（＝ブリンクしていない側）に表示する
            if (sp.alias == "ui_blink")
            {
                // 押した側のフラグ
                const bool blinkRequestedByFollower = inputFollow->isBlinkRequested;

                // ブリンク実行者（フォロワーとは逆のプレイヤー）のEntityIdを取得
                EntityId blinkerId = follower.targetId; // 初期値は追従先。同一なら後で差し替え
                world.View<PlayerInputComponent>(
                    [&](EntityId e, const PlayerInputComponent& pic)
                    {
                        if (pic.playerIndex != followIdx)
                        {
                            blinkerId = e;
                        }
                    }
                );

                MovementIntentComponent* blinkerIntent = world.TryGet<MovementIntentComponent>(blinkerId);
                Rigidbody2DComponent* blinkerRb = world.TryGet<Rigidbody2DComponent>(blinkerId);

                const bool blinking = blinkerIntent ? blinkerIntent->isBlinking : false;
                const bool risingEdge = (blinking && !ui.prevBlinking);
                const bool canShowAgain = blinkerRb ? blinkerRb->onGround : false;

                // 地面に着いたら使用済み解除（再表示可能）。かつ、押した側がL押下中ならタイマーを即再更新して表示し直しOK。
                if (canShowAgain)
                {
                    ui.blinkUsed = false;
                    if (blinkRequestedByFollower)
                    {
                        ui.blinkActive = true;
                        ui.blinkTimer  = UI_SHOW_TIME;
                        sp.visible = true;
                        follower.offset.y = OFFSET_BLINK_Y;
                        // 押した側のワンショットはクリア（継続押しでも毎フレーム再スタートは防ぐ）
                        inputFollow->isBlinkRequested = false;
                    }
                }

                // 立ち上がり時に開始。未使用のときだけ。
                if (risingEdge && blinkRequestedByFollower && !ui.blinkActive && !ui.blinkUsed)
                {
                    ui.blinkActive = true;
                    ui.blinkTimer  = UI_SHOW_TIME;
                    sp.visible = true;
                    follower.offset.y = OFFSET_BLINK_Y;
                    ui.blinkUsed = true;
                    ui.blinkOwnerIndex = (followIdx ^ 1); // 0/1反転で相手
                    inputFollow->isBlinkRequested = false;
                }

                // タイマー進行
                if (ui.blinkActive)
                {
                    ui.blinkTimer -= dt;
                    if (ui.blinkTimer <= 0.0f)
                    {
                        ui.blinkActive = false;
                        ui.blinkTimer = 0.0f;
                        sp.visible = false;
                    }
                }

                // 前フレームの状態を保存（立ち上がり検出用）
                ui.prevBlinking = blinking;
                ui.playerYO = ui.blinkActive;
                return;
            }

            // その他は非表示
            sp.visible = false;
        }
    );
}
