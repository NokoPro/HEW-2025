/*****************************************************************//**
 * @file   PrefabBackGround.cpp
 * @brief  背景を生成するプレハブ実装
 *
 * @author 土本蒼翔
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
            // エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            /// Transform（scale はそのまま使えるようにしておく）
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position.x = 35.0f;
            tr.position.y = 45.0f;  // y座標調整しました
            tr.scale = { 1.0f,1.0f,1.0f };

            // Sprite背景
            auto& spb = w.Add<BackGroundComponent>(e);
            spb.alias = "tex_w1920h2160"; // 背景画像

            // 追加した背景画像名前
            //"tex_newbackground" newBackGround.jpg
            //"tex_w1920h1080"    w1920h1080.jpg
            //"tex_w1920h2160"    w1920h2160.jpg

                                   // 試した値
            spb.width = 76.8f;     //76.8f  ,64.0f
            spb.height = 86.4f;    //172.8f ,86.4f   64.0f
            spb.originX = 0.5f;
            spb.originY = 0.5f;
            spb.layer = -10;      // 背景後面
            spb.visible = true;   // 表示

            return e;
        }
    );


}
