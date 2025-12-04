/**********************************************************************************************
 * @file      ResultRankingSystem.h
 * @brief     Result画面とのランキング表示システム
 *
 * @author    浅野勇生
 * @date      2025/12/2
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/2 ]
 * 
 *    - [◎] ResultRankingSystem作成
 *
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

class ResultRankingSystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
private:
    float m_posY = 0;
};