/**********************************************************************************************
 * @file      PlayerOffscreenIndicatorFaceSystem.cpp
 * @brief     画面下端インジケータ上の HI / GO / アイコン切り替え System 実装
 *
 * 自分が相手を動かしたフレームにのみ UI を出すように、対象ではなく「相手プレイヤー」の
 * リモートコマンドフラグを参照して HI/GO を点灯します。
 **********************************************************************************************/
#include "PlayerOffscreenIndicatorFaceSystem.h"
#include "ECS/Components/UI/OffscreenIndicatorComponent.h"
#include "ECS/Components/UI/OffscreenIndicatorFaceComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/PlayerStateComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include <algorithm>

namespace {
    // 2人前提で、対象でない方を相手とみなす
    EntityId FindOpponent(World& world, EntityId self)
    {
        EntityId opp = kInvalidEntity;
        world.View<PlayerInputComponent>(
            [&](EntityId e, PlayerInputComponent&)
            {
                if (e != self) opp = e;
            }
        );
        return opp;
    }
}

void PlayerOffscreenIndicatorFaceSystem::Update(World& world, float dt)
{
    world.View<OffscreenIndicatorFaceComponent,
        OffscreenIndicatorComponent,
        Sprite2DComponent>(
            [&](EntityId,
                OffscreenIndicatorFaceComponent& face,
                OffscreenIndicatorComponent& ind,
                Sprite2DComponent& sprite)
            {
                // 可視状態はベースのインジケータに合わせる
                sprite.visible = ind.visible;
                if (!sprite.visible)
                {
                    if (face.hiTimer > 0.0f) face.hiTimer = std::max(0.0f, face.hiTimer - dt);
                    if (face.goTimer > 0.0f) face.goTimer = std::max(0.0f, face.goTimer - dt);
                    return;
                }

                // --- タイマー減算 ---
                if (face.hiTimer > 0.0f)
                {
                    face.hiTimer -= dt;
                    if (face.hiTimer < 0.0f) face.hiTimer = 0.0f;
                }
                if (face.goTimer > 0.0f)
                {
                    face.goTimer -= dt;
                    if (face.goTimer < 0.0f) face.goTimer = 0.0f;
                }

                // --- 自分が相手を動かしたフレームか？相手側のフラグを見る ---
                const EntityId opponentId = FindOpponent(world, ind.targetId);
                if (opponentId != kInvalidEntity && world.Has<PlayerStateComponent>(opponentId))
                {
                    auto& oppState = world.Get<PlayerStateComponent>(opponentId);

                    // 相手にブリンクをさせた → GO（優先度高）
                    if (oppState.m_remoteCommandBlinkThisFrame)
                    {
                        face.goTimer = face.goDuration;
                    }

                    // 相手にジャンプをさせた → HI
                    if (oppState.m_remoteCommandJumpThisFrame)
                    {
                        face.hiTimer = face.hiDuration;
                    }
                }

                // --- 表示モードを決定（GO > HI > ICON） ---
                if (face.goTimer > 0.0f && !face.aliasGo.empty())
                {
                    face.currentMode = OffscreenIndicatorFaceComponent::Mode::Go;
                    sprite.alias = face.aliasGo;
                }
                else if (face.hiTimer > 0.0f && !face.aliasHi.empty())
                {
                    face.currentMode = OffscreenIndicatorFaceComponent::Mode::Hi;
                    sprite.alias = face.aliasHi;
                }
                else
                {
                    face.currentMode = OffscreenIndicatorFaceComponent::Mode::Icon;
                    if (!face.aliasIcon.empty()) sprite.alias = face.aliasIcon;
                }
            }
        );
}
