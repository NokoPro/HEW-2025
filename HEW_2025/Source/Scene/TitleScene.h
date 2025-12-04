/**********************************************************************************************
/* @file      TitleScene.h
/* @brief     タイトルシーンヘッダ
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

#include "Scene/Scene.h"

//シーンの遷移
#include "Scene/SceneAPI.h"

//key入力
#include"System/Input.h"

 // ECS関連
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"
#include "ECS/Prefabs/PrefabRegistry.h"

class TitleScene : public Scene
{
public:
    TitleScene();
    ~TitleScene() override;

    void Update() override;
    void Draw();

private:
	void Initialize(); //初期化処理

    // ECS
    World m_world;
    SystemRegistry m_sys;
    PrefabRegistry m_prefabs;

    // システムへの参照
    SpriteRenderSystem* m_drawSprite = nullptr;
    FollowCameraSystem* m_followCamera = nullptr;

    // UI用エンティティID
    EntityId m_entStageIcon = 0;
    EntityId m_entDiffIcon = 0;
    EntityId m_entGuide = 0;
};