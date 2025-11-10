/*****************************************************************//**
 * @file   IRenderSystem.h
 * @brief  描画処理を担当するシステムの基底クラス
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "../World.h"

 /**
  * @class IRenderSystem
  * @brief 描画処理を担当するシステムの基底クラス
  * @details
  * Update 系とは分離し、描画順序を制御しやすくします。
  */
class IRenderSystem
{
public:
	virtual ~IRenderSystem() = default;

	/**
	 * @brief 描画処理
	 * @param[in] world ECSワールド全体への参照
	 */
	virtual void Render(const class World& world) = 0;
};
