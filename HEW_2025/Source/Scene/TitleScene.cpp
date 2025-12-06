/**********************************************************************************************
/* @file      TitleScene.cpp
/* @brief	  タイトルシーン
/*
/* @author    堤翔
/* @author	  奥田修也
/*
/* @date      2025 / 11 / 25
/*
/* =============================================================================================
/*  Progress Log  - 進捗ログ
/* ---------------------------------------------------------------------------------------------
/*  [ @date 2025/11/25 ]
/*
/*    - [◎]ゲームステージシーンへの遷移
/*	  - []	タイトルＵＩ表示
/*
/**********************************************************************************************/

#include "TitleScene.h"
#include "Scene/TestStageScene.h"
#include "Scene/StageSelectScene.h"

#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

#include "ECS/Prefabs/PrefabUIOverlay.h"
#include "System/AssetManager.h"
#include "System/Defines.h"
#include "System/DirectX/ShaderList.h"

#define IS_DECIDE (IsKeyTrigger(VK_SPACE) || IsKeyTrigger(VK_RETURN) || IsPadTrigger(0, XINPUT_GAMEPAD_A))

TitleScene::TitleScene()
{
	Initialize();
}

TitleScene::~TitleScene()
{

}
	
void TitleScene::Initialize()
{
	// プレファブ登録
	RegisterUIOverlayPrefab(m_prefabs);

	// システム登録
	m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();
	m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();

	// UIオーバーレイ生成
	{
		PrefabRegistry::SpawnParams params{};
		params.position = { 35.0f, 10.0f, 0.0f };
		params.scale = { 1.0f, 1.0f, 1.0f };
		params.layer = 100; // UI用レイヤ
		params.spriteWidth = 65.f;  // 画面幅に合わせる
		params.spriteHeight = 65.f;  // 画面高さに合わせる
		params.modelAlias = "tex_title_ui"; // タイトル用
		m_uiTitleEntity = m_prefabs.Spawn("UIOverlay", m_world, params);
	}

	{
		PrefabRegistry::SpawnParams params{};
		params.position = { 37.0f, -8.0f, 0.0f };
		params.scale = { 1.0f, 1.0f, 1.0f };
		params.layer = 100; // UI用レイヤ
		params.spriteWidth = 30.f;  // 画面幅に合わせる
		params.spriteHeight = 8.f;  // 画面高さに合わせる
		params.modelAlias = "tex_title_overlay"; // タイトル用
		m_prefabs.Spawn("UIOverlay", m_world, params);
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
		cam.target = m_uiTitleEntity;
		cam.aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
		cam.nearZ = 0.1f;
		cam.farZ = 300.0f;
		cam.fovY = 20.0f;

		cam.scrollSpeed = 0.0f;

		cam.followOffsetY = 1.5f;
		cam.followMarginY = 0.5f;
		cam.sideFixedZ = -185.0f;
		cam.orthoHeight = 39.5f;
		cam.sideFixedX = (cam.orthoHeight * cam.aspect) * 0.5f;
		cam.sideLookAtX = 0.0f;
		cam.lookAtOffset = DirectX::XMFLOAT3{ 0.0f, 8.0f, 0.0f };
	}
}

void TitleScene::Update()
{
	const float dt = 1.0f / 60.0f;
	m_sys.Tick(m_world, dt);

	// シーン変更
	if (IS_DECIDE)
	{
		ChangeScene<StageSelectScene>();
	}
}

void TitleScene::Draw()
{
	if (m_followCamera)
	{
		const auto& V = m_followCamera->GetView();
		const auto& P = m_followCamera->GetProj();

		if (m_drawSprite)     m_drawSprite->SetViewProj(V, P);
		ShaderList::SetL(V, P);
	}

	m_sys.Render(m_world);
}
