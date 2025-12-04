/*********************************************************************/
/* @file   PrefabWhiteUI.cpp
 * @brief  背景をぼかすための白いやつ実装
 *
 * @author 奥田修也
 * @date   2025/12/4
 *********************************************************************/
#include "PrefabWhiteUI.h"
#include <ECS/Components/Physics/TransformComponent.h>
#include <ECS/Components/Render/Sprite2DComponent.h>
#include "ECS/World.h"
#include "ECS/Components/Render/DigitUIComponent.h"
#include <ECS/Systems/Render/SpriteRenderSystem.h>
#include <ECS/Systems/Render/BackGroundRenderSystem.h>
#include <ECS/Systems/Render/ModelRenderSystem.h>
#include "ECS/Systems/Update/Game/DeathZoneSystem.h"
#include "ECS/Tag/Tag.h"

void RegisteWhiteUIPrefab(PrefabRegistry& registry)
{
	registry.Register("WhiteUI", [](World& w, const PrefabRegistry::SpawnParams& sp)->EntityId
		{
			// エンティティ生成
			EntityId e = w.Create();
			if (e == kInvalidEntity)
			{
				return kInvalidEntity;
			}
			// Transform
			auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
			// 白い四角スプライトの生成
			auto& spw = w.Add<Sprite2DComponent>(e);
			spw.alias = "tex_white"; // 白いテクスチャ
			spw.visible = false; // 最初は表示
			spw.width = 76.8f;     //76.8f  ,64.0f
			spw.height = 86.4f;    //172.8f ,86.4f   64.0f
			spw.originX = 0.5f;
			spw.originY = 0.5f;
			spw.layer = 99; // UIの前面に表示
			spw.color = { 1.0f,1.0f,1.0f,0.5f }; // 半透明

			w.Add<GameOverMenu>(e); // ゲームオーバーメニュータグ追加

			
			return e;
		}
	);
}
