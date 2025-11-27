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
#include "ECS/Tag/Tag.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"

#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"
#include "System/Model.h"

using AnimeNo = Model::AnimeNo;

// Data.csv 側の aliases と対応
static const char* kAnimIdleAlias = "anim_player_idle";
static const char* kAnimRunAlias  = "anim_player_run";
static const char* kAnimJumpAlias = "anim_player_jump";
static const char* kAnimFallAlias = "anim_player_fall";

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

            //プレイヤーであることを示すタグを付与
            w.Add<TagPlayer>(e);

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

            auto& mr = w.Add<ModelRendererComponent>(e);
            // 共有キャッシュを使わず、毎回独立インスタンスを生成してアニメ状態の共有を防ぐ
            const std::string modelAlias = sp.modelAlias.empty() ? std::string("mdl_2Pplayer") : sp.modelAlias;
            mr.model = AssetManager::CreateModelInstance(modelAlias);
            mr.localScale = { .7f, 0.35f, .7f }; // スケール調整
            mr.localOffset = { 0.f, 0.4f, 0.f }; // 足元を原点に合わせる
            
            // このモデルインスタンスに対して個別にアニメを追加
            AnimeNo idleNo = Model::ANIME_NONE;
            AnimeNo runNo  = Model::ANIME_NONE;
            AnimeNo jumpNo = Model::ANIME_NONE;
            AnimeNo fallNo = Model::ANIME_NONE;

            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

                const auto idlePath = AssetManager::ResolveAnimationPath(kAnimIdleAlias);
                idleNo = mr.model->AddAnimation(idlePath.c_str());

                const auto runPath = AssetManager::ResolveAnimationPath(kAnimRunAlias);
                runNo = mr.model->AddAnimation(runPath.c_str());

                const auto jumpPath = AssetManager::ResolveAnimationPath(kAnimJumpAlias);
                jumpNo = mr.model->AddAnimation(jumpPath.c_str());

                const auto fallPath = AssetManager::ResolveAnimationPath(kAnimFallAlias);
                fallNo = mr.model->AddAnimation(fallPath.c_str());
            }

            // アニメ制御コンポーネント
            //auto& anim = w.Add<ModelAnimationComponent>(e);
            //anim.animeNo = (idleNo != Model::ANIME_NONE) ? static_cast<int>(idleNo) : -1;
            //anim.loop = true;
            //anim.speed = 1.0f;
            //anim.playRequested = (anim.animeNo >= 0);

            //// ステートテーブル
            //auto& table = w.Add<ModelAnimationTableComponent>(e);
            //// まず全クリップ無効化
            //for (auto& d : table.table)
            //{
            //    d.animeNo = -1;
            //    d.loop = true;
            //    d.speed = 1.0f;
            //}

            //if (idleNo != Model::ANIME_NONE)
            //{
            //    auto& d = table.table[static_cast<size_t>(ModelAnimState::Idle)];
            //    d.animeNo = static_cast<int>(idleNo);
            //    d.loop = true;
            //    d.speed = 1.0f;
            //}
            //if (runNo != Model::ANIME_NONE)
            //{
            //    auto& d = table.table[static_cast<size_t>(ModelAnimState::Run)];
            //    d.animeNo = static_cast<int>(runNo);
            //    d.loop = true;
            //    d.speed = 0.01f;
            //}
            //if (jumpNo != Model::ANIME_NONE)
            //{
            //    auto& d = table.table[static_cast<size_t>(ModelAnimState::Jump)];
            //    d.animeNo = static_cast<int>(jumpNo);
            //    d.loop = false;   // ジャンプ開始を1回再生とか
            //    d.speed = 0.05f;
            //}
            //if (fallNo != Model::ANIME_NONE)
            //{
            //    auto& d = table.table[static_cast<size_t>(ModelAnimState::Fall)];
            //    d.animeNo = static_cast<int>(fallNo);
            //    d.loop = false;
            //    d.speed = 0.05f;
            //}

            //// ステート現在値
            //auto& state = w.Add<ModelAnimationStateComponent>(e);
            //state.current = ModelAnimState::Idle;
            //state.requested = ModelAnimState::Idle;

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
