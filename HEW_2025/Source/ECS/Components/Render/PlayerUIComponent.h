/*********************************************************************/
/* @file   PlayerUIComponent.h
 * @brief  プレイヤーUI表示状態(ジャンプ/ブリンクなど)管理用コンポーネント
 *
 * 旧フィールド playerHI / playerYO は後方互換のため残し、新しい明確な名前の
 * jumpActive / blinkActive と個別タイマ jumpTimer / blinkTimer を追加。
 * playerUItimer は旧仕様(単一タイマ)互換のため残し、未使用なら 0 にしておく。
 *
 * @actor 土本
 * @date   2025/11/17
 *********************************************************************/
#pragma once

struct PlayerUIComponet
{
	// --- 旧仕様互換フィールド ---
	bool playerHI = false;      // (旧)ジャンプ表示フラグ
	bool playerYO = false;      // (旧)ブリンク/YO表示フラグ再利用
	float playerUItimer = 0.0f; // (旧)単一タイマ

	// --- 新仕様: 個別管理 ---
	bool  jumpActive = false;   // ジャンプUIアクティブ
	float jumpTimer  = 0.0f;    // ジャンプUI残時間

	bool  blinkActive = false;  // ブリンクUIアクティブ
	float blinkTimer  = 0.0f;   // ブリンクUI残時間
};