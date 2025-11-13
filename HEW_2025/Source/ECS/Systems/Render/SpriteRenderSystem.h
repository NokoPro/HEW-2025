/*****************************************************************//**
 * @file   SpriteRenderSystem.h
 * @brief  Sprite2DComponent を持つエンティティを描画するSystem
 *
 * - Sprite2DComponent を持っているエンティティをすべて描画する
 * 
 * @author 土本蒼翔
 * @author 浅野勇生
 * @author 川谷優真
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "ECS/Systems/IRenderSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

#include "System/Sprite.h"
#include "System/AssetManager.h"
#include <DirectXMath.h>
#include <unordered_map>

 /**
  * @class SpriteRenderSystem
  * @brief Sprite2DComponent を持っているものをすべて描画する
  */
class SpriteRenderSystem : public IRenderSystem
{
public:
    // カメラ行列を受け取る
    void SetViewProj(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& proj)
    {
        m_view = view;
        m_proj = proj;
    }

    void Render(const World& world) override
    {
        using namespace DirectX;

        // 毎フレームカメラ行列を Sprite に渡す
        Sprite::SetView(m_view);
        Sprite::SetProjection(m_proj);

        world.View<TransformComponent, Sprite2DComponent>(
            [&](EntityId, const TransformComponent& tr, const Sprite2DComponent& sp)
            {
                // テクスチャをキャッシュしておく（毎フレーム AssetManager を叩かない）
                AssetHandle<Texture> hTexHandle;
                auto it = m_texCache.find(sp.alias);
                if (it != m_texCache.end())
                {
                    hTexHandle = it->second;
                }
                else
                {
                    hTexHandle = AssetManager::GetTexture(sp.alias);
                    // キャッシュに登録（nullptr でも記録しておけば次回探査が省略できる）
                    m_texCache[sp.alias] = hTexHandle;
                }

                Texture* hTex = hTexHandle.get();
                if (!hTex)
                {
                    return;
                }

                // サイズ
                XMFLOAT2 size(sp.width, sp.height);

                // 原点(originX, originY) を考慮したオフセットを計算
                // 頂点は (-0.5 .. 0.5) で定義されているため、offset = position + (0.5 - origin) * size
                XMFLOAT2 offset;
                offset.x = tr.position.x + (0.5f - sp.originX) * sp.width;
                offset.y = tr.position.y + (0.5f - sp.originY) * sp.height;

                // ワールド行列には Z 深度のみを反映（XY はオフセットで扱う）
                XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, tr.position.z);
                XMFLOAT4X4 Wf;
                XMStoreFloat4x4(&Wf, XMMatrixTranspose(W));

                Sprite::SetWorld(Wf);
                Sprite::SetOffset(offset);
                Sprite::SetSize(size);

                Sprite::SetTexture(hTex);
                Sprite::Draw();
            }
        );
    }

private:
    DirectX::XMFLOAT4X4 m_view{};   ///< カメラのビュー行列
    DirectX::XMFLOAT4X4 m_proj{};   ///< カメラのプロジェクション行列

    // alias -> AssetHandle<Texture> キャッシュ
    std::unordered_map<std::string, AssetHandle<Texture>> m_texCache;
};
