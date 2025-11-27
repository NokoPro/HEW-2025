/**********************************************************************************************
/* @file      StageSelectScene.cpp
/* @brief	  ステージセレクトシーン
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

#include "StageSelectScene.h"
#include "Scene/TestStageScene.h"


StageSelectScene::StageSelectScene()
{

}

StageSelectScene::~StageSelectScene()
{

}

void StageSelectScene::Update()
{
	// シーン変更
	if (IsKeyPress(VK_SPACE))
	{
		ChangeScene<TestStageScene>();
	}
}

void StageSelectScene::Draw()
{

}
