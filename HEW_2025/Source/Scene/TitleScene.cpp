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
/*　@author    川谷優真 (追記)
/*  @date      2025/12/04
/*
/**********************************************************************************************/

#include "TitleScene.h"
#include "Scene/TestStageScene.h"
#include "Scene/StageSelectScene.h"

// ECSコンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"


TitleScene::TitleScene()
{
	Initialize();
}

TitleScene::~TitleScene()
{

}

void TitleScene::Initialize()
{
	// システムを登録
	// カメラシステムを登録（Update用）
	m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

	// UI描画システム（Render用）
	m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();

	// Prefab登録

}

void TitleScene::Update()
{
	// シーン変更
	if (IsKeyPress(VK_SPACE))
	{
		ChangeScene<StageSelectScene>();
	}
}

void TitleScene::Draw()
{

}
