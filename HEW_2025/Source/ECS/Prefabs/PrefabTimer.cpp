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
#include "ECS/Components/Render/DigitUIComponent.h"
#include "System/TimeAttackManager.h"

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
			
			// シングルトンをリセットして開始
			TimeAttackManager::Get().Reset();
			TimeAttackManager::Get().StartRun(); // 計測開始

			// 生成する桁の定義リスト
			DigitUIComponent::Type types[] = {
				DigitUIComponent::Type::MinTens,
				DigitUIComponent::Type::MinOnes,
				DigitUIComponent::Type::Separator,
				DigitUIComponent::Type::SecTens,
				DigitUIComponent::Type::SecOnes
			};

			for(int i = 0; i < 5; ++i)
			{
				EntityId digitEnt = w.Create();
				
				// 初期位置
				w.Add<TransformComponent>(digitEnt,
					sp.position, sp.rotationDeg, sp.scale);

				auto& spr = w.Add<Sprite2DComponent>(digitEnt);
				spr.alias = "tex_number";
				spr.visible = true;
				spr.layer = 103;
				spr.width = 5.0f;
				spr.height = 5.0f;
				
				auto& dig = w.Add<DigitUIComponent>(digitEnt);
				dig.type = types[i]; // 役割を設定
			}
			return masterEntity;
		}
	);
}
