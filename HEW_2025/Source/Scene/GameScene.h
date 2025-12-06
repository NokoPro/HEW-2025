/*****************************************************************//**
 * @file   GameScene.h
 * @brief  本番ゲームプレイ用シーン
 * 難易度とステージ番号を受け取り、適切なJSONとパラメータで初期化する
 *
 * @author 浅野勇生
 * @date   2025/11/28
 *********************************************************************/
#pragma once

#include "Scene.h"
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"
#include "System/GameCommon.h" // Difficulty定義

 // コア
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// 表示
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"

// カメラ
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

// 入力～物理系
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
#include "ECS/Systems/Update/Audio/AudioPlaySystem.h"
#include "System/TimeAttackManager.h"

// 前方宣言
class GoalSystem;
class DeathZoneSystem;
class SpriteRenderSystem;
class BackGroundRenderSystem;

/**
 * @class GameScene
 * @brief メインゲーム画面のシーンクラス
 */
class GameScene : public Scene
{
public:
    /**
     * @brief コンストラクタ
     * @param stageNo 読み込むステージ番号
     * @param difficulty 難易度設定
     */
    GameScene(int stageNo, Difficulty difficulty);
    ~GameScene() override;

    void Update() override;
    void Draw() override;

    int GetStageNo() const { return m_stageNo; }
    Difficulty GetDifficulty() const { return m_difficulty; }

    World& GetWorld() { return m_world; }
    const World& GetWorld() const { return m_world; }

    // チート用インターフェイス
    void ForceClearCheat();
    void ForceGameOverCheat();

private:
    /**
     * @brief 初期化処理（コンストラクタから呼ばれる）
     */
    void Initialize();

    // メンバ変数
    int        m_stageNo;
    Difficulty m_difficulty;

    World          m_world;
    SystemRegistry m_sys;
    PrefabRegistry m_prefabs;

    // 描画システム
    ModelRenderSystem* m_drawModel = nullptr;
    SpriteRenderSystem* m_drawSprite = nullptr;
    BackGroundRenderSystem* m_drawBackGround = nullptr;
    CollisionDebugRenderSystem* m_debugCollision = nullptr;

    // 更新システム
    FollowCameraSystem* m_followCamera = nullptr;
    GoalSystem* m_goalSystem = nullptr;
    DeathZoneSystem* m_deathSystem = nullptr;

    // 入力・物理バッファ
    CollisionEventBuffer m_colBuf;

    // モデル資産
    AssetHandle<Model>   m_playerModel;
    AssetHandle<Model>   m_groundModel;

	TimeAttackManager::State m_prevState = TimeAttackManager::State::Ready;

    // エンティティID管理
    EntityId             m_playerEntity1 = kInvalidEntity;
    EntityId             m_playerEntity2 = kInvalidEntity;
    EntityId             m_goalEntity = kInvalidEntity;
};