#include "PrefabUIOverlay.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/PlayerUIComponent.h"
#include "System/AssetManager.h"

void RegisterUIOverlayPrefab(PrefabRegistry& registry)
{
    registry.Register("UIOverlay",
        [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = world.Create();
            if (e == kInvalidEntity)
                return kInvalidEntity;

            // Transform: スクリーン座標相当。ZはUI専用レイヤ（負値で前面）に固定
            const auto pos = DirectX::XMFLOAT3{
                sp.position.x, sp.position.y, (sp.position.z != 0.0f ? sp.position.z : -100.0f)
            };
            const auto rot = DirectX::XMFLOAT3{ 0.f, 0.f, 0.f };
            const auto scl = DirectX::XMFLOAT3{
                (sp.scale.x != 0.f ? sp.scale.x : 1.f),
                (sp.scale.y != 0.f ? sp.scale.y : 1.f),
                1.f
            };
            auto& tr = world.Add<TransformComponent>(e, pos, rot, scl);

            // スプライト
            auto& spr = world.Add<Sprite2DComponent>(e);
			spr.alias = sp.modelAlias.empty() ? "ui_overlay_default" : sp.modelAlias;
			spr.width = sp.spriteWidth;
			spr.height = sp.spriteHeight;
			spr.visible = true;
			spr.layer = sp.layer; // UI最前面

            return e;
        }
    );
}