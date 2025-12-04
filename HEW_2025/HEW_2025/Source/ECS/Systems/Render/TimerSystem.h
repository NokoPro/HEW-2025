/*****************************************************************//**
 * @file   TimerSystem.h 
 * @brief       
  * 
 * @author êÏíJóDê^
 * @date   2025/11/25
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