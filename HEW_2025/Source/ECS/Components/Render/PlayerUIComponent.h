/*********************************************************************/
/* @file   PlayerUIComponent.h
 * @brief  プレイヤーのHIフラグとYOフラグ
 *
 * @author 土本蒼翔
 * @date   2025/11/17
 *********************************************************************/
#pragma once

struct PlayerUIComponet
{
	bool playerHI = false;	// プレイヤーのHIフラグ
	bool playerYO = false;	// プレイヤーのYOフラグ

	float playerUItimer = 1.0f;	// UIを表示する時間
};