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
	/// 当たり判定レイヤマスク型
    using LayerMask = std::uint32_t;

	/// 当たり判定レイヤ定義
    enum Layer : LayerMask
    {
		LAYER_DEFAULT = 1 << 0,     /// デフォルトレイヤ
		LAYER_PLAYER = 1 << 1,      /// プレイヤー本体用
		LAYER_PLAYER_HURT = 1 << 2, /// プレイヤーの被ダメージ判定用
		LAYER_GROUND = 1 << 3,      /// 地面・床用
		LAYER_MOVING_PLAT = 1 << 4, /// 動く足場用
		LAYER_GOAL = 1 << 5,        /// ゴール用
		LAYER_DESU_ZONE = 1 << 6,	/// Deathゾーン用
        // 必要に応じて足していく
    };
}
