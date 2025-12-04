/*****************************************************************//**
 * @file   FollowerSystem.h 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/15
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include <DirectXMath.h>

/**
 * @class FollowerSystem
 * @brief FollowerComponent を持つエンティティをプレイヤーに追従させる.
 */
class FollowerSystem : public IUpdateSystem
{
public:
	void Update(World& world, float dt) override;
};





