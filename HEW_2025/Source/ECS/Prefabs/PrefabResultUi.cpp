/*****************************************************************//**
* @file   PrefabBackGround.h
* @brief  îwåiÇê∂ê¨Ç∑ÇÈÉvÉåÉnÉu
*
* @author é¬å¥èÉ
* @date   2025/11/25
* ********************************************************************/

#include"ECS/Prefabs/PrefabResultUi.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ResultUiComponent.h"


void RegisterResultUiPrefab(PrefabRegistry& registry)
{
    registry.Register("ResultUI",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // ÉGÉìÉeÉBÉeÉBê∂ê¨
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            /// TransformÅiscale ÇÕÇªÇÃÇ‹Ç‹égÇ¶ÇÈÇÊÇ§Ç…ÇµÇƒÇ®Ç≠Åj
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position = sp.position;
            tr.position.x = sp.position.x;
            tr.position.y = sp.position.y;  // yç¿ïWí≤êÆÇµÇ‹ÇµÇΩ
            tr.scale = sp.scale;
            
           
            const char* fileName[] = 
            { "tex_w1920h2160" ,
                "testui",
            };
            // Spriteîwåi
            auto& spb = w.Add<ResultUiComponent>(e);
            spb.alias = sp.modelAlias;
            spb.alias = fileName[(int)sp.scale.z]; // îwåiâÊëú

            // í«â¡ÇµÇΩîwåiâÊëúñºëO
            //"tex_newbackground" newBackGround.jpg
            //"tex_w1920h1080"    w1920h1080.jpg
            //"tex_w1920h2160"    w1920h2160.jpg

                                   // ééÇµÇΩíl
            spb.width = 16.8f+ sp.scale.x;     //76.8f  ,64.0f
            spb.height = 16.4f+ sp.scale.y;    //172.8f ,86.4f   64.0f
            spb.originX = 0.5f + sp.position.x;
            spb.originY = 0.5f + sp.position.y;
            spb.layer = -100;      // îwåiå„ñ 
            spb.layer = -10;      // îwåiå„ñ 
            spb.visible = true;   // ï\é¶

            return e;
        }
    );
}