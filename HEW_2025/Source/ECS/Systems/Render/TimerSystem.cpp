/*****************************************************************//**
 * @file   TimerSystem.cpp 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/25
 *********************************************************************/
#include "ECS/Systems/Render/TimerSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/TimerComponent.h"
#include "ECS/Components/Render/DigitUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include <cmath>

static const int ANIM_SCORE_SPLITE_X = 5; // 横分割数
static const int ANIM_SCORE_SPLITE_Y = 5; // 縦分割数

// UIの配置設定(カメラからの相対距離)
static const float UI_OFFSET_Y = 15.0f;
static const float UI_OFFSET_Z = 185.0f;
static const float DIGIT_STEP_X = 4.0f;

void TimerSystem::Update(World& world, float dt)
{
	// 1.時間を進める
	int currentScoreInt = 0;

	world.View<TimerComponent>([&](EntityId, TimerComponent& timer)
		{
			if (timer.isRunning)
			{
				timer.currentTime += dt;
				// 必要なら上限処理
				if (timer.currentTime > 999.0f) timer.currentTime = 999.0f;
			}
			currentScoreInt = static_cast<int>(timer.currentTime);
		});

	// 追加：カメラの位置を取得する
	DirectX::XMFLOAT3 camPos = { 0.0f, 0.0f, 0.0f };
	bool foundCamera = false;

	world.View<ActiveCameraTag, TransformComponent>(
		[&](EntityId, const ActiveCameraTag&, const TransformComponent& tr)
		{
			camPos = tr.position;
			foundCamera = true;
		}
	);

	// 2.各桁のUVを更新
	float sizeTexX = 1.0f / (float)ANIM_SCORE_SPLITE_X;
	float sizeTexY = 1.0f / (float)ANIM_SCORE_SPLITE_Y;

	world.View<DigitUIComponent, Sprite2DComponent, TransformComponent>(
		[&](EntityId, const DigitUIComponent& digit, Sprite2DComponent& sprite, TransformComponent&tr)
		{
			// この桁に表示すべき数字を計算
			int number = (currentScoreInt / digit.digitPlace) % 10;

			// UV計算
			int col = number % ANIM_SCORE_SPLITE_X;
			int row = number / ANIM_SCORE_SPLITE_X;

			sprite.uvOffset.x = sizeTexX * col;
			sprite.uvOffset.y = sizeTexY * row;
			sprite.uvSize.x = sizeTexX;
			sprite.uvSize.y = sizeTexY;

			// 座標追従
			if (foundCamera)
			{
				// 桁ごとのXズレを計算
				float xOffset = 0.0f;
				if (digit.digitPlace == 100) xOffset = -DIGIT_STEP_X;
				else if (digit.digitPlace == 10) xOffset = 0.0f;
				else if (digit.digitPlace == 1) xOffset = DIGIT_STEP_X;
				
				// カメラ位置を基準に配置
				tr.position.x = camPos.x + xOffset;
				tr.position.y = camPos.y + UI_OFFSET_Y;

				tr.position.z = camPos.z + UI_OFFSET_Z;
				
			}
		}
	);
}
