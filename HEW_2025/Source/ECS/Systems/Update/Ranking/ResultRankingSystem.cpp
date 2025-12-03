#include "ECS/Systems/Update/Ranking/ResultRankingSystem.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Ranking/RankingDigitUIComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "System/RankingManager.h"

#include <DirectXMath.h>
#include <algorithm>

// 数字テクスチャの分割
static const int ANIM_SCORE_SPLIT_X = 5;
static const int ANIM_SCORE_SPLIT_Y = 5;

// 配置
static const float BASE_OFFSET_Y	= 8.0f;
static const float ROW_STEP_Y		= -5.0f;
static const float BASE_OFFSET_Z	= 20.0f;
static const float DIGIT_STEP_X		= 4.0f;

static void ComputeDigits(float seconds,int& min10,int& min1,int& sec10,int& sec1)
{
	seconds = std::max(0.0f, std::min(seconds,5999.0f));
	int totalSec = static_cast<int>(seconds);
	int min = totalSec / 60;
	int sec = totalSec % 60;

	min10 = (min / 10) % 10;
	min1 = min % 10;
	sec10 = (sec / 10) % 10;
	sec1 = sec % 10;
}

void ResultRankingSystem::Update(World& world, float /*dt*/)
{
	// カメラ位置
	DirectX::XMFLOAT3 camPos = { 0.0f,0.0f,0.0f };
	bool foundCam = false;
	world.View<ActiveCameraTag, TransformComponent>(
		[&](EntityId /*e*/,const ActiveCameraTag& /*tag*/, const TransformComponent& tr)
		{
			camPos = tr.position;
			foundCam = true;
		}
	);

	// 上位6件取得
	const auto times = RankingManager::Get().GetTop(6);

	const float sizeTexX = 1.0f / static_cast<float>(ANIM_SCORE_SPLIT_X);
	const float sizeTexY = 1.0f / static_cast<float>(ANIM_SCORE_SPLIT_Y);

	world.View<RankingDigitUIComponent, Sprite2DComponent, TransformComponent>(
		[&](EntityId /*e*/, const RankingDigitUIComponent& digit, Sprite2DComponent& sprite, TransformComponent& tr)
		{
			const int row = std::max(0, std::min(digit.row,5));
			const bool hasRow = (row < static_cast<int>(times.size()));

			// 行が存在しない場合は非表示
			if (!hasRow)
			{
				sprite.visible = false;
				return;
			}

			// 行のタイム -> 桁ごとの数字に分解
			int min10 = 0;
			int min1 = 0;
			int sec10 = 0;
			int sec1 = 0;

			ComputeDigits(times[row], min10, min1, sec10, sec1);

			// 桁のIndex計算
			int numberIndex = 0;
			float xUnit = 0.0f;
			switch (digit.type)
			{
			case RankingDigitUIComponent::Type::MinTens:
				numberIndex = min10;
				xUnit = -2.0f;
				break;
			case RankingDigitUIComponent::Type::MinOnes:
				numberIndex = min1;
				xUnit = -1.0f;
				break;
			case RankingDigitUIComponent::Type::Separator:
				numberIndex = 10; // コロン
				xUnit = 0.0f;
				break;
			case RankingDigitUIComponent::Type::SecTens:
				numberIndex = sec10;
				xUnit = 1.0f;
				break;
			case RankingDigitUIComponent::Type::SecOnes:
				numberIndex = sec1;
				xUnit = 2.0f;
				break;
			}

			// UV設定
			const int col = numberIndex % ANIM_SCORE_SPLIT_X;
			const int rowIdx = numberIndex / ANIM_SCORE_SPLIT_X;
			sprite.uvOffset.x = static_cast<float>(col) * sizeTexX;
			sprite.uvOffset.y = static_cast<float>(rowIdx) * sizeTexY;
			sprite.uvSize.x = sizeTexX;
			sprite.uvSize.y = sizeTexY;
			sprite.visible = true;

			// 配置
			if(foundCam)
			{
				const float x = camPos.x + (xUnit * DIGIT_STEP_X);
				const float y = camPos.y + BASE_OFFSET_Y + (ROW_STEP_Y * static_cast<float>(row));
				const float z = camPos.z + BASE_OFFSET_Z;
				tr.position = { x,y,z };
			}
		}
	);
}
