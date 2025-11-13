/*********************************************************************/
/* @file   Collision2DSystem.cpp
 * @brief  当たり判定の2Dシステム実装
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "Collision2DSystem.h"
#include <algorithm>
#include <cmath>
#include "ECS/Components/Physics/CollisionEvents.h"
#include "ECS/Components/Physics/PhysicsLayers.h"

void Collision2DSystem::Update(World& world, float dt)
{
    if (m_buf) m_buf->Clear();

    // 衝突イベントバッファをクリア
    if (m_eventBuffer) m_eventBuffer->Clear();

    // 全部と全部を見回す
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e1, TransformComponent& t1, Collider2DComponent& c1)
        {
            world.View<TransformComponent, Collider2DComponent>(
                [&](EntityId e2, TransformComponent& t2, Collider2DComponent& c2)
                {
                    // 同じペアを2回やらない
                    if (e1 >= e2) return;

                    // レイヤで除外
                    const bool hit12 = (c1.hitMask & c2.layer) != 0;
                    const bool hit21 = (c2.hitMask & c1.layer) != 0;
                    if (!hit12 && !hit21) return;

                    // 今はAABBだけ
                    if (c1.shape != ColliderShapeType::AABB2D ||
                        c2.shape != ColliderShapeType::AABB2D)
                    {
                        return;
                    }

                    // 重なってなければおしまい
                    if (!IsAabbOverlap(t1, c1, t2, c2)) return;

                    // トリガーならイベントだけ
                    if (c1.isTrigger || c2.isTrigger)
                    {
                        if (m_buf)
                        {
                            m_buf->Add(e1, e2, true);
                            m_buf->Add(e2, e1, true);
                        }
                        if (m_eventBuffer)
                        {
                            m_eventBuffer->Add(e1, e2, true);
                            m_eventBuffer->Add(e2, e1, true);
                        }
                        return;
                    }

                    // ここからは押し戻すパス

                    // Rigidbody があるか見る（床にはない想定）
                    auto* rb1 = world.TryGet<Rigidbody2DComponent>(e1);
                    auto* rb2 = world.TryGet<Rigidbody2DComponent>(e2);

					// めり込みを解消する
                    ResolveAabbPair(t1, c1, rb1, t2, c2, rb2);

                    // イベントも出しておく
                    if (m_buf)
                    {
                        m_buf->Add(e1, e2, false);
                        m_buf->Add(e2, e1, false);
                    }
                    if (m_eventBuffer)
                    {
                        m_eventBuffer->Add(e1, e2, false);
                        m_eventBuffer->Add(e2, e1, false);
                    }
                }
            );
        }
    );
}

void Collision2DSystem::ResolveAabbPair(TransformComponent& tA, Collider2DComponent& cA, Rigidbody2DComponent* rbA,
    TransformComponent& tB, Collider2DComponent& cB, Rigidbody2DComponent* rbB)
{
    // Aの範囲（offsetを反映）
    const float aMinX = (tA.position.x + cA.offset.x) - cA.aabb.halfX;
    const float aMaxX = (tA.position.x + cA.offset.x) + cA.aabb.halfX;
    const float aMinY = (tA.position.y + cA.offset.y) - cA.aabb.halfY;
    const float aMaxY = (tA.position.y + cA.offset.y) + cA.aabb.halfY;

    // Bの範囲（offsetを反映）
    const float bMinX = (tB.position.x + cB.offset.x) - cB.aabb.halfX;
    const float bMaxX = (tB.position.x + cB.offset.x) + cB.aabb.halfX;
    const float bMinY = (tB.position.y + cB.offset.y) - cB.aabb.halfY;
    const float bMaxY = (tB.position.y + cB.offset.y) + cB.aabb.halfY;

    // Y方向のめり込み量を計算
    const float penFromTop = bMaxY - aMinY; // Bの上面→Aの下面
    const float penFromBottom = aMaxY - bMinY; // Aの上面→Bの下面

    // X方向のめり込み量
    const float penFromLeft = aMaxX - bMinX;  // Aが左からBにめり込んだ
    const float penFromRight = bMaxX - aMinX;  // Aが右からBにめり込んだ

    // どっちを動かすかを決める
    // 原則: 静的は動かさない → 動的側だけを動かす
    const bool aStatic = cA.isStatic;
    const bool bStatic = cB.isStatic;

    // 1) まずYを直す（上下関係をはっきりさせる）
    if (penFromBottom < penFromTop)
    {
        // AがBの上に乗った（Bの下面にAの上面がめり込んだ）
        if (!aStatic)
        {
            // tA.position.y を直接セットするが、offset を考慮して Transform が参照する位置を決める
            tA.position.y = (bMinY - cA.aabb.halfY) - cA.offset.y;
            if (rbA)
            {
                rbA->velocity.y = 0.0f;
                rbA->onGround = true;   // <== ここ大事：ジャンプ判定で使える
            }
        }
        else if (!bStatic)
        {
            tB.position.y = (aMaxY + cB.aabb.halfY) - cB.offset.y;
            if (rbB) rbB->velocity.y = 0.0f;
        }
    }
    else
    {
        // AがBの下からめり込んだ
        if (!aStatic)
        {
            tA.position.y = (bMaxY + cA.aabb.halfY) - cA.offset.y;
            if (rbA) rbA->velocity.y = 0.0f;
        }
        else if (!bStatic)
        {
            tB.position.y = (aMinY - cB.aabb.halfY) - cB.offset.y;
            if (rbB) rbB->velocity.y = 0.0f;
        }
    }

    // 2) 次にXを直す
    //    Yで動かしてもまだXが被ってることがあるので軽くどかす
    if (penFromLeft < penFromRight)
    {
        // Aが左からBに入った → Aを左に戻す
        if (!aStatic)
        {
            tA.position.x = (bMinX - cA.aabb.halfX) - cA.offset.x;
            if (rbA) rbA->velocity.x = 0.0f;
        }
        else if (!bStatic)
        {
            tB.position.x = (aMaxX + cB.aabb.halfX) - cB.offset.x;
            if (rbB) rbB->velocity.x = 0.0f;
        }
    }
    else
    {
        // Aが右からBに入った → Aを右に戻す
        if (!aStatic)
        {
            tA.position.x = (bMaxX + cA.aabb.halfX) - cA.offset.x;
            if (rbA) rbA->velocity.x = 0.0f;
        }
        else if (!bStatic)
        {
            tB.position.x = (aMinX - cB.aabb.halfX) - cB.offset.x;
            if (rbB) rbB->velocity.x = 0.0f;
        }
    }
}
