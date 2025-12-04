/*****************************************************************//**
 * @file   FollowCameraSystem.h
 * @brief  カメラのシステム
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include <DirectXMath.h>

/**
 * @brief サイドビュー / TPS / 固定スクロール をまとめて扱うカメラシステム
 *
 * 対象エンティティには最低でも
 *  - ActiveCameraTag
 *  - CameraRigComponent
 *  - TransformComponent
 * が付いていることを想定します。
 *
 * Update後に GetView()/GetProj() で転置済み行列を取り出せます。
 */
class FollowCameraSystem final : public IUpdateSystem
{
public:
    void Update(class World& world, float dt) override;

    /// 転置済みビュー行列を返す
    const DirectX::XMFLOAT4X4& GetView() const { return m_viewT; }

    /// 転置済みプロジェクション行列を返す
    const DirectX::XMFLOAT4X4& GetProj() const { return m_projT; }

private:
    DirectX::XMFLOAT4X4 m_viewT{};  ///< 転置済みビュー
    DirectX::XMFLOAT4X4 m_projT{};  ///< 転置済みプロジェクション
};
