/*********************************************************************/
/* @file   CollisionDebugRenderSystem.cpp
 * @brief  衝突可視化のデバッグレンダリングシステム実装
 * 
 * @author 奥田
 * @date   2025/11/13
 *********************************************************************/
#include "CollisionDebugRenderSystem.h"
#include "ECS/World.h"

// コンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

// ジオメトリ描画
#include "System/Geometory.h"   // Geometory::SetWorld, DrawBox など
#include "System/DebugSettings.h"

#include <DirectXMath.h>
using namespace DirectX;

void CollisionDebugRenderSystem::Render(const World& world)
{
    // F2 トグル+グローバル設定の同期
    if (IsKeyTrigger(VK_F2))
    {
        m_enabled = !m_enabled;
        DebugSettings::Get().showCollision = m_enabled;
    }
    else
    {
        // 他から変更された場合に追従
        m_enabled = DebugSettings::Get().showCollision;
    }

    if (!m_enabled)
        return;

    // View / Proj は毎フレームカメラ側から設定されている想定
    Geometory::SetView(m_view);
    Geometory::SetProjection(m_proj);

    // すべてのコライダを描画
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId,
            const TransformComponent& tr,
            const Collider2DComponent& col)
        {
            // 対象: AABB のみ
            if (col.shape != ColliderShapeType::AABB2D)
                return;

            // AABBは2D箱なのでZは薄く
            const float w = col.aabb.halfX * 2.0f;
            const float h = col.aabb.halfY * 2.0f;
            const float d = 0.1f; // 薄め

            // ワールドマトリクス（S * T）
            XMMATRIX S = XMMatrixScaling(w, h, d);
            XMMATRIX T = XMMatrixTranslation(tr.position.x + col.offset.x,
                tr.position.y + col.offset.y,
                0.0f);

            XMMATRIX W = S * T;

            XMFLOAT4X4 Wf;
            XMStoreFloat4x4(&Wf, XMMatrixTranspose(W)); // Geometoryは転置で受け取る
            Geometory::SetWorld(Wf);

            // 色分け: トリガー=黄, 静的=青, 動的=赤
            XMFLOAT4 fillColor;
            if (col.isTrigger)
            {
                fillColor = { 1.0f, 1.0f, 0.2f, 0.4f };
            }
            else if (col.isStatic)
            {
                fillColor = { 0.2f, 0.5f, 1.0f, 0.4f };
            }
            else
            {
                fillColor = { 1.0f, 0.2f, 0.2f, 0.4f };
            }

            Geometory::SetColor(fillColor);
            Geometory::DrawBox();

            // 枠線（白）
            const float left = tr.position.x + col.offset.x - col.aabb.halfX;
            const float right = tr.position.x + col.offset.x + col.aabb.halfX;
            const float top = tr.position.y + col.offset.y + col.aabb.halfY;
            const float bottom = tr.position.y + col.offset.y - col.aabb.halfY;
            const float z = 0.0f;

            XMFLOAT4 lineColor = { 1.0f, 1.0f, 1.0f, 1.0f };
            Geometory::AddLine({ left, top, z }, { right, top, z }, lineColor);
            Geometory::AddLine({ right, top, z }, { right, bottom, z }, lineColor);
            Geometory::AddLine({ right, bottom, z }, { left, bottom, z }, lineColor);
            Geometory::AddLine({ left, bottom, z }, { left, top, z }, lineColor);
        }
    );

    // 蓄積されたラインを描画
    Geometory::DrawLines();
}
