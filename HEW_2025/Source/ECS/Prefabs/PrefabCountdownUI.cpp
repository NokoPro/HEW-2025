#include "PrefabCountdownUI.h"
#include "ECS/World.h"
#include "ECS/Components/Render/CountdownUIComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "System/AssetManager.h"

void RegisterCountdownUIPrefab(PrefabRegistry& registry)
{
	registry.Register("CountdownUI",
		[](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
		{
			EntityId root = w.Create();
			if (root == kInvalidEntity) return kInvalidEntity;

			auto& trRoot = w.Add<TransformComponent>(root,sp.position,sp.rotationDeg,sp.scale);
			auto& ui = w.Add<CountdownUIComponent>(root);

			ui.position = trRoot.position;
			ui.scale = trRoot.scale;

			const std::string kDigitsAlias = "tex_countdown_digits";
			const std::string kStartAlias  = "tex_countdown_start";
			ui.digitsAlias = kDigitsAlias;
			ui.startAlias  = kStartAlias;

			const int cellW = ui.cellWidth;
			const int cellH = ui.cellHeight;

			auto spawnDigitSprite = [&](EntityId& out, const std::string& texAlias, int cellIndex, int layer)
				{
					EntityId e = w.Create();
					out = e;
					w.Add<TransformComponent>(e, ui.position, DirectX::XMFLOAT3{ 0,0,0 }, ui.scale);
					auto& spc = w.Add<Sprite2DComponent>(e);
					spc.alias = texAlias;
					spc.visible = false;
					const float cellU = 1.0f / 3.0f; // 横3セル想定
					spc.uvOffset = { cellU * cellIndex, 0.0f };
					spc.uvSize   = { cellU, 1.0f };
					spc.layer = 150;
					spc.width  = 4.0f;
					spc.height = 4.0f;
					// 基準サイズへ反映
					ui.digitBaseWidth  = spc.width;
					ui.digitBaseHeight = spc.height;
				};

			spawnDigitSprite(ui.sprite3, kDigitsAlias, 0, 100);
			spawnDigitSprite(ui.sprite2, kDigitsAlias, 1, 100);
			spawnDigitSprite(ui.sprite1, kDigitsAlias, 2, 100);

			{
				EntityId e = w.Create();
				ui.spriteStart = e;
				w.Add<TransformComponent>(e, ui.position, DirectX::XMFLOAT3{ 0,0,0 }, ui.scale);
				auto& spc = w.Add<Sprite2DComponent>(e);
				spc.alias = kStartAlias;
				spc.visible = false;
				spc.uvOffset = { 0.0f, 0.0f };
				spc.uvSize   = { 1.0f, 1.0f };
				spc.layer = 150;
				spc.width  = 40.0f;
				spc.height = 40.0f;
				ui.startBaseWidth  = spc.width;
				ui.startBaseHeight = spc.height;
			}

			ui.startShowTime = 0.0f;
			return root;
		});
}