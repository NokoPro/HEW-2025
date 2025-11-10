#include "ModelRenderSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"

#include "System/CameraMath.h"
#include "System/AssetManager.h"
#include "System/Model.h"
#include "System/Geometory.h"
#include "System/DirectX/ShaderList.h"

using namespace DirectX;

//--------------------------------------------------------------
// Render : モデル描画処理
//--------------------------------------------------------------
void ModelRenderSystem::Render(const World& world)
{
    // World内の全エンティティから Transform + ModelRenderer を持つものを走査
    world.View<TransformComponent, ModelRendererComponent>(
        [&](EntityId /*e*/, const TransformComponent& tr, const ModelRendererComponent& mr)
        {
            // モデルが無効 or 非表示ならスキップ
            if (!mr.visible || !mr.model)
            {
                return;
            }

            //------------------------------------------------------
            // 1) ワールド行列を構築
            //------------------------------------------------------
            // スケーリング行列
            const XMMATRIX S = XMMatrixScaling(tr.scale.x, tr.scale.y, tr.scale.z);
            // 回転行列（度→ラジアン変換）
            const XMMATRIX Rx = XMMatrixRotationX(XMConvertToRadians(tr.rotationDeg.x));
            const XMMATRIX Ry = XMMatrixRotationY(XMConvertToRadians(tr.rotationDeg.y));
            const XMMATRIX Rz = XMMatrixRotationZ(XMConvertToRadians(tr.rotationDeg.z));
            // 平行移動行列
            const XMMATRIX T = XMMatrixTranslation(tr.position.x, tr.position.y, tr.position.z);

            // 行列合成（スケール→回転→平行移動）
            const XMMATRIX W = S * Rx * Ry * Rz * T;

            //------------------------------------------------------
            // 2) WVP行列をGPUに渡せる形式にする
            //------------------------------------------------------
            DirectX::XMFLOAT4X4 wvp[3]; ///< [0]=World, [1]=View, [2]=Proj

            // World行列は転置して格納（HLSL行列と合わせる）
            XMStoreFloat4x4(&wvp[0], XMMatrixTranspose(W));

            // View / Proj は外部から受け取ったものをそのまま使用
            wvp[1] = m_V;
            wvp[2] = m_P;

            // シェーダ側に行列セット
            ShaderList::SetWVP(wvp);

            //------------------------------------------------------
            // 3) モデル描画
            //------------------------------------------------------
            mr.model->Draw();
        });
}

//--------------------------------------------------------------
// ApplyDefaultLighting : 照明・カメラ設定
//--------------------------------------------------------------
void ModelRenderSystem::ApplyDefaultLighting(float camY, float camRadius)
{
    // 白色光を上方から当てるような単純な設定
    ShaderList::SetLight({ 1.0f, 1.0f, 1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f });

    // カメラ位置をセット（円軌道上に配置する想定）
    ShaderList::SetCameraPos({ 0.0f, camY, -camRadius });
}
