// BackGroundScrollSystem.cpp

#include "BackGroundScrollSystem.h"

void BackGroundScrollSystem::Update(World& world, float dt)
{
	// カメラの位置Y
	float camY = 0.0f;	
	// カメラの取得
	world.View<TransformComponent, Camera3DComponent>(
		[&](EntityId e, TransformComponent& tr, Camera3DComponent& cam)
		{
			camY = tr.position.y;	// カメラの高さ
		}
	);

	// ワープの処理
	world.View<TransformComponent,BackGroundComponent,BackGroundScrollComponent>(
		[&](EntityId e, TransformComponent& tr,BackGroundComponent& bg,
			BackGroundScrollComponent& warp)
		{
			
			float h = bg.height * tr.scale.y;	// 背景の高さ

			//背景の位置がカメラから見て見切れるラインにあったら
			//ワープ
			if ( tr.position.y > camY + h)
			{
				warp.UpWarp = true;
			}
 
			if (tr.position.y < camY - h)
			{
				warp.DownWarp = true;
			}
		
			// カメラワープ上
			if (warp.UpWarp)
			{
				// 上に背景の位置をずらす
				tr.position.y -= h * 2.0f;	//画像2枚分ずらすので * 2.0f
				warp.UpWarp = false;
			}

			// カメラワープ下
			if (warp.DownWarp)
			{
				// 下に背景の位置をずらす
				tr.position.y += h * 2.0f;
				warp.DownWarp = false;
			}
		});

}
