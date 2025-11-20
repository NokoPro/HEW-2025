/*****************************************************************//**
 * @file   PrefabMovingPlatform.cpp
 * @brief  移動足場のプレハブ登録
 * 
 * @author 浅野勇生
 * @date   2025/11/14
 *********************************************************************/
#include "PrefabMovingPlatform.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Physics/PhysicsLayers.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Gimick/MovingPlatformComponent.h"
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"

void RegisterMovingPlatformPrefab(PrefabRegistry& registry)
{
    registry.Register("MovingPlatform",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // Collider（足場＝地面扱い）
            auto& col = w.Add<Collider2DComponent>(e);
            col.shape = ColliderShapeType::AABB2D;
            col.aabb.halfX = tr.scale.x;
            col.aabb.halfY = tr.scale.y;
            col.layer = Physics::LAYER_GROUND;
            col.hitMask = Physics::LAYER_PLAYER;
            col.isStatic = true; // 物理的には静的だが Transform をシステムで動かす（擬似キネマティック）

            // 見た目
            auto& mr = w.Add<ModelRendererComponent>(e);
            const char* mdlName = (!sp.modelAlias.empty()) ? sp.modelAlias.c_str() : "mdl_ground";
            mr.model = AssetManager::GetModel(mdlName);
            mr.visible = true;
            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            }

            // 動作パラメータ（デフォルトは水平方向に4ユニット往復）
            auto& mp = w.Add<MovingPlatformComponent>(e);
            mp.start = { tr.position.x, tr.position.y };
            mp.end   = { tr.position.x + 4.0f, tr.position.y };
            mp.speed = 2.0f;
            mp.waitTime = 0.0f;

            return e;
        }
    );
}