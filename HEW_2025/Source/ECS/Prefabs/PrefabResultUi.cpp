/*****************************************************************//**
* @file   PrefabBackGround.h
* @brief  背景を生成するプレハブ
*
* @author 篠原純
* @date   2025/11/25
* ********************************************************************/

#include"ECS/Prefabs/PrefabResultUi.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ResultUiComponent.h"


void RegisterResultUiPrefab(PrefabRegistry& registry)
{
    registry.Register("ResultBackGround",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            // エンティティ生成
            EntityId e = w.Create();
            if (e == kInvalidEntity) return kInvalidEntity;

            /// Transform（scale はそのまま使えるようにしておく）
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);
            tr.position.x = sp.position.x;
            tr.position.y = sp.position.y;  // y座標調整しました
            tr.scale = sp.scale;

            // Sprite背景
            auto& spb = w.Add<ResultUiComponent>(e);
            spb.alias = "tex_w1920h2160"; // 背景画像

            // 追加した背景画像名前
            //"tex_newbackground" newBackGround.jpg
            //"tex_w1920h1080"    w1920h1080.jpg
            //"tex_w1920h2160"    w1920h2160.jpg

                                   // 試した値
            spb.width = 16.8f+ sp.scale.x;     //76.8f  ,64.0f
            spb.height = 16.4f+ sp.scale.y;    //172.8f ,86.4f   64.0f
            spb.originX = 0.5f + sp.position.x;
            spb.originY = 0.5f + sp.position.y;
            spb.layer = -10;      // 背景後面
            spb.visible = true;   // 表示

            return e;
        }
    );
}