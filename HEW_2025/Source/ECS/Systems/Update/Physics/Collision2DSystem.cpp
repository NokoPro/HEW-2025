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
#include <vector>
#include "ECS/Components/Physics/CollisionEvents.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Tag/Tag.h"

void Collision2DSystem::Update(World& world, float dt)
{
    if (m_buf) m_buf->Clear();
    if (m_eventBuffer) m_eventBuffer->Clear();

    //====================================================
    // まず全コライダ収集（1回のViewで終わらせる）
    //====================================================
    struct CollInfo
    {
        EntityId e;
        TransformComponent* t;
        Collider2DComponent* c;
        float cx; // 中心X (position + offset)
        float cy; // 中心Y
        bool isPlayer;
    };

    std::vector<CollInfo> colliders;
    colliders.reserve(128); // 適当な初期容量（必要なら調整）

    std::vector<DirectX::XMFLOAT2> playerCenters;

    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e, TransformComponent& t, Collider2DComponent& c)
        {
            CollInfo info{ e, &t, &c, t.position.x + c.offset.x, t.position.y + c.offset.y, world.Has<TagPlayer>(e) };
            colliders.emplace_back(info);
            if (info.isPlayer)
            {
                playerCenters.emplace_back(info.cx, info.cy);
            }
        }
    );

    if (colliders.empty()) return; // 何も無い

    // プレイヤーがいない場合は全ペア判定（タイトル画面など）
    const bool hasPlayers = !playerCenters.empty();

    // プレイヤーの周囲のみ判定したい半径（必要に応じて調整）
    static constexpr float kPlayerCollisionRadius = 500.0f;
    const float playerRadiusSq = kPlayerCollisionRadius * kPlayerCollisionRadius;

    //====================================================
    // ペア判定（O(N^2)だが内部View呼び出しを排除）
    //====================================================
    const size_t n = colliders.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto& A = colliders[i];
        for (size_t j = i + 1; j < n; ++j)
        {
            auto& B = colliders[j];

            auto& c1 = *A.c;
            auto& c2 = *B.c;

            // レイヤマスクで除外
            const bool hit12 = (c1.hitMask & c2.layer) != 0;
            const bool hit21 = (c2.hitMask & c1.layer) != 0;
            if (!hit12 && !hit21) continue;

            // 形状（今はAABBのみ対応）
            if (c1.shape != ColliderShapeType::AABB2D || c2.shape != ColliderShapeType::AABB2D)
            {
                continue;
            }

            //-------------------------------
            // プレイヤー中心の距離によるスキップ
            // 非プレイヤー同士かつ両方が全プレイヤー半径外ならスキップ
            //-------------------------------
            if (hasPlayers && !A.isPlayer && !B.isPlayer)
            {
                bool nearPlayer = false;
                for (auto& pc : playerCenters)
                {
                    const float dxA = A.cx - pc.x;
                    const float dyA = A.cy - pc.y;
                    const float dxB = B.cx - pc.x;
                    const float dyB = B.cy - pc.y;
                    if ((dxA * dxA + dyA * dyA) <= playerRadiusSq || (dxB * dxB + dyB * dyB) <= playerRadiusSq)
                    {
                        nearPlayer = true;
                        break;
                    }
                }
                if (!nearPlayer) continue; // プレイヤーから遠い非重要オブジェクト同士
            }

            // AABB重なりテスト
            if (!IsAabbOverlap(*A.t, c1, *B.t, c2)) continue;

            // トリガー処理（イベントのみ）
            if (c1.isTrigger || c2.isTrigger)
            {
                if (m_buf)
                {
                    m_buf->Add(A.e, B.e, true);
                    m_buf->Add(B.e, A.e, true);
                }
                if (m_eventBuffer)
                {
                    m_eventBuffer->Add(A.e, B.e, true);
                    m_eventBuffer->Add(B.e, A.e, true);
                }
                continue;
            }

            // 衝突解決
            auto* rb1 = world.TryGet<Rigidbody2DComponent>(A.e);
            auto* rb2 = world.TryGet<Rigidbody2DComponent>(B.e);
            ResolveAabbPair(*A.t, c1, rb1, *B.t, c2, rb2);

            // イベント記録
            if (m_buf)
            {
                m_buf->Add(A.e, B.e, false);
                m_buf->Add(B.e, A.e, false);
            }
            if (m_eventBuffer)
            {
                m_eventBuffer->Add(A.e, B.e, false);
                m_eventBuffer->Add(B.e, A.e, false);
            }
        }
    }
}

void Collision2DSystem::ResolveAabbPair(TransformComponent& tA, Collider2DComponent& cA, Rigidbody2DComponent* rbA,
    TransformComponent& tB, Collider2DComponent& cB, Rigidbody2DComponent* rbB)
{
    const float aMinX = (tA.position.x + cA.offset.x) - cA.aabb.halfX;
    const float aMaxX = (tA.position.x + cA.offset.x) + cA.aabb.halfX;
    const float aMinY = (tA.position.y + cA.offset.y) - cA.aabb.halfY;
    const float aMaxY = (tA.position.y + cA.offset.y) + cA.aabb.halfY;

    const float bMinX = (tB.position.x + cB.offset.x) - cB.aabb.halfX;
    const float bMaxX = (tB.position.x + cB.offset.x) + cB.aabb.halfX;
    const float bMinY = (tB.position.y + cB.offset.y) - cB.aabb.halfY;
    const float bMaxY = (tB.position.y + cB.offset.y) + cB.aabb.halfY;

    const float penFromTop = bMaxY - aMinY;
    const float penFromBottom = aMaxY - bMinY;
    const float penFromLeft = aMaxX - bMinX;
    const float penFromRight = bMaxX - aMinX;

    const bool aStatic = cA.isStatic;
    const bool bStatic = cB.isStatic;

    if (penFromBottom < penFromTop)
    {
        if (!aStatic)
        {
            tA.position.y = (bMinY - cA.aabb.halfY) - cA.offset.y;
            if (rbA)
            {
                rbA->velocity.y = 0.0f;
                rbA->onGround = true;
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

    if (penFromLeft < penFromRight)
    {
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
