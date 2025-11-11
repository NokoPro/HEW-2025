/*********************************************************************/
/* @file   MovingPlatformComponent.h
 * @brief  毎フレーム動く床の情報
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

struct MovingPlatformComponent
{
    DirectX::XMFLOAT2 currentDelta{ 0.0f, 0.0f };  ///< 今フレーム動いた量（プレイヤーに渡す用）
    // ここにパスや速度など、あとでいくらでも足せる
};
