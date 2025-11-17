/*****************************************************************//**
 * @file   FollowerPrefab.h 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/17
 *********************************************************************/
#pragma once
#include "ECS/Prefabs/PrefabRegistry.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"

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

				// 1.位置(Transform)
				auto& tr = world.Add<TransformComponent>(e);
				tr.position = p.position;
				tr.rotationDeg = p.rotationDeg;
				tr.scale = p.scale;

				// 2.画像(Sprite2D)
				auto& sp = world.Add<Sprite2DComponent>(e);
				sp.alias = cfg.textureName;
				sp.width = cfg.width;
				sp.height = cfg.height;
				sp.layer = 5; // プレイヤーより少し手前に表示
				// 原点を足元にするなら(0.5, 1.0)など調整
				sp.originX = 0.5f;
				sp.originY = 0.5f;

				// 3.追従機能(Follower)
				// targetIdは精製後にSceneGameで設定するため、ここでは初期値
				auto& fol = world.Add<FollowerComponent>(e);
				fol.targetId = 0;

				return e;
			};
	}
}

inline void RegisterFollowerPrefab(PrefabRegistry& registry)
{
	Prefabs::FollowerConfig cfg;
	// テスト用に既存の画像を指定
	// 本番用の画像があれば指定する
	cfg.textureName = "ui_gauge_cursor";
	cfg.width = 40.0f;
	cfg.height = 40.0f;

	// "Follower"という名前で作成機能を登録
	registry.Register("Follower", Prefabs::MakeFollowerPrefab(cfg));
}
