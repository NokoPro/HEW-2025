#include "ModelAnimationStateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/ModelComponent.h"

void ModelAnimationStateSystem::Update(World& world, float /*dt*/)
{
    world.View<ModelAnimationComponent,
        ModelAnimationStateComponent,
        ModelAnimationTableComponent>(
            [&](EntityId /*e*/,
                ModelAnimationComponent& anim,
                ModelAnimationStateComponent& state,
                ModelAnimationTableComponent& table)
            {
                // まだ何も要求されていない
                if (state.requested == ModelAnimState::None)
                {
                    if (state.current != ModelAnimState::None)
                    {
                        // 停止をリクエスト（animeNo < 0 の約束）
                        anim.animeNo = -1;
                        anim.loop = false;
                        anim.speed = 0.0f;
                        anim.playRequested = true;

                        state.current = ModelAnimState::None;
                    }
                    return;
                }

                // 同じステートなら何もしない（Playの多重呼び出し防止）
                if (state.requested == state.current)
                {
                    return;
                }

                const size_t idx = static_cast<size_t>(state.requested);
                if (idx >= table.table.size())
                {
                    return;
                }

                const ModelAnimationClipDesc& desc = table.table[idx];
                if (desc.animeNo < 0)
                {
                    // 対応するクリップが登録されていない
                    return;
                }

                // ModelAnimationComponent に反映
                anim.animeNo = desc.animeNo;
                anim.loop = desc.loop;
                anim.speed = desc.speed;
                anim.playRequested = true;  // 次の ModelAnimationSystem で Play される

                // 現在ステートを更新
                state.current = state.requested;
            });
}
