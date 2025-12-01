/**********************************************************************************************
 * @file      ModelAnimationSystem.cpp
 * @brief     ModelAnimationComponent を使って3Dモデルのアニメーションを進めるシステム
 *
 * @author    浅野勇生
 * @date      2025/11/27
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/27 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#include "ModelAnimationSystem.h"
#include "ECS/World.h"
#include "System/Model.h"
#include "ECS/Components/Render/ModelComponent.h"

void ModelAnimationSystem::Update(World& world, float dt)
{
    // ModelRenderer + ModelAnimation を持つエンティティを全て処理
    world.View<ModelRendererComponent, ModelAnimationComponent>(
        [&](EntityId /*e*/, ModelRendererComponent& mr, ModelAnimationComponent& anim)
        {
            // モデルが無ければ何もしない
            if (!mr.model)
            {
                return;
            }

            Model* model = mr.model.get();

            // 再生リクエストがあれば、ここで Play を叩く
            if (anim.playRequested)
            {
                if (anim.animeNo >= 0)
                {
                    // 合成アニメーションやブレンドはとりあえず後回し。
                    model->Play(anim.animeNo, anim.loop, anim.speed);
                }
                else
                {
                    // animeNo < 0 は「アニメ停止」の意味にする
                    model->StopAnimation();
                }
                // TODO: animeNo < 0 の場合に「再生停止」をどう扱うかは今後決める

                anim.playRequested = false;
            }

            // アニメーションの時間を進める
            model->Step(dt);
        });
}
