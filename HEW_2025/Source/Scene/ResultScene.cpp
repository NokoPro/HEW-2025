/**********************************************************************************************
/* @file      ResultScene.cpp
/* @brief	  タイトルシーン
/*
/* @author    堤翔
/* @author	  奥田修也
/* @author    篠原純
/* @author    清水光之介
/*
/* @date      2025 / 11 / 25
/*
/* =============================================================================================
/*  Progress Log  - 進捗ログ
/* ---------------------------------------------------------------------------------------------
/*  [ @date 2025/11/25 ]
/*
/*    - [◎]シーンからの遷移
/*    - [] UIの表示
/*	  - [] ランキングの表示
/*
/**********************************************************************************************/

#include "ResultScene.h"
#include "Scene/TitleScene.h"
#include "Scene/StageSelectScene.h"
// ECSコンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include"ECS/Systems/Render/ResultTimerSystem.h"
#include"ECS/Prefabs/PrefabTimer.h"
#include"ECS/Prefabs/PrefabResultTimer.h"
#include "ECS/Systems/Update/Ranking/ResultRankingSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"
#include "ECS/Prefabs/PrefabResultRankingUI.h"

// アセット
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h" // ShaderList::SetL用
#include "System/RankingManager.h"

//背景
#include "ECS/Prefabs/PrefabResultUi.h"

ResultScene::ResultScene()
{
    // -------------------------------------------------------
    // 0. プレハブ登録
    // -------------------------------------------------------
    RegisterResultUiPrefab(m_prefabs);
    RegisterResultTimerPrefab(m_prefabs);
    RegisterResultRankingUIPrefab(m_prefabs);

    // ランキングの読込（表示用）。空ならダミーを投入して可視化
    {
        // 任意の保存先（存在しなくてもOK）
        const std::string rankingPath = "Assets/Save/ranking.json";
        RankingManager::Get().Load(rankingPath);

        
    }

    // -------------------------------------------------------
    // 2. System登録
    // -------------------------------------------------------
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();
    m_drawResultUI = &m_sys.AddRender<ResultUiRenderSystem>();
    m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();
   
    m_sys.AddUpdate<ResultRankingSystem>();   // ランキング描画制御

    // -------------------------------------------------------
    // 4. 固定エンティティ生成
    // -------------------------------------------------------
    //背景
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 0.0f, 0.0f, 2.0f };
        sp.scale = { 55.0f, 55.0f, 55.0f };
        sp.modelAlias = "tex_resultbackground";
        m_prefabs.Spawn("ResultUI", m_world, sp);

    }
    
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -0.65f, -0.6f, 0.0f };
        sp.scale = { 13.0f, 8.0f, 1.0f };
        sp.modelAlias = "tex_1st";
        m_prefabs.Spawn("ResultUI", m_world, sp);

    } 
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -0.65f, -0.3f, 0.0f };
        sp.scale = { 13.0f, 8.0f, 1.0f };
        sp.modelAlias = "tex_2st";
        m_prefabs.Spawn("ResultUI", m_world, sp);

    }
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -0.65f, 0.0f, 0.0f };
        sp.scale    = { 13.0f, 8.0f, 1.0f };
        sp.modelAlias = "tex_3st";
        m_prefabs.Spawn("ResultUI", m_world, sp);
        
    }

    //セレクト画面にもどるUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -0.5f, 1.0f, 1.0f };
        sp.scale = { 1.0f, 1.0f, 1.0f };
        sp.modelAlias = "tex_selectstage";
        m_prefabs.Spawn("ResultUI", m_world, sp);

    }
    //リトライUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 0.5f, 1.0f, 1.0f };
        sp.scale = { 1.0f, 1.0f, 1.0f };
        sp.modelAlias = "tex_relrystage";
        m_prefabs.Spawn("ResultUI", m_world, sp);

    }
    // ランキングUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -4.0f, -1.0f, 0.0f };
        sp.scale = { 0.5f, 0.5f, 0.5f };
        m_prefabs.Spawn("ResultRankingUI", m_world, sp);
    }
    // 5. カメラ生成
    {
        EntityId cam = m_world.Create();
        m_world.Add<ActiveCameraTag>(cam);

        // Fixedモードなので位置は固定
        auto& tr = m_world.Add<TransformComponent>(cam);
        tr.position = { 0.0f, 0.0f, -10.0f }; // 原点中心、手前に引く

        auto& c3d = m_world.Add<Camera3DComponent>(cam);
        c3d.mode = Camera3DComponent::Mode::Fixed; // Fixedモード

        // 画面の高さ(720)を基準にする
        // これで (0,0) が画面中央、Y範囲が -360～+360 になる
        c3d.orthoHeight = 720.0f;
        c3d.aspect = 1280.0f / 720.0f; // アスペクト比も設定推奨
        c3d.nearZ = 0.1f;
        c3d.farZ = 300.0f;
		
		
		c3d.fovY = 20.0f;

		c3d.orthoHeight = 39.5f;
		c3d.sideFixedX = (c3d.orthoHeight * c3d.aspect) * 0.5f;
		c3d.sideLookAtX = 0.0f;
		c3d.lookAtOffset = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
		
		
	}
	
}

ResultScene::~ResultScene()
{

}

void ResultScene::Update()
{
	const float dt = 1.0f / 60.0f;
	m_sys.Tick(m_world, dt);

	


	if (IsKeyPress(VK_ESCAPE))
	{
		ChangeScene<StageSelectScene>();
	}
	
}

void ResultScene::Draw()
{
	if (m_followCamera)
	{
		const auto& V = m_followCamera->GetView();
		const auto& P = m_followCamera->GetProj();

		

        if (m_drawResultUI)   m_drawResultUI->SetViewProj(V, P);

		
		if (m_drawSprite)     m_drawSprite->SetViewProj(V, P);
		
		

	
		ShaderList::SetL(V, P);
	}

	m_sys.Render(m_world);
}
