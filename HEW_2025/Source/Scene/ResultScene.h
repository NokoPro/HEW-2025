/**********************************************************************************************
/* @file      ResultScene.h
/* @brief     リザルトシーンヘッダ
/*
/* @author    堤翔
/* @author	  奥田修也
/* @author    篠原純
/* 
/* @date      2025/11/25
/*
/* =============================================================================================
/*  Progress Log  - 進捗ログ
/* ---------------------------------------------------------------------------------------------
/*  [ @date 2025/11/25 ]
/*
/*    - [◎]クラスの作成
/*    - []
/*
/**********************************************************************************************/
#pragma once

#include "Scene/Scene.h"

//シーンの遷移
#include "Scene/SceneAPI.h"
#include "System/GameCommon.h" // Difficult

//key入力
#include"System/Input.h"

 // ECS関連
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

class ResultScene : public Scene
{
public:
    ResultScene();
    ~ResultScene() override;

    void Update() override;
    void Draw();

private:
    

    void UpdateInput();   // 入力処理
    void UpdateUI();      // UIの見た目更新

    // ECS
    World m_world;
    SystemRegistry m_sys;

    // システムへの参照
    SpriteRenderSystem* m_drawSprite = nullptr;
    FollowCameraSystem* m_followCamera = nullptr;

    // UI用エンティティID
    EntityId m_entResultBackground = 0;
    EntityId m_entRetryButton = 0;
    

};