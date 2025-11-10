/*****************************************************************//**
 * @file   CameraMath.cpp
 * @brief  3Dカメラ計算の実装
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "CameraMath.h"

using namespace DirectX;

DirectX::XMMATRIX MakeViewMatrix(const Camera3DComponent& cam)
{
    const XMVECTOR eye = XMVectorSet(cam.eyeX, cam.eyeY, cam.eyeZ, 1.0f);
    const XMVECTOR target = XMVectorSet(cam.targetX, cam.targetY, cam.targetZ, 1.0f);
    const XMVECTOR up = XMVectorSet(cam.upX, cam.upY, cam.upZ, 0.0f);

    // 右手座標系ビュー行列
    return XMMatrixLookAtRH(eye, target, up);
}

DirectX::XMMATRIX MakeProjMatrix(const Camera3DComponent& cam)
{
    // FOVは度数で持っているのでラジアンに変換
    const float fovRad = XMConvertToRadians(cam.fovY);
    return XMMatrixPerspectiveFovRH(
        fovRad,
        cam.aspect,
        cam.nearZ,
        cam.farZ
    );
}
