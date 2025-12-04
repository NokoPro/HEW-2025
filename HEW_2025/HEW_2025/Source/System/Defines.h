/*********************************************************************/
/* @file   Defines.h
 * @brief プロジェクト全体で使用する定数・マクロ定義
 * @details 時間・FPSを統一するため kTargetFps / kFixedDt を導入します。
 *          旧 FPS / fFPS は互換目的で残しますが新規コードでは使用しません。
 *********************************************************************/
#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <assert.h>
#include <Windows.h>
#include <stdarg.h>
#include <stdio.h>

/**
 * @name フレームレート関連
 * @{ */
/** @brief 目標(固定)フレームレート */
constexpr double kTargetFps = 60.0; ///< 新しい統一FPS
/** @brief 固定タイムステップ(秒) = 1 / kTargetFps */
constexpr double kFixedDt   = 1.0 / kTargetFps; ///< 物理/ロジック更新用
/** @brief 旧来互換: 整数FPS (非推奨) */
static const int FPS = static_cast<int>(kTargetFps); ///< Deprecated: use kTargetFps
/** @brief 旧来互換: float FPS (非推奨) */
static const float fFPS = static_cast<float>(kTargetFps); ///< Deprecated: use kTargetFps
/** @} */

/**
 * @def ASSET(path)
 * @brief リソースへの相対パスを生成するマクロ
 * @param path Assets フォルダ以下のパス
 */
#define ASSET(path) "Assets/" path

/** @name 3D 空間単位変換マクロ */
//@{
#define CMETER(value) (value * 0.01f)   ///< センチ→メートル
#define METER(value)  (value * 1.0f)    ///< メートル(ラッパ)
#define MSEC(value)   (value / fFPS)    ///< 旧: valueミリ秒相当 (非推奨)
#define CMSEC(value)  MSEC(CMETER(value))
//@}

/** @brief 重力の加速度的な定数(ゲーム内調整用) */
static const float GRAVITY = 0.98f;
/** @brief アプリケーションのウィンドウタイトル */
static const char* APP_TITLE = "DX22_2PGame";

/** @name DeathZone 関連定数 */
//@{
constexpr static float kDeathZoneSpeedY = 0.5f; ///< DeathZoneのY速度
constexpr static float kDeathZoneHalfHeight = METER(15.f); ///< DeathZone高さ半分
//@}

/** @name 画面サイズ */
//@{
static const int SCREEN_WIDTH  = 1920; ///< 画面幅(px)
static const int SCREEN_HEIGHT = 1080; ///< 画面高(px)
//@}

/** @name デバッグ用グリッド設定 */
//@{
static const int   DEBUG_GRID_NUM    = 10;      ///< グリッド線数
static const float DEBUG_GRID_MARGIN = METER(1.0f); ///< グリッド間隔
//@}

#endif // __DEFINES_H__