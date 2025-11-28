/**********************************************************************************************
 * @file      StageSelectScene.h
 * @brief     ステージセレクトヘッダ
 * 
 * @author    堤翔
 * @author	  奥田修也
 * @author    浅野勇生 (追記)
 * @date      2025/11/28
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
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

class StageSelectScene : public Scene
{
public:
    StageSelectScene();
    ~StageSelectScene() override;

    void Update() override;
    void Draw() override;

private:
    // 画面の状態
    enum class State 
    {
        SelectStage,      // ステージを選んでいる
        SelectDifficulty  // 難易度を選んでいる
    };

    void UpdateInput();   // 入力処理
    void UpdateUI();      // UIの見た目更新

    // ECS
    World m_world;
    SystemRegistry m_sys;

    // システムへの参照
    SpriteRenderSystem* m_drawSprite = nullptr;
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
};