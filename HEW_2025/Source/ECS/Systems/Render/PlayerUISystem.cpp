/*********************************************************************/
/* @file   PlayerUISystem.cpp
 * @brief  プレイヤーのUI描画するかを切り替える処理
 *
 * ルール: プレイヤーがジャンプ入力した瞬間にUI表示を有効化し、
 * 一定時間経過で自動的に非表示。
 *
 * @author 土本蒼翔
 * @date   2025/11/17
 *********************************************************************/
#include "PlayerUISystem.h"

void PlayerUISystem::Update(World& world, float dt)
{
    // 1) 入力を見て、ジャンプが押されたらUIを点灯
    bool anyJump = false;
    world.View<PlayerInputComponent, MovementIntentComponent>(
        [&](EntityId, const PlayerInputComponent&, const MovementIntentComponent& intent)
        {
            if (intent.jump || intent.forceJumpRequested) anyJump = true;
        }
    );

    // 2) UIエンティティ群を更新（点灯/消灯、タイマ更新）
    world.View<PlayerUIComponet, Sprite2DComponent>(
        [&](EntityId, PlayerUIComponet& ui, Sprite2DComponent& sp)
        {
            if (anyJump)
            {
                ui.playerHI = true;            // ジャンプ通知
                ui.playerUItimer = 1.0f;       // 表示時間リセット
                if (sp.alias.empty())
                {
                    sp.alias = "uI_HOP";     // ジャンプUI用のテクスチャエイリアス
                }
            }

            // タイマ減衰
            if (ui.playerHI)
            {
                ui.playerUItimer -= dt;
                if (ui.playerUItimer <= 0.0f)
                {
                    ui.playerHI = false;
                    ui.playerUItimer = 0.0f;
                    sp.alias.clear(); // 非表示にする
                }
            }
        }
    );
}
