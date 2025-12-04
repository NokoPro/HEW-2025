/**********************************************************************************************
 * @file      TestStageScene.h
 * @brief     ステージ読み込みを試してみるシーン
 *
 * @author    浅野勇生
 * @date      2025/11/22
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/22 ]
 * 
 *    - [◎] TestSceneから移行
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
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
//追加
#include"ECS/Systems/Update/Audio/AudioPlaySystem.h"

class GoalSystem;
class  DeathZoneSystem;

  /**
  * @class TestScene
  * @brief 開発中にECSの動作確認をするためのシーン
  */
class TestStageScene : public Scene
{
public:
    TestStageScene();
    ~TestStageScene() override;

    void Update() override;
    void Draw() override;

    // ImGui 等の外部デバッグ表示向け: World 参照取得
    World& GetWorld() { return m_world; }
    const World& GetWorld() const { return m_world; }

private:
    World          m_world;
    SystemRegistry m_sys;

    PrefabRegistry m_prefabs;

    // 描画システム（行列を渡す用）
    ModelRenderSystem* m_drawModel = nullptr;
    // Sprite 描画システム（2D UI 等）
    class SpriteRenderSystem* m_drawSprite = nullptr;

    // 背景 描画システム
    class BackGroundRenderSystem* m_drawBackGround = nullptr;

    // カメラシステム（行列をもらう用）
    FollowCameraSystem* m_followCamera = nullptr;

    CollisionDebugRenderSystem* m_debugCollision = nullptr;

    //ゴールシステム
    GoalSystem* m_goalSystem = nullptr;

    //デスシステム
    DeathZoneSystem* m_deathSystem = nullptr;

    // 入力・物理
    CollisionEventBuffer m_colBuf;  // 今フレームの当たりが入る

    // モデル資産
    AssetHandle<Model>   m_playerModel;
    AssetHandle<Model>   m_groundModel;

    // プレイヤーを覚えておくとカメラのターゲットに使える
    EntityId             m_playerEntity1 = kInvalidEntity;
    EntityId             m_playerEntity2 = kInvalidEntity;
    EntityId             m_goalEntity = kInvalidEntity;

};