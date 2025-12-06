#include "PlayerLocomotionStateSystem.h"
#include <cmath>

void PlayerLocomotionStateSystem::Update(World& world, float /*deltaTime*/)
{
    world.View<MovementIntentComponent,
        Rigidbody2DComponent,
        PlayerStateComponent>(
            [&](EntityId /*entity*/,
                const MovementIntentComponent& intent,
                const Rigidbody2DComponent& rb,
                PlayerStateComponent& state)
            {
                // 前フレーム状態を保存
                state.m_prevLocomotion = state.m_locomotion;
                state.m_prevFacing = state.m_facing;

                // --- 向きの決定 ---
                // ゴール中など「強制Forward」にしたい場合は、
                // ここで別のフラグを見て分岐させる想定。
                // 今は単純に Intent から決める。
                if (intent.facing > 0)
                {
                    state.m_facing = PlayerFacingState::Right;
                }
                else if (intent.facing < 0)
                {
                    state.m_facing = PlayerFacingState::Left;
                }
                // 強制Forward用の状態になったらここで
                // state.m_facing = PlayerFacingState::Forward;

                // --- ロコモーションの決定 ---
                // 空中か地上か
                if (!rb.onGround)
                {
                    if (rb.velocity.y > 0.0f)
                    {
                        state.m_locomotion = PlayerLocomotionState::Jump;
                    }
                    else
                    {
                        state.m_locomotion = PlayerLocomotionState::Fall;
                    }
                }
                else
                {
                    // 地上なら Idle / Walk を速度で分ける
                    const float speedX = std::fabs(rb.velocity.x);
                    constexpr float kWalkSpeedThreshold = 0.1f;

                    if (speedX < kWalkSpeedThreshold)
                    {
                        // Idleクリップがないので、ここでは「アニメ停止」の None を使う
                        state.m_locomotion = PlayerLocomotionState::None;
                        // Idle クリップを用意したら：
                        // state.m_locomotion = PlayerLocomotionState::Idle;
                    }
                    else
                    {
                        state.m_locomotion = PlayerLocomotionState::Walk;
                    }
                }

                // Dash / Blink など「特別な状態」があったら、最後に上書きするイメージ
                // if (intent.dash)  { state.m_locomotion = PlayerLocomotionState::Dash; }
                // if (intent.blink) { state.m_locomotion = PlayerLocomotionState::Blink; }
            });
}
