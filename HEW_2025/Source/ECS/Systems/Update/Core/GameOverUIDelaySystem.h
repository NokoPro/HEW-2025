/**********************************************************************************************
 * @file      GameOverUIDelaySystem.h
 * @brief     ゲームオーバーUIの表示を遅延させるシステムの宣言
 *
 * @author    浅野勇生
 * @date      2025/12/5
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/5 ]
 * 
 *    - [◎] GameOverUIDelaySystem作成
 *
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

// ゲームオーバーUIの表示を遅延させるシステム。GameStateComponentのバッファに従う。
class GameOverUIDelaySystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};
