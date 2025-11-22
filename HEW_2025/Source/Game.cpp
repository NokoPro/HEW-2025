/*****************************************************************//**
 * @file   Game.cpp
 * @brief  SceneManagerを使ったゲーム全体の入口
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

namespace
{
    SceneManager g_SceneManager;
}

bool Game_Init(HWND /*hWnd*/, unsigned int /*width*/, unsigned int /*height*/)
{
    // 最初にテストシーンを起動
    g_SceneManager.Change<TestStageScene>();
    // レコード読み込み (固定パス仮)
    TimeAttackManager::Get().LoadRecord("time_record.dat");
    // カウントダウン開始 (将来設定値から) 現状3秒
    TimeAttackManager::Get().StartCountdown(3.0f);
    return true;
}

void Game_Update()
{
    // 新タイムアタック更新
    TimeAttackManager::Get().Update();

    // 旧デバッグ用簡易タイマー維持 (互換性) - 今後削除予定
    auto& dbg = DebugSettings::Get();
    if (!dbg.gameTimerRunning && !dbg.gameCleared && !dbg.gameDead)
    {
        dbg.gameTimerRunning = true;
    }
    if (dbg.gameTimerRunning && TimeAttackManager::Get().GetState() == TimeAttackManager::State::Running)
    {
        dbg.gameElapsedSeconds += (1.0f / 60.0f); // 仮: 固定タイムステップ
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
    // SceneManagerが勝手に片付ける想定ならここは空でOK
}
