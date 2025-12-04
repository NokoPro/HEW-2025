/**********************************************************************************************
 * @file      CountdownUIComponent.h
 * @brief     カウントダウンUIコンポーネント
 *
 * @author    浅野勇生
 * @date      2025/12/4
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/4 ]
 * 
 *    - [◎] CountdownUIComponent.h 作成
 *
 **********************************************************************************************/
#pragma once
#include <DirectXMath.h>
#include <string>
#include "ECS/World.h"

struct CountdownUIComponent
{
	// 配置・スケール
	DirectX::XMFLOAT3 position{ 35.0f,20.0f,-50.0f };
	DirectX::XMFLOAT3 scale{ 8.0f,8.0f,1.0f };

	// 使用するテクスチャエイリアス（必要なら保持）
	std::string digitsAlias;
	std::string startAlias;

	// スプライトシート1セルサイズ
	int cellWidth = 128;
	int cellHeight = 128;

	// 子スプライトのエンティティID
	EntityId sprite3{ kInvalidEntity };
	EntityId sprite2{ kInvalidEntity };
	EntityId sprite1{ kInvalidEntity };
	EntityId spriteStart{ kInvalidEntity };

	// START表示時間
	float startShowTime = 0.0f;
	// STARTを1回だけトリガするためのフラグ
	bool startTriggered = false;
};