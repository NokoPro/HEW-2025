/**********************************************************************************************
 * @file      ResultRankingSystem.h
 * @brief     Result画面でタイムランキングを表示するUpdate System
 *
 * @author    浅野勇生
 * @date      2025/12/2
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/2 ]
 *    - [◎] ResultRankingSystem 作成
 *
 *  [ @date 2025/12/5 ]
 *    - [◎] 分・秒・ミリ秒の桁ごとのスケール比を調整
 *          (分=1.0, 秒=2/3, ミリ秒=1/3)
 *    - [◎] 横方向の桁間隔を BASE_DIGIT_W と合わせて重なりを防止
 *
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

 /**
  * @class ResultRankingSystem
  * @brief Result 画面でランキング上位タイムを桁ごとにレイアウトする System.
  *
  * - RankingManager から上位 N 件のタイムを取得
  * - RankingDigitUIComponent を持つエンティティごとに
  *   mm:ss.hh のどの桁を表示するかを決定
  * - Sprite2DComponent / TransformComponent に対して
  *   UV・サイズ・座標（行ごとに横一列）を反映する
  */
class ResultRankingSystem final : public IUpdateSystem
{
public:
    /**
     * @brief ランキング上位タイムを桁ごとに Sprite2D に反映する.
     * @param world ECS の World
     * @param dt    経過時間（本 System では未使用）
     */
    void Update(World& world, float dt) override;

private:
    float m_posY = 0.7f;  ///< 将来のレイアウト調整用(Y位置の基準)。現状は未使用.
};
