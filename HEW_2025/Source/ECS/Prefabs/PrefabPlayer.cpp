/*****************************************************************//**
 * @file   PrefabPlayer.cpp
 * @brief  プレイヤー用プレハブの実装（1P/2Pでモデルを変えられる版）
 *
 * SpawnParams.modelAlias が指定されていればそれを使い、
 * なければ padIndex に応じてモデルを切り替えます。
 *
 * @author 浅野勇生
 * @date   2025/11/12
 *********************************************************************/
#include "PrefabPlayer.h"

#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"

#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"

static const char* kDefaultPlayerModel1P = "mdl_player_1p";
static const char* kDefaultPlayerModel2P = "mdl_player_2p";
static const char* kFallbackModel = "mdl_slime";

void RegisterPlayerPrefab(PrefabRegistry& registry)
{
    registry.Register("Player",
        [](World& w, const PrefabRegistry::SpawnParams& sp) -> EntityId
        {
            EntityId e = w.Create();
            if (e == kInvalidEntity)
            {
                return kInvalidEntity;
            }

            // Transform
            auto& tr = w.Add<TransformComponent>(e, sp.position, sp.rotationDeg, sp.scale);

            // 入力
            auto& inp = w.Add<PlayerInputComponent>(e);
            inp.playerIndex = (sp.padIndex >= 0) ? sp.padIndex : 0;

            // 移動意図
            w.Add<MovementIntentComponent>(e);

            // 物理
            auto& rb = w.Add<Rigidbody2DComponent>(e);
            rb.useGravity = true;

            // モデル描画
            auto& mr = w.Add<ModelRendererComponent>(e);

            // ───────────────────────────
            // モデルの選び方
            // 1) SpawnParamsがモデル名を持っていたらそれを使う
            // 2) なければ padIndex で分ける
            // ───────────────────────────
            const char* modelName = nullptr;

            if (!sp.modelAlias.empty())
            {
                // 呼び出し側が「これ使って！」と指定したパターン
                modelName = sp.modelAlias.c_str();
            }
            else
            {
                // 指定が無いときはプレイヤー番号で分ける
                if (inp.playerIndex == 0)
                {
                    modelName = kDefaultPlayerModel1P;
                }
                else if (inp.playerIndex == 1)
                {
                    modelName = kDefaultPlayerModel2P;
                }
                else
                {
                    modelName = kFallbackModel;
                }
            }

            mr.model = AssetManager::GetModel(modelName ? modelName : kFallbackModel);

            mr.visible = true;
			mr.localScale = { 1.f, 1.25f, 0.95f }; // 適当な大きさに調整
			mr.localOffset = { 0.f, 0.f, 0.f }; // 足元を原点に合わせる

            // レイヤー
            mr.layer = 0; // プレイヤーの基準は「0」

            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            }

            // 当たり判定は前回と同じ例
            auto& col = w.Add<Collider2DComponent>(e);
            col.shape = ColliderShapeType::AABB2D;
            col.aabb.halfX = tr.scale.x;
            col.aabb.halfY = tr.scale.y;
            col.layer = Physics::LAYER_PLAYER;
            col.hitMask = Physics::LAYER_GROUND || Physics::LAYER_GOAL;
            col.isStatic = false;
            col.offset = { 0.0f, 1.f };

            return e;
        }
    );
}
