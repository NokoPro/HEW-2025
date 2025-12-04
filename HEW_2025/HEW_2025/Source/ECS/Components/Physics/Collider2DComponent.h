/*********************************************************************/
/* @file   Collider2DComponent.h
 * @brief  2Dコライダーコンポーネント定義
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include <DirectXMath.h>
#include "ColliderShape.h"
#include "PhysicsLayers.h"
#include "PhysicsMaterial.h"

struct Collider2DComponent
{
    ColliderShapeType shape = ColliderShapeType::AABB2D;

    // どのレイヤに属するか
    Physics::LayerMask layer = Physics::LAYER_DEFAULT;
    // どのレイヤと当てるか（ビットで複数指定）
    Physics::LayerMask hitMask = 0xFFFFFFFF;

    PhysicsMaterial material;

    // 形状ごとのサイズ
    Aabb2D    aabb;
    Circle2D  circle;
    Capsule2D capsule;

    // 当たっても位置を直さず「当たったよ」だけ通知したいとき
    bool isTrigger = false;

    // trueならこのコライダーは動かさない（床・壁）
    bool isStatic = false;

    // このコライダーの中心が Transform.position からどれだけずれているか
    // 2Dなので X/Y だけ使います（デフォルトは原点）
    DirectX::XMFLOAT2 offset{ 0.0f, 0.0f };
};