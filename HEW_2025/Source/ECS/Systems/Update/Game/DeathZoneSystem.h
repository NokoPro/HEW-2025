/*********************************************************************/
/* @file   DeathZoneSystem.h
 * @brief  €–Sƒ][ƒ“ƒVƒXƒeƒ€
 * 
 * @author ó–ì—E¶
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
private:

	Collision2DSystem* m_colSys = nullptr;  /// €–Sƒ][ƒ“‚Ì“–‚½‚è”»’èƒVƒXƒeƒ€
	bool m_triggered = false;               /// €–Sƒ][ƒ“‚ÉG‚ê‚½‚©‚Ç‚¤‚©
};
