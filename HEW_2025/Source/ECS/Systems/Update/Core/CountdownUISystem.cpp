#include "CountdownUISystem.h"
#include "ECS/Components/Render/CountdownUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "System/TimeAttackManager.h"

namespace
{
	inline void SetVisible(World& world, EntityId e, bool v)
	{
		if(e != kInvalidEntity && world.Has<Sprite2DComponent>(e))
		{
			auto& sprite = world.Get<Sprite2DComponent>(e);
			sprite.visible = v;
		}
	}
}

void CountdownUISystem::Update(World& world, float dt)
{
	const auto state = TimeAttackManager::Get().GetState();

	world.View<CountdownUIComponent>(
		[&](EntityId e, CountdownUIComponent& ui)
		{
			// STARTの残り表示時間更新
			if (ui.startShowTime > 0.0f)
			{
				ui.startShowTime -= dt;
				if(ui.startShowTime < 0.0f)
				{
					ui.startShowTime = 0.0f;
				}
			}

			// デフォルト不可視
			SetVisible(world, ui.sprite3, false);
			SetVisible(world, ui.sprite2, false);
			SetVisible(world, ui.sprite1, false);
			SetVisible(world, ui.spriteStart, false);

            // 状態に応じて表示切替
            if (state == TimeAttackManager::State::Countdown)
			{
                // 次にRunningへ遷移したときにSTARTを出すためにリセット
                ui.startTriggered = false;
				const float remain = TimeAttackManager::Get().GetCountdownRemaining();
				const int sec = (std::max)(0, static_cast<int>(std::ceil(remain)));
				if (sec >= 3)      SetVisible(world, ui.sprite3, true);
				else if (sec == 2) SetVisible(world, ui.sprite2, true);
				else if (sec == 1) SetVisible(world, ui.sprite1, true);
			}
			else if (state == TimeAttackManager::State::Running)
			{
                // Countdown -> Running 遷移直後に一度だけタイマーをセット
                if (!ui.startTriggered)
                {
                    ui.startTriggered = true;
                    // 表示したい秒数（必要に応じて調整）
                    ui.startShowTime = 1.0f;
                }

                if (ui.startShowTime > 0.0f)
				{
					SetVisible(world, ui.spriteStart, true);
					// フェードしたい場合は color.a を減衰させる処理を追加可能
				}
			}
		});
}
