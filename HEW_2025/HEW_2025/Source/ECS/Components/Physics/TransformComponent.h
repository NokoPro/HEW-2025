/*****************************************************************//**
 * @file   TransformComponent.h
 * @brief  エンティティの空間的な位置・回転・拡大を表すコンポーネント
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <DirectXMath.h>

 /**
  * @brief 位置・回転・拡大をまとめたコンポーネント
  * @details
  * 今回はとりあえずサイドビュー寄りを想定して、位置はXYZで持ちます。
  * 回転・拡大は必要になったら使ってください。
  */
struct TransformComponent
{
    DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 rotationDeg{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };
};
