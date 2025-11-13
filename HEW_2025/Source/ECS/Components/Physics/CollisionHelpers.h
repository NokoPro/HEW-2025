/*********************************************************************/
/* @file   CollisionHelpers.h
 * @brief  当たり判定のヘルパー関数群
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include <DirectXMath.h>
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

/**
 * @brief 2D AABB同士の重なり判定
 *
 * @param t1 コライダー1のTransformComponent
 * @param c1 コライダー1のCollider2DComponent
 * @param t2 コライダー2のTransformComponent
 * @param c2 コライダー2のCollider2DComponent
 * @return true 重なっている
 * @return false 重なっていない
 */
inline bool IsAabbOverlap(const TransformComponent& t1, const Collider2DComponent& c1,
    const TransformComponent& t2, const Collider2DComponent& c2)
{
	/// AABBの境界を計算
    const float axMin = (t1.position.x + c1.offset.x) - c1.aabb.halfX;
    const float axMax = (t1.position.x + c1.offset.x) + c1.aabb.halfX;
    const float ayMin = (t1.position.y + c1.offset.y) - c1.aabb.halfY;
    const float ayMax = (t1.position.y + c1.offset.y) + c1.aabb.halfY;

	/// BのAABB境界
    const float bxMin = (t2.position.x + c2.offset.x) - c2.aabb.halfX;
    const float bxMax = (t2.position.x + c2.offset.x) + c2.aabb.halfX;
    const float byMin = (t2.position.y + c2.offset.y) - c2.aabb.halfY;
    const float byMax = (t2.position.y + c2.offset.y) + c2.aabb.halfY;

	/// 重なってるか
    const bool overlapX = (axMin < bxMax) && (axMax > bxMin);
    const bool overlapY = (ayMin < byMax) && (ayMax > byMin);

    return overlapX && overlapY;
}
