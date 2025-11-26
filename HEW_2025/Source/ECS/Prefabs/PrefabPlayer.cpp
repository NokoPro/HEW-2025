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

static const char* kDefaultPlayerModel1P = "mdl_player_1p";
static const char* kDefaultPlayerModel2P = "mdl_player_2p";
static const char* kFallbackModel = "mdl_slime";

using AnimeNo = Model::AnimeNo;
// モデルごとに共有したいアニメ番号を static でキャッシュ
static AnimeNo s_playerIdleAnime = Model::ANIME_NONE;
static AnimeNo s_playerRunAnime = Model::ANIME_NONE;
static AnimeNo s_playerJumpAnime = Model::ANIME_NONE;
static AnimeNo s_playerFallAnime = Model::ANIME_NONE;

// Data.csv 側の aliases と対応
static const char* kAnimIdleAlias = "anim_player_idle";
static const char* kAnimRunAlias = "anim_player_run";
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
            mr.model = AssetManager::GetModel("mdl_player");   // Data.csv 側で登録済みの想定
            mr.localScale = { 1.25f, 0.5f, 1.25f }; // スケール調整
            mr.localOffset = { 0.f, -0.5f, 0.f }; // 足元を原点に合わせる
            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_ANIME));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

                // 初回だけアニメを追加して AnimeNo をキャッシュ
                if (s_playerIdleAnime == Model::ANIME_NONE)
                {
                    auto idlePath = AssetManager::ResolveAnimationPath(kAnimIdleAlias);
                    s_playerIdleAnime = mr.model->AddAnimation(idlePath.c_str());

                    auto runPath = AssetManager::ResolveAnimationPath(kAnimRunAlias);
                    s_playerRunAnime = mr.model->AddAnimation(runPath.c_str());

                    auto jumpPath = AssetManager::ResolveAnimationPath(kAnimJumpAlias);
                    s_playerJumpAnime = mr.model->AddAnimation(jumpPath.c_str());

                    auto fallPath = AssetManager::ResolveAnimationPath(kAnimFallAlias);
                    s_playerFallAnime = mr.model->AddAnimation(fallPath.c_str());
                }
            }

            // アニメ制御コンポーネント
            auto& anim = w.Add<ModelAnimationComponent>(e);
            anim.animeNo = (s_playerIdleAnime != Model::ANIME_NONE)
                ? static_cast<int>(s_playerIdleAnime)
                : -1;
            anim.loop = true;
            anim.speed = 1.0f;
            anim.playRequested = (anim.animeNo >= 0);

            // ステートテーブル
            auto& table = w.Add<ModelAnimationTableComponent>(e);
            // まず全クリップ無効化
            for (auto& d : table.table)
            {
                d.animeNo = -1;
                d.loop = true;
                d.speed = 1.0f;
            }

            if (s_playerIdleAnime != Model::ANIME_NONE)
            {
                auto& d = table.table[static_cast<size_t>(ModelAnimState::Idle)];
                d.animeNo = static_cast<int>(s_playerIdleAnime);
                d.loop = true;
                d.speed = 1.0f;
            }
            if (s_playerRunAnime != Model::ANIME_NONE)
            {
                auto& d = table.table[static_cast<size_t>(ModelAnimState::Run)];
                d.animeNo = static_cast<int>(s_playerRunAnime);
                d.loop = true;
                d.speed = 1.2f;
            }
            if (s_playerJumpAnime != Model::ANIME_NONE)
            {
                auto& d = table.table[static_cast<size_t>(ModelAnimState::Jump)];
                d.animeNo = static_cast<int>(s_playerJumpAnime);
                d.loop = false;   // ジャンプ開始を1回再生とか
                d.speed = 0.05f;
            }
            if (s_playerFallAnime != Model::ANIME_NONE)
            {
                auto& d = table.table[static_cast<size_t>(ModelAnimState::Fall)];
                d.animeNo = static_cast<int>(s_playerFallAnime);
                d.loop = false;
                d.speed = 0.05f;
            }

            // ステート現在値
            auto& state = w.Add<ModelAnimationStateComponent>(e);
            state.current = ModelAnimState::Idle;
            state.requested = ModelAnimState::Idle;

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
