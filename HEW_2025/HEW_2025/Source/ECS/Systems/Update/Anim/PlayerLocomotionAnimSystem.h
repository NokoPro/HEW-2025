/*********************************************************************//**
 * @file   PlayerLocomotionAnimSystem.h
 * @brief  移動状態からアニメーションの論理ステートを決めるSystem
 *
 * - Rigidbody2DComponent の速度と onGround を使って、
 *   Idle / Run / Jump / Fall を切り替える。
 *
 * - 実際のクリップ再生は ModelAnimationStateSystem / ModelAnimationSystem が担当。
 *
 * @author 浅野勇生
 * @date   2025/11/27
 *********************************************************************/
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"

#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"

class PlayerLocomotionAnimSystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};
