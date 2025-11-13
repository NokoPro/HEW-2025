/*****************************************************************//**
 * @file   FollowCameraSystem.cpp
 * @brief  縦スクロール（SideScroll）：強制スクロール優先 → 上位プレイヤー追従
 * @details
 *  - 強制スクロールの発火条件は「Deathゾーンの上端が画面下端から25%ラインに達したら」。
 *  - 発火中は下限拘束（Death上端が25%を超えない）を守りつつ、上にいるプレイヤーがいればそちらへも追従。
 *  - 非発火中は、上昇も下降も“常に”最上位プレイヤーに追従。
 *  - 追従はデッドゾーン＋指数スムージングでガタつきを抑制。
 *
 * @author 浅野勇生
 * @date   2025/11/13
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

using namespace DirectX;
using std::min;
using std::max;

void FollowCameraSystem::Update(World& world, float dt)
{
    // ============================
    // 1) Deathゾーンの「上端Y」を収集
    //    top = 中心Y + 半高さ
    // ============================
    float deathTopY = -FLT_MAX;
    world.View<TagDeathZone, TransformComponent>(
        [&](EntityId, const TagDeathZone&, const TransformComponent& t)
        {
            const float topY = t.position.y + t.scale.y;
            if (topY > deathTopY)
            {
                deathTopY = topY;
            }
        }
    );

    // ============================
    // 2) アクティブなカメラを処理
    // ============================
    world.View<ActiveCameraTag, Camera3DComponent, TransformComponent>(
        [&](EntityId /*camEnt*/,
            ActiveCameraTag& /*tag*/,
            Camera3DComponent& rig,
            TransformComponent& camTr)
        {
            // ---- 最上位（最もYが高い）プレイヤーを収集 ----
            XMFLOAT3 highestTargetPos = { 0.0f, -FLT_MAX, 0.0f };
            bool anyTargetFound = false;

            world.View<PlayerInputComponent, TransformComponent>(
                [&](EntityId /*e*/, const PlayerInputComponent& /*pic*/, TransformComponent& t)
                {
                    const float ty = t.position.y + rig.lookAtOffset.y;
                    if (!anyTargetFound || ty > highestTargetPos.y)
                    {
                        highestTargetPos.x = t.position.x + rig.lookAtOffset.x;
                        highestTargetPos.y = ty;
                        highestTargetPos.z = t.position.z + rig.lookAtOffset.z;
                        anyTargetFound = true;
                    }
                }
            );
            
            // ---- 基準pivot（ターゲットがいればそれ） ----
            XMFLOAT3 pivot = camTr.position;
            if (anyTargetFound)
            {
                pivot = highestTargetPos;
            }

            // ---- 現在カメラ位置 ----
            XMFLOAT3 camPos = camTr.position;

            switch (rig.mode)
            {
            case Camera3DComponent::Mode::Orbit:
            {
                // 既存のオービット挙動
				const float yawRad = XMConvertToRadians(rig.orbitYawDeg);     /// 方位角
				const float pitchRad = XMConvertToRadians(rig.orbitPitchDeg); /// 仰俯角
				const float cosP = cosf(pitchRad);                            /// 仰俯角のcos/sin
				const float sinP = sinf(pitchRad);                            /// 仰俯角のcos/sin
				const float cosY = cosf(yawRad);                              /// 方位角のcos/sin
				const float sinY = sinf(yawRad);                              /// 方位角のcos/sin

				/// カメラ位置計算（球面座標→直交座標変換）
                camPos.x = pivot.x + rig.orbitDistance * cosP * cosY;
                camPos.y = pivot.y + rig.orbitDistance * sinP;
                camPos.z = pivot.z + rig.orbitDistance * cosP * sinY;
                camTr.position = camPos;
                break;
            }

            case Camera3DComponent::Mode::SideScroll:
            {
                // ---- 画面幾何（正射影） ----
                const float H = rig.orthoHeight;
                const float camCenterY = camPos.y;
                const float camBottomY = camCenterY - H * 0.5f;

                // 画面下端から25%のライン（ここにDeath“上端”が触れたら強制スクロール発火）
                const float triggerY = camBottomY + H * 0.25f;
                const bool  autoRequired = (deathTopY >= triggerY);

                // 強制スクロール時に死守すべき下限：
                //   (camBottomY + 0.25H >= deathTopY) ⇔ camPos.y >= deathTopY + 0.25H
                const float minCamYByDeath = deathTopY + H * 0.25f;

                // プレイヤー追従（“いちばん上のプレイヤー”）
                const bool  hasPlayer = anyTargetFound;
				const float followY = hasPlayer ? highestTargetPos.y : camPos.y;    /// プレイヤー追従目標Y

                // 目標Yの決定
                float targetY = camPos.y;
                if (autoRequired)
                {
                    // 優先度1：強制スクロール（絶対・下限拘束＋上昇速度）
                    const float steppedUp = camPos.y + rig.scrollSpeed * dt;
                    const float desiredAuto = max(steppedUp, minCamYByDeath);

                    // 上にプレイヤーがいれば、下限を守りつつそちらへも追従
                    targetY = hasPlayer ? max(desiredAuto, followY) : desiredAuto;
                }
                else
                {
                    // 優先度2/3：非発火中は“常に”最上位プレイヤーに追従（上昇・下降とも）
                    targetY = followY;
                }

                // 任意のベース下限を割らない（使っている場合）
                if (targetY < rig.baseScrollY)
                {
                    targetY = rig.baseScrollY;
                }

                // デッドゾーン → 指数スムージング（ガタつき抑制）
                const float diff = targetY - camPos.y;
                if (std::fabs(diff) > rig.followDeadzoneY)
                {
                    const float t = (rig.followSmoothTimeY <= 1e-5f)
                        ? 1.0f
                        : (1.0f - std::exp(-(1.0f / rig.followSmoothTimeY) * dt));
                    camPos.y += diff * t;
                }

                /// X/Zはサイドビュー固定。Zはなめらかに復帰（depthLerpSpeed=0なら即時）
                camPos.x = rig.sideFixedX;
                if (rig.depthLerpSpeed <= 0.0f)
                {
                    camPos.z = rig.sideFixedZ;
                }
                else
                {
                    const float dz = rig.sideFixedZ - camPos.z;
                    const float tz = min(1.0f, rig.depthLerpSpeed * dt);
                    camPos.z += dz * tz;
                }

                camTr.position = camPos;

                // 注視点はX方向にオフセット、YはカメラY、Zは前方固定ぶんを足す
                pivot.x = camPos.x + rig.sideLookAtX;
                pivot.y = camPos.y;
                pivot.z = camPos.z + 10.0f;
                break;
            }

            default:
                break;
            }

            // ============================
            // 3) ビュー / プロジェクション更新（転置して保持）
            // ============================
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&pivot);
            XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P;

			/// モード別プロジェクション行列
            if (rig.mode == Camera3DComponent::Mode::SideScroll)
            {
				/// 正射影（サイドビュー用）
                const float orthoHeight = rig.orthoHeight;
                const float orthoWidth = orthoHeight * rig.aspect;
                P = XMMatrixOrthographicLH(orthoWidth, orthoHeight, rig.nearZ, rig.farZ);
            }
            else
            {
				/// 通常の透視投影
                P = XMMatrixPerspectiveFovLH(
                    XMConvertToRadians(rig.fovY),
                    rig.aspect, rig.nearZ, rig.farZ);
            }

			/// 転置して保存
            XMStoreFloat4x4(&m_viewT, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_projT, XMMatrixTranspose(P));

			/// Geometory 側にも設定
            Geometory::SetView(m_viewT);
            Geometory::SetProjection(m_projT);
        });
}
