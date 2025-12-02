#include "PlayerPresentationSystem.h"
#include <cmath>

#include "ECS/Components/Effect/EffectComponent.h"
#include "ECS/Components/Effect/EffectSlotsComponent.h"

// 前方宣言 or ヘッダから
// enum class ModelAnimState;
// struct ModelAnimationStateComponent;

static float LerpAngle(float current, float target, float t)
{
    // ざっくりでOK（既存のヘルパーがあればそちらを使って）
    float diff = target - current;
    // -180～+180 に正規化とかやるならここ
    return current + diff * t;
}

void PlayerPresentationSystem::Update(World& world, float /*dt*/)
{
    world.View<
        PlayerStateComponent,
        ModelAnimationStateComponent,
        TransformComponent,
        EffectComponent,
        EffectSlotsComponent
    >(
        [&](EntityId /*entity*/,
            PlayerStateComponent& state,
            ModelAnimationStateComponent& animState,
            TransformComponent& transform,
            EffectComponent& effect,
            const EffectSlotsComponent& effectSlots)
        {
            // ============================================================
            // 1. 向きに応じてモデル回転を決める
            // ============================================================
            // 今は単純に Y 回転だけで左右反転している想定
            switch (state.m_facing)
            {
            case PlayerFacingState::Right:
                transform.rotationDeg.y = 130.0f;
                break;
            case PlayerFacingState::Left:
                transform.rotationDeg.y = -130.0f;
                break;
            default:
                break;
            }

            // ============================================================
            // 2. LocomotionState -> モデルアニメの指定
            // ============================================================
            // ここは既存のまま（Idle / Walk / Jump / Fall など）
            animState.requested = ModelAnimState::None;

            switch (state.m_locomotion)
            {
            case PlayerLocomotionState::None:
            case PlayerLocomotionState::Idle:
                // Idle アニメがないので、ここでは何も再生しない
                animState.requested = ModelAnimState::Idle;
                break;

            case PlayerLocomotionState::Walk:
                animState.requested = ModelAnimState::Walk;
                break;

            case PlayerLocomotionState::Jump:
            case PlayerLocomotionState::Fall:
                animState.requested = ModelAnimState::Jump;
                break;

            default:
                break;
            }

            // ============================================================
            // 3. 「状態が変わった瞬間」にエフェクトを鳴らす
            // ============================================================
            const bool locomotionChanged =
                (state.m_locomotion != state.m_prevLocomotion);

            if (locomotionChanged)
            {
                const auto isGround =
                    [](PlayerLocomotionState s)
                    {
                        return s == PlayerLocomotionState::None ||
                            s == PlayerLocomotionState::Idle ||
                            s == PlayerLocomotionState::Walk;
                    };

                const auto isAir =
                    [](PlayerLocomotionState s)
                    {
                        return s == PlayerLocomotionState::Jump ||
                            s == PlayerLocomotionState::Fall;
                    };

                const bool wasGround = isGround(state.m_prevLocomotion);
                const bool isGroundNow = isGround(state.m_locomotion);
                const bool wasAir = isAir(state.m_prevLocomotion);
                const bool isAirNow = isAir(state.m_locomotion);

                const bool startedJump = (wasGround && isAirNow);
                const bool landed = (wasAir && isGroundNow);

                // 向きに応じて EffectParams を反転させる小ヘルパ
                auto MakeParamsWithFacing =
                    [&](const EffectParams& src) -> EffectParams
                    {
                        EffectParams p = src;

                        const int sign =
                            (state.m_facing == PlayerFacingState::Right) ? 1 : -1;

                        // X 方向だけ左右反転（オフセット & スケール）
                        p.rotationDeg.y = (sign >= 0) ? 180.0f : -180.0f;

                        p.offset.x = (sign >= 0 ? std::fabs(p.offset.x) : -std::fabs(p.offset.x));
                        p.scale.x = (sign >= 0 ? std::fabs(p.scale.x) : -std::fabs(p.scale.x));

                        // Y 回転も左右で反転させる（モデル側の回転に合わせる）

                        return p;
                    };

                // -------------------------
                // 3-1. ジャンプ開始
                // -------------------------
                if (startedJump && effectSlots.onJump)
                {
                    EffectParams p = MakeParamsWithFacing(effectSlots.onJumpParams);

                    effect.effect = effectSlots.onJump;
                    effect.offset = p.offset;
                    effect.rotationDeg = p.rotationDeg;
                    effect.scale = p.scale;
                    effect.loop = false;
                    effect.playRequested = true;
                    // Player 本体なので autoDestroyEntity は false のまま
                }

                // -------------------------
                // 3-2. 着地（空中 → 地上）
                // -------------------------
                if (landed && effectSlots.onLand)
                {
                    EffectParams p = MakeParamsWithFacing(effectSlots.onLandParams);

                    effect.effect = effectSlots.onLand;
                    effect.offset = p.offset;
                    effect.offset.y = p.offset.y - 1.0f;
                    effect.rotationDeg = p.rotationDeg;
                    effect.scale = p.scale;
                    effect.loop = false;
                    effect.playRequested = true;
                }

                // 将来的に:
                //   - Dash 状態に入った瞬間 → effectSlots.onDash
                //   - Blink 状態に入った瞬間 → effectSlots.onBlink
                //   - GoalPose 状態に入った瞬間 → Goal 用スロット
                // という感じでここに case を足していけば OK。
            }
        }
    );
}