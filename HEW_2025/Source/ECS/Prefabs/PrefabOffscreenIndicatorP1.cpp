/**********************************************************************************************
 * @file      PrefabOffscreenIndicatorP1.cpp
 * @brief     1P用 画面下端インジケータUIのプレハブ登録・生成
 *
 * 吹き出し背景用の `Sprite2DComponent` と、インジケータ制御用の `OffscreenIndicatorComponent` を
 * 持つUIエンティティを生成します。テクスチャ別名やマージン/オフセットはここで初期化します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#include "PrefabOffscreenIndicatorP1.h"
#include "ECS/Components/UI/OffscreenIndicatorComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

void RegisterOffscreenIndicatorP1Prefab(PrefabRegistry& reg)
{
    reg.Register("OffscreenIndicatorP1",
        [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = world.Create();

            // Transform 初期化（位置・回転・スケールは呼び出し元の指定を尊重）
            world.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // 背景スプライト（吹き出し）
            auto& spr = world.Add<Sprite2DComponent>(e);
            spr.visible = false;                     // 初期は非表示（Systemが制御）
            spr.alias = "tex_bubble";           // アセット側に合わせて設定
			spr.width = 7.0f;
			spr.height = 7.0f;

            // インジケータ制御
            auto& ind = world.Add<OffscreenIndicatorComponent>(e);
            ind.visible = false;
            ind.bottomMargin = 0.6f;                 // 少し上に持ち上げる量
            ind.offset = { 0.0f, 5.0f, 0.0f };
            // targetId は生成後に外部で設定（1PプレイヤーID）

            return e;
        }
    );
}
