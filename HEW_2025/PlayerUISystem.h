/*********************************************************************/
/* @file   PlayerUISystem.cpp
 * @brief  プレイヤーのUI描画するかを切り替える処理
 *
 * 時間経過でUIを消す処理
 *
 * @author 土本蒼翔
 * @date   2025/11/17
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "PlayerUIComponent.h"

class PlayerUISystem
{
public:
	/*PlayerUISystem();
	~PlayerUISystem();*/

	void PlayerUIUpdate();

};