/*********************************************************************/
/* @file   PlayerInputComponent.h
 * @brief このエンティティがどのプレイヤー入力を使うかを示す
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once

struct PlayerInputComponent
{
	int playerIndex = 0;   ///< 0番なら1P、1番なら2P、など
	bool isJumpRequested = false; // ジャンプ入力があったか
	bool isBlinkRequested = false;// ブリンク入力があったか
};
