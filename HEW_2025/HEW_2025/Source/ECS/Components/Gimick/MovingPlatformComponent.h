/*********************************************************************/
/* @file   MovingPlatformComponent.h
 * @brief  毎フレーム動く床の情報
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

 /**
  * @brief 移動足場の動作パラメータ
  * - start/end の間を往復移動
  * - 端で待機（optional）
  */
struct MovingPlatformComponent
{
    DirectX::XMFLOAT2 start{ 0.f, 0.f };
    DirectX::XMFLOAT2 end{ 0.f, 0.f };
    float speed = 2.0f;         // 単位/秒（線形補間の移動速度）
    float waitTime = 0.0f;      // 端での待機秒
    float t = 0.0f;             // 0..1 の補間係数
    int   dir = +1;             // +1: start→end, -1: end→start
    float waitTimer = 0.0f;     // 残り待機時間
};