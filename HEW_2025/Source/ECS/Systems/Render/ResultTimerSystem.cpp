/*****************************************************************//**
 * @file   TimerSystem.cpp
 * @brief  タイマーの表示更新とカメラ追従を行うシステム
 * * - TimeAttackManagerから時間を取得して表示内容(UV)を更新
  * - カメラの位置を取得し、常に画面貞一に表示されるよう座標を更新
 * @author 川谷優真
 * @date   2025/11/25
 *********************************************************************/
#include "ECS/Systems/Render/TimerSystem.h"
#include "ECS/World.h"

 // コンポーネント群
#include "ECS/Components/Render/DigitUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// 時間管理マネージャー
#include "System/TimeAttackManager.h"

#include <cmath>

// 画面分割設定
static const int ANIM_SCORE_SPLITE_X = 5; // 横分割数
static const int ANIM_SCORE_SPLITE_Y = 5; // 縦分割数

// UIの配置設定(カメラからの相対距離)
static const float UI_OFFSET_Y = 15.0f;
static const float UI_OFFSET_Z = 185.0f;
static const float DIGIT_STEP_X = 4.0f;

void TimerSystem::Update(World& world, float dt)
{
	TimeAttackManager::Get().Update();

	float totalTime = TimeAttackManager::Get().GetElapsed();

	// カンスト処理
	if (totalTime > 5999.0f) totalTime = 5999.0f;

	// 分・秒に分解
	int totalSec = static_cast<int>(totalTime);
	int min = totalSec / 60;
	int sec = totalSec % 60;

	int min10 = (min / 10) % 10;
	int min1 = min % 10;
	int sec10 = (sec / 10) % 10;
	int sec1 = sec % 10;

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
		[&](EntityId, const DigitUIComponent& digit, Sprite2DComponent& sprite, TransformComponent& tr)
		{
			//-----表示する数字を決定-----
			int numberIndex = 0;
			float xOffsetUnit = 0.0f; // 中央からのズレ位置

			switch (digit.type)
			{
			case DigitUIComponent::Type::MinTens:
				numberIndex = min10;
				xOffsetUnit = -2.0f; // 一番左
				break;
			case DigitUIComponent::Type::MinOnes:
				numberIndex = min1;
				xOffsetUnit = -1.0f;
				break;
			case DigitUIComponent::Type::Separator:
				numberIndex = 10; // セパレーターのインデックス
				xOffsetUnit = 0.0f; // 真ん中
				break;
			case DigitUIComponent::Type::SecTens:
				numberIndex = sec10;
				xOffsetUnit = 1.0f;
				break;
			case DigitUIComponent::Type::SecOnes:
				numberIndex = sec1;
				xOffsetUnit = 2.0f; // 一番右
				break;
			}

			//-----UV計算-----
			int col = numberIndex % ANIM_SCORE_SPLITE_X;
			int row = numberIndex / ANIM_SCORE_SPLITE_X;

			sprite.uvOffset.x = sizeTexX * static_cast<int>(col);
			sprite.uvOffset.y = sizeTexY * static_cast<int>(row);
			sprite.uvSize.x = sizeTexX;
			sprite.uvSize.y = sizeTexY;

			//-----座標追従-----
			if (foundCamera)
			{
				float xOffset = xOffsetUnit * DIGIT_STEP_X;

				tr.position.x = camPos.x + xOffset;
				tr.position.y = camPos.y + UI_OFFSET_Y;
				tr.position.z = camPos.z + UI_OFFSET_Z;
			}
		}
	);
}
