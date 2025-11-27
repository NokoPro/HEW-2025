/*****************************************************************//**
 * @file   Game.cpp
 * @brief  SceneManagerを使ったゲーム全体の制御
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "Game.h"
#include "Scene/SceneManager.h"
#include "Scene/TestScene.h"
#include "Scene/TestStageScene.h"
#include "System/DebugSettings.h"
#include "System/TimeAttackManager.h"

//タイトルシーン
#include "Scene/TitleScene.h"

namespace
{
    SceneManager g_SceneManager; // ゲーム全体で 1 つ
}

// 他所からアクセスするためのアクセサ関数 (SceneAPI.h が利用)
SceneManager& GetSceneManager()
{
    return g_SceneManager;
}

bool Game_Init(HWND /*hWnd*/, unsigned int /*width*/, unsigned int /*height*/)
{
    // 最初にテストステージへ遷移
    g_SceneManager.Change<TitleScene>();
    // レコード読み込み (タイムアタック用)
    TimeAttackManager::Get().LoadRecord("time_record.dat");
    // カウントダウン開始 (初期設定値から) 長さ3秒
    TimeAttackManager::Get().StartCountdown(3.0f);
    return true;
}

void Game_Update()
{
    // タイムアタック更新
    TimeAttackManager::Get().Update();

    // デバッグ用タイマー操作 (暫定) - 後日削除予定
    auto& dbg = DebugSettings::Get();
    if (!dbg.gameTimerRunning && !dbg.gameCleared && !dbg.gameDead)
    {
        dbg.gameTimerRunning = true;
    }
    if (dbg.gameTimerRunning && TimeAttackManager::Get().GetState() == TimeAttackManager::State::Running)
    {
        dbg.gameElapsedSeconds += (1.0f / 60.0f); // 固定タイムステップ
    }
    g_SceneManager.Update();
}

void Game_Draw()
{
    g_SceneManager.Draw();
}

void Game_Uninit()
{
    TimeAttackManager::Get().SaveRecord("time_record.dat");
    // SceneManagerは終了時に自動的に破棄されるため追加処理不要
}
