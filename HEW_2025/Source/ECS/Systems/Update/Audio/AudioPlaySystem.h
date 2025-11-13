#pragma once
#include"ECS/World.h"
#include"ECS/Systems/IUpdateSystem.h"
#include"ECS/Systems/Update/Audio/AudioManagerSystem.h"
class AudioPlaySystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};