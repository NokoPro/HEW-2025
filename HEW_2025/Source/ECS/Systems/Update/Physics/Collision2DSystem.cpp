/*********************************************************************/
/* @file   Collision2DSystem.cpp
 * @brief  重なり(当たり)検出のみを行いイベントを記録する2DコリジョンSystem
 * 
 * 位置や速度の解決は PhysicsStepSystem 側に委譲。ここでは
 * AABB 重なりを見てイベントバッファに積むだけ。
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "Collision2DSystem.h"
#include <algorithm>
#include <vector>
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Tag/Tag.h"

void Collision2DSystem::Update(World& world, float /*dt*/)
{
    if (m_eventBuffer) m_eventBuffer->Clear();

    // 全コライダ収集
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
    colliders.reserve(128);
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

    if (colliders.empty()) return;

    const bool hasPlayers = !playerCenters.empty();
    static constexpr float kPlayerCollisionRadius = 500.0f;
    const float playerRadiusSq = kPlayerCollisionRadius * kPlayerCollisionRadius;

    const size_t n = colliders.size();
    for (size_t i = 0; i < n; ++i)
    {
        auto& A = colliders[i];
        for (size_t j = i + 1; j < n; ++j)
        {
            auto& B = colliders[j];

            auto& c1 = *A.c;
            auto& c2 = *B.c;

            // レイヤマスクチェック
            const bool hit12 = (c1.hitMask & c2.layer) != 0;
            const bool hit21 = (c2.hitMask & c1.layer) != 0;
            if (!hit12 && !hit21) continue;

            // AABBのみ対応
            if (c1.shape != ColliderShapeType::AABB2D || c2.shape != ColliderShapeType::AABB2D)
                continue;

            // プレイヤーから遠い非プレイヤー同士はスキップ
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
                if (!nearPlayer) continue;
            }

            // 重なりテスト
            if (!IsAabbOverlap(*A.t, c1, *B.t, c2)) continue;

            // イベント追加 (triggerフラグはどちらかがトリガーならtrue)
            const bool isTrigger = c1.isTrigger || c2.isTrigger;
            if (m_eventBuffer)
            {
                m_eventBuffer->Add(A.e, B.e, isTrigger);
                m_eventBuffer->Add(B.e, A.e, isTrigger);
            }
        }
    }
}
