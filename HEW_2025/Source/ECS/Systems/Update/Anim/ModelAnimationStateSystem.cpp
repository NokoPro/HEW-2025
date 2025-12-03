#include "ModelAnimationStateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/ModelComponent.h"

void ModelAnimationStateSystem::Update(World& world, float /*dt*/)
{
    world.View<ModelAnimationComponent,
        ModelAnimationStateComponent,
        ModelAnimationTableComponent,
        ModelRendererComponent>(
            [&](EntityId /*e*/,
                ModelAnimationComponent& anim,
                ModelAnimationStateComponent& state,
                ModelAnimationTableComponent& table,
                ModelRendererComponent& mr)
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

                // 「着地アニメ再生中」は Idle への遷移だけを抑止
                if (state.current == ModelAnimState::Land && state.requested == ModelAnimState::Idle)
                {
                    const int landClipNo = table.table[static_cast<size_t>(ModelAnimState::Land)].animeNo;
                    if (landClipNo >= 0 && mr.model && mr.model->IsPlay(landClipNo))
                    {
                        // Idle への切り替えをスキップ（他のリクエストは通す）
                        return;
                    }
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
