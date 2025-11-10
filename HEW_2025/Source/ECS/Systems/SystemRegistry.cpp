/*****************************************************************//**
 * @file   SystemRegistry.cpp
 * @brief  SystemRegistryクラスの実装
 *
 * Update系・Render系システムの一括管理と実行処理を行います。
 * 衝突処理(CollisionWorld)には依存しません。
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "SystemRegistry.h"
#include "../World.h"

SystemRegistry::~SystemRegistry() = default;

void SystemRegistry::Tick(World& world, float dt)
{
    for (auto& s : m_updates)
    {
        s->Update(world, dt);
    }
}

void SystemRegistry::Render(World& world)
{
    for (auto& s : m_renders)
    {
        s->Render(world);
    }
}
