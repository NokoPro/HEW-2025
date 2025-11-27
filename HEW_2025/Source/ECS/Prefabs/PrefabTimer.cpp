/*****************************************************************//**
 * @file   PrefabTimer.cpp 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/25
 *********************************************************************/
#include "ECS/Prefabs/PrefabTimer.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/TimerComponent.h"
#include "ECS/Components/Render/DigitUIComponent.h"

void RegisterTimerPrefab(PrefabRegistry& registry)
{
	registry.Register("Timer", [](World& w, const PrefabRegistry::SpawnParams& sp)->EntityId
		{
			// 親エンティティ
			EntityId masterEntity = w.Create();
			if(masterEntity == kInvalidEntity)
			{
				return kInvalidEntity;
			}
			
			auto& timer = w.Add<TimerComponent>(masterEntity);
			timer.currentTime = 0.0f; // 0秒スタート
			timer.isRunning = true;

			// 3桁の数字を生成
			int places[] = { 100, 10, 1 };
			float stepX = 4.0f;

			for(int i = 0; i < 3; ++i)
			{
				EntityId digitEnt = w.Create();
				
				float offsetX = (i * stepX) - stepX;

				float x = sp.position.x + offsetX;
				float y = sp.position.y;
				float z = sp.position.z;

				w.Add<TransformComponent>(digitEnt,
					DirectX::XMFLOAT3(x, y, z),
					sp.rotationDeg,
					sp.scale);

				auto& spr = w.Add<Sprite2DComponent>(digitEnt);
				spr.alias = "tex_number";
				spr.visible = true;
				spr.layer = 101;

				spr.width = 5.0f;
				spr.height = 5.0f;

				auto& dig = w.Add<DigitUIComponent>(digitEnt);
				dig.digitPlace = places[i];
			}
			return masterEntity;
		}
	);
}
