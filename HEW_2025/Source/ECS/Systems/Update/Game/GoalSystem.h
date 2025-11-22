#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"

class GoalSystem : public IUpdateSystem {
public:
    GoalSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(World& world, float dt) override;

    // 外部(チート/リトライ)用インターフェイス
    void ForceClear() { m_goalCleared = true; }
    void ResetClearFlag() { m_goalCleared = false; }
    bool IsCleared() const { return m_goalCleared; }
private:
    Collision2DSystem* m_colSys = nullptr;
    bool m_goalCleared = false; // ゴール到達済みフラグ
};
