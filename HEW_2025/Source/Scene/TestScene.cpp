/*****************************************************************//**
 * @file   TestScene.cpp
 * @brief  ECSを試すためのテストシーン実装
 * - プレイヤー操作、物理、当たり判定、カメラ、描画などを統合した簡易ステージ
 * - 2人同時プレイ対応
 * - シンプルなゴール判定とDeathゾーンも実装
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "TestScene.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/World.h"
#include "ECS/Tag/Tag.h"

 /// ECS コンポーネント群
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Render/FollowerComponent.h"

/// ECS システム群
#include "ECS/Systems/Update/Physics/PhysicsStepSystem.h"
#include "ECS/Systems/Update/Game/GoalSystem.h"
#include "ECS/Systems/Update/Game/DeathZoneSystem.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Render/FollowerSystem.h"
#include "ECS/Systems/Render/PlayerUISystem.h"

/// 入力・物理関連コンポーネント
#include "System/CameraHelper.h"
#include "System/DirectX/ShaderList.h"
#include "System/Defines.h"
#include "System/CameraMath.h"

/// プレファブ群
#include "ECS/Prefabs/PrefabPlayer.h"
#include "ECS/Prefabs/PrefabFloor.h"
#include "ECS/Prefabs/PrefabWall.h"
#include "ECS/Prefabs/PrefabStaticBlock.h"
#include "ECS/Prefabs/PrefabGoal.h"
#include "ECS/Prefabs/PrefabDeathZone.h"
#include "ECS/Prefabs/PrefabFollower.h"
#include "ECS/Prefabs/PrefabFollowerJump.h"
#include "ECS/Prefabs/PrefabFollowerBlink.h"

#include <cstdio>
#include <DirectXMath.h>
#include <Windows.h> // For MessageBox


using namespace DirectX;

// テストステージ作成
namespace
{
	/// @brief ステージの床・壁・ブロックを作成する
	void MakeStage(World& world, PrefabRegistry& prefabs)
	{
        // 最初の床
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 0.0f, 39.0f, 0.0f };
            sp.scale = { 10.0f, 1.0f, 1.0f };   // 横長の床
            sp.rotationDeg = { 0.f,0.f,0.f };
            prefabs.Spawn("Floor", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 0.0f, -15.0f, 0.0f };
            sp.scale = { 35.0f, 6.0f, 1.0f };   // 横長の床
            prefabs.Spawn("Floor", world, sp);
        }

		// 壁
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 0.0f, 8.0f, 0.0f };
            sp.scale = { 0.5f, 30.0f, 1.0f };   // 細長い柱 → 壁にしてもOK
            prefabs.Spawn("Wall", world, sp);
        }


		// 静的ブロック
		// 1P側
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -17.0f, -5.0f, 0.0f };
            sp.scale = { 2.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -23.0f, 3.0f, 0.0f };
            sp.scale = { 2.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -10.0f, 10.5f, 0.0f };
            sp.scale = { 1.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -2.5f, 18.5f, 0.0f };
            sp.scale = { 2.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -11.0f, 26.0f, 0.0f };
            sp.scale = { 10.5f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -22.f, 21.5f, 0.0f };
            sp.scale = { 3.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { -19.0f, 33.0f, 0.0f };
            sp.scale = { 0.5f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }

		// 2P側
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 15.0f, 0.0f, 0.0f };
            sp.scale = { 2.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
		}
		{
			PrefabRegistry::SpawnParams sp;
			sp.position = { 24.0f, 8.0f, 0.0f };
			sp.scale = { 1.0f, 0.5f, 1.0f };
			sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
			prefabs.Spawn("StaticBlock", world, sp);
		}
		{
			PrefabRegistry::SpawnParams sp;
			sp.position = { 3.5f, 2.0f, 0.0f };
			sp.scale = { 3.0f, 0.5f, 1.0f };
			sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
			prefabs.Spawn("StaticBlock", world, sp);
		}
		{
			PrefabRegistry::SpawnParams sp;
			sp.position = { 24.0f, 15.5f, 0.0f };
			sp.scale = { 1.0f, 0.5f, 1.0f };
			sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
			prefabs.Spawn("StaticBlock", world, sp);
		}
		{
			PrefabRegistry::SpawnParams sp;
			sp.position = { 2.5f, 17.5f, 0.0f };
		 sp.scale = { 2.0f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }
        {
			PrefabRegistry::SpawnParams sp;
			sp.position = { 2.5f, 25.5f, 0.0f };
			sp.scale = { 2.0f, 0.5f, 1.0f };
			sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
			prefabs.Spawn("StaticBlock", world, sp);
		}
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 12.0f, 31.0f, 0.0f };
            sp.scale = { 0.5f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
		}
		{
			PrefabRegistry::SpawnParams sp;
			sp.position = { 23.f, 26.5f, 0.0f };
			sp.scale = { 3.0f, 0.5f, 1.0f };
			sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
			prefabs.Spawn("StaticBlock", world, sp);
		}
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 20.0f, 38.0f, 0.0f };
            sp.scale = { 0.5f, 0.5f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 11.0f, 43.0f, 0.0f };
            sp.scale = { 1.f, 5.f, 1.0f };
            sp.modelAlias = "mdl_ground"; // 別のモデルにしたかったらここだけ変える
            prefabs.Spawn("StaticBlock", world, sp);
        }
	}
}

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------
TestScene::TestScene()
{
    RegisterPlayerPrefab(m_prefabs);
    RegisterFloorPrefab(m_prefabs);
    RegisterWallPrefab(m_prefabs);
    RegisterStaticBlockPrefab(m_prefabs);
    RegisterGoalPrefab(m_prefabs); // ゴールプレハブ登録
    RegisterDeathZonePrefab(m_prefabs); // Deathゾーンプレハブ登録
    RegisterFollowerPrefab(m_prefabs); // 旧汎用フォロワープレハブ(後方互換)
    RegisterFollowerJumpPrefab(m_prefabs);  // 新: ジャンプUI専用フォロワー
    RegisterFollowerBlinkPrefab(m_prefabs); // 新: ブリンクUI専用フォロワー

    //
    // 1. アセット取得
    //
    m_playerModel = AssetManager::GetModel("mdl_slime");
    if (m_playerModel)
    {
        m_playerModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
        m_playerModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
    }
    m_groundModel = AssetManager::GetModel("mdl_ground");
    if (m_groundModel)
    {
        m_groundModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
        m_groundModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
    }

    //
    // 2. System登録（順番が重要）
    //
    // 2-1 入力 → Intent
    m_sys.AddUpdate<PlayerInputSystem>();
    m_sys.AddUpdate<PlayerUISystem>();

    // 2-2 Intent → Rigidbody（加速・ジャンプ・重力）
    m_sys.AddUpdate<MovementApplySystem>();
    // 2-2.5 Rigidbody の速度を位置に反映する物理ステップ（重力で変化した速度を位置へ適用）
    m_sys.AddUpdate<PhysicsStepSystem>(&m_colBuf);
    // 2-3.1 当たり判定（めり込み解消＆イベント）
    auto* colSys = &m_sys.AddUpdate<Collision2DSystem>(&m_colBuf);

	// 2-3.2 ゴール判定システム追加
    m_sys.AddUpdate<GoalSystem>(colSys); // GoalSystemを追加
    
	// 2-3.3 Deathゾーン判定システム追加
    m_sys.AddUpdate<DeathZoneSystem>(colSys); // Deathゾーンシステム追加

    // 追加：追従システム
    // 物理演算が終わった後の座標をもとに追従させる
    m_sys.AddUpdate<FollowerSystem>();

    // 2-4 カメラ（最終位置を見たいので最後）
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

    
    // 2-5 描画
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();

    // 2Dスプライト描画システム（UI等）
    m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();

    m_debugCollision = &m_sys.AddRender<CollisionDebugRenderSystem>();

    // テスト用UIスプライトを1つ生成 (Assets/Data.csv に alias "icon" が登録されている想定)
    //{
    //    EntityId ui = m_world.Create();
    //    if (ui != kInvalidEntity)
    //    {
    //        m_world.Add<TransformComponent>(ui, XMFLOAT3{ 0.0f, 20.0f, 0.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 1.0f, 1.0f, 1.0f });
    //        Sprite2DComponent sp;
    //        sp.alias = "tex_background"; // alias must exist in Assets/Data.csv

    //        sp.width = 70.0f;
    //        sp.height = 50.0f;
    //        sp.originX = 0.5f;
    //        sp.originY = 0.6f;
    //        m_world.Add<Sprite2DComponent>(ui, sp);
    //    }
    //}

    //
    // 3. ステージ作成
    //
	MakeStage(m_world, m_prefabs);

    // ゴールオブジェクト生成
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 0.0f, 43.0f, 0.0f }; // ステージ上部など適当な位置
        sp.scale = { 2.0f, 2.0f, 2.0f };
        m_goalEntity = m_prefabs.Spawn("Goal", m_world, sp);
    }



    //
    // 4. プレイヤー生成（カメラのターゲット）
    // プレイヤー1 + UI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -10.0f, -10.0f, 0.0f };
        sp.rotationDeg = { 0.0f, 120.0f, 0.0f };
        sp.scale = { 1.f, 1.f, 1.f };
        sp.padIndex = 0;                         // 1P
        sp.modelAlias = "mdl_slime";           

        m_playerEntity = m_prefabs.Spawn("Player", m_world, sp);

        // --- 1P用ジャンプUIフォロワー生成 ---
        {
            PrefabRegistry::SpawnParams spF; spF.position = sp.position;
            EntityId jumpUI = m_prefabs.Spawn("FollowerJump", m_world, spF);
            if (m_world.Has<FollowerComponent>(jumpUI))
            {
                m_world.Get<FollowerComponent>(jumpUI).targetId = m_playerEntity; // 1Pに追従
            }
        }
        // --- 1P用ブリンクUIフォロワー生成 ---
        {
            PrefabRegistry::SpawnParams spF; spF.position = sp.position;
            EntityId blinkUI = m_prefabs.Spawn("FollowerBlink", m_world, spF);
            if (m_world.Has<FollowerComponent>(blinkUI))
            {
                m_world.Get<FollowerComponent>(blinkUI).targetId = m_playerEntity; // 1Pに追従
            }
        }
    } // <-- 1P ブロック閉じ

    // プレイヤー2 + UI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 10.0f, -10.0f, 0.0f };
        sp.rotationDeg = { 0.0f, 120.0f, 0.0f };
        sp.scale = { 1.f, 1.f, 1.f };
        sp.padIndex = 1;                         // 2P
        sp.modelAlias = "mdl_slime";           // ← 2P用モデル

        m_playerEntity2 = m_prefabs.Spawn("Player", m_world, sp);

        // --- 2P用ジャンプUIフォロワー生成 ---
        {
            PrefabRegistry::SpawnParams spF; spF.position = sp.position;
            EntityId jumpUI = m_prefabs.Spawn("FollowerJump", m_world, spF);
            if (m_world.Has<FollowerComponent>(jumpUI))
            {
                m_world.Get<FollowerComponent>(jumpUI).targetId = m_playerEntity2; // 2Pに追従
            }
        }
        // --- 2P用ブリンクUIフォロワー生成 ---
        {
            PrefabRegistry::SpawnParams spF; spF.position = sp.position;
            EntityId blinkUI = m_prefabs.Spawn("FollowerBlink", m_world, spF);
            if (m_world.Has<FollowerComponent>(blinkUI))
            {
                m_world.Get<FollowerComponent>(blinkUI).targetId = m_playerEntity2; // 2Pに追従
            }
        }
    } // <-- 2P ブロック閉じ

    // Deathゾーン生成（画面下部に設置）
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 0.0f, -30.0f, 0.0f }; // ステージ下部
        sp.scale = { 60.0f, kDeathZoneHalfHeight, 1.0f };     // 横幅広め
        m_prefabs.Spawn("DeathZone", m_world, sp);
    }

    // 左右の壁
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 26.0f, 25.0f, 0.0f };
        sp.scale = { 1.0f, 50.0f, 1.0f };
        m_prefabs.Spawn("Wall", m_world, sp);
    }
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -26.0f, 25.0f, 0.0f };
        sp.scale = { 1.0f, 50.0f, 1.0f };
        m_prefabs.Spawn("Wall", m_world, sp);
    }

    //
    // 5. カメラエンティティ生成
    //
    {
        EntityId camEnt = m_world.Create();

        // カメラとして認識させるタグ
        m_world.Add<ActiveCameraTag>(camEnt);

        // 位置
        auto& tr = m_world.Add<TransformComponent>(
            camEnt,
            XMFLOAT3{ 0.0f, 0.f, -16.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 1.0f, 1.0f, 1.0f }
        );

        // カメラ本体（制御情報もここに統合した版）
        auto& cam = m_world.Add<Camera3DComponent>(camEnt);
        cam.mode = Camera3DComponent::Mode::SideScroll;     // ←今回のモード
        cam.target = m_playerEntity;                          // 追従先

        // 基本パラメータ
        cam.aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        cam.nearZ = 0.1f;
        cam.farZ = 10.0f;
        cam.fovY = 20.0f;

        // サイドビュー用の固定値
        cam.scrollSpeed = kDeathZoneSpeedY;   // 好きな速度に調整
        cam.followOffsetY = 1.5f;
        cam.followMarginY = 0.5f;
        cam.sideFixedX = 0.0f;
        cam.sideFixedZ = -85.0f;
        cam.sideLookAtX = 0.0f;
        cam.lookAtOffset = XMFLOAT3{ 0.0f, 0.0f, 0.0f };
        cam.orthoHeight = 30.0f;
    }

    //
    // 6. ライト
    //
    ModelRenderSystem::ApplyDefaultLighting(2.75f, 6.0f);
}


//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------
TestScene::~TestScene()
{

    m_world.Destroy(1);
}

//----------------------------------------------------------
// 毎フレーム更新
//----------------------------------------------------------
void TestScene::Update()
{
	// 固定フレームレート想定で更新
    const float dt = 1.0f / 60.0f;
    m_sys.Tick(m_world, dt);
}

//----------------------------------------------------------
// 毎フレーム描画
//----------------------------------------------------------
void TestScene::Draw()
{
	// カメラのビュー・プロジェクション行列をセット
    if (m_followCamera)
    {
        const auto& V = m_followCamera->GetView();
        const auto& P = m_followCamera->GetProj();

        if (m_drawModel)
            m_drawModel->SetViewProj(V, P);
        if (m_drawSprite)
            m_drawSprite->SetViewProj(V, P);
        if (m_debugCollision)
            m_debugCollision->SetViewProj(V, P);
    }

	// モデル描画
    m_sys.Render(m_world);
}
