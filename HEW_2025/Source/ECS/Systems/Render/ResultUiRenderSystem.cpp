#include "ECS/Systems/Render/ResultUiRenderSystem.h"

void ResultUiRenderSystem::Render(const World& world)
{
	using namespace DirectX;

	// 1.描画リストをクリア
	m_spriteList.clear();

	// 2.すべてのスプライトを収集(Cullingは必要なら別途)
	world.View<TransformComponent, ResultUiComponent>(
		[&](EntityId, const TransformComponent& tr, const ResultUiComponent& sp)
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
			spriteData.layer = sp.layer;
			spriteData.zDepth = tr.position.z;
			spriteData.hTex = hTex;
			spriteData.size = XMFLOAT2(sp.width, sp.height);

			// オフセット計算(Render時に行わないでここで計算)
			spriteData.offset.x = tr.position.x + (0.5f - sp.originX) * sp.width;
			spriteData.offset.y = tr.position.y + (0.5f - sp.originY) * sp.height;

			// ワールド行列(Zレベルだけ反映。XYはオフセットで表現)
			XMMATRIX W = XMMatrixTranslation(0.0f, 0.0f, tr.position.z);
			XMStoreFloat4x4(&spriteData.world, XMMatrixTranspose(W));

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
		Sprite::SetUVPos({ 0,0 });
		Sprite::SetUVScale({ 1,1 });
		Sprite::SetTexture(s.hTex);
		Sprite::Draw();
	}
}
