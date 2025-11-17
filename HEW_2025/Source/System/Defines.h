/*********************************************************************/
/* @file   Defines.h
 * @brief プロジェクト全体で使用する定数・マクロ定義
 *
 * C++17 を想定した共通定義をまとめたヘッダファイルです。
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <assert.h>
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>

/** @brief 目標フレームレート(FPS) */
static const int FPS = 60;
/** @brief FPS を float 型に変換した値 */
static const float fFPS = static_cast<float>(FPS);

/**
 * @def ASSET(path)
 * @brief リソースへの相対パスを生成するマクロ
 * @param path Assets フォルダ以下のパス
 */
#define ASSET(path)	"Assets/"path

/** @name 3D 空間単位変換マクロ */
//@{
/** @brief センチメートルをメートルの比率で変換 (例: CMETER(100) == 1.0f) */
#define CMETER(value) (value * 0.01f)

/** @brief メートル単位 (表現のためのラッパー) */
#define METER(value) (value * 1.0f)

/** @brief フレーム時間(ミリ秒相当)を計算するマクロ(値 / fFPS) */
#define MSEC(value) (value / fFPS)

/** @brief センチメートル→メートル→フレーム時間 に変換するマクロ */
#define CMSEC(value) MSEC(CMETER(value))
//@}

/** @brief 重力の加速度的な定数(ゲーム内調整用) */
static const float GRAVITY = 0.98f;

/** @brief アプリケーションのウィンドウタイトル */
static const char* APP_TITLE = "DX22_2PGame";

/** @name DeathZone 関連定数 */
//@{
/** @brief DeathZone の垂直移動速度 (Y方向) */
constexpr static float kDeathZoneSpeedY = 0.5f; ///< DeathZoneのY速度
/** @brief DeathZone の半分の高さ (ワールド単位) */
constexpr static float kDeathZoneHalfHeight = METER(15.f); ///< DeathZoneの高さ半分
//@}

/** @name 画面サイズ */
//@{
/** @brief 画面幅 (ピクセル) */
static const int SCREEN_WIDTH	= 1920;
/** @brief 画面高 (ピクセル) */
static const int SCREEN_HEIGHT	= 1080;
//@}

/** @name デバッグ用グリッド設定 */
//@{
/** @brief グリッド中心から端までの線の本数 */
static const int	DEBUG_GRID_NUM		= 10;
/** @brief グリッドの間隔（ワールド単位） */
static const float	DEBUG_GRID_MARGIN	= METER(1.0f);
//@}


#endif // __DEFINES_H__