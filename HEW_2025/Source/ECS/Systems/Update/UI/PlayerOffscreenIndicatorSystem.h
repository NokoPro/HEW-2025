/**********************************************************************************************
 * @file      PlayerOffscreenIndicatorSystem.h
 * @brief     画面下端インジケータUIの更新システム宣言
 *
 * アクティブカメラの視野とプレイヤー位置を用いて、落下中で画面外（下側）にいるプレイヤーに
 * 対してインジケータUIを表示し、X軸に追従しながら画面下端に固定します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

class PlayerOffscreenIndicatorSystem : public IUpdateSystem
{
public:
    /**
     * @brief インジケータUIのフレーム更新
     * @param world ECS の World
     * @param dt 経過時間（秒）
     */
    void Update(World& world, float dt) override;
};
