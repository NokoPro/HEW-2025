/*****************************************************************//**
 * @file   Rigidbody2DComponent.h
 * @brief  力や速度の影響を受けるためのボディ
 *
 * ここでは「今の速度」「次のフレームで足す力」「質量」「重力を受けるか」
 * などを持っておきます。移動床に乗ったときはここに加算するだけで済むように。
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

struct Rigidbody2DComponent
{
    DirectX::XMFLOAT2 velocity{ 0.0f, 0.0f };    ///< 現在の速度
    DirectX::XMFLOAT2 accumulatedForce{ 0.0f, 0.0f }; ///< 今フレームぶんの外力（風・ジャンプなど）
    float mass = 1.0f;                            ///< 質量。0(=無限)にしたら動かさないでもOK
    bool  useGravity = true;                      ///< 重力を受けるか
    bool  onGround = false;                     ///< 足場にいる判定（ジャンプ可などで使う）
};
