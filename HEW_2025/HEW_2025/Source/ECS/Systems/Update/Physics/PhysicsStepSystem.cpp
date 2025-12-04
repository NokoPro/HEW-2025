/*********************************************************************/
/* @file   PhysicsStepSystem.cpp
 * @brief  重力・衝突解決などを行う物理ステップシステム (検出は Collision2DSystem に委譲)
 * 
 * 安定化: 侵入量のスロップ/割合補正・軸別速度減衰でガクつき低減。
 * 横方向はスナップし過ぎないよう緩和。
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "PhysicsStepSystem.h"
#include <algorithm> // std::min, std::max, std::abs
#include <cmath>
#include "ECS/Components/Physics/CollisionEvents.h"

namespace 
{
    // 統一重力加速度（Y+が上の場合は負値）
    constexpr float kGravity = -25.0f;
    // 垂直方向の押し戻し割合 (接地/段差安定用)
    constexpr float kVerticalCorrectionPercent = 0.85f;
    // 水平方向の押し戻し割合 (過度なスナップを避けるため弱める)
    constexpr float kHorizontalCorrectionPercent = 0.30f;
    // 許容スロップ (これ以下の侵入は無視)
    constexpr float kPenetrationSlop = 0.01f;
    // 水平方向でめり込み中かつ更にめり込む向きの速度減衰係数 (0～1)
    constexpr float kHorizontalPenetrationDampen = 0.5f;
}

void PhysicsStepSystem::Update(World& world, float dt)
{


    // 1) 重力を速度へ適用　速度 -> 位置 反映
    world.View<TransformComponent, Rigidbody2DComponent>(
        [&](EntityId /*e*/, TransformComponent& tr, Rigidbody2DComponent& rb)
        {
            if (rb.useGravity)
            {
                rb.velocity.y += kGravity * dt;
            }

            tr.position.x += rb.velocity.x * dt;
            tr.position.y += rb.velocity.y * dt;
            rb.onGround = false; // 接地は後で再判定
        }
    );

    // 2) 衝突解決 (イベント経由)
    if (m_eventBuffer)
    {
        for (const auto& ev : m_eventBuffer->events)
        {
            if (ev.trigger) continue;
            if (ev.self >= ev.other) continue; // ペアを一方向に統一

            auto* tA = world.TryGet<TransformComponent>(ev.self);
            auto* tB = world.TryGet<TransformComponent>(ev.other);
            auto* cA = world.TryGet<Collider2DComponent>(ev.self);
            auto* cB = world.TryGet<Collider2DComponent>(ev.other);
            if (!tA || !tB || !cA || !cB) continue;
            if (cA->shape != ColliderShapeType::AABB2D || cB->shape != ColliderShapeType::AABB2D) continue;

            auto* rbA = world.TryGet<Rigidbody2DComponent>(ev.self);
            auto* rbB = world.TryGet<Rigidbody2DComponent>(ev.other);

            // AABB
            const float axMin = tA->position.x + cA->offset.x - cA->aabb.halfX;
            const float axMax = tA->position.x + cA->offset.x + cA->aabb.halfX;
            const float ayMin = tA->position.y + cA->offset.y - cA->aabb.halfY;
            const float ayMax = tA->position.y + cA->offset.y + cA->aabb.halfY;
            const float bxMin = tB->position.x + cB->offset.x - cB->aabb.halfX;
            const float bxMax = tB->position.x + cB->offset.x + cB->aabb.halfX;
            const float byMin = tB->position.y + cB->offset.y - cB->aabb.halfY;
            const float byMax = tB->position.y + cB->offset.y + cB->aabb.halfY;

            const bool overlapX = (axMin < bxMax) && (axMax > bxMin);
            const bool overlapY = (ayMin < byMax) && (ayMax > byMin);
            if (!overlapX || !overlapY) continue;

            float overlapYAmount = std::min(ayMax, byMax) - std::max(ayMin, byMin);
            float overlapXAmount = std::min(axMax, bxMax) - std::max(axMin, bxMin);

            const bool aStatic = cA->isStatic || cA->isTrigger;
            const bool bStatic = cB->isStatic || cB->isTrigger;

            // スロップ適用
            overlapXAmount = std::max(0.0f, overlapXAmount - kPenetrationSlop);
            overlapYAmount = std::max(0.0f, overlapYAmount - kPenetrationSlop);
            if (overlapXAmount <= 0.0f && overlapYAmount <= 0.0f) continue;

            // 優先軸: 少ない侵入軸 (段差などで横押し戻しを避ける)
            const bool resolveYFirst = overlapYAmount <= overlapXAmount;

            if (resolveYFirst && overlapYAmount > 0.0f)
            {
                const float aCenterY = tA->position.y + cA->offset.y;
                const float bCenterY = tB->position.y + cB->offset.y;
                const float dir = (aCenterY >= bCenterY) ? 1.0f : -1.0f; // +1 A above B
                float correction = overlapYAmount * kVerticalCorrectionPercent;

                if (!aStatic && bStatic)
                {
                    tA->position.y += dir * correction;
                    if (rbA && rbA->velocity.y * dir < 0.0f) rbA->velocity.y = 0.0f;
                    if (rbA && dir > 0.0f) rbA->onGround = true;
                }
                else if (aStatic && !bStatic)
                {
                    tB->position.y -= dir * correction;
                    if (rbB && rbB->velocity.y * (-dir) < 0.0f) rbB->velocity.y = 0.0f;
                    if (rbB && dir < 0.0f) rbB->onGround = true;
                }
                else if (!aStatic && !bStatic)
                {
                    tA->position.y += dir * (correction * 0.5f);
                    tB->position.y -= dir * (correction * 0.5f);
                    if (rbA && rbA->velocity.y * dir < 0.0f) rbA->velocity.y = 0.0f;
                    if (rbB && rbB->velocity.y * (-dir) < 0.0f) rbB->velocity.y = 0.0f;
                    if (rbA && dir > 0.0f) rbA->onGround = true;
                    if (rbB && dir < 0.0f) rbB->onGround = true;
                }
            }
            else if (overlapXAmount > 0.0f)
            {
                // 水平方向はスナップを弱める: 低い割合 + 侵入方向速度のみ減衰/保持
                const float aCenterX = tA->position.x + cA->offset.x;
                const float bCenterX = tB->position.x + cB->offset.x;
                const float dir = (aCenterX >= bCenterX) ? 1.0f : -1.0f; // +1 A right of B
                float correction = overlapXAmount * kHorizontalCorrectionPercent;

                if (!aStatic && bStatic)
                {
                    tA->position.x += dir * correction;
                    if (rbA && rbA->velocity.x * dir < 0.0f)
                    {
                        rbA->velocity.x *= kHorizontalPenetrationDampen; // ゼロにはしない
                    }
                }
                else if (aStatic && !bStatic)
                {
                    tB->position.x -= dir * correction;
                    if (rbB && rbB->velocity.x * (-dir) < 0.0f)
                    {
                        rbB->velocity.x *= kHorizontalPenetrationDampen;
                    }
                }
                else if (!aStatic && !bStatic)
                {
                    tA->position.x += dir * (correction * 0.5f);
                    tB->position.x -= dir * (correction * 0.5f);
                    if (rbA && rbA->velocity.x * dir < 0.0f) rbA->velocity.x *= kHorizontalPenetrationDampen;
                    if (rbB && rbB->velocity.x * (-dir) < 0.0f) rbB->velocity.x *= kHorizontalPenetrationDampen;
                }
                // onGround は横軸では付与しない
            }
        }
    }

    // 3) 地面安定化: 小さい下向き速度をゼロに (浮き/震え防止)
    world.View<Rigidbody2DComponent>(
        [&](EntityId /*e*/, Rigidbody2DComponent& rb)
        {
            if (rb.onGround && rb.velocity.y < 0.0f && rb.velocity.y > -2.0f)
            {
                rb.velocity.y = 0.0f;
            }
        }
    );
}

void PhysicsStepSystem::ResolveAabbVsAabb(EntityId, TransformComponent&, Collider2DComponent&, Rigidbody2DComponent*,
    EntityId, TransformComponent&, Collider2DComponent&, Rigidbody2DComponent*)
{
    // 未使用 (イベントベース解決に移行)
}
