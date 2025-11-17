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

// 表示保持時間
static constexpr float UI_SHOW_TIME = 1.0f;
// オフセット（カメラのオーソサイズに合わせ小さめ)
static constexpr float OFFSET_JUMP_Y  = 3.03f;       // 無理やり合わせてるから本番は統一
static constexpr float OFFSET_BLINK_Y = 2.999f;

void PlayerUISystem::Update(World& world, float dt)
{
    // プレイヤー番号ごとの入力状態取得
    // プレイヤーインデックス -> 入力トリガ保持
    bool playerForceJump[2] = { false,false };
    bool playerBlink[2]     = { false,false };

    world.View<PlayerInputComponent>(
        [&](EntityId, const PlayerInputComponent& pic)
        {
            const int idx = pic.playerIndex;
            if (idx < 0 || idx > 1) return; // 想定外はスキップ

            playerBlink[idx] = pic.isBlinkRequested;
            playerForceJump[idx] = pic.isJumpRequested;
        }
    );

    // UIエンティティ更新: Follower + PlayerUI + Sprite
    world.View<FollowerComponent, PlayerUIComponet, Sprite2DComponent>(
        [&](EntityId, FollowerComponent& follower, PlayerUIComponet& ui, Sprite2DComponent& sp)
        {
            // 追従対象の PlayerInputComponent が必要
            const PlayerInputComponent* input = world.TryGet<PlayerInputComponent>(follower.targetId);
            if (!input) { sp.visible = false; return; }
            int idx = input->playerIndex;
            if (idx < 0 || idx > 1) { sp.visible = false; return; }

            // ジャンプUI
            if (sp.alias == "ui_jump")
            {
                if (playerForceJump[idx])
                {
                    ui.jumpActive = true;
                    ui.jumpTimer  = UI_SHOW_TIME;
                    sp.visible = true;
                    follower.offset.y = OFFSET_JUMP_Y;
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
                ui.playerHI = ui.jumpActive; // 旧互換
                ui.playerUItimer = ui.jumpTimer;
                return;
            }
            // ブリンクUI
            if (sp.alias == "ui_blink")
            {
                if (playerBlink[idx])
                {
                    ui.blinkActive = true;
                    ui.blinkTimer  = UI_SHOW_TIME;
                    sp.visible = true;
                    follower.offset.y = OFFSET_BLINK_Y;
                }
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
                ui.playerYO = ui.blinkActive; // 旧互換
                return;
            }
            // その他は非表示
            sp.visible = false;
        }
    );
}
