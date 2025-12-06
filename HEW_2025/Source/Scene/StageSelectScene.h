/**********************************************************************************************
 * @file      StageSelectScene.h
 * @brief     ステージセレクトヘッダ
 * 
 * @author    堤翔
 * @author	  奥田修也
 * @author    浅野勇生 (追記)
 * @date      2025/11/28
 * @author    川谷優真 (追記)
 * @date      2025/12/02
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/28 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once

#include "Scene.h"
#include "Scene/SceneAPI.h"
#include "System/Input.h"
#include "System/GameCommon.h" // Difficulty

 // ECS関連
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"
#include "ECS/Prefabs/PrefabRegistry.h"

#include <vector>
#include <DirectXMath.h>

class StageSelectScene : public Scene
{
public:
    StageSelectScene();
    ~StageSelectScene() override;

    void Update() override;
    void Draw() override;

private:

	void Initialize(); // 初期化処理:川谷(追記)

    // 画面の状態
    enum class State 
    {
        SelectStage,      // ステージを選んでいる
        SelectDifficulty  // 難易度を選んでいる
    };

    void UpdateInput();   // 入力処理
    void UpdateUI();      // UIの見た目更新
	void InitStages();    // ステージ環境(床・塔)の初期位置:川谷(追記)

	void UpdatePlayerMove(); // プレイヤーの位置を更新するヘルパー関数:川谷(追記)

    // ECS
    World m_world;
    SystemRegistry m_sys;
	PrefabRegistry m_prefabs; // プレハブ管理:川谷(追記)

    // システムへの参照
    SpriteRenderSystem* m_drawSprite = nullptr;
	ModelRenderSystem* m_drawModel = nullptr; // 3Dモデル描画用:川谷(追記)
    FollowCameraSystem* m_followCamera = nullptr;

    // 選択状態
    State m_state = State::SelectStage;
    int m_currentStage = 1;         // 現在選んでいるステージ (1~)
    int m_maxStages = 3;            // ステージ数（仮）
    Difficulty m_currentDiff = Difficulty::Normal;

    // UI用エンティティID
    EntityId m_entStageIcon = 0;
    EntityId m_entDiffIcon = 0;
    EntityId m_entGuide = 0;

    // ステージ座標管理
	EntityId m_playerEntity = 0;  // プレイヤーエンティティID:川谷(追記)
	std::vector<DirectX::XMFLOAT3>m_stagePoints; // 各ステージの中心座標:川谷(追記)
};