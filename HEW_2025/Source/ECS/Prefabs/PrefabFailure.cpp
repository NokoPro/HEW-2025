/*********************************************************************/
/* @file   PrefabFailure.cpp
 * @brief  失敗プレハブ実装
 *
 * @author 奥田修也
 * @date   2025/12/2
 *********************************************************************/
//#include"PrefabGameOver.h"
//#include "ECS/World.h"
//#include "ECS/Components/Physics/TransformComponent.h"
//#include "ECS/Components/Render/Sprite2DComponent.h"
//#include "ECS/Components/Render/DigitUIComponent.h"
//#include <ECS/Systems/Render/SpriteRenderSystem.h>
//#include <ECS/Systems/Render/BackGroundRenderSystem.h>
//#include <ECS/Systems/Render/ModelRenderSystem.h>
//void RegisterGameOverPrefab(PrefabRegistry& registry)
//{
//	registry.Register("GameOver", [](World& w, const PrefabRegistry::SpawnParams& sp)->EntityId
//		{
//			// 親エンティティ
//			EntityId masterEntity = w.Create();
//			if (masterEntity == kInvalidEntity)
//			{
//				return kInvalidEntity;
//			}
//
//			// Transform
//			auto& tr = w.Add<TransformComponent>(masterEntity, sp.position, sp.rotationDeg, sp.scale);
//
//			// ゲームオーバースプライトの生成
//			EntityId gameOverEnt = w.Create();
//			w.Add<TransformComponent>(gameOverEnt,
//				sp.position, sp.rotationDeg, sp.scale);
//
//			// SpriteGameOver
//			auto& spgo = w.Add<Sprite2DComponent>(gameOverEnt);
//			spgo.alias = "tex_gameover";
//			spgo.visible = true;
//			spgo.layer = 102; // UIより前面に表示
//			spgo.width = 32.0f; // 幅
//			spgo.height = 20.8f; // 高さ
//
//			return masterEntity;
//		}
//	);
//}