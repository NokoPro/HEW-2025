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
			spriteData.layer	= sp.layer;
			spriteData.zDepth	= tr.position.z;
			spriteData.hTex		= hTex;
			spriteData.size		= XMFLOAT2(sp.width, sp.height);
			spriteData.color	= sp.color;
            spriteData.hTex		= hTex;
            spriteData.size		= XMFLOAT2(sp.width, sp.height);

			// オフセット計算(Render時に行わないでここで計算)
            spriteData.offset.x = tr.position.x + (0.5f - sp.originX) * spriteData.size.x;
            spriteData.offset.y = tr.position.y + (0.5f - sp.originY) * spriteData.size.y;

			// ワールド行列(Zレベルだけ反映。XYはオフセットで表現)
			XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, tr.position.z);
			XMStoreFloat4x4(&spriteData.world, XMMatrixTranspose(W));

			// UV情報をコンポーネントからコピー
			spriteData.uvOffset = sp.uvOffset;
			spriteData.uvSize = sp.uvSize;

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








 //void SpriteRenderSystem::Render(const World& world)
 //{
 //    using namespace DirectX;
 //
 //    // 毎フレームカメラ行列を Sprite に渡す
 //    Sprite::SetView(m_view);
 //    Sprite::SetProjection(m_proj);
 //
 //    world.View<TransformComponent, Sprite2DComponent>(
 //        [&](EntityId, const TransformComponent& tr, const Sprite2DComponent& sp)
 //        {
 //            // テクスチャをキャッシュしておく（毎フレーム AssetManager を叩かない）
 //            AssetHandle<Texture> hTexHandle;
 //            auto it = m_texCache.find(sp.alias);
 //            if (it != m_texCache.end())
 //            {
 //                hTexHandle = it->second;
 //            }
 //            else
 //            {
 //                hTexHandle = AssetManager::GetTexture(sp.alias);
 //                // キャッシュに登録（nullptr でも記録しておけば次回探査が省略できる）
 //                m_texCache[sp.alias] = hTexHandle;
 //            }
 //
 //            Texture* hTex = hTexHandle.get();
 //            if (!hTex)
 //            {
 //                return;
 //            }
 //
 //            // サイズ
 //            XMFLOAT2 size(sp.width, sp.height);
 //
 //            // 原点(originX, originY) を考慮したオフセットを計算
 //            // 頂点は (-0.5 .. 0.5) で定義されているため、offset = position + (0.5 - origin) * size
 //            XMFLOAT2 offset;
 //            offset.x = tr.position.x + (0.5f - sp.originX) * sp.width;
 //            offset.y = tr.position.y + (0.5f - sp.originY) * sp.height;
 //
 //            // ワールド行列には Z 深度のみを反映（XY はオフセットで扱う）
 //            XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, tr.position.z);
 //            XMFLOAT4X4 Wf;
 //            XMStoreFloat4x4(&Wf, XMMatrixTranspose(W));
 //
 //            Sprite::SetWorld(Wf);
 //            Sprite::SetOffset(offset);
 //            Sprite::SetSize(size);
 //
 //            Sprite::SetTexture(hTex);
 //            Sprite::Draw();
 //        }
 //    );
 //}
