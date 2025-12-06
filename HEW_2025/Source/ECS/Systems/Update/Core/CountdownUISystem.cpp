#include "CountdownUISystem.h"
#include "ECS/Components/Render/CountdownUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "System/TimeAttackManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    inline void SetVisible(World& world, EntityId e, bool v)
    {
        if (e != kInvalidEntity && world.Has<Sprite2DComponent>(e))
        {
            auto& sprite = world.Get<Sprite2DComponent>(e);
            sprite.visible = v;
        }
    }

    inline void ResetDigitAnimTime(CountdownUIComponent& ui, EntityId e)
    {
        if (e == ui.sprite3) ui.digit3AnimTime = 0.0f;
        else if (e == ui.sprite2) ui.digit2AnimTime = 0.0f;
        else if (e == ui.sprite1) ui.digit1AnimTime = 0.0f;
    }

    inline float& GetDigitAnimTime(CountdownUIComponent& ui, EntityId e)
    {
        if (e == ui.sprite3) return ui.digit3AnimTime;
        if (e == ui.sprite2) return ui.digit2AnimTime;
        return ui.digit1AnimTime;
    }

    inline void ApplyPopScaleToSprite(World& world, EntityId e, float baseW, float baseH, float from, float to, float dur, float& animTime, float dt)
    {
        if (e == kInvalidEntity || !world.Has<Sprite2DComponent>(e)) return;
        auto& sp = world.Get<Sprite2DComponent>(e);
        const float alpha = std::min(1.0f, animTime / std::max(0.0001f, dur));
        const float s = from + (to - from) * alpha;
        sp.width  = baseW * s;
        sp.height = baseH * s;
        animTime += dt;
    }

    inline void ApplyRotateZToTransform_DigitPhased(World& world, EntityId e, float speedDegPerSec, float dt)
    {
        if (e == kInvalidEntity || !world.Has<TransformComponent>(e)) return;
        auto& tr = world.Get<TransformComponent>(e);
        // 0.5•b‰ñ“] ¨ 0.5•b’â~ ‚Ì1•büŠú
        const float period = 1.0f;
        const float active = 0.5f;
        tr.rotationDeg.z += (speedDegPerSec * dt);
        // ’â~ƒtƒF[ƒY‚Í‰ñ“]—Ê‚ğ˜‚¦’u‚­‚½‚ßA’´‰ß•ª‚ğ·‚µˆø‚­
        // À‘•ŠÈ—ª‰»‚Ì‚½‚ßA’â~‚Í‘¬“x0ˆµ‚¢‚É‚µAŒ©‚©‚¯ã‚Ì‰ñ“]‚ğs‚í‚È‚¢
        // ‰ñ“]©‘Ì‚Í Transform ‚Ì’l‚É’~Ï‚³‚ê‚é‚½‚ßAƒŒƒ“ƒ_‚Å³‚µ‚­‰ñ“]
    }
}

void CountdownUISystem::Update(World& world, float dt)
{
    const auto state = TimeAttackManager::Get().GetState();

    world.View<CountdownUIComponent>(
        [&](EntityId e, CountdownUIComponent& ui)
        {
            if (ui.startShowTime > 0.0f)
            {
                ui.startShowTime -= dt;
                if (ui.startShowTime < 0.0f)
                {
                    ui.startShowTime = 0.0f;
                }
            }

            SetVisible(world, ui.sprite3, false);
            SetVisible(world, ui.sprite2, false);
            SetVisible(world, ui.sprite1, false);
            SetVisible(world, ui.spriteStart, false);

            if (state == TimeAttackManager::State::Countdown)
            {
                ui.startTriggered = false;
                const float remain = TimeAttackManager::Get().GetCountdownRemaining();
                const int sec = (std::max)(0, static_cast<int>(std::ceil(remain)));

                auto showDigit = [&](EntityId digitE, float& animTime)
                {
                    SetVisible(world, digitE, true);
                    ApplyPopScaleToSprite(world, digitE, ui.digitBaseWidth, ui.digitBaseHeight,
                        ui.digitPopScaleFrom, ui.digitPopScaleTo, ui.digitPopDuration,
                        animTime, dt);
                    // ‰ñ“]ƒtƒF[ƒYi0.5•bj‚¾‚¯‰ñ‚·
                    const float tPhase = std::fmod(animTime, 1.0f);
                    if (tPhase <= 0.5f)
                    {
                        if (digitE != kInvalidEntity && world.Has<TransformComponent>(digitE))
                        {
                            auto& tr = world.Get<TransformComponent>(digitE);
                            tr.rotationDeg.z += ui.digitSpinSpeedDeg * dt;
                            if (tr.rotationDeg.z >= 360.0f) tr.rotationDeg.z -= 360.0f;
                        }
                    }
                };

                if (sec >= 3)
                {
                    if (ui.digit3AnimTime == 0.0f) ResetDigitAnimTime(ui, ui.sprite3);
                    showDigit(ui.sprite3, ui.digit3AnimTime);
                }
                else if (sec == 2)
                {
                    if (ui.digit2AnimTime == 0.0f) ResetDigitAnimTime(ui, ui.sprite2);
                    showDigit(ui.sprite2, ui.digit2AnimTime);
                }
                else if (sec == 1)
                {
                    if (ui.digit1AnimTime == 0.0f) ResetDigitAnimTime(ui, ui.sprite1);
                    showDigit(ui.sprite1, ui.digit1AnimTime);
                }
                else
                {
                    ui.digit3AnimTime = 0.0f;
                    ui.digit2AnimTime = 0.0f;
                    ui.digit1AnimTime = 0.0f;
                }
            }
            else if (state == TimeAttackManager::State::Running)
            {
                if (!ui.startTriggered)
                {
                    ui.startTriggered = true;
                    ui.startShowTime = 1.0f;
                    ui.currentStartAnimTime = 0.0f;
                }

                if (ui.startShowTime > 0.0f)
                {
                    SetVisible(world, ui.spriteStart, true);
                    ApplyPopScaleToSprite(world, ui.spriteStart, ui.startBaseWidth, ui.startBaseHeight,
                        ui.startPopScaleFrom, ui.startPopScaleTo, ui.startPopDuration,
                        ui.currentStartAnimTime, dt);
                }
                else
                {
                    ui.currentStartAnimTime = 0.0f;
                }
            }
        });
}
