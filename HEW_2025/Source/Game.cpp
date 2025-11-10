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

namespace
{
    SceneManager g_SceneManager;
}

bool Game_Init(HWND /*hWnd*/, unsigned int /*width*/, unsigned int /*height*/)
{
    // 最初にテストシーンを起動
    g_SceneManager.Change<TestScene>();
    return true;
}

void Game_Update()
{
    g_SceneManager.Update();
}

void Game_Draw()
{
    g_SceneManager.Draw();
}

void Game_Uninit()
{
    // SceneManagerが勝手に片付ける想定ならここは空でOK
}
