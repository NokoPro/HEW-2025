/*****************************************************************//**
 * @file   IUpdateSystem.h
 * @brief  毎フレームの更新処理を担当するシステムの基底クラス
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "../World.h"

 /**
  * @class IUpdateSystem
  * @brief 毎フレームの更新処理を担当するシステムの基底クラス
  * @details
  * ECSにおける「System」のうち、ロジック更新（物理・入力・アニメーションなど）
  * を行うクラスが継承します。
  */
class IUpdateSystem
{
public:
	virtual ~IUpdateSystem() = default;

	/**
	 * @brief 更新処理
	 * @param[in,out] world ECSワールド全体への参照
	 * @param[in] dt 前フレームからの経過時間（秒）
	 */
	virtual void Update(class World& world, float dt) = 0;
};
