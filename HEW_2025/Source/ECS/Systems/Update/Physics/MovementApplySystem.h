#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"

// コンポーネント
#include "../../../Components/Input/MovementIntentComponent.h"
#include "../../../Components/Physics/Rigidbody2DComponent.h"
#include "../../../Components/Physics/Collider2DComponent.h"

/**
 * @brief 入力(Intent)を実際の物理ボディ(Rigidbody)に反映するSystem
 *
 * - 地上と空中で加速の仕方を変える
 * - ジャンプ入力を見て上方向の速度を与える
 * - コライダーが持つマテリアル(摩擦)を見て、滑りやすさを変える
 * - 今フレームぶん蓄積されていたforce(accumulatedForce)を最後に足す
 *
 * ※ Transformの位置を本当に動かすのはこのあとに来る「物理ステップ」側に任せると分離しやすい
 */
class MovementApplySystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;

private:
    // 地上にいるときの加速の強さ
    const float m_groundAccel = 40.0f;
    // 空中での加速（空中制御）少し弱め
    const float m_airAccel = 15.0f;
    // 地上での最大速度
    const float m_maxSpeedX = 6.0f;
    // ジャンプ速度
    const float m_jumpSpeed = 20.5f;
    // 重力（ここで足してもいいし、専用のGravitySystemを作ってもよい）
    const float m_gravity = -25.0f;
};
