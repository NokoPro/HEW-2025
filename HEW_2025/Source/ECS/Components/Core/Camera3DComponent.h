/*****************************************************************//**
 * @file   Camera3DComponent.h
 * @brief  3Dシーンを描画するためのカメラ情報を保持するコンポーネント
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include "ECS/ECS.h"  // EntityId

 /**
  * @brief サイドビュー縦スクロール＋ふつうの3Dカメラをまとめたコンポーネント
  */
struct Camera3DComponent
{
    // =========================
    // 純粋なカメラパラメータ
    // =========================
    float eyeX = 0.0f;
    float eyeY = 5.0f;
    float eyeZ = -10.0f;

    float targetX = 0.0f;
    float targetY = 0.0f;
    float targetZ = 0.0f;

    float upX = 0.0f;
    float upY = 1.0f;
    float upZ = 0.0f;

    float fovY = 60.0f;
    float aspect = 16.0f / 9.0f;
    float nearZ = 0.1f;
    float farZ = 1000.0f;

    // =========================
    // ここから制御(リグ)用
    // =========================
    enum class Mode
    {
        Orbit,      // 既存のターゲットを回り込むやつ
        SideScroll  // 縦スクロールのサイドビュー
    };

    Mode mode = Mode::Orbit;

    // 追従対象
    EntityId target = 0;

    // Orbit用
    float orbitYawDeg = 90.0f;
    float orbitPitchDeg = 0.0f;
    float orbitDistance = 8.0f;
    DirectX::XMFLOAT3 lookAtOffset{ 0.0f, 1.5f, 0.0f };

    // SideScroll用（今回新しく足したやつ）
    float scrollSpeed = 1.0f;    // カメラが自動で上に進む速度
    float followOffsetY = 1.5f;    // プレイヤー頭よりちょい上を見るとき
    float followMarginY = 0.5f;    // これだけ離れたら追従に切り替える

    float sideFixedX = 0.0f;    // サイドビューなのでX固定
    float sideFixedZ = -10.0f;  // 奥行き固定
    float sideLookAtX = 0.0f;    // 常にこのX方向を見る

    // Orthographic view height (world units). Smaller => more zoomed in.
    // Used in SideScroll mode to control zoom.
    float orthoHeight = 10.0f;

    // How fast camera depth (Z) interpolates towards sideFixedZ (units per second factor)
    // Larger -> snappier. 0 means instant set.
    float depthLerpSpeed = 10.0f;

    // Reference Y position considered as the "default" camera Y (e.g. initial Y).
    // If all players are below this Y, the camera will resume automatic vertical scrolling.
    float baseScrollY = 0.0f;

    // Runtime flag: whether camera is currently locked to follow target Y.
    bool followingTarget = false;

    // When deciding to resume auto-scroll, require the highest target to be this much below baseScrollY
    // (useful to keep camera above hazards like rising magma).
    float returnToAutoMargin = 0.5f;
};