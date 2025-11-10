/*****************************************************************//**
 * @file   FollowCameraSystem.cpp
 * @brief  縦スクロールのサイドビュー用に拡張したフォローカメラ
 * @details
 * - カメラを表すエンティティに ActiveCameraTag を付けておくと対象になる
 * - CameraRigComponent の mode で挙動を変える
 *   - Orbit … 従来通りのターゲット注視オービット
 *   - SideScroll … 上方向に一定速度で進みつつ、ターゲットがさらに上に行ったらそっちに合わせる
 * - 毎フレーム View / Proj を転置して保持し、Geometory にも書き込む
 *********************************************************************/
#include "FollowCameraSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "System/Geometory.h"

#include <DirectXMath.h>

using namespace DirectX;

void FollowCameraSystem::Update(World& world, float dt)
{
    // カメラを1つ探す
    world.View<ActiveCameraTag, Camera3DComponent, TransformComponent>(
        [&](EntityId /*camEnt*/,
            ActiveCameraTag& /*tag*/,
            Camera3DComponent& rig,
            TransformComponent& camTr)
        {
            // まずターゲットの位置を決める
            XMFLOAT3 pivot = camTr.position; // デフォルトは今の位置を見る
            if (rig.target != 0)
            {
                if (auto* tTr = world.TryGet<TransformComponent>(rig.target))
                {
                    pivot = tTr->position;
                    // オフセットがあれば足す
                    pivot.x += rig.lookAtOffset.x;
                    pivot.y += rig.lookAtOffset.y;
                    pivot.z += rig.lookAtOffset.z;
                }
            }

            // カメラ位置を決める
            XMFLOAT3 camPos = camTr.position;

            switch (rig.mode)
            {
            case Camera3DComponent::Mode::Orbit:
            {
                // 従来通りのオービット
                // yaw/pitch は度
                const float yawRad = XMConvertToRadians(rig.orbitYawDeg);
                const float pitchRad = XMConvertToRadians(rig.orbitPitchDeg);

                const float cosPitch = cosf(pitchRad);
                const float sinPitch = sinf(pitchRad);
                const float cosYaw = cosf(yawRad);
                const float sinYaw = sinf(yawRad);

                camPos.x = pivot.x + rig.orbitDistance * cosPitch * cosYaw;
                camPos.y = pivot.y + rig.orbitDistance * sinPitch;
                camPos.z = pivot.z + rig.orbitDistance * cosPitch * sinYaw;

                // カメラTransformに書き戻し
                camTr.position = camPos;
                break;
            }

            case Camera3DComponent::Mode::SideScroll:
            {
                bool hasTarget = false;
                float targetY = 0.0f;

                if (rig.target != 0)
                {
                    if (auto* tTr = world.TryGet<TransformComponent>(rig.target))
                    {
                        hasTarget = true;
                        targetY = tTr->position.y + rig.followOffsetY;
                    }
                }

                // Y方向にスクロール
                camPos.y += rig.scrollSpeed * dt;

                // プレイヤーが上に行ったらカメラも上げる
                if (hasTarget)
                {
                    const float desiredY = targetY - rig.followMarginY;
                    if (desiredY > camPos.y)
                    {
                        camPos.y = desiredY;
                    }
                }

                // X/Zは固定
                camPos.x = rig.sideFixedX;   // 例: 0.0f
                camPos.z = rig.sideFixedZ;   // 例: -10.0f
                camTr.position = camPos;

                // pivot（注視点）はカメラ位置より少し奥
                pivot.x = camPos.x;
                pivot.y = camPos.y;
                pivot.z = camPos.z + 10.0f; // ← ここを十分大きくする（例: +10.0f）

                break;
            }


            default:
                break;
            }

            // ビュー/プロジェクションを作る（左手系で統一）
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&pivot);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(rig.fovY),
                rig.aspect,
                rig.nearZ,
                rig.farZ
            );

            // 転置して保持
            XMStoreFloat4x4(&m_viewT, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_projT, XMMatrixTranspose(P));

            // Geometory にも流す（過去コードと同じ） 
            Geometory::SetView(m_viewT);
            Geometory::SetProjection(m_projT);
        });
}
