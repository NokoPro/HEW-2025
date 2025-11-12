/*****************************************************************//**
 * @file   PhysicsLayers.h
 * @brief  当たり判定で使うレイヤ定義
 *
 * アクションゲームだと「プレイヤー同士は押し合うけどダメージはしない」
 * 「プレイヤーと敵は攻撃判定以外ではぶつからない」などが出てくるので
 * こういうビットマスクを最初に用意しておくと後が楽です。
 *********************************************************************/
#pragma once
#include <cstdint>

namespace Physics
{
    using LayerMask = std::uint32_t;

    enum Layer : LayerMask
    {
        LAYER_DEFAULT = 1 << 0,
        LAYER_PLAYER = 1 << 1,
        LAYER_PLAYER_HURT = 1 << 2,  // プレイヤーの被弾用
        LAYER_GROUND = 1 << 3,
        LAYER_MOVING_PLAT = 1 << 4,
        LAYER_GOAL = 1 << 5,
        LAYER_DESU_ZONE = 1 << 6,
        // 必要に応じて足していく
    };
}
