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

	// 2.すべてのスプライト情報を収集(Cullingは一旦省略)
	world.View<TransformComponent, Sprite2DComponent>(
		[&](EntityId, const TransformComponent& tr, const Sprite2DComponent& sp)
		{
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
			if (!hTex) return; // テクスチャがない場合はスキップ

			// ソート用データ作成
			SortableSprite spriteData;
			spriteData.layer	= sp.layer;
			spriteData.zDepth	= tr.position.z;
			spriteData.hTex		= hTex;
			spriteData.size		= XMFLOAT2(sp.width, sp.height);

			// オフセット計算(Render時に行う計算をここで事前計算)
			// offset = position + (0.5 - origin) * size
			spriteData.offset.x = tr.position.x + (0.5 - sp.originX) * sp.width;
			spriteData.offset.y = tr.position.y + (0.5 - sp.originY) * sp.height;

			// ワールド行列(Z深度のみ反映し、XYはオフセットで扱う仕様に準拠)
			XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, tr.position.z);
			XMStoreFloat4x4(&spriteData.world, XMMatrixTranspose(W));

			// リストに追加
			m_spriteList.push_back(spriteData);
		}
	);

	// 3.ソート
	// ルート1:レイヤーが小さい順(奥->手前)
	// ルート2:レイヤーが同じなら、Zが大きい順(奥->手前) ※Zが小さいほど手前という仕様のため
	std::sort(m_spriteList.begin(), m_spriteList.end(),
		[](const SortableSprite& a, const SortableSprite& b)
		{
			if (a.layer != b.layer)
			{
				return a.layer < b.layer; // レイヤー昇順(-10, 0, 10)
			}
			return a.zDepth > b.zDepth; // Z降順 (5.0, 0.0, -5.0) = 奥から手前へ
		}
	);

	// 4.描画実行順
	Sprite::SetView(m_view);
	Sprite::SetProjection(m_proj);

	for (const auto& s : m_spriteList)
	{
		Sprite::SetWorld(s.world);
		Sprite::SetOffset(s.offset);
		Sprite::SetSize(s.size);
		Sprite::SetTexture(s.hTex);
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
