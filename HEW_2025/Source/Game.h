/*****************************************************************//**
 * @file   Game.h
 * @brief  アプリケーション側の入口。SceneManager をここで管理する。
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <Windows.h>

 /**
  * @brief ゲーム全体の初期化を行う
  * @param hWnd   作成済みのウィンドウハンドル
  * @param width  画面横幅
  * @param height 画面縦幅
  * @return 初期化に成功したかどうか
  */
bool Game_Init(HWND hWnd, unsigned int width, unsigned int height);

/**
 * @brief 毎フレーム更新処理 (固定タイムステップを想定)
 * @details メインループ側で kFixedDt に従い呼び出される。
 */
void Game_Update();

/**
 * @brief 毎フレームの描画処理
 */
void Game_Draw();

/**
 * @brief 終了処理
 */
void Game_Uninit();
