/*********************************************************************/
/* @file   PrefabGoal.cpp
 * @brief  ゴールプレハブ実装
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "PrefabGoal.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"

void RegisterGoalPrefab(PrefabRegistry& registry)
{
    registry.Register("Goal", 
        [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId 
        {
			// エンティティ生成
            EntityId e = world.Create();
            if (e == kInvalidEntity)
                return kInvalidEntity;

            // Transform
            auto& tr = world.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // Collider2D (AABB)
            auto& col = world.Add<Collider2DComponent>(e);
            col.shape = ColliderShapeType::AABB2D;
            col.aabb.halfX = tr.scale.x;
            col.aabb.halfY = tr.scale.y;
            col.layer = Physics::LAYER_GOAL;
            col.hitMask = Physics::LAYER_PLAYER;
			col.isTrigger = true; // 当たり判定はトリガーにする


            // Model (見た目は地面と同じにしておく)
            auto& mr = world.Add<ModelRendererComponent>(e);
            const char* mdlName =
                (!sp.modelAlias.empty()) ? sp.modelAlias.c_str() : "mdl_ground";
            mr.model = AssetManager::GetModel(mdlName);
            mr.visible = true;
            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            }

            return e;
        }
);
}
