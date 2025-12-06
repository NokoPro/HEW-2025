/*****************************************************************//**
 * @file   Game.h
 * @brief  アプリケーション側の入口。SceneManager をここで管理する。
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <Windows.h>
#include "Scene/SceneManager.h" // SceneManager 型参照用追加

// ゲーム初期化
bool Game_Init(HWND hWnd, unsigned int width, unsigned int height);
// フレーム更新
void Game_Update();
// 描画
void Game_Draw();
// 終了処理
void Game_Uninit();

// グローバル SceneManager アクセサ (Game.cpp で定義)
SceneManager& GetSceneManager();
