/*****************************************************************//**
 * @file   DigitUIComponent.h 
 * @brief  タイマー表示用の桁情報を管理するコンポーネント
 * * 数字の「桁(位)」や「:」の役割を保持します。
 * System側はこの情報を見て、度の数字を表示するか決定します。   
  * 
 * @author 川谷優真
 * @date   2025/11/25
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

/**
 * @brief 数字・記号表示用コンポーネント(UV制御用)
 */
struct DigitUIComponent
{
	/**
	 * @enum Type
	 * @brief このエンティティが担当する役割の種類.
	 */
	enum class Type
	{
		MinTens, // 分の10の位
		MinOnes, // 分の1の位
		Separator, // セパレーター(:)
		SecTens, // 秒の10の位
		SecOnes  // 秒の1の位
	};

	// 自分の役割(デフォルトは秒の1の位)
	Type type = Type::SecOnes;
};




