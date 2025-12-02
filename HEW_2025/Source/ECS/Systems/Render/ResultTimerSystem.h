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

class TimerSystem : public IUpdateSystem
{
public:
	void Update(World& world, float dt) override;
};