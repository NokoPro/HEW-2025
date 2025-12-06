/*****************************************************************//**
 * @file   BackGronudScrollSystem.h
 * @brief  背景をスクロール(ワープ)させるSystem
 *
 * @author 土本蒼翔
 * @date   2025/11/25
 *********************************************************************/
#pragma once

#include "ECS/World.h"
#include "ECS/Systems/IUpdateSystem.h"

#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/BackGroundComponent.h"
#include "ECS/Components/Game/BackGroundScrollComponent.h"

#include "ECS/Components/Core/Camera3DComponent.h"
//#include "ECS/Components/Core/ActiveCameraTag.h"



 /**
   * @class BackGroundScrollSystem
   * @brief 背景をスクロールする
   */
class BackGroundScrollSystem: public IUpdateSystem
{

public:
	// ワープ
	void Update(class World& world, float dt) override;

private:

};