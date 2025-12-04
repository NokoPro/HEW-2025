/**********************************************************************************************
 * @file      ResultScene.h
 * @brief     リザルトシーンの宣言。UI生成・更新、入力処理、描画を行う。
 * 
 * @authors   堤翔 / 奥田修也 / 篠原純 / 清水光之介
 * @date      2025/12/5
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/5 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once
#include "Scene/Scene.h"
#include "Scene/SceneAPI.h"       // シーン遷移API
#include "System/GameCommon.h"    // Difficulty
#include "System/Input.h"         // 入力

// ECS関連
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"
#include "ECS/Prefabs/PrefabRegistry.h"

class SpriteRenderSystem;

/**
 * @class ResultScene
 * @brief リザルトシーン。メニューUIの更新と入力処理、描画を担当。
 */
class ResultScene : public Scene
{
public:
    ResultScene();
    ~ResultScene() override;
    void Update() override;  ///< シーン更新（UI更新・入力処理）
    void Draw() override;    ///< シーン描画

private:
    void UpdateInput();  ///< 入力処理（選択・決定・キャンセル）
    void UpdateUI();     ///< UI見た目更新（選択状態に応じてサイズ変更）

    // ECS
    World m_world;
    SystemRegistry m_sys;
    PrefabRegistry m_prefabs;

    // 描画システム
    SpriteRenderSystem*    m_drawSprite   = nullptr;

    // 更新システム
    FollowCameraSystem* m_followCamera = nullptr;

    enum class MenuItem { Retry, StageSelect };
    MenuItem m_menuItem = MenuItem::Retry;

    // ボタンのエンティティID
    EntityId m_entSelectStageButton = 0;
    EntityId m_entRetryButton       = 0;
};
