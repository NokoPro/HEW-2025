/*****************************************************************//**
 * @file   PrefabWall.cpp
 * @brief  縦長の壁を生成するプレハブ実装
 *
 * TestSceneで右/左に立てていた細長い当たりをここにまとめます。
 *
 * @author 浅野勇生
 * @date   2025/11/12
 *********************************************************************/
#include "PrefabWall.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"

void RegisterWallPrefab(PrefabRegistry& registry)
{
    registry.Register("Wall",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
			/// エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity)
                return kInvalidEntity;

			/// Transform（scale はそのまま使えるようにしておく）
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.rotationDeg = { 0.0f, 180.0f, 0.0f };

			/// 見た目
            auto& mr = w.Add<ModelRendererComponent>(e);
            const char* mdlName =
                (!sp.modelAlias.empty()) ? sp.modelAlias.c_str() : "mdl_column";
            mr.model = AssetManager::GetModel(mdlName);
            mr.visible = true;
            //mr.overrideTexture = AssetManager::GetTexture("tex_block");
            // レイヤー
            mr.layer = 20;

            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            }

			/// 当たり：静的な床
            auto& col = w.Add<Collider2DComponent>(e);
            col.shape = ColliderShapeType::AABB2D;
            col.aabb.halfX = tr.scale.x;
            col.aabb.halfY = tr.scale.y;
            col.layer = Physics::LAYER_GROUND;
            col.hitMask = Physics::LAYER_PLAYER;
            col.isStatic = true;

            return e;
        }
    );
}
