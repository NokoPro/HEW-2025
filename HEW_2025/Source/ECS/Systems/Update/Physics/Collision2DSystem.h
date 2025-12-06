/*********************************************************************/
/* @file   Collision2DSystem.h
 * @brief  当たり判定結果(重なり)のみを記録する2DコリジョンSystem
 * 
 * 位置/速度の修正は PhysicsStepSystem 側が担当し、ここでは
 * "当たったこと" と "トリガーかどうか" だけをイベントバッファへ積む。
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

/**
 * @brief AABB2D の重なり検出のみを行い、イベントを記録する System
 *        (解決/押し戻し/速度補正は行わない)
 */
class Collision2DSystem : public IUpdateSystem
{
public:
    Collision2DSystem(CollisionEventBuffer* buf)
        : m_eventBuffer(buf)
    {
    }

    void Update(World& world, float dt) override;
    CollisionEventBuffer* GetEventBuffer() const { return m_eventBuffer; }

private:
    // 衝突イベントバッファ (Enter/Stay などの区別は後段で差分処理する想定)
    CollisionEventBuffer* m_eventBuffer = nullptr;
};