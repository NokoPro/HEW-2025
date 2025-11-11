#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"

// 使うコンポーネントたち
#include "../../../Components/Physics/TransformComponent.h"
#include "../../../Components/Physics/Rigidbody2DComponent.h"
#include "../../../Components/Physics/Collider2DComponent.h"
#include "../../../Components/Gimick/MovingPlatformComponent.h"

#include "../../../Components/Physics/CollisionEvents.h"

/**
 * @brief 速度を位置に反映し、コライダー同士のめり込みを解消する物理ステップ
 *
 * このSystemは「今フレームどう動いたか」をまとめて処理する担当です。
 * 入力→Intent→Rigidbody までは別のSystemで済んでいる前提。
 *
 * 対応すること:
 *  - Rigidbody付きのものを dt 秒ぶん動かす
 *  - Collider付きのもの同士でAABBをチェックする
 *  - 静的オブジェクト(床)にめり込んだら動的オブジェクト側だけを押し戻す
 *  - トリガーなら押し戻さずにイベントバッファに詰める
 *  - MovingPlatform が持ってる今フレームの移動量を、乗ってる側に適用できるようにする
 */
class PhysicsStepSystem : public IUpdateSystem
{
public:
    PhysicsStepSystem(CollisionEventBuffer* eventBuffer)
        : m_eventBuffer(eventBuffer)
    {
    }

    void Update(World& world, float dt) override;

private:
    CollisionEventBuffer* m_eventBuffer = nullptr;

    // AABB同士の当たりを解決する補助
    void ResolveAabbVsAabb(EntityId a, TransformComponent& ta, Collider2DComponent& ca, Rigidbody2DComponent* ra,
        EntityId b, TransformComponent& tb, Collider2DComponent& cb, Rigidbody2DComponent* rb);
};
