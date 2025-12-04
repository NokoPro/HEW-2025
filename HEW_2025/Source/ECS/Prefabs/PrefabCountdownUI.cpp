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

			// 親 Transform
			auto& trRoot = w.Add<TransformComponent>(root,sp.position,sp.rotationDeg,sp.scale);

			// コンポーネント初期化
			auto& ui = w.Add<CountdownUIComponent>(root);

			// 位置、スケールはSpawnParamsから設定
			ui.position = trRoot.position;
			ui.scale = trRoot.scale;

            // テクスチャはエイリアス名で管理（Sprite2DComponent.alias に合わせる）
            const std::string kDigitsAlias = "tex_countdown_digits";
            const std::string kStartAlias  = "tex_countdown_start";
            ui.digitsAlias = kDigitsAlias;
            ui.startAlias  = kStartAlias;
				
			const int cellW = ui.cellWidth;
			const int cellH = ui.cellHeight;

            // 共通Transformの原点合わせ（各子は親と同座標・スケール）
            // 画像の並びに応じてセルインデックスからUVを算出するヘルパ
            auto spawnDigitSprite = [&](EntityId& out, const std::string& texAlias, int cellIndex, int layer)
                {
                    EntityId e = w.Create();
                    out = e;
                    w.Add<TransformComponent>(e, ui.position, DirectX::XMFLOAT3{ 0,0,0 }, ui.scale);
                    auto& spc = w.Add<Sprite2DComponent>(e);
                    spc.alias = texAlias;
                    spc.visible = false;
                    const float cellU = 1.0f / 3.0f; // 横3セル固定
                    spc.uvOffset = { cellU * cellIndex, 0.0f };
                    spc.uvSize   = { cellU, 1.0f };
                    spc.layer = 150;
                    spc.width = 32.0f;
					spc.height = 32.0f;
                };

            // スプライトシートの3/2/1セル（横3×縦1を想定）
            // 3=セル0, 2=セル1, 1=セル2 とする（横3セル固定UV計算）
            spawnDigitSprite(ui.sprite3, kDigitsAlias, 0, 100);
            spawnDigitSprite(ui.sprite2, kDigitsAlias, 1, 100);
            spawnDigitSprite(ui.sprite1, kDigitsAlias, 2, 100);

            // START（単一画像はフルUV）
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
                spc.width = 40.0f;
                spc.height = 40.0f;
            }

            // STARTは最初非表示
            ui.startShowTime = 0.0f;

            return root;


		});
}