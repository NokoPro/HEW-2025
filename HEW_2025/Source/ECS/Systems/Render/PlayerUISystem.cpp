/*********************************************************************/
/* @file   PlayerUISystem.cpp
 * @brief  プレイヤーのUI描画するかを切り替える処理
 *
 * ルール: 対象プレイヤー(=FollowerComponent.targetId)がジャンプ入力/強制ジャンプ要求した瞬間に
 * UI表示を有効化し、一定時間経過で自動的に非表示。
 *
 * @author 土本蒼翔
 * @date   2025/11/17
 *********************************************************************/
#include "PlayerUISystem.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"

void PlayerUISystem::Update(World& world, float dt)
{
    // 各UIエンティティごとに、対応するターゲットの入力状態を参照して表示を切替
    world.View<FollowerComponent, PlayerUIComponet, Sprite2DComponent>(
        [&](EntityId /*uiEnt*/, const FollowerComponent& follower, PlayerUIComponet& ui, Sprite2DComponent& sp)
        {
            // ターゲットのMovementIntentを取得
            const PlayerInputComponent* intent = world.TryGet<PlayerInputComponent>(follower.targetId);
            bool trigger = false;
            if (intent)
            {
                // 強制ジャンプ要求で点灯
                if (intent->isJumpRequested)
                {
                    trigger = true;
                }
            }

            if (trigger)
            {
                ui.playerHI = true;      // 点灯フラグ
                ui.playerUItimer = 1.0f; // 表示時間リセット
                sp.visible = true;       // 表示ON（テクスチャaliasは固定でOK）
            }

            // タイマ減衰して自動消灯
            if (ui.playerHI)
            {
                ui.playerUItimer -= dt;
                if (ui.playerUItimer <= 0.0f)
                {
                    ui.playerHI = false;
                    ui.playerUItimer = 0.0f;
                    sp.visible = false;  // 表示OFF
                }
            }
        }
    );
}
