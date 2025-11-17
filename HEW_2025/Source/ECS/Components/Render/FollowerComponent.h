/*****************************************************************//**
 * @file   FollowerComponent.h 
 * @brief       
  * 
 * @author 川谷優真
 * @date   2025/11/15
 *********************************************************************/
#pragma once
#include "ECS/ECS.h"
#include <DirectXMath.h>

/**
 * @brief このエンティティを targetId の位置へ追従させるコンポーネント
 */
struct FollowerComponent
{
	EntityId targetId = 0;              // 追従対象のエンティティID
	DirectX::XMFLOAT2 offset{0.0f,0.0f}; // 追従時に加算するXYオフセット（UI位置調整など）
};




