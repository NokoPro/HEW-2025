

/*****************************************************************//**
 * @file   PrefabFollowerBlink.cpp
 * @brief  ブリンク用UIフォロワーエンティティ生成プレハブ実装
 *
 * 生成後に Scene で FollowerComponent.targetId をプレイヤーへ設定してください。
 * 
 * @author 浅野勇生
 * @date   2025/11/17
 *********************************************************************/
#include "PrefabFollowerBlink.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Render/PlayerUIComponent.h"

void RegisterFollowerBlinkPrefab(PrefabRegistry& registry)
{
    registry.Register("FollowerBlink",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            // Transform
            w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // Sprite2D (ブリンクUI)
            auto& sp2d = w.Add<Sprite2DComponent>(e);
            sp2d.alias = "ui_blink"; // ブリンクのUI
            sp2d.width = 5.0f;
            sp2d.height = 5.0f;
            sp2d.originX = 0.5f;
            sp2d.originY = 0.5f;
            sp2d.layer = 100;      // UI前面
            sp2d.visible = false;  // 初期は非表示

            // Follower (targetId は後で設定)
            auto& fol = w.Add<FollowerComponent>(e);
            fol.targetId = 0;      // 未設定
            fol.offset = {-1.45f, 6.0f}; // PlayerUISystemで再設定されるため初期値のみ

            // UI状態コンポーネント
            w.Add<PlayerUIComponet>(e);

            return e;
        }
    );
}
