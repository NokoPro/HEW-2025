/*****************************************************************//**
 * @file   PrefabPreesA.cpp 
 * @brief  Aボタンを押してくださいを表示するプレハブ     
  * 
 * @author 川谷優真
 * @date   2025/12/4
 *********************************************************************/
#include "PrefabPreesA.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/PreesAComponent.h"

void RegisterPreesAPrefab(PrefabRegistry& registry)
{
    registry.Register("PreesA",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            /// Transform（scale はそのまま使えるようにしておく）
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position.x = 35.0f;
            tr.position.y = 45.0f;
            tr.scale = { 1.0f,1.0f,1.0f };

            
            auto& spb = w.Add<BackGroundComponent>(e);
            spb.alias = "tex_ui_preesA";


                                  
            spb.width = 76.8f;     
            spb.height = 86.4f;    
            spb.originX = 0.5f;
            spb.originY = 0.5f;
            spb.layer = -11;      
            spb.visible = true;   

            return e;
        }
    );


}
