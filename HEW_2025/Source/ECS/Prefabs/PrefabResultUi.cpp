/*****************************************************************//**
* @file   PrefabBackGround.h
* @brief  背景を生成するプレハブ（Sprite2D 専用に簡素化）
*
* @author 篠原純
* @date   2025/11/25
* ********************************************************************/

#include"ECS/Prefabs/PrefabResultUi.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"


void RegisterResultUiPrefab(PrefabRegistry& registry)
{
    registry.Register("ResultUI",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            // Transform
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position = sp.position;
            tr.scale    = sp.scale;

            // Sprite2DComponent のみ付与（シンプルなスプライト表示）
            auto& spr = w.Add<Sprite2DComponent>(e);
            spr.alias   = sp.modelAlias;      // テクスチャ alias
            spr.width   = 16.8f + sp.scale.x; // 既存の見た目を継承
            spr.height  = 16.4f + sp.scale.y;
            spr.originX = 0.5f;
            spr.originY = 0.5f;
            spr.layer   = -100;               // 背景後面
            spr.visible = true;

            return e;
        }
    );
}