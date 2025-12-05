#include "PrefabOffscreenIndicatorFaceP2.h"
#include "ECS/Components/UI/OffscreenIndicatorComponent.h"
#include "ECS/Components/UI/OffscreenIndicatorFaceComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

void RegisterOffscreenIndicatorFaceP2Prefab(PrefabRegistry& reg)
{
    reg.Register("OffscreenIndicatorFaceP2",
        [](World& world, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = world.Create();

            // Transform 初期化（位置・回転・スケールは呼び出し元を尊重）
            world.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // 上物スプライト（最初はキャラアイコン）
            auto& spr = world.Add<Sprite2DComponent>(e);
            spr.visible = false;                 // 画面外ロジック側で制御
            spr.alias = "tex_p2_icon";         // 通常時に出す 2P アイコン
            spr.width = 6.0f;
            spr.height = 6.0f;

            // プレイヤー位置に追従させるためのインジケータ設定
            auto& ind = world.Add<OffscreenIndicatorComponent>(e);
            ind.visible = false;
            ind.bottomMargin = 0.6f;             // 吹き出しと同じくらいの高さ
            ind.offset = { 0.0f, 5.3f, 0.0f }; // 吹き出しより少し上に出す

            // HI / GO 切り替え情報
            auto& face = world.Add<OffscreenIndicatorFaceComponent>(e);
            face.aliasIcon = "tex_p2_icon";
            face.aliasHi = "ui_jump";
            face.aliasGo = "ui_blink";
            face.hiDuration = 2.0f;            // 「数秒」表示したければここを調整
            face.goDuration = 2.0f;

            // targetId は生成後に外部で設定（2PプレイヤーID）
            return e;
        }
    );
}
