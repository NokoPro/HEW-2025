/*********************************************************************/
/* @file   CollisionEvents.h
 * @brief  今フレームで発生した衝突情報を貯めておくバッファ
 *
 * これをWorldか、どこか共有できる場所に置いておいて、
 * 当たり判定Systemがここにpush_back、
 * 風SystemやダメージSystemが後で読む、という流れにします。
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <vector>
#include "ECS/ECS.h"

struct CollisionEvent
{
    EntityId self;     ///< 自分
    EntityId other;    ///< ぶつかった相手
    bool     trigger;  ///< トリガーだったかどうか
};

struct CollisionEventBuffer
{
    std::vector<CollisionEvent> events;

    void Clear()
    {
		/// 衝突イベントをクリア
        events.clear();
    }

    void Add(EntityId a, EntityId b, bool trigger)
    {
		/// 衝突イベントを追加
        events.push_back(CollisionEvent{ a, b, trigger });
    }
};
