/*****************************************************************//**
 * @file   GameScene.cpp
 * @brief  本番ゲームプレイ用シーン実装
 *
 * @author 浅野勇生
 * @date   2025/11/28
 *********************************************************************/
#include "GameScene.h"

  // システム・ECS関連
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/World.h"
#include "ECS/Tag/Tag.h"
#include "System/StageLoader.h"

// コンポーネント群
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Render/FollowerComponent.h"
#include "ECS/Components/Render/BackGroundComponent.h"

// 背景ワープ
#include "ECS/Components/Game/BackGroundScrollComponent.h"

// システム群
#include "ECS/Systems/Update/Physics/PhysicsStepSystem.h"
#include "ECS/Systems/Update/Game/GoalSystem.h"
#include "ECS/Systems/Update/Game/DeathZoneSystem.h"
#include "ECS/Systems/Update/Gimick/MovingPlatformSystem.h"
#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Render/FollowerSystem.h"
#include "ECS/Systems/Render/PlayerUISystem.h"
#include "ECS/Systems/Render/TimerSystem.h"
#include "ECS/Systems/Render/BackGroundRenderSystem.h"
#include "ECS/Systems/Update/Anim/ModelAnimationSystem.h"
#include "ECS/Systems/Update/Effect/EffectSystem.h"
#include "ECS/Systems/Update/Anim/ModelAnimationStateSystem.h"
#include "ECS/Systems/Render/GameOverUISystem.h"
#include "ECS/Systems/Update/Anim/PlayerLocomotionStateSystem.h"
#include "ECS/Systems/Update/Anim/PlayerPresentationSystem.h"
#include "ECS/Systems/Update/Core/CountdownUISystem.h"

// 背景ワープ
#include "ECS/Systems/Update/Game/BackGroundScrollSystem.h"

// その他ツール
#include "System/CameraHelper.h"
#include "System/DirectX/ShaderList.h"
#include "System/Defines.h"
#include "System/CameraMath.h"
#include "System/RankingManager.h"
#include "System/TimeAttackManager.h"
#include "SceneAPI.h"
#include "ResultScene.h"

// プレハブ群
#include "ECS/Prefabs/PrefabPlayer.h"
#include "ECS/Prefabs/PrefabFloor.h"
#include "ECS/Prefabs/PrefabWall.h"
#include "ECS/Prefabs/PrefabStaticBlock.h"
#include "ECS/Prefabs/PrefabGameOver.h"
#include "ECS/Prefabs/PrefabGoal.h"
#include "ECS/Prefabs/PrefabDeathZone.h"
#include "ECS/Prefabs/PrefabMovingPlatform.h"
#include "ECS/Prefabs/PrefabFollower.h"
#include "ECS/Prefabs/PrefabFollowerJump.h"
#include "ECS/Prefabs/PrefabFollowerBlink.h"
#include "ECS/Prefabs/PrefabTimer.h"
#include "ECS/Prefabs/PrefabBackGround.h"
#include "ECS/Prefabs/PrefabWhiteUI.h"
#include "ECS/Prefabs/PrefabCountdownUI.h"

#include <cstdio>
#include <string>
#include <DirectXMath.h>
#include <Windows.h>
#include "StageSelectScene.h"

GameScene::GameScene(int stageNo, Difficulty difficulty)
    : m_stageNo(stageNo), m_difficulty(difficulty)
{
    Initialize();
}

GameScene::~GameScene()
{
    AudioManager::StopBGM();
}

