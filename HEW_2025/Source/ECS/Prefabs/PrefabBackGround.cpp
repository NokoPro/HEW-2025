/*****************************************************************//**
 * @file   PrefabBackGround.cpp
 * @brief  îwåiÇê∂ê¨Ç∑ÇÈÉvÉåÉnÉué¿ëï 
 *         2025/12/01 îwåiÉXÉNÉçÅ[Éãé¿ëïÇÃà◊ÅAîwåi2í«â¡
 *
 * @author ìyñ{ëì„ƒ
 * @date   2025/11/25
 *********************************************************************/
#include "PrefabBackGround.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/BackGroundComponent.h"
#include "ECS/Components/Game/BackGroundScrollComponent.h"

void RegisterBackGroundPrefab(PrefabRegistry& registry)
{
    registry.Register("BackGround",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // îwåi1
            // ÉGÉìÉeÉBÉeÉBê∂ê¨
            EntityId e1 = w.Create();
            if (e1 == kInvalidEntity)
            {
                return kInvalidEntity;
            }

            /// TransformÅiscale ÇÕÇªÇÃÇ‹Ç‹égÇ¶ÇÈÇÊÇ§Ç…ÇµÇƒÇ®Ç≠Åj
            auto& tr1 = w.Add<TransformComponent>(e1, sp.position, sp.rotationDeg, sp.scale);
            tr1.position.x = 35.0f;
            tr1.position.y = 45.0f;  // yç¿ïWí≤êÆÇµÇ‹ÇµÇΩ 45.0f
            tr1.scale = { 1.0f,1.0f,1.0f };

            // Spriteîwåi
            auto& spb1 = w.Add<BackGroundComponent>(e1);
            spb1.alias = "tex_w1920h2160"; // îwåiâÊëú

            // í«â¡ÇµÇΩîwåiâÊëúñºëO
            //"tex_newbackground" newBackGround.jpg
            //"tex_w1920h1080"    w1920h1080.jpg
            //"tex_w1920h2160"    w1920h2160.jpg

                                   // ééÇµÇΩíl
            spb1.width = 76.8f;     //76.8f  ,64.0f
            spb1.height = 86.4f;    //172.8f ,86.4f   64.0f
            spb1.originX = 0.5f;
            spb1.originY = 0.5f;
            spb1.layer = -10;      // îwåiå„ñ 
            spb1.visible = true;   // ï\é¶

            w.Add<BackGroundScrollComponent>(e1);//e1Ç…BackGroundScrollComponentí«â¡

            // îwåi2
            EntityId e2 = w.Create();   // 2Ç¬ñ⁄ÇÃÉGÉìÉeÉBÉeÉBê∂ê¨
            if (e2 == kInvalidEntity)
            {
               return kInvalidEntity;//ñ≥å¯ÉGÉìÉeÉBÉeÉBÇ∂Ç·Ç»Ç¢Ç©ämîF
            }

            auto& tr2 = w.Add<TransformComponent>(e2, sp.position, sp.rotationDeg, sp.scale);

            // îwåi1ÇÃè„Ç…îzíu
            tr2.position.x = tr1.position.x;
            tr2.position.y = (tr1.position.y * 3.0f) -3.6f; // àÍíUÇ±ÇÍÇ≈
                                                            // tr1.position.y - bg1.height;
            tr2.scale = tr1.scale;

            auto& spb2 = w.Add<BackGroundComponent>(e2);
            // îwåi1Ç∆ìØÇ∂ê›íËÇ…Ç∑ÇÈ
          //  spb2 = spb1;
            spb2.alias = spb1.alias;
            spb2.width = spb1.width; 
            spb2.height = spb1.height; 
            spb2.originX = spb1.originX;
            spb2.originY = spb1.originY;
            spb2.layer = spb1.layer;      // îwåiå„ñ 
            spb2.visible = spb1.visible;

            w.Add<BackGroundScrollComponent>(e2);//e2Ç…BackGroundScrollComponentí«â¡

            return e1;
            
            spb.width = 76.8f;     //76.8f  ,64.0f
            spb.height = 86.4f;    //172.8f ,86.4f   64.0f
            spb.originX = 0.5f;
            spb.originY = 0.5f;
            spb.layer = -10;      // îwåiå„ñ 
            spb.visible = true;   // ï\é¶
			spb.color = { 1.0f,1.0f,1.0f,1.0f };
            return e;
        }
    );

}
