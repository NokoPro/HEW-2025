/*****************************************************************//**
 * @file   SpriteRenderSystem.cpp
 * @brief  Sprite2DComponent を持つエンティティを描画するSystem
 * 
 * 回転は TransformComponent.rotationDeg.z（Z回り）を考慮。
 * サイズは Sprite2DComponent.width/height を使用します。
 * 回転時に移動しないよう、XYの平行移動はワールド行列に含め、
 * SetOffset は (0,0) を渡します（原点補正はワールド平行移動側で適用）。
 * 
 * @author 浅野勇生
 * @date   2025/11/14
 *********************************************************************/
#include "SpriteRenderSystem.h"

void SpriteRenderSystem::Render(const World& world)
{
    using namespace DirectX;

    // 1.描画リストをクリア
    m_spriteList.clear();

    // 2.すべてのスプライトを収集(Cullingは必要なら別途)
    world.View<TransformComponent, Sprite2DComponent>(
        [&](EntityId, const TransformComponent& tr, const Sprite2DComponent& sp)
        {
            // 非表示ならスキップ（テクスチャ取得もしない）
            if (!sp.visible) return;
            // alias が空ならスキップ
            if (sp.alias.empty()) return;

            // テクスチャ取得 & キャッシュ
            AssetHandle<Texture> hTexHandle;
            auto it = m_texCache.find(sp.alias);
            if (it != m_texCache.end())
            {
                hTexHandle = it->second;
            }
            else
            {
                hTexHandle = AssetManager::GetTexture(sp.alias);
                m_texCache[sp.alias] = hTexHandle;
            }

            Texture* hTex = hTexHandle.get();
            if (!hTex) return; // テクスチャ無ければスキップ

            // ソート用データ作成
            SortableSprite spriteData;
            spriteData.layer   = sp.layer;
            spriteData.zDepth  = tr.position.z;
            spriteData.hTex    = hTex;
            spriteData.size    = XMFLOAT2(sp.width, sp.height);
            spriteData.color   = sp.color;

            // 原点補正分をXY平行移動へ反映（原点0.5=中心なら補正0）
            const float dx = (0.5f - sp.originX) * spriteData.size.x;
            const float dy = (0.5f - sp.originY) * spriteData.size.y;
            const float tx = tr.position.x + dx;
            const float ty = tr.position.y + dy;
            const float tz = tr.position.z;

            // オフセットは(0,0)に固定（移動はワールド行列で行う）
            spriteData.offset = { 0.0f, 0.0f };

            // ワールド行列: 回転(Rz) → 平行移動(Txyz)
            const float rad = XMConvertToRadians(tr.rotationDeg.z);
            const XMMATRIX Rz = XMMatrixRotationZ(rad);
            const XMMATRIX T  = XMMatrixTranslation(tx, ty, tz);
            const XMMATRIX W  = Rz * T;
            XMStoreFloat4x4(&spriteData.world, XMMatrixTranspose(W));

            // UV情報をコンポーネントからコピー
            spriteData.uvOffset = sp.uvOffset;
            spriteData.uvSize   = sp.uvSize;

            // リストに追加
            m_spriteList.push_back(spriteData);
        }
    );

    // 3.ソート
    std::sort(m_spriteList.begin(), m_spriteList.end(),
        [](const SortableSprite& a, const SortableSprite& b)
        {
            if (a.layer != b.layer)
            {
                return a.layer < b.layer; // レイヤー優先(-10, 0, 10)
            }
            return a.zDepth > b.zDepth; // Z降順 (大きい=手前)
        }
    );

    // 4.描画
    Sprite::SetView(m_view);
    Sprite::SetProjection(m_proj);

    for (const auto& s : m_spriteList)
    {
        Sprite::SetWorld(s.world);
        Sprite::SetOffset(s.offset);
        Sprite::SetSize(s.size);
        Sprite::SetTexture(s.hTex);
        Sprite::SetUVPos(s.uvOffset);
        Sprite::SetUVScale(s.uvSize);
        Sprite::SetColor(s.color);
        Sprite::Draw();
    }
}
