#include "ModelAnimationStateSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Core/PlayerStateComponent.h"
#include "System/DebugSettings.h"

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
                // ゲームオーバー中は常にDeathアニメを要求
                if (DebugSettings::Get().gameDead)
                {
                    state.requested = ModelAnimState::Death;
                }

                // まず何も要求されていない
                if (state.requested == ModelAnimState::None)
                {
                    if (state.current != ModelAnimState::None)
                    {
                        // 停止のリクエスト（animeNo < 0 の特殊値）
                        anim.animeNo = -1;
                        anim.loop = false;
                        anim.speed = 0.0f;
                        anim.playRequested = true;
                        state.current = ModelAnimState::None;
                    }
                    return;
                }

                // 着地アニメ再生中から Idle への遷移は少し抑制
                if (state.current == ModelAnimState::Land && state.requested == ModelAnimState::Idle)
                {
                    const int landClipNo = table.table[static_cast<size_t>(ModelAnimState::Land)].animeNo;
                    if (landClipNo >= 0 && mr.model && mr.model->IsPlay(landClipNo))
                    {
                        // Idle への切替をスキップ（今のリクエストは無視）
                        return;
                    }
                }

                // 今と同じリクエストの場合（非ループは都度再生する）
                if (state.requested == state.current)
                {
                    const size_t idxSame = static_cast<size_t>(state.current);
                    const ModelAnimationClipDesc& curDesc = table.table[idxSame];
                    // 非ループの場合、現在のクリップが再生していなければ再生を要求
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

                // Facing に合わせて Run を左右へ分岐
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
                    // 対応クリップが登録されていない
                    return;
                }

                // ModelAnimationComponent へ反映
                anim.animeNo = desc.animeNo;
                anim.loop = desc.loop;
                anim.speed = desc.speed;
                anim.playRequested = true;  // 後段 ModelAnimationSystem が Play する

                // 現在ステートを更新
                state.current = requested;
            });
}
