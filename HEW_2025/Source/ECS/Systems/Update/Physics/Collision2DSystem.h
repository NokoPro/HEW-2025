/*********************************************************************/
/* @file   Collision2DSystem.h
 * @brief  当たり判定の2Dシステム定義
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
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
	// 衝突イベントバッファ（外部からセットされる想定）
    CollisionEventBuffer* m_eventBuffer = nullptr;

	//  internal用バッファ（ResolveAabbPair内でイベント追加に使う）
    CollisionEventBuffer* m_buf = nullptr;

    // AABB同士のめり込みを直す
    void ResolveAabbPair(TransformComponent& tA, Collider2DComponent& cA, Rigidbody2DComponent* rbA,
        TransformComponent& tB, Collider2DComponent& cB, Rigidbody2DComponent* rbB);
};