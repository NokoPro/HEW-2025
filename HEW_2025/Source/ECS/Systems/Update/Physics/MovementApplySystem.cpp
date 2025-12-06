/*********************************************************************/
/* @file   MovementApplySystem.cpp
 * @brief  移動適用システム実装
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "MovementApplySystem.h"
#include <algorithm>  // std::clamp
#include "System/DebugSettings.h"
#include "ECS/Components/Effect/EffectComponent.h"
#include "ECS/Components/Effect/EffectSlotsComponent.h"
#include "ECS/Components/Core/PlayerStateComponent.h"

// CSV値との同期
void MovementApplySystem::SyncConfig()
{
    const auto& cfg = GlobalGameplayConfig::Instance().Get();
    m_groundAccel    = cfg.Get("groundAccel",    m_groundAccel);
    m_airAccel       = cfg.Get("airAccel",       m_airAccel);
    m_maxSpeedX      = cfg.Get("maxSpeedX",      m_maxSpeedX);
    m_jumpSpeed      = cfg.Get("jumpSpeed",      m_jumpSpeed);
    m_blinkUpImpulse = cfg.Get("blinkUpImpulse", m_blinkUpImpulse);
}

void MovementApplySystem::Update(World& world, float dt)
{
    // 毎フレーム外部設定を反映 (必要なら頻度調整可能)
    SyncConfig();

    // Intent + Rigidbody を持ってるやつだけ対象
    world.View<MovementIntentComponent,
           Rigidbody2DComponent,
           PlayerStateComponent>(
    [&](EntityId e,
        MovementIntentComponent& intent,
        Rigidbody2DComponent& rb,
        PlayerStateComponent& state)
        {
            // 相手コマンド実行フラグは 1 フレーム限定なので毎フレームリセット
            state.m_remoteCommandJumpThisFrame = false;
            state.m_remoteCommandBlinkThisFrame = false;

            const bool onGround = rb.onGround;

            float friction = 1.0f;
            if (auto* col = world.TryGet<Collider2DComponent>(e))
            {
                friction = col->material.friction;
                friction = std::clamp(friction, 0.0f, 1.0f);
            }

            const float targetVelX = intent.moveX * m_maxSpeedX * DebugSettings::Get().playerSpeed;
            const float accel = onGround ? m_groundAccel : m_airAccel;
            float vx = rb.velocity.x;
            const float toTarget = targetVelX - vx;
            float add = accel * dt;
            add *= friction;

            if (std::abs(toTarget) <= add)
            {
                vx = targetVelX;
            }
            else
            {
                vx += (toTarget > 0.0f) ? add : -add;
            }

            if (rb.onGround)
            {
                intent.forceJumpConsumed = false;
                intent.blinkConsumed = false;
                intent.isBlinking = false;
            }

            if (intent.forceJumpRequested)
            {
                rb.velocity.y = m_jumpSpeed;
                intent.forceJumpRequested = false;
                intent.forceJumpConsumed = true;

				// 強制ジャンプが実行されたフレームとしてフラグを立てる
                state.m_remoteCommandJumpThisFrame = true;
            }
            else if (intent.jump && (onGround || DebugSettings::Get().infiniteJump))
            {
                rb.velocity.y = m_jumpSpeed;
                rb.onGround = false;
            }

            vx += rb.accumulatedForce.x * dt;
            rb.velocity.y += rb.accumulatedForce.y * dt;
            rb.accumulatedForce = { 0.0f, 0.0f };
            rb.velocity.x = vx;

            if (intent.blinkRequested)
            {
                if (!intent.blinkConsumed)
                {
                    rb.velocity.x = intent.blinkSpeed;
                    if (rb.velocity.y < m_blinkUpImpulse)
                    {
                        rb.velocity.y = m_blinkUpImpulse;
                    }
                    intent.isBlinking = true;
                    intent.blinkConsumed = true;

					// 強制ブリンクが実行されたフレームとしてフラグを立てる
                    state.m_remoteCommandBlinkThisFrame = true;

                    // ここでブリンクを実行したタイミングでエフェクト再生を指示
                    if (auto* efc = world.TryGet<EffectComponent>(e))
                    {
                        if (auto* slots = world.TryGet<EffectSlotsComponent>(e))
                        {
                            if (slots->onBlink)
                            {
                                efc->effect = slots->onBlink;
                                // スロットの既定パラメータを反映
                                efc->offset      = slots->onBlinkParams.offset;
                                efc->rotationDeg = slots->onBlinkParams.rotationDeg;
                                efc->scale       = slots->onBlinkParams.scale;
                            }
                        }
                        // 向きに合わせてミラー（右:+1, 左:-1）。ここではキャラクターの facing を利用
                        efc->rotationDeg.y = (intent.facing >= 0) ? 180.0f : -180.0f;
                        efc->offset.x = (intent.facing >= 0) ? std::fabs(efc->offset.x) : -std::fabs(efc->offset.x);
                        efc->scale.x  = (intent.facing >= 0) ? std::fabs(efc->scale.x)  : -std::fabs(efc->scale.x);

                        efc->loop = false;
                        efc->playRequested = true;
                    }
                }
                intent.blinkRequested = false;
                intent.blinkSpeed = 0.0f;
            }
        }
    );
}
