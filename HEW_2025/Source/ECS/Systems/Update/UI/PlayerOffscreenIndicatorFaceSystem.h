/**********************************************************************************************
 * @file      PlayerOffscreenIndicatorFaceSystem.h
 * @brief     画面下端インジケータ上の HI / GO / アイコン切り替え System 宣言
 *
 * OffscreenIndicatorFaceComponent を持つエンティティについて、
 * 対象プレイヤーの入力（ジャンプ / ブリンク）を見て
 * スプライトのテクスチャ別名を HI / GO / アイコンのいずれかに切り替えます。
 *
 * 表示 / 非表示や位置追従は PlayerOffscreenIndicatorSystem が担当します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

class PlayerOffscreenIndicatorFaceSystem : public IUpdateSystem
{
public:
    /**
     * @brief インジケータ上物のフレーム更新
     * @param world ECS の World
     * @param dt    経過時間（秒）
     */
    void Update(World& world, float dt) override;
};
