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
	//int digitPlace = 1; // 1 = 1の位←これは残す必要がないけど一応残す
	// 桁の役割を定義
	enum class Type
	{
		MinTens, // 分の10の位
		MinOnes, // 分の1の位
		Separator, // セパレーター(:)
		SecTens, // 秒の10の位
		SecOnes  // 秒の1の位
	};

	Type type = Type::SecOnes;
};




