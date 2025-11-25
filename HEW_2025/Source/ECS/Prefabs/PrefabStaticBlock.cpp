/*****************************************************************//**
 * @file   PrefabStaticBlock.cpp
 * @brief  汎用の静的ブロック（当たり＋描画）を生成するプレハブ
 *
 * @author 浅野勇生
 * @date   2025/11/12
 *********************************************************************/
#include "PrefabStaticBlock.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"

#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"

void RegisterStaticBlockPrefab(PrefabRegistry& registry)
{
    registry.Register("StaticBlock",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
			/// エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity)
                return kInvalidEntity;

			/// Transform（scale はそのまま使えるようにしておく）
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
			tr.rotationDeg = { 0.0f, 180.0f, 0.0f }; // 回転は無し固定
            tr.scale = { 1.f,1.f,1.f };
			tr.position.z = 0.0f; // Z位置は0固定

			/// 見た目
            auto& mr = w.Add<ModelRendererComponent>(e);
            const char* mdlName =
                (!sp.modelAlias.empty()) ? sp.modelAlias.c_str() : "mdl_ground";
            mr.model = AssetManager::GetModel(mdlName);
            mr.visible = true;
			mr.layer = 10;
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
