/*****************************************************************//**
 * @file   BackGronudRenderSystem.h
 * @brief  BackGroundRenderComponent を持つエンティティを描画するSystem
 *
 * - BackGroundRenderComponent を持っているエンティティをすべて描画する
 * - テクスチャは AssetManager から取得し、キャッシュしておく
 *
 * @author 土本蒼翔
 * @date   2025/11/25
 *********************************************************************/

#pragma once
#include "ECS/Systems/IRenderSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/BackGroundComponent.h"

#include "System/Sprite.h"
#include "System/AssetManager.h"
#include <DirectXMath.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

 /**
   * @class BackGroundRenderSystem
   * @brief BackGroundComponent を持っているものをすべて描画する
   */
class BackGroundRenderSystem : public IRenderSystem
{
private:
    /**
    * @brief ソートと描画のためのスプライト一時情報.
    */
    struct SortableSprite
    {
        // ソート用キー
        int layer;      // BackGroundComponent::layer
        float zDepth;   // TransformComponent::position.z

        // 描画用データ
        DirectX::XMFLOAT4X4 world; // ワールド行列
        DirectX::XMFLOAT2 offset;
        DirectX::XMFLOAT2 size; // 表示サイズ
        Texture* hTex;          // テクスチャハンドル
    };

public:
    // カメラ行列を受け取る
    void SetViewProj(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj)
    {
        m_view = view;
        m_proj = proj;
    }

    void Render(const World& world) override;

private:
    DirectX::XMFLOAT4X4 m_view{};   ///< カメラのビュー行列
    DirectX::XMFLOAT4X4 m_proj{};   ///< カメラのプロジェクション行列

    // alias -> AssetHandle<Texture> キャッシュ
    std::unordered_map<std::string, AssetHandle<Texture>> m_texCache;

    // 描画リスト(毎フレーム使いまわす)
    std::vector<SortableSprite> m_spriteList;
    
};

