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

 // コンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"

// Renderシステム
#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

 /**
  * @class TestScene
  * @brief 開発中にECSの動作確認をするためのシーン
  */
class TestScene : public Scene
{
public:
    TestScene();
    ~TestScene();

    void Update() override;
    void Draw() override;

private:
    World          m_world;
    SystemRegistry m_sys;

    // 描画システム（行列を渡す用）
    ModelRenderSystem* m_drawModel = nullptr;
    // カメラシステム（行列をもらう用）
    FollowCameraSystem* m_followCamera = nullptr;

    // モデル資産
    AssetHandle<Model>   m_playerModel;

    // プレイヤーを覚えておくとカメラのターゲットに使える
    EntityId             m_playerEntity = kInvalidEntity;
};