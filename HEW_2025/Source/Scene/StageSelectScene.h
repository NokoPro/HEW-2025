/**********************************************************************************************
/* @file      StageSelectScene.h
/* @brief     ステージセレクトヘッダ
/*
/* @author    堤翔
/* @author	  奥田修也
/*
/* @date      2025/11/25
/*
/* =============================================================================================
/*  Progress Log  - 進捗ログ
/* ---------------------------------------------------------------------------------------------
/*  [ @date 2025/11/25 ]
/*
/*    - [◎]クラスの作成
/*
/**********************************************************************************************/
#pragma once

#include "Scene.h"

//シーンの遷移
#include "Scene/SceneAPI.h"

//key入力
#include"System/Input.h"

class StageSelectScene : public Scene
{
public:
    StageSelectScene();
    ~StageSelectScene() override;

    void Update() override;
    void Draw();

private:
};

