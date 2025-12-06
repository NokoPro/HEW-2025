#pragma once
#include "ECS/World.h"
#include "ECS/Components/Core/PlayerStateComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Systems/IUpdateSystem.h"

class PlayerPresentationSystem : public IUpdateSystem
{
public:
    void Update(World& world, float deltaTime);
};
