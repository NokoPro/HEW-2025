/**********************************************************************************************
/* @file      ResultScene.cpp
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
/*    - [◎]シーンからの遷移
/*    - [] UIの表示
/*	  - [] ランキングの表示
/*
/**********************************************************************************************/

#include "ResultScene.h"
#include "Scene/TitleScene.h"
#include "Scene/StageSelectScene.h"


ResultScene::ResultScene()
{

}

ResultScene::~ResultScene()
{

}

void ResultScene::Update()
{
	// シーン変更
	if (IsKeyPress(VK_ESCAPE))
	{
		ChangeScene<StageSelectScene>();
	}
}

void ResultScene::Draw()
{

}
