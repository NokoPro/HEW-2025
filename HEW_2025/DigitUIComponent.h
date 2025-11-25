/*****************************************************************//**
 * @file   DigitUIComponent.h 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/25
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

/**
 * @brief 数学表示用コンポーネント(UV情報付き).
 */
struct DigitUIComponent
{
	enum class Place
	{
		Sec1,  // 秒の1の位
		Sec10, // 秒の10の位
		Min1,  // 分の1の位
		Min10, // 分の10の位
		Colon  // コロン
	};

	Place place = Place::Sec1;        // 位置情報

	DirectX::XMFLOAT2 uvOffset{ 0.0f, 0.0f }; // UVオフセット
	DirectX::XMFLOAT2 uvScale{ 1.0f, 1.0f };  // UVスケール

	// 表示サイズ
	float width = 64.0f;
	float height = 80.0f;
};




