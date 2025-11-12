#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"

class GoalSystem : public IUpdateSystem {
public:
    GoalSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(World& world, float dt) override;
private:
    Collision2DSystem* m_colSys = nullptr;
    bool m_goalCleared = false;
};
