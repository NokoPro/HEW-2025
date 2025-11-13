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
    world.View<TransformComponent, ModelRendererComponent>(
        [&](EntityId /*e*/, const TransformComponent& tr, const ModelRendererComponent& mr)
        {
            if (!mr.visible || !mr.model)
                return;

            // ---------- 1) モデル側のローカル行列 ----------
            using namespace DirectX;

            // モデルだけのスケーリング
            const XMMATRIX LS = XMMatrixScaling(
                mr.localScale.x,
                mr.localScale.y,
                mr.localScale.z
            );

            // モデルだけの回転（度 → rad）
            const XMMATRIX LRx = XMMatrixRotationX(XMConvertToRadians(mr.localRotationDeg.x));
            const XMMATRIX LRy = XMMatrixRotationY(XMConvertToRadians(mr.localRotationDeg.y));
            const XMMATRIX LRz = XMMatrixRotationZ(XMConvertToRadians(mr.localRotationDeg.z));

            // モデルだけの平行移動
            const XMMATRIX LT = XMMatrixTranslation(
                mr.localOffset.x,
                mr.localOffset.y,
                mr.localOffset.z
            );

            // ローカル変換は「モデルの原点をどうするか」なので先に作る
            const XMMATRIX L = LS * LRx * LRy * LRz * LT;

            // ---------- 2) エンティティのワールド行列 ----------
            const XMMATRIX S = XMMatrixScaling(tr.scale.x, tr.scale.y, tr.scale.z);
            const XMMATRIX Rx = XMMatrixRotationX(XMConvertToRadians(tr.rotationDeg.x));
            const XMMATRIX Ry = XMMatrixRotationY(XMConvertToRadians(tr.rotationDeg.y));
            const XMMATRIX Rz = XMMatrixRotationZ(XMConvertToRadians(tr.rotationDeg.z));
            const XMMATRIX T = XMMatrixTranslation(tr.position.x, tr.position.y, tr.position.z);
            const XMMATRIX Wentity = S * Rx * Ry * Rz * T;

            // ---------- 3) 最終ワールド行列 ----------
            // ローカル → エンティティ の順で掛ける
            const XMMATRIX W = L * Wentity;

            // ---------- 4) WVP送信 ----------
            DirectX::XMFLOAT4X4 wvp[3];
            XMStoreFloat4x4(&wvp[0], XMMatrixTranspose(W));
            wvp[1] = m_V;
            wvp[2] = m_P;
            ShaderList::SetWVP(wvp);

            // ---------- 5) 描画 ----------
            mr.model->Draw();
        });
}

//--------------------------------------------------------------
// ApplyDefaultLighting : 照明・カメラ設定
//--------------------------------------------------------------
void ModelRenderSystem::ApplyDefaultLighting(float camY, float camRadius)
{
    // カメラ位置を基にライトを当てる: ライトがカメラ位置からシーンの手前->奥に向かうようにする
    DirectX::XMFLOAT3 camPos = { 0.0f, -1.0, 1.0f };

    // 光の強さを少し上げて全体を明るくする
    // ライト方向にはカメラ位置を渡す（シェーダ内で negation されるため、結果的にカメラからシーンへ向かう光になる）
    ShaderList::SetLight({ 1.0f, 1.0f, 1.0f, 1.0f }, camPos);

    // カメラ位置もシェーダへ
    ShaderList::SetCameraPos(camPos);
}
