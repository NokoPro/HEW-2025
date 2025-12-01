#include "PlayerLocomotionAnimSystem.h"
#include <cmath>

void PlayerLocomotionAnimSystem::Update(World& world, float /*dt*/)
{
    //// 移動Intent + 物理ボディ + アニメ状態を持っているエンティティを対象
    //world.View<MovementIntentComponent,
    //    Rigidbody2DComponent,
    //    ModelAnimationStateComponent>(
    //        [&](EntityId /*e*/,
    //            const MovementIntentComponent& intent,
    //            const Rigidbody2DComponent& rb,
    //            ModelAnimationStateComponent& animState)
    //        {
    //            // MovementApplySystem が rb.onGround / rb.velocity を更新済み :contentReference[oaicite:2]{index=2}
    //            const bool  onGround = rb.onGround;
    //            const float vx = rb.velocity.x;
    //            const float vy = rb.velocity.y;

    //            // 切り替えノイズを減らすためのしきい値
    //            constexpr float RUN_THRESHOLD = 0.2f;  // これ以上の速さなら走り
    //            constexpr float STOP_THRESHOLD = 0.1f; // これ未満なら停止扱い

    //            ModelAnimState next = animState.current;

    //            if (onGround)
    //            {
    //                // 地上：ほぼ停止なら Idle、ある程度速度があれば Run
    //                if (std::fabs(vx) < STOP_THRESHOLD)
    //                {
    //                    next = ModelAnimState::Idle;
    //                }
    //                else if (std::fabs(vx) > RUN_THRESHOLD)
    //                {
    //                    next = ModelAnimState::Run;
    //                }
    //                else
    //                {
    //                    // 中間帯はとりあえず前の状態を維持（ガクガク防止）
    //                    // 何もしない
    //                }
    //            }
    //            else
    //            {
    //                // 空中：上昇中と下降中で分ける
    //                if (vy > 0.0f)
    //                {
    //                    next = ModelAnimState::Jump;
    //                }
    //                else
    //                {
    //                    next = ModelAnimState::Fall;
    //                }
    //            }

    //            // 変化がなければ何もしない
    //            if (next == animState.current)
    //            {
    //                return;
    //            }

    //            // ステート切り替えリクエスト
    //            RequestModelAnimation(animState, next);
    //        });
}
