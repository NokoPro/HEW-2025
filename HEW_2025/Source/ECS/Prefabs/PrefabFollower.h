/*****************************************************************//**
 * @file   PrefabFollower.h 
 * @brief  旧: フォロワーUIプレハブ (ジャンプ/ブリンク分離版は PrefabFollowerJump/Blink を参照)
 * 
 * 既存の呼び出し互換のため残すが、新規は RegisterFollowerJumpPrefab / RegisterFollowerBlinkPrefab を使用。
 * @author 川谷優真
 * @date   2025/11/17
 *********************************************************************/
#pragma once
#include "ECS/Prefabs/PrefabRegistry.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/PlayerUIComponent.h"

namespace Prefabs
{
	struct FollowerConfig
	{
		std::string textureName;
		float width = 64.0f;
		float height = 64.0f;
	};

	inline PrefabRegistry::SpawnFunc MakeFollowerPrefab(const FollowerConfig& cfg)
	{
		return[cfg](World& world, const PrefabRegistry::SpawnParams& p)->EntityId
			{
				const EntityId e = world.Create();
				if (e == kInvalidEntity) return kInvalidEntity;

				// Transform
				world.Add<TransformComponent>(e, p.position, p.rotationDeg, p.scale);

				// Sprite2D (旧:汎用。現行はジャンプ用に ui_jump を想定)
				auto& sp = world.Add<Sprite2DComponent>(e);
				sp.alias = cfg.textureName; // 例: "ui_jump"
				sp.width = cfg.width;
				sp.height = cfg.height;
				sp.layer = 100;
				sp.originX = 0.5f;
				sp.originY = 0.5f;
				sp.visible = false;

				// Follower
				auto& fol = world.Add<FollowerComponent>(e);
				fol.targetId = 0;
				fol.offset = {0.0f,4.0f};

				// UI状態
				world.Add<PlayerUIComponet>(e);
				return e;
			};
	}
}

inline void RegisterFollowerPrefab(PrefabRegistry& registry)
{
	// 旧API: 汎用フォロワー (ジャンプ用)
	Prefabs::FollowerConfig cfg;
	cfg.textureName = "ui_jump";
	cfg.width = 10.0f;
	cfg.height = 10.0f;
	registry.Register("Follower", Prefabs::MakeFollowerPrefab(cfg));
}
