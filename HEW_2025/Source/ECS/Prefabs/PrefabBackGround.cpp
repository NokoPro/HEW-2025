/*****************************************************************//**
 * @file   PrefabBackGround.cpp
 * @brief  背景を生成するプレハブ実装 
 *         2025/12/01 背景スクロール実装の為、背景2追加
 *
 * @author 土本蒼翔
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
            // 背景1
            // エンティティ生成
            EntityId e1 = w.Create();
            if (e1 == kInvalidEntity)
            {
                return kInvalidEntity;
            }

            /// Transform（scale はそのまま使えるようにしておく）
            auto& tr1 = w.Add<TransformComponent>(e1, sp.position, sp.rotationDeg, sp.scale);
            tr1.position.x = 35.0f;
            tr1.position.y = 45.0f;  // y座標調整しました 45.0f
            tr1.scale = { 1.0f,1.0f,1.0f };

            // Sprite背景
            auto& spb1 = w.Add<BackGroundComponent>(e1);
            spb1.alias = "tex_w1920h2160"; // 背景画像

            // 追加した背景画像名前
            //"tex_newbackground" newBackGround.jpg
            //"tex_w1920h1080"    w1920h1080.jpg
            //"tex_w1920h2160"    w1920h2160.jpg

                                   // 試した値
            spb1.width = 76.8f;     //76.8f  ,64.0f
            spb1.height = 86.4f;    //172.8f ,86.4f   64.0f
            spb1.originX = 0.5f;
            spb1.originY = 0.5f;
            spb1.layer = -10;      // 背景後面
            spb1.visible = true;   // 表示

            w.Add<BackGroundScrollComponent>(e1);

            // 背景2
            EntityId e2 = w.Create();   // 2つ目のエンティティ生成
            if (e2 == kInvalidEntity)
            {
               return kInvalidEntity;//無効エンティティじゃないか確認
            }

            auto& tr2 = w.Add<TransformComponent>(e2, sp.position, sp.rotationDeg, sp.scale);

            // 背景1の上に配置
            tr2.position.x = tr1.position.x;
            tr2.position.y = (tr1.position.y * 3.0f) -3.6f; // 一旦これで
                                                            // tr1.position.y - bg1.height;
            tr2.scale = tr1.scale;

            auto& spb2 = w.Add<BackGroundComponent>(e2);
            // 背景1と同じ設定にする
            spb2 = spb1;
            spb2.visible = true;

            w.Add<BackGroundScrollComponent>(e2);

            return e1;
            
        }
    );

}
