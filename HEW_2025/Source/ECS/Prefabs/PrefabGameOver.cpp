/*********************************************************************/
/* @file   PrefabGameOver.cpp
 * @brief  ゲームオーバープレハブ実装
 *
 * @author 奥田修也
 * @date   2025/12/2
 *********************************************************************/
#include "PrefabGameOver.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/DigitUIComponent.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Render/BackGroundRenderSystem.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Update/Game/DeathZoneSystem.h"
#include "ECS/Tag/Tag.h"

void RegisterGameOverPrefab(PrefabRegistry& registry)
{
	registry.Register("GameOver", [](World& w, const PrefabRegistry::SpawnParams& sp)->EntityId
		{
			
			// ゲームオーバースプライトの生成
			EntityId gameOverEnt = w.Create();
			w.Add<TransformComponent>(gameOverEnt,
				sp.position, sp.rotationDeg, sp.scale);
			
			// SpriteGameOver
			auto& spgo = w.Add<Sprite2DComponent>(gameOverEnt);
			spgo.alias = "tex_gameover";
			spgo.visible = false; // 最初は非表示
			spgo.layer = 102; // UIより前面に表示
			spgo.width = 64.0f; // 幅
			spgo.height = 34.0f; // 高さ
			spgo.originX = 0.5f; // 原点X
			spgo.originY = 0.5f; // 原点Y
			spgo.color.w = 1.0f; // 色
			
			
			w.Add<GameOverMenu>(gameOverEnt); // ゲームオーバーメニュータグ追加
			
			
			return gameOverEnt;
		}

	);

}