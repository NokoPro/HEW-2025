/*****************************************************************//**
 * @file   TestScene.h
 * @brief  ECSを試行するためのテスト用シーン
 *
 *        ここでWorldやSystemを好きに登録して、
 *        表示・更新の確認を行うことを想定しています。
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once

#include "Scene.h"
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"

 // コア
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// 表示
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"

// カメラ
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

// 入力～物理系（今回追加するやつ）
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

#include "ECS/Systems/Update/Input/PlayerInputSystem.h"
#include "ECS/Systems/Update/Physics/MovementApplySystem.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"
#include "ECS/Systems/Render/CollisionDebugRenderSystem.h"

#include "ECS/Prefabs/PrefabRegistry.h"
#include "ECS/Prefabs/PrefabPlayer.h"

 /**
  * @class TestScene
  * @brief 開発中にECSの動作確認をするためのシーン
  */
class TestScene : public Scene
{
public:
    TestScene();
    ~TestScene() override;

    void Update() override;
    void Draw() override;

private:
    World          m_world;
    SystemRegistry m_sys;

    PrefabRegistry m_prefabs;

    // 描画システム（行列を渡す用）
    ModelRenderSystem* m_drawModel = nullptr;
    // Sprite 描画システム（2D UI 等）
    class SpriteRenderSystem* m_drawSprite = nullptr;
    // カメラシステム（行列をもらう用）
    FollowCameraSystem* m_followCamera = nullptr;

    CollisionDebugRenderSystem* m_debugCollision = nullptr;

    // 入力・物理
    CollisionEventBuffer m_colBuf;  // 今フレームの当たりが入る

    // モデル資産
    AssetHandle<Model>   m_playerModel;
    AssetHandle<Model>   m_groundModel;

    // プレイヤーを覚えておくとカメラのターゲットに使える
    EntityId             m_playerEntity = kInvalidEntity;
    EntityId             m_playerEntity2 = kInvalidEntity;
    EntityId             m_goalEntity = kInvalidEntity;
};