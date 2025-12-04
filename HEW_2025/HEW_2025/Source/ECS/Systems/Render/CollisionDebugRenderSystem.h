/*********************************************************************/
/* @file   CollisionDebugRenderSystem.h
 * @brief  当たり判定のデバッグレンダリングシステム定義
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include "../IRenderSystem.h"
#include <DirectXMath.h>

#include "System/Input.h"

class World;

/**
 * @brief Collider2DComponent を線の箱で描画するデバッグ用レンダリングシステム
 *
 * カメラ行列は事前に SetViewProj で渡す。
 * AABBしか対応していないが、当たり判定の可視化には十分。
 */
class CollisionDebugRenderSystem : public IRenderSystem
{
public:
    // カメラからもらう
    void SetViewProj(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj)
    {
        m_view = view;
        m_proj = proj;
    }

    void Render(const World& world) override;

private:
	DirectX::XMFLOAT4X4 m_view{};   /// カメラのビュー行列
	DirectX::XMFLOAT4X4 m_proj{};   /// カメラのプロジェクション行列

    // 表示のオン/オフフラグ（F2 でトグル）
    bool m_enabled = true;
};
