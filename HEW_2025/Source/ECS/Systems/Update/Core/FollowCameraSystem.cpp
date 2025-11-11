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
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "System/Geometory.h"

#include <DirectXMath.h>
#include <algorithm>

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
            // 複数プレイヤーの中で最もYが高いものを探す
            XMFLOAT3 highestTargetPos = { 0.0f, -FLT_MAX, 0.0f };
            bool anyTargetFound = false;

            world.View<PlayerInputComponent, TransformComponent>(
                [&](EntityId /*e*/, const PlayerInputComponent& /*pic*/, TransformComponent& t)
                {
                    float ty = t.position.y + rig.lookAtOffset.y;
                    if (!anyTargetFound || ty > highestTargetPos.y)
                    {
                        highestTargetPos.x = t.position.x + rig.lookAtOffset.x;
                        highestTargetPos.y = ty;
                        highestTargetPos.z = t.position.z + rig.lookAtOffset.z;
                        anyTargetFound = true;
                    }
                }
            );

            // カメラのpivot初期値
            XMFLOAT3 pivot = camTr.position;
            if (anyTargetFound)
            {
                pivot = highestTargetPos;
            }

            // カメラ現在位置
            XMFLOAT3 camPos = camTr.position;

            switch (rig.mode)
            {
            case Camera3DComponent::Mode::Orbit:
            {
                const float yawRad = XMConvertToRadians(rig.orbitYawDeg);
                const float pitchRad = XMConvertToRadians(rig.orbitPitchDeg);
                const float cosPitch = cosf(pitchRad);
                const float sinPitch = sinf(pitchRad);
                const float cosYaw = cosf(yawRad);
                const float sinYaw = sinf(yawRad);
                camPos.x = pivot.x + rig.orbitDistance * cosPitch * cosYaw;
                camPos.y = pivot.y + rig.orbitDistance * sinPitch;
                camPos.z = pivot.z + rig.orbitDistance * cosPitch * sinYaw;
                camTr.position = camPos;
                break;
            }

            case Camera3DComponent::Mode::SideScroll:
            {
                const float baseY = rig.baseScrollY;
                const float camCenterY = camPos.y;

                if (anyTargetFound)
                {
                    const float highestY = highestTargetPos.y;
                    if (!rig.followingTarget && highestY > camCenterY + rig.followMarginY)
                    {
                        rig.followingTarget = true;
                    }
                    if (rig.followingTarget)
                    {
                        if (highestY < baseY - rig.returnToAutoMargin)
                        {
                            rig.followingTarget = false;
                        }
                    }
                }

                if (rig.followingTarget && anyTargetFound)
                {
                    camPos.y = highestTargetPos.y;
                }
                else
                {
                    camPos.y += rig.scrollSpeed * dt;
                    if (camPos.y < baseY) camPos.y = baseY;
                }

                camPos.x = rig.sideFixedX;

                if (rig.depthLerpSpeed <= 0.0f)
                {
                    camPos.z = rig.sideFixedZ;
                }
                else
                {
                    const float diff = rig.sideFixedZ - camPos.z;
                    const float t = (std::min)(1.0f, rig.depthLerpSpeed * dt);
                    camPos.z += diff * t;
                }

                camTr.position = camPos;

                pivot.x = camPos.x + rig.sideLookAtX;
                pivot.y = camPos.y;
                pivot.z = camPos.z + 10.0f;
                break;
            }

            default:
                break;
            }

            // ビュー/プロジェクション作成
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&pivot);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P;
            if (rig.mode == Camera3DComponent::Mode::SideScroll)
            {
                const float orthoHeight = rig.orthoHeight;
                const float orthoWidth = orthoHeight * rig.aspect;
                P = XMMatrixOrthographicLH(orthoWidth, orthoHeight, rig.nearZ, rig.farZ);
            }
            else
            {
                P = XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(rig.fovY), rig.aspect, rig.nearZ, rig.farZ);
            }

            XMStoreFloat4x4(&m_viewT, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_projT, XMMatrixTranspose(P));
            Geometory::SetView(m_viewT);
            Geometory::SetProjection(m_projT);
        });
}
