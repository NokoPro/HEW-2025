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
    world.View<MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId e,
            MovementIntentComponent& intent,
            Rigidbody2DComponent& rb)
        {
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
                }
                intent.blinkRequested = false;
                intent.blinkSpeed = 0.0f;
            }
        }
    );
}
