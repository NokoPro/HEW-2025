/*********************************************************************/
/* @file   ModelRenderSystem.cpp
 * @brief  モデルを描画するレンダリングシステム実装
 * - TransformComponent と ModelRendererComponent を組み合わせて描画する
 * - ビュー・プロジェクション行列は外部からセットされる設計
 * - 照明やカメラ位置は ShaderList の静的関数経由で設定する
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "ModelRenderSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

#include "System/CameraMath.h"
#include "System/AssetManager.h"
#include "System/Model.h"
#include "System/Geometory.h"
#include "System/DirectX/ShaderList.h"

#include <algorithm>

using namespace DirectX;

//--------------------------------------------------------------
// Render : モデル描画処理
//--------------------------------------------------------------
void ModelRenderSystem::Render(const World& world)
{
    // 描画リストをクリア
    m_modelList.clear();

    // =====================
    // 1. カメラ可視領域算出
    // =====================
    bool hasSideCam = false;
    float viewLeft = 0.0f, viewRight = 0.0f, viewBottom = 0.0f, viewTop = 0.0f;
    world.View<ActiveCameraTag, Camera3DComponent, TransformComponent>(
        [&](EntityId, const ActiveCameraTag&, const Camera3DComponent& cam, const TransformComponent& tr)
        {
            if (cam.mode == Camera3DComponent::Mode::SideScroll && !hasSideCam)
            {
                const float orthoHeight = cam.orthoHeight;
                const float orthoWidth = orthoHeight * cam.aspect;
                viewLeft   = tr.position.x - orthoWidth * 0.5f;
                viewRight  = tr.position.x + orthoWidth * 0.5f;
                viewBottom = tr.position.y - orthoHeight * 0.5f;
                viewTop    = tr.position.y + orthoHeight * 0.5f;
                hasSideCam = true; // 最初のサイドスクロールカメラのみ使用
            }
        }
    );

    // =====================
    // 2. モデル収集 (カリング適用)
    // =====================
    world.View<TransformComponent, ModelRendererComponent>(
        [&](EntityId /*e*/, const TransformComponent& tr, const ModelRendererComponent& mr)
        {
            // .get()でポインタを取得
            Model* modelPtr = mr.model.get();

            if (!mr.visible || !mr.model)
                return;

            // サイドスクロールカメラがある場合のみカリング
            if (hasSideCam)
            {
                // Transform scale は半径(半幅/半高)として扱われている前提 (Collider登録参照)
                float minX = tr.position.x - tr.scale.x;
                float maxX = tr.position.x + tr.scale.x;
                float minY = tr.position.y - tr.scale.y;
                float maxY = tr.position.y + tr.scale.y;

                // 画面矩形と交差しなければ除外
                if (maxX < viewLeft || minX > viewRight || maxY < viewBottom || minY > viewTop)
                {
                    return;
                }
            }

            // ---------- ローカル行列 ----------
            const XMMATRIX LS = XMMatrixScaling(mr.localScale.x, mr.localScale.y, mr.localScale.z);
            const XMMATRIX LRx = XMMatrixRotationX(XMConvertToRadians(mr.localRotationDeg.x));
            const XMMATRIX LRy = XMMatrixRotationY(XMConvertToRadians(mr.localRotationDeg.y));
            const XMMATRIX LRz = XMMatrixRotationZ(XMConvertToRadians(mr.localRotationDeg.z));
            const XMMATRIX LT = XMMatrixTranslation(mr.localOffset.x, mr.localOffset.y, mr.localOffset.z);
            const XMMATRIX L = LS * LRx * LRy * LRz * LT;

            // ---------- エンティティ行列 ----------
            const XMMATRIX S = XMMatrixScaling(tr.scale.x, tr.scale.y, tr.scale.z);
            const XMMATRIX Rx = XMMatrixRotationX(XMConvertToRadians(tr.rotationDeg.x));
            const XMMATRIX Ry = XMMatrixRotationY(XMConvertToRadians(tr.rotationDeg.y));
            const XMMATRIX Rz = XMMatrixRotationZ(XMConvertToRadians(tr.rotationDeg.z));
            const XMMATRIX T = XMMatrixTranslation(tr.position.x, tr.position.y, tr.position.z);
            const XMMATRIX Wentity = S * Rx * Ry * Rz * T;

            // ---------- 最終ワールド行列 ----------
            const XMMATRIX W = L * Wentity;
            XMFLOAT4X4 wvp0_transposed;
            XMStoreFloat4x4(&wvp0_transposed, XMMatrixTranspose(W));

            // ---------- リストへの追加 ----------
            // ※元コードの描画処理はここでは行わない
            m_modelList.push_back(SortableModel{
                mr.layer,
                wvp0_transposed,
                modelPtr
                });
        });

    // =====================
    // 3. ソート & 描画
    // =====================
    std::sort(m_modelList.begin(), m_modelList.end(),
        [](const SortableModel& a, const SortableModel& b) { return a.layer < b.layer; });

    XMFLOAT4X4 wvp[3];
    wvp[1] = m_V; // 転置済みView
    wvp[2] = m_P; // 転置済みProj

    for (const auto& s : m_modelList)
    {
        wvp[0] = s.world;
        ShaderList::SetWVP(wvp);
        s.model->Draw();
    }
}

//--------------------------------------------------------------
// ApplyDefaultLighting : 照明・カメラ設定
//--------------------------------------------------------------
void ModelRenderSystem::ApplyDefaultLighting(float camY, float camRadius)
{
    // もう使わない
}
