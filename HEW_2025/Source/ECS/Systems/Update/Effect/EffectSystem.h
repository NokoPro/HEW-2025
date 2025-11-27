/*********************************************************************/
/* @file   EffectSystem.h
 * @brief  エフェクト再生制御システム定義
 *
 * - EffectComponent と TransformComponent を見て、
 *   エフェクトの再生開始・位置追従・終了監視を行う。
 *
 * - 実際の描画は EffectRuntime::Render() でまとめて行う想定。
 *
 * @author 浅野勇生
 * @date   2025/11/27
 *********************************************************************/
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"
#include "System/EffectRuntime.h"

 // コンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Effect/EffectComponent.h"

class EffectSystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};
