#pragma once
#include "../../IUpdateSystem.h"

class World;

class ParallaxSystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};
