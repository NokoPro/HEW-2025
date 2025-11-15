/*********************************************************************/
/* @file   PrefabDeathZone.cpp
 * @brief  死亡ゾーン（DeathZone）プレハブ実装
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "PrefabDeathZone.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"
#include "ECS/World.h"
#include "ECS/Tag/Tag.h"

// DeathZone（Deathゾーン）プレハブ登録
void RegisterDeathZonePrefab(PrefabRegistry& registry)
{
    registry.Register("DeathZone", [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId {
        EntityId e = world.Create();
        // Transform
        auto& tr = world.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

        // Collider
        Collider2DComponent col;
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_DESU_ZONE;
        col.hitMask = Physics::LAYER_PLAYER; // プレイヤーのみ当たり判定
		col.isTrigger = true;          // トリガーにする
        // サイズはscaleで調整
        world.Add<Collider2DComponent>(e, col);

        // モデル描画用コンポーネント追加
        std::string modelName = sp.modelAlias.empty() ? "mdl_deathzone" : sp.modelAlias;
        auto model = AssetManager::GetModel(modelName.c_str());



        if (model) {
            // シェーダ設定（必要に応じて変更）
            model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
            model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            auto& mr = world.Add<ModelRendererComponent>(e, model);
			mr.localScale = { 1.f, 0.5f, 1.f };
            mr.layer = -10;
        }

		// DeathZoneタグ付与
		world.Add<TagDeathZone>(e);
       

        return e;
    });
}
