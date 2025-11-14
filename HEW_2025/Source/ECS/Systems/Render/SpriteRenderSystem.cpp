/*****************************************************************//**
 * @file   SpriteRenderSystem.cpp
 * @brief  Sprite2DComponent を持つエンティティを描画するSystem
 * 
 * @author 浅野勇生
 * @date   2025/11/14
 *********************************************************************/
#include "SpriteRenderSystem.h"

void SpriteRenderSystem::Render(const World& world)
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
