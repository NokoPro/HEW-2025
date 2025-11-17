/*****************************************************************//**
 * @file   FollowerSystem.cpp 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/17
 *********************************************************************/
#include "FollowerSystem.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Tag/Tag.h"
#include <unordered_map> // 座標キャッシュ用

void FollowerSystem::Update(World& world, float dt)
{
	//-----1.追従対象の位置をID付きで収集-----
	// 毎フレーム一時的なマップを作る
	std::unordered_map<EntityId, DirectX::XMFLOAT3> targetPositions;

	world.View<TagPlayer, TransformComponent>(
		[&](EntityId e, const TagPlayer&, const TransformComponent& tr)
		{
			targetPositions[e] = tr.position;
		}
	);

	//-----2.フォロワーの位置を更新-----
	world.View<FollowerComponent, TransformComponent>(
		[&](EntityId e, const FollowerComponent& follower, TransformComponent& tr)
		{
			// 自分の追うべき相手(targetId)が、さっき収集したリストにいるか確認
			auto it = targetPositions.find(follower.targetId);
			if (it != targetPositions.end())
			{
				// 見つかったら、その座標(it->second)をコピー
				// Z軸は維持し、XYのみ追従
				const auto& targetPos = it->second;
				tr.position.x = targetPos.x;
				tr.position.y = targetPos.y + 3;
			}
		}
	);
}
