/**********************************************************************************************
 * @file      CountdownUISystem.h
 * @brief     カウントダウンUI更新システム
 *
 * @author    浅野勇生
 * @date      2025/12/4
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/4 ]
 * 
 *    - [◎] CountdownUISystem作成
 *
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

class CountdownUISystem : public IUpdateSystem
{
public:
	void Update(World& world, float dt) override;
};
