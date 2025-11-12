// ECS/Systems/Physics/Collision2DSystem.h
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/CollisionEvents.h"
#include "ECS/Components/Physics/CollisionHelpers.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"

/**
 * @brief AABB2D同士の衝突を検出して、
 *        ・トリガーならイベントだけ
 *        ・それ以外ならめり込みを解消する
 *        System
 *
 * サイドビュー前提なので Y を先に解消します。
 */
class Collision2DSystem : public IUpdateSystem
{
public:
    Collision2DSystem(CollisionEventBuffer* buf)
        : m_buf(buf), m_eventBuffer(buf)
    {
    }

    void Update(World& world, float dt) override;

    // m_eventBufferへのgetter（外部アクセス用）
    CollisionEventBuffer* GetEventBuffer() const { return m_eventBuffer; }

private:
    CollisionEventBuffer* m_eventBuffer = nullptr;
    CollisionEventBuffer* m_buf = nullptr;

    // AABB同士のめり込みを直す
    void ResolveAabbPair(TransformComponent& tA, Collider2DComponent& cA, Rigidbody2DComponent* rbA,
        TransformComponent& tB, Collider2DComponent& cB, Rigidbody2DComponent* rbB);
};