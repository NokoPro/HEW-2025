/*********************************************************************/
/* @file   MovementApplySystem.h
 * @brief  移動適用システム定義
 * 
 * - 入力(Intent)を実際の物理ボディ(Rigidbody)に反映するSystem
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"

// コンポーネント
#include "../../../Components/Input/MovementIntentComponent.h"
#include "../../../Components/Physics/Rigidbody2DComponent.h"
#include "../../../Components/Physics/Collider2DComponent.h"
#include "System/GameplayConfig.h" // 追加: 外部設定

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
    // 外部CSVから読み取るプレイヤー調整値 (初期デフォルトは従来値)
    float m_groundAccel     = 40.0f;
    float m_airAccel        = 10.0f;
    float m_maxSpeedX       = 6.0f;
    float m_jumpSpeed       = 15.5f;
    float m_blinkUpImpulse  = 2.5f;

    // 毎フレーム最初に最新CSV値を反映（ホットリロード想定）
    void SyncConfig();
};
