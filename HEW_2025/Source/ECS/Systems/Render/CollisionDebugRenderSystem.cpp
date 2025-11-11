#include "CollisionDebugRenderSystem.h"
#include "ECS/World.h"

// あなたのコンポーネントたち
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

// ジオメトリ描画
#include "System/Geometory.h"   // Geometory::SetWorld, DrawBox など

#include <DirectXMath.h>
using namespace DirectX;

void CollisionDebugRenderSystem::Render(const World& world)
{
    // F2 トグル
    if (IsKeyTrigger(VK_F2))
    {
        m_enabled = !m_enabled;
    }

    if (!m_enabled)
        return;

    // View / Proj は毎フレームカメラから渡される想定
    Geometory::SetView(m_view);
    Geometory::SetProjection(m_proj);

    // すべてのコライダーを走査
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId,
            const TransformComponent& tr,
            const Collider2DComponent& col)
        {
            // 今は AABB だけ可視化
            if (col.shape != ColliderShapeType::AABB2D)
                return;

            // AABBは2D前提なので、Zは0あたりに薄く出す
            const float w = col.aabb.halfX * 2.0f;
            const float h = col.aabb.halfY * 2.0f;
            const float d = 0.1f; // 薄い箱にしておく

            // ワールド行列を組む（S * T）
            XMMATRIX S = XMMatrixScaling(w, h, d);
            XMMATRIX T = XMMatrixTranslation(tr.position.x + col.offset.x,
                tr.position.y + col.offset.y,
                0.0f); // 当たりはZ=0にそろえて可視化

            XMMATRIX W = S * T;

            XMFLOAT4X4 Wf;
            XMStoreFloat4x4(&Wf, XMMatrixTranspose(W)); // Geometoryは転置で持ってるので合わせる
            Geometory::SetWorld(Wf);

            // 色を分けるとわかりやすい
            // 静的: 青、トリガ: 黄、動的: 赤 みたいな感じ
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

            // 枠を引く: AABB の四辺をワールド座標で計算して AddLine で追加
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

    // 追加したラインを描画
    Geometory::DrawLines();
}