void GameScene::Initialize()
{
    // -------------------------------------------------------
    // 0. プレハブ登録
    // -------------------------------------------------------
    RegisterPlayerPrefab(m_prefabs);
    RegisterFloorPrefab(m_prefabs);
    RegisterWallPrefab(m_prefabs);
    RegisterStaticBlockPrefab(m_prefabs);
    RegisterGameOverPrefab(m_prefabs);
    RegisterGoalPrefab(m_prefabs);
    RegisterDeathZonePrefab(m_prefabs);
    RegisterMovingPlatformPrefab(m_prefabs);
    RegisterFollowerJumpPrefab(m_prefabs);
    RegisterFollowerBlinkPrefab(m_prefabs);
    RegisterTimerPrefab(m_prefabs);
    RegisterBackGroundPrefab(m_prefabs);
    RegisteWhiteUIPrefab(m_prefabs);
	RegisterCountdownUIPrefab(m_prefabs);

    // -------------------------------------------------------
    // 1. アセット取得
    // -------------------------------------------------------
    m_playerModel = AssetManager::GetModel("mdl_2Pplayer");
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

    // -------------------------------------------------------
    // 2. System登録
    // -------------------------------------------------------
    m_sys.AddUpdate<MovingPlatformSystem>();
    m_sys.AddUpdate<PlayerInputSystem>();
    m_sys.AddUpdate<PlayerUISystem>();
    m_sys.AddUpdate<MovementApplySystem>();
    m_sys.AddUpdate<PhysicsStepSystem>(&m_colBuf);

    auto* colSys = &m_sys.AddUpdate<Collision2DSystem>(&m_colBuf);
    m_goalSystem = &m_sys.AddUpdate<GoalSystem>(colSys);

    // DeathZoneSystem登録と難易度パラメータ設定
    m_deathSystem = &m_sys.AddUpdate<DeathZoneSystem>(colSys);

    // 難易度に応じた上昇速度設定
    float deathSpeed = 1.0f; // Default (Normal)
    switch (m_difficulty)
    {
    case Difficulty::Easy:   deathSpeed = 0.5f; break;
    case Difficulty::Normal: deathSpeed = 1.0f; break;
    case Difficulty::Hard:   deathSpeed = 2.0f; break;
    }
    m_deathSystem->SetRiseSpeed(deathSpeed);
	m_sys.AddUpdate<PlayerLocomotionStateSystem>();
    m_sys.AddUpdate<ModelAnimationStateSystem>();
    m_sys.AddUpdate<ModelAnimationSystem>();
	m_sys.AddUpdate<PlayerPresentationSystem>();
    m_sys.AddUpdate<EffectSystem>();
    m_sys.AddUpdate<FollowerSystem>();
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

    // 背景ワープシステム
    m_sys.AddUpdate<BackGroundScrollSystem>();

    m_sys.AddUpdate<TimerSystem>();
	m_sys.AddUpdate<CountdownUISystem>();
    m_sys.AddUpdate<AudioPlaySystem>();

    RankingManager::Get().Load("Assets/Ranking.csv");

    m_drawBackGround = &m_sys.AddRender<BackGroundRenderSystem>();
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();
#ifdef _DEBUG | _IMGUI_ENABLED
    m_debugCollision = &m_sys.AddRender<CollisionDebugRenderSystem>();
#endif

    // -------------------------------------------------------
    // 3. ステージロード (JSON)
    // -------------------------------------------------------
    {
        StageLoader loader;

        // 難易度とステージ番号からファイル名を生成
        std::string diffStr;
        switch (m_difficulty)
        {
        case Difficulty::Easy:   diffStr = "Easy"; break;
        case Difficulty::Normal: diffStr = "Normal"; break;
        case Difficulty::Hard:   diffStr = "Hard"; break;
        }

        // ファイルパス生成
        std::string jsonPath = "Assets/Stages/Stage" + std::to_string(m_stageNo) + "_" + diffStr + ".json";

        bool loadSuccess = loader.Load(jsonPath, m_world, m_prefabs);
        if (!loadSuccess)
        {
            loader.Load("Assets/Stages/TestStage2.json", m_world, m_prefabs);
        }
    }

    // -------------------------------------------------------
    // 4. 固定エンティティ生成
    // -------------------------------------------------------
    // 背景
    {
        PrefabRegistry::SpawnParams sp;
        m_prefabs.Spawn("BackGround", m_world, sp);
    }

    // 左右の壁
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 0.0f, 30.0f, 0.0f };
        sp.scale = { 2.0f, 50.0f, 4.0f };
        m_prefabs.Spawn("Wall", m_world, sp);
    }
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 70.0f, 30.0f, 0.0f };
        sp.scale = { 2.0f, 50.0f, 4.0f };
        m_prefabs.Spawn("Wall", m_world, sp);
    }
    // 真ん中の壁
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 35.0f, 16.0f, 0.0f };
        sp.scale = { 1.0f, 44.0f, 4.0f };
        m_prefabs.Spawn("Wall", m_world, sp);
    }
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 35.0f, -5.0f, 0.0f };
        sp.scale = { 40.0f, 7.0f, 1.0f };   // 横長の床
        sp.rotationDeg = { 0.f,0.f,0.f };
        m_prefabs.Spawn("Floor", m_world, sp);
    }

    // --- DeathZone ---
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 35.0f, -30.0f, 0.0f };
        sp.scale = { 60.0f, kDeathZoneHalfHeight, 4.0f };
        sp.rotationDeg = { 0.f,180.f,0.f };
        m_prefabs.Spawn("DeathZone", m_world, sp);
    }



    // --- プレイヤー生成 ---
    // 1P
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 10.0f, 2.0f, -20.0f };
        sp.rotationDeg = { 0.0f, 180.0f, 0.0f };
        sp.scale = { 1.f, 2.f, 1.f };
        sp.padIndex = 0;
        sp.modelAlias        = "mdl_1Pplayer";
		sp.animIdleAlias     = "anim_player1P_idle";
		sp.animRunLeftAlias  = "anim_player1P_run_left";
		sp.animRunRightAlias = "anim_player1P_run_right";
		sp.animJumpAlias     = "anim_player1P_jump";
		sp.animFallAlias     = "anim_player1P_fall";
		sp.animWalkAlias     = "anim_player1P_walk";
		sp.animLandAlias     = "anim_player1P_land";

        m_playerEntity1 = m_prefabs.Spawn("Player", m_world, sp);

        PrefabRegistry::SpawnParams spF; spF.position = sp.position;
        EntityId jumpUI = m_prefabs.Spawn("FollowerJump", m_world, spF);
        if (m_world.Has<FollowerComponent>(jumpUI)) m_world.Get<FollowerComponent>(jumpUI).targetId = m_playerEntity1;

        EntityId blinkUI = m_prefabs.Spawn("FollowerBlink", m_world, spF);
        if (m_world.Has<FollowerComponent>(blinkUI)) m_world.Get<FollowerComponent>(blinkUI).targetId = m_playerEntity1;
    }

    // 2P
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 60.0f, 2.0f, -20.0f };
        sp.rotationDeg = { 0.0f, 180.0f, 0.0f };
        sp.scale = { 1.f, 2.f, 1.f };
        sp.padIndex = 1;
        sp.modelAlias        = "mdl_2Pplayer";
		sp.animIdleAlias     = "anim_player2P_idle";
		sp.animRunLeftAlias  = "anim_player2P_run_left";
		sp.animRunRightAlias = "anim_player2P_run_right";
		sp.animJumpAlias     = "anim_player2P_jump";
		sp.animFallAlias     = "anim_player2P_fall";
		sp.animWalkAlias     = "anim_player2P_walk";
		sp.animLandAlias     = "anim_player2P_land";

        m_playerEntity2 = m_prefabs.Spawn("Player", m_world, sp);

        PrefabRegistry::SpawnParams spF; spF.position = sp.position;
        EntityId jumpUI = m_prefabs.Spawn("FollowerJump", m_world, spF);
        if (m_world.Has<FollowerComponent>(jumpUI)) m_world.Get<FollowerComponent>(jumpUI).targetId = m_playerEntity2;

        EntityId blinkUI = m_prefabs.Spawn("FollowerBlink", m_world, spF);
        if (m_world.Has<FollowerComponent>(blinkUI)) m_world.Get<FollowerComponent>(blinkUI).targetId = m_playerEntity2;
    }

    // タイマーUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 5.0f, 3.0f, 0.0f };
        sp.scale = { 2.0f, 2.0f, 1.0f };
        m_prefabs.Spawn("Timer", m_world, sp);
    }

    // GameOverUI
    {
        if (m_deathSystem)
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 35.0f, 10.0f, 0.0f };
            sp.scale = { 1.0f, 1.0f, 1.0f };
            m_prefabs.Spawn("GameOver", m_world, sp);
        }
    }

    //白いぼかしUI
    {
       
        if (m_deathSystem)
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = { 35.0f, 18.0f, 0.0f };
            sp.scale = { 4.0f, 4.0f, 1.0f };
            m_prefabs.Spawn("WhiteUI", m_world, sp);
        }
    }

    // -------------------------------------------------------
    // 5. カメラ設定
    // -------------------------------------------------------
    {
        EntityId camEnt = m_world.Create();
        m_world.Add<ActiveCameraTag>(camEnt);
        auto& tr = m_world.Add<TransformComponent>(
            camEnt,
            DirectX::XMFLOAT3{ 0.0f, 8.f, -10.0f },
            DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f }
        );

        auto& cam = m_world.Add<Camera3DComponent>(camEnt);
        cam.mode = Camera3DComponent::Mode::SideScroll;
        cam.target = m_playerEntity1;
        cam.aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        cam.nearZ = 0.1f;
        cam.farZ = 300.0f;
        cam.fovY = 20.0f;

        cam.scrollSpeed = m_deathSystem ? deathSpeed : 1.0f;

        cam.followOffsetY = 1.5f;
        cam.followMarginY = 0.5f;
        cam.sideFixedZ = -185.0f;
        cam.orthoHeight = 39.5f;
        cam.sideFixedX = (cam.orthoHeight * cam.aspect) * 0.5f;
        cam.sideLookAtX = 0.0f;
        cam.lookAtOffset = DirectX::XMFLOAT3{ 0.0f, 8.0f, 0.0f };
    }

    // -------------------------------------------------------
    // 6. ゲーム開始準備
    // -------------------------------------------------------
    // タイムアタックリセット＆カウントダウン開始
    TimeAttackManager::Get().Reset();
    TimeAttackManager::Get().StartCountdown(3.0f); // 3秒前

     AudioManager::LoadAudioAlias("bgm_main");
     AudioManager::SetBGMVolume(0.1f);

     AudioManager::LoadAudioAlias("se_jump_p1");

     // 4. 固定エンティティ生成 へ追記（UIの位置・スケールは好みで）
     {
         PrefabRegistry::SpawnParams sp;
         sp.position = { 35.0f, 10.0f, -50.0f };
         sp.rotationDeg = { 0,0,0 };
         sp.scale = { 4.0f, 4.0f, 1.0f };
         m_prefabs.Spawn("CountdownUI", m_world, sp);
     }
}

