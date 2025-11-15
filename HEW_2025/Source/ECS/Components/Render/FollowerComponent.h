/*****************************************************************//**
 * @file   FollowerComponent.h 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/15
 *********************************************************************/
#pragma once
#include "ECS/ECS.h"

/**
 * @brief 他のエンティティに追従するコンポーネント.
 */
struct FollowerComponent
{
	EntityId targetId = 0; // 追従対象のエンティティID
};




