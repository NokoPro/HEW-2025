#pragma once
/*****************************************************************//**
 * @file   TimerSystem.h
 * @brief
  *
 * @author ê¥êÖåıîVâÓ
 * @date   2025/12/02
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/DigitUIComponent.h"

class ResultTimerSystem : public IUpdateSystem
{
private:
	float m_Time=0.0f;
public:
	void Update(World& world, float dt) override;
	float SetTime(float Rank);
};