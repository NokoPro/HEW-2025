#include "DeathZoneSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Input/PlayerInputComponent.h"

#include "System/Defines.h"

#include <Windows.h> // MessageBox

void DeathZoneSystem::Update(World& world, float dt)
{
    // Deathゾーンを上昇させる
    world.View<TransformComponent, Collider2DComponent>([&](EntityId e, TransformComponent& tr, Collider2DComponent& col) {
        if (col.layer == Physics::LAYER_DESU_ZONE) {
            tr.position.y += dt * kDeathZoneSpeedY; // 上昇速度（調整可）
        }
    });

    if (m_triggered || !m_colSys) return;
    CollisionEventBuffer* eventBuffer = m_colSys->GetEventBuffer();
    if (!eventBuffer) return;

    // プレイヤーエンティティを特定
    EntityId player1 = 0, player2 = 0;
    world.View<PlayerInputComponent>(
        [&](EntityId e, const PlayerInputComponent& pic)
        {
            if (player1 == 0) player1 = e;
            else if (player2 == 0) player2 = e;
        });

    // 衝突イベントバッファからプレイヤーとDeathゾーンの衝突を探す
    for (const auto& ev : eventBuffer->events) {
        if ((ev.self == player1 || ev.self == player2) && ev.trigger) {
            auto* colOther = world.TryGet<Collider2DComponent>(ev.other);
            if (colOther && colOther->layer == Physics::LAYER_DESU_ZONE) {
                m_triggered = true;
                MessageBoxA(nullptr, "Deathゾーンに当たった！", "Game Over", MB_OK | MB_ICONEXCLAMATION);
                break;
            }
        }
    }
}
