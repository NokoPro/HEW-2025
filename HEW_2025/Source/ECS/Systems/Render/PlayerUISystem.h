/*********************************************************************/
/* @file   PlayerUISystem.cpp
 * @brief  プレイヤーのUI描画するかを切り替える処理（更新システム）
 *
 * 時間経過でUIを消す処理
 *
 * @author 土本蒼翔
 * @date   2025/11/17
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

#include "ECS/Components/Render/PlayerUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"

class PlayerUISystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};