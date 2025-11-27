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


TitleScene::TitleScene()
{

}

TitleScene::~TitleScene()
{

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
