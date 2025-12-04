/*********************************************************************/
/* @file   DeathZoneSystem.cpp
 * @brief  死亡ゾーン（DeathZone）システム実装
 * * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "DeathZoneSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Input/PlayerInputComponent.h"

#include "System/Defines.h"
#include "System/DebugSettings.h"
#include "System/TimeAttackManager.h"

#include <Windows.h> // MessageBox

void DeathZoneSystem::Update(World& world, float dt)
{
    const bool magmaOn = DebugSettings::Get().magmaEnabled;
    const float speedScale = DebugSettings::Get().magmaSpeedScale;
    const bool god = DebugSettings::Get().godMode;

    // Deathゾーンの上昇処理
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e, TransformComponent& tr, Collider2DComponent& col)
        {
            if (col.layer == Physics::LAYER_DESU_ZONE)
            {
                if (magmaOn)
                {
                    // 変更点: 定数ではなくメンバ変数の m_riseSpeed を使用
                    tr.position.y += dt * m_riseSpeed * speedScale; // 上昇速度（倍率）
                }
            }
        });

    /// Deathゾーンに被弾したかチェック
    if (!magmaOn || m_triggered || !m_colSys) return;
    CollisionEventBuffer* eventBuffer = m_colSys->GetEventBuffer();
    if (!eventBuffer) return;

    // プレイヤーエンティティ取得
    EntityId player1 = 0, player2 = 0;
    world.View<PlayerInputComponent>(
        [&](EntityId e, const PlayerInputComponent& pic)
        {
            if (player1 == 0) player1 = e;
            else if (player2 == 0) player2 = e;
        });

    // 衝突イベントバッファからプレイヤーとDeathゾーンの接触を検出
    for (const auto& ev : eventBuffer->events)
    {
        if ((ev.self == player1 || ev.self == player2) && ev.trigger)
        {
            auto* colOther = world.TryGet<Collider2DComponent>(ev.other);
            if (colOther && colOther->layer == Physics::LAYER_DESU_ZONE)
            {
                if (god) { continue; }
                m_triggered = true;
                DebugSettings::Get().gameDead = true;
                DebugSettings::Get().gameTimerRunning = false; // 停止
                TimeAttackManager::Get().NotifyDeath();
                MessageBoxA(nullptr, "Deathゾーンに接触しました!", "Game Over", MB_OK | MB_ICONEXCLAMATION);
                break;
            }
        }
    }
}