void GameScene::Update()
{
    const float dt = 1.0f / 60.0f;
    // 1. タイムアタック計測更新
    auto state = TimeAttackManager::Get().GetState();
    TimeAttackManager::Get().Update();

    // ステート遷移を検出
    if (m_prevState != state) 
    {
        if (state == TimeAttackManager::State::Running) 
        {
            // ゲーム開始で再生
            AudioManager::PlayBGM("bgm_main", true);
        }
        else 
        {
            // 終了・待機・カウントダウンに遷移したら停止
            AudioManager::StopBGM();
        }
        m_prevState = state;
    }

    // 2. システム更新の選別
    // カウントダウン中 (State::Countdown) または 待機中 (Ready) は
    // ゲームプレイに関わるシステムを止める
    bool isPlaying = (state == TimeAttackManager::State::Running);

    if (isPlaying)
    {
        m_sys.Tick(m_world, dt);
    }
    else
    {
        // アニメーション、カメラ、タイマー、オーディオのみ更新
        if (auto* sys = m_sys.GetUpdate<FollowCameraSystem>())      sys->Update(m_world, dt);
        if (auto* sys = m_sys.GetUpdate<ModelAnimationSystem>())    sys->Update(m_world, dt);
        if (auto* sys = m_sys.GetUpdate<AudioPlaySystem>())         sys->Update(m_world, dt);
        if (auto* sys = m_sys.GetUpdate<TimerSystem>())             sys->Update(m_world, dt);
		if (auto* sys = m_sys.GetUpdate<Collision2DSystem>())      sys->Update(m_world, dt);
		if (auto* sys = m_sys.GetUpdate<PhysicsStepSystem>())        sys->Update(m_world, dt);
		if (auto* sys = m_sys.GetUpdate<CountdownUISystem>())    sys->Update(m_world, dt);

        // ステート系も回さないとアイドル状態が反映されない可能性があるなら追加
        if (auto* sys = m_sys.GetUpdate<ModelAnimationStateSystem>()) sys->Update(m_world, dt);
    }

    // シーン遷移ロジック
	if (m_deathSystem && m_deathSystem->IsDead())
	{
        
        m_deathSystem->GameOverUpdate(m_world);
        
		return;
	}
	
    if (m_goalSystem && m_goalSystem->IsCleared())
    {
        AudioManager::StopBGM();
        ChangeScene<ResultScene>();
		return;
	}

    EffectRuntime::Update(dt);
}

void GameScene::Draw()
{
    if (m_followCamera)
    {
        const auto& V = m_followCamera->GetView();
        const auto& P = m_followCamera->GetProj();

        if (m_drawBackGround) m_drawBackGround->SetViewProj(V, P);
        if (m_drawModel)      m_drawModel->SetViewProj(V, P);
        if (m_drawSprite)     m_drawSprite->SetViewProj(V, P);
        if (m_debugCollision) m_debugCollision->SetViewProj(V, P);
        
        EffectRuntime::SetCamera(V, P);
        ShaderList::SetL(V, P);
    }

    m_sys.Render(m_world);
    EffectRuntime::Render();
}