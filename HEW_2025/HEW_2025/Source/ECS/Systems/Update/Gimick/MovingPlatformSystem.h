/*****************************************************************//**
 * @file   MovingPlatformSystem.h
 * @brief  ˆÚ“®°‚ğ§Œä‚·‚éSystem
 * 
 * @author ó–ì—E¶
 * @date   2025/11/14
 *********************************************************************/
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Gimick/MovingPlatformComponent.h"

class MovingPlatformSystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};