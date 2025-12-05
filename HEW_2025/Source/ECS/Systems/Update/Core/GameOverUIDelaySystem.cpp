#include "GameOverUIDelaySystem.h"
#include "ECS/Components/Core/GameStateComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Tag/Tag.h"

// GameOver状態のみ監視し、postDeathDelaySec 経過後にだけ GameOver スプライトを表示する。
void GameOverUIDelaySystem::Update(World& world, float /*dt*/)
{
    // 状態取得（単一前提）
    const GameStateComponent* gs = nullptr;
    world.View<GameStateComponent>([&](EntityId, GameStateComponent& st){ gs = &st; });
    if (!gs) return;

    const bool isGameOver = (gs->state == GamePlayState::GameOver);
    const bool allowShow  = isGameOver && (gs->elapsedInStateSec >= gs->postDeathDelaySec);

    world.View<GameOverMenu,Sprite2DComponent, TransformComponent>(
        [&](EntityId, GameOverMenu& /**/,Sprite2DComponent& sp, TransformComponent&)
        {
            // GameOver以外の状態では常に非表示
            if (!isGameOver)
            {
                sp.visible = false;
            }
            else
            {
                // GameOver中は遅延経過後に表示、それまでは非表示
                sp.visible = allowShow;
            }
        }
    );
}
