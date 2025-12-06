/*****************************************************************//**
 * @file   FollowCameraSystem.cpp
 * @brief  縦スクロール（SideScroll）：強制スクロール＋上下25%デッドゾーン追従
 * @details
 *  - 強制スクロール発火: Deathゾーン上端が画面下端から25%ライン到達。
 *  - 発火中: 下限拘束＋自動上昇。プレイヤーが上部25%ラインに入ったらさらに追従（上方向のみ）。
 *  - 非発火中: 画面中央50%領域ではカメラ固定。プレイヤーが下部25%へ入ったら下方向追従開始、
 *              上部25%へ入ったら上方向追従開始。
 *  - 追従時の基準: プレイヤー位置を対応する境界ライン(25%)に合わせる。
 *    上方向: camCenterY = playerY - H*0.25 / 下方向: camCenterY = playerY + H*0.25
 *  - スムージング: デッドゾーン外差分のみ指数補間。
 *********************************************************************/
#include "FollowCameraSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "System/Geometory.h"
#include "ECS/Tag/Tag.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <cfloat>
using namespace DirectX; using std::min; using std::max;

void FollowCameraSystem::Update(World& world, float dt)
{
    // Deathゾーン上端
    float deathTopY = -FLT_MAX;
    world.View<TagDeathZone, TransformComponent>([&](EntityId, const TagDeathZone&, const TransformComponent& t) {
        const float topY = t.position.y + t.scale.y; if (topY > deathTopY) deathTopY = topY; });

    // カメラ処理
    world.View<ActiveCameraTag, Camera3DComponent, TransformComponent>(
        [&](EntityId, ActiveCameraTag&, Camera3DComponent& rig, TransformComponent& camTr)
        {
            // 最上位プレイヤー取得 (表示用pivotは lookAtOffset 加算、判定はオフセット除外)
            XMFLOAT3 highestTargetPos{ 0.f,-FLT_MAX,0.f }; float highestBodyY = -FLT_MAX; bool anyTarget = false;
            world.View<PlayerInputComponent, TransformComponent>(
                [&](EntityId, const PlayerInputComponent&, TransformComponent& t) {
                    const float bodyY = t.position.y; // オフセット無しの“本体”Y
                    const float pivotY = bodyY + rig.lookAtOffset.y; // 視点用
                    if (!anyTarget || pivotY > highestTargetPos.y) {
                        highestTargetPos.x = t.position.x + rig.lookAtOffset.x;
                        highestTargetPos.y = pivotY;
                        highestTargetPos.z = t.position.z + rig.lookAtOffset.z;
                        highestBodyY = bodyY;
                        anyTarget = true;
                    }
                });
            XMFLOAT3 pivot = anyTarget ? highestTargetPos : camTr.position;
            XMFLOAT3 camPos = camTr.position;

            switch (rig.mode)
            {
            case Camera3DComponent::Mode::Orbit:
            {
                const float yawRad = XMConvertToRadians(rig.orbitYawDeg);
                const float pitchRad = XMConvertToRadians(rig.orbitPitchDeg);
                const float cosP = cosf(pitchRad); const float sinP = sinf(pitchRad);
                const float cosY = cosf(yawRad);   const float sinY = sinf(yawRad);
                camPos.x = pivot.x + rig.orbitDistance * cosP * cosY;
                camPos.y = pivot.y + rig.orbitDistance * sinP;
                camPos.z = pivot.z + rig.orbitDistance * cosP * sinY;
                camTr.position = camPos; break;
            }
            case Camera3DComponent::Mode::SideScroll:
            {
                const float H = rig.orthoHeight;
                const float camBottomY = camPos.y - H * 0.5f;
                const float camTopY = camPos.y + H * 0.5f;
                const float bottom25LineY = camBottomY + H * 0.25f;
                const bool autoRequired = (deathTopY >= bottom25LineY);
                const float minCamYByDeath = deathTopY + H * 0.25f;

                // 25%境界 (判定は本体Yで行う)
                const float bottomThreshold = camBottomY + H * 0.25f; // 下部境界
                const float topThreshold = camTopY - H * 0.25f; // 上部境界

                float targetY = camPos.y;
                if (anyTarget)
                {
                    const bool inTopZone = highestBodyY > topThreshold;    // 本体が上部25%侵入
                    const bool inBottomZone = highestBodyY < bottomThreshold; // 本体が下部25%侵入

                    // 上方向: プレイヤー本体が topThreshold を超えたら camCenter = bodyY - H*0.25
                    const float desiredUpCenter = highestBodyY - H * 0.25f;
                    // 下方向: プレイヤー本体が bottomThreshold を下回ったら camCenter = bodyY + H*0.25
                    const float desiredDownCenter = highestBodyY + H * 0.25f;

                    if (autoRequired)
                    {
                        const float steppedUp = camPos.y + rig.scrollSpeed * dt;
                        const float desiredAuto = max(steppedUp, minCamYByDeath);
                        targetY = desiredAuto;
                        if (inTopZone)
                        {
                            targetY = max(targetY, desiredUpCenter);
                        }
                    }
                    else
                    {
                        if (inTopZone)
                        {
                            targetY = desiredUpCenter;
                        }
                        else if (inBottomZone)
                        {
                            targetY = desiredDownCenter;
                        }
                        // 中央帯は固定
                    }
                }

                // 最低Y制限
                if (targetY < rig.baseScrollY) targetY = rig.baseScrollY;

                // スムージング（必要差分のみ）
                const float diff = targetY - camPos.y;
                if (std::fabs(diff) > rig.followDeadzoneY)
                {
                    const float t = (rig.followSmoothTimeY <= 1e-5f) ? 1.f : (1.f - std::exp(-(1.f / rig.followSmoothTimeY) * dt));
                    camPos.y += diff * t;
                }

                // X/Z 固定処理
                camPos.x = rig.sideFixedX;
                if (rig.depthLerpSpeed <= 0.f) camPos.z = rig.sideFixedZ; else {
                    const float dz = rig.sideFixedZ - camPos.z; camPos.z += dz * min(1.f, rig.depthLerpSpeed * dt);
                }
                camTr.position = camPos;
                pivot.x = camPos.x + rig.sideLookAtX; pivot.y = camPos.y; pivot.z = camPos.z + 10.f;
                break;
            }
            case Camera3DComponent::Mode::Fixed:
            {
                // 固定モード：移動計算は行わない（TransformComponentの位置をそのまま使う）
                // 向きは単純にZ+方向（奥）を向くように設定
                pivot = camPos;
                pivot.z += 1.0f;
                break;
            }
            default: break;
            }

            // ビュー / プロジェクション
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&pivot);
            XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);

            // SideScroll または Fixed の場合は並行投影 (Orthographic)
            bool isOrtho = (rig.mode == Camera3DComponent::Mode::SideScroll || rig.mode == Camera3DComponent::Mode::Fixed);

            XMMATRIX P = isOrtho
                ? XMMatrixOrthographicLH(rig.orthoHeight * rig.aspect, rig.orthoHeight, rig.nearZ, rig.farZ)
                : XMMatrixPerspectiveFovLH(XMConvertToRadians(rig.fovY), rig.aspect, rig.nearZ, rig.farZ);

            DirectX::XMStoreFloat4x4(&m_viewT, XMMatrixTranspose(V));
            DirectX::XMStoreFloat4x4(&m_projT, XMMatrixTranspose(P));
            Geometory::SetView(m_viewT); Geometory::SetProjection(m_projT);
        });
}