/*****************************************************************//**
 * @file   PrefabBackGround.cpp
 * @brief  îwåiÇê∂ê¨Ç∑ÇÈÉvÉåÉnÉué¿ëï
 *
 * @author ìyñ{ëì„ƒ
 * @date   2025/11/25
 *********************************************************************/
#include "PrefabBackGround.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/BackGroundComponent.h"

void RegisterBackGroundPrefab(PrefabRegistry& registry)
{
    registry.Register("BackGround",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // ÉGÉìÉeÉBÉeÉBê∂ê¨
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            // Transform

            /// TransformÅiscale ÇÕÇªÇÃÇ‹Ç‹égÇ¶ÇÈÇÊÇ§Ç…ÇµÇƒÇ®Ç≠Åj
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position.x = 35.0f;
            tr.scale = { 1.0f,1.0f,1.0f };

            // Spriteîwåi
            auto& spb = w.Add<BackGroundComponent>(e);
            spb.alias = "tex_newbackground"; // îwåiâÊëú
            spb.width = 65.0f;
            spb.height = 65.0f;
            spb.originX = 0.5f;
            spb.originY = 0.5f;
            spb.layer = -10;      // îwåiå„ñ 
            spb.visible = true;  // ï\é¶

            return e;
        }
    );


}
