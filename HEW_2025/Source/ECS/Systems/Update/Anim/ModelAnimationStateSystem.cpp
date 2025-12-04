#include "ModelAnimationStateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Core/PlayerStateComponent.h"

void ModelAnimationStateSystem::Update(World& world, float /*dt*/)
{
    world.View<ModelAnimationComponent,
        ModelAnimationStateComponent,
        ModelAnimationTableComponent,
        ModelRendererComponent,
        PlayerStateComponent>(
            [&](EntityId /*e*/,
                ModelAnimationComponent& anim,
                ModelAnimationStateComponent& state,
                ModelAnimationTableComponent& table,
                ModelRendererComponent& mr,
                const PlayerStateComponent& pstate)
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

                // 要求と現在が同じ場合の扱い（非ループは再再生許可）
                if (state.requested == state.current)
                {
                    const size_t idxSame = static_cast<size_t>(state.current);
                    const ModelAnimationClipDesc& curDesc = table.table[idxSame];
                    // 非ループの場合、現在そのクリップが再生されていなければ再生を再要求する
                    if (!curDesc.loop && curDesc.animeNo >= 0 && mr.model)
                    {
                        const bool playingSame = mr.model->IsPlay(curDesc.animeNo);
                        if (!playingSame)
                        {
                            anim.animeNo = curDesc.animeNo;
                            anim.loop = curDesc.loop;
                            anim.speed = curDesc.speed;
                            anim.playRequested = true;
                        }
                    }
                    return;
                }

                // Facing に応じて Run を左右で差し替え
                ModelAnimState requested = state.requested;
                if (requested == ModelAnimState::Run)
                {
                    if (pstate.m_facing == PlayerFacingState::Left)
                        requested = ModelAnimState::RunLeft;
                    else if (pstate.m_facing == PlayerFacingState::Right)
                        requested = ModelAnimState::RunRight;
                }

                const size_t idx = static_cast<size_t>(requested);
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
                state.current = requested;
            });
}
