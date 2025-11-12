#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"

class DeathZoneSystem : public IUpdateSystem
{
public:
    DeathZoneSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(class World& world, float dt) override;
private:

    Collision2DSystem* m_colSys = nullptr;
    bool m_triggered = false;
};
