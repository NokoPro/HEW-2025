/*********************************************************************/
/* @file   DeathZoneSystem.h
 * @brief  死亡ゾーンシステム
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"

class DeathZoneSystem : public IUpdateSystem
{
public:
    DeathZoneSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(class World& world, float dt) override;

    // 外部(チート/リトライ)用インターフェイス
    void ForceDeath() { m_triggered = true; }
    void ResetDeathFlag() { m_triggered = false; }
    bool IsDead() const { return m_triggered; }
private:
    Collision2DSystem* m_colSys = nullptr;  /// 死亡ゾーンの当たり判定システム
    bool m_triggered = false;               /// 死亡ゾーンに触れたかどうか
};
