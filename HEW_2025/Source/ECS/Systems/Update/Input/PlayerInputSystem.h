/*********************************************************************/
/* @file   PlayerInputSystem.h
 * @brief  1P/2Pなどプレイヤーごとの入力をECSに書き込むSystem
 *
 * MovementIntentComponent に「こう動きたい」を書くだけで、
 * 実際の移動・ジャンプは別のSystemが担当できるようにします。
 *
 * 将来的に操作方法を変えたいときは、このSystem内のマッピングだけ書き換えればOK。
 *
 * 依存：
 *  - System/Input.h （実際のキー状態を取るあなたのプロジェクトのやつ）
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include "../../IUpdateSystem.h"
#include "ECS/World.h"

#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Effect/EffectComponent.h"

class PlayerInputSystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;

private:
    // プレイヤー番号ごとの入力取得を関数に分けておくと読みやすい
    void ReadPlayer0(MovementIntentComponent& outIntent);
    void ReadPlayer1(MovementIntentComponent& outIntent);
};
