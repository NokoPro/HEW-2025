#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Ranking/RankingDigitUIComponent.h"
#include "ECS/Prefabs/PrefabResultRankingUI.h"

void RegisterResultRankingUIPrefab(PrefabRegistry& registry)
{
    registry.Register("ResultRankingUI",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId master = w.Create();
            if (master == kInvalidEntity)
            {
                return kInvalidEntity;
            }

            // 各桁のタイプ（分の十/分の一/コロン/秒の十/秒の一）
            RankingDigitUIComponent::Type types[5] =
            {
                RankingDigitUIComponent::Type::MinTens,
                RankingDigitUIComponent::Type::MinOnes,
                RankingDigitUIComponent::Type::Separator,
                RankingDigitUIComponent::Type::SecTens,
                RankingDigitUIComponent::Type::SecOnes
            };
            
            // 6行 × 5桁を生成
            for (int row = 0; row < 6; ++row)
            {
                for (int i = 0; i < 5; ++i)
                {
                    const EntityId e = w.Create();

                    // 初期Transform（位置はシステム側で毎フレームカメラ基準に再配置されます）
                    w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

                    // 数字スプライト（Timerと同じテクスチャ/サイズを使用）
                    auto& spr = w.Add<Sprite2DComponent>(e);
                    spr.alias = "tex_number";
                    spr.visible = true;
                    spr.layer = 101;
                    spr.width = 5.0f;
                    spr.height = 5.0f;
                    spr.originX = sp.position.x;
                    spr.originY = sp.position.y;

                    // ランキング用の桁情報（行番号と桁種別）
                    auto& digit = w.Add<RankingDigitUIComponent>(e);
                    digit.row = row;
                    digit.type = types[i];
                }
            }

            return master;
        });
}