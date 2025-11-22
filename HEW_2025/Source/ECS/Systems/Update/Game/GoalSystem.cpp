/*********************************************************************/
/* @file   GoalSystem.cpp
 * @brief  ゴールシステム実装
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "GoalSystem.h"
#include "ECS/Components/Physics/CollisionEvents.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "System/DebugSettings.h"
#include "System/TimeAttackManager.h"
#include <Windows.h>

void GoalSystem::Update(World& world, float dt)
{
	// すでにクリアしていたら何もしない
    if (m_goalCleared || !m_colSys) return;
    CollisionEventBuffer* eventBuffer = m_colSys->GetEventBuffer();
    if (!eventBuffer) return;

    // プレイヤーエンティティを特定
    EntityId player1 = 0, player2 = 0;

    // PlayerInputComponentを持つものを探す
    world.View<PlayerInputComponent>(
        [&](EntityId e,
            const PlayerInputComponent& pic)
        {
                if (player1 == 0) player1 = e;
                else if (player2 == 0) player2 = e;
    });

	// プレイヤーが2人いない場合は何もしない
    bool touched[2] = { false, false };

    // 衝突イベントバッファからプレイヤーとゴールの衝突を探す
    for (const auto& ev : eventBuffer->events) {
        // プレイヤー→ゴール
        if ((ev.self == player1 || ev.self == player2) && ev.trigger) 
        {
            auto* colOther = world.TryGet<Collider2DComponent>(ev.other);
            if (colOther && colOther->layer == Physics::LAYER_GOAL) 
            {
                if (ev.self == player1) touched[0] = true;
                if (ev.self == player2) touched[1] = true;
            }
        }
    }

	/// 両方のプレイヤーがゴールに触れていたらクリア
    if (touched[0] && touched[1])
    {
        m_goalCleared = true;
        DebugSettings::Get().gameCleared = true;
        DebugSettings::Get().gameTimerRunning = false; // 停止
        TimeAttackManager::Get().NotifyClear();
        MessageBoxA(nullptr, "ステージクリア！", "Clear", MB_OK | MB_ICONINFORMATION);
    }
}
