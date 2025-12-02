/**********************************************************************************************
/* @file      ResultScene.cpp
/* @brief	  タイトルシーン
/*
/* @author    堤翔
/* @author	  奥田修也
/* @author    篠原純
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

// アセット
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h" // ShaderList::SetL用

//背景
#include "ECS/Prefabs/PrefabResultUi.h"

ResultScene::ResultScene()
{
	// -------------------------------------------------------
	// 0. プレハブ登録
	// -------------------------------------------------------
	RegisterResultUiPrefab(m_prefabs);
	// -------------------------------------------------------
   // 2. System登録
   // -------------------------------------------------------
	m_drawBackGround = &m_sys.AddRender<ResultUiRenderSystem>();
	m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();
	// カメラシステムを登録（Update用）
	m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();
	// -------------------------------------------------------
	// 4. 固定エンティティ生成
	// -------------------------------------------------------
	// 背景(test)
	{
		PrefabRegistry::SpawnParams sp;
		sp.position = { 0.0f, 0.0f, 0.0f };
		
		sp.scale.x = 1.0f;
		sp.scale.y = 1.0f;
		sp.scale.z = 1.0f;//UI番号変数名気にしないで
		
		m_prefabs.Spawn("ResultUI", m_world, sp);
	}
	//UItest
	{
		PrefabRegistry::SpawnParams sp;
		sp.position = { 1.0f,1.0f,1.0f };
		sp.scale.x = 1.0f;
		sp.scale.y = 1.0f;
		sp.scale.z = 0.0f;//UI番号変数名気にしないで
		m_prefabs.Spawn("ResultUI", m_world, sp);
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

		if (m_drawBackGround) 
			m_drawBackGround->SetViewProj(V, P);
		
		if (m_drawSprite)     m_drawSprite->SetViewProj(V, P);
		

	
		ShaderList::SetL(V, P);
	}

	m_sys.Render(m_world);
}
