/*********************************************************************/
/* @file   ColliderShape.h
 * @brief  コライダーの形状を表す型
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

enum class ColliderShapeType
{
    AABB2D,     // 横スク用の四角
    CIRCLE2D,   // 攻撃判定などで使いやすい
    CAPSULE2D,  // プレイヤーを引っかかりにくくするならこれ
};

struct Aabb2D
{
    float halfX = 0.5f;
    float halfY = 0.5f;
};

struct Circle2D
{
    float radius = 0.5f;
};

struct Capsule2D
{
    float halfHeight = 0.5f;   // 縦方向の半分
    float radius = 0.25f;  // 丸い部分の半径
};
