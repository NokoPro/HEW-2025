/**********************************************************************************************
 * @file      PrefabOffscreenIndicatorP2.cpp
 * @brief     2P用 画面下端インジケータUIのプレハブ登録・生成
 *
 * 吹き出し背景用の `Sprite2DComponent` と、インジケータ制御用の `OffscreenIndicatorComponent` を
 * 持つUIエンティティを生成します。P2向けのテクスチャ別名で初期化します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#include "PrefabOffscreenIndicatorP2.h"
#include "ECS/Components/UI/OffscreenIndicatorComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

void RegisterOffscreenIndicatorP2Prefab(PrefabRegistry& reg)
{
    reg.Register("OffscreenIndicatorP2",
        [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = world.Create();

            world.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            auto& spr = world.Add<Sprite2DComponent>(e);
            spr.visible = false;
            spr.alias = "tex_bubble";     // 2P用吹き出し背景
			spr.width = 7.0f;
			spr.height = 7.0f;

            auto& ind = world.Add<OffscreenIndicatorComponent>(e);
            ind.visible = false;
            ind.bottomMargin = 0.6f;
            ind.offset = { 0.0f, 5.0f, 0.0f };

            return e;
        }
    );
}
