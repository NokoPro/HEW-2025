/**********************************************************************************************
 * @file      PlayerLocomotionStateSystem.h
 * @brief     プレイヤーの移動状態管理システム
 *
 * @author    浅野勇生
 * @date      2025/12/2
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/2 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once
#include "ECS/World.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Core/PlayerStateComponent.h"
#include "ECS/Systems/IUpdateSystem.h"

class PlayerLocomotionStateSystem : public IUpdateSystem
{
public:
    void Update(World& world, float deltaTime);
};
