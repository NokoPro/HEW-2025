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
#include "ECS/World.h"
#include "ECS/Systems/SystemRegistry.h"

// カメラ
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"

// プレファブ
#include "ECS/Prefabs/PrefabRegistry.h"

//シーンの遷移
#include "Scene/SceneAPI.h"

// システム

#include"System/Input.h"

class SpriteRenderSystem;
class FollowCameraSystem;

class TitleScene : public Scene
{
public:
    TitleScene();
    ~TitleScene() override;

    void Update() override;
    void Draw();

private:
    /**
     * @brief 初期化処理（コンストラクタから呼ばれる）
     */
    void Initialize();

    SpriteRenderSystem* m_drawSprite = nullptr;
    FollowCameraSystem* m_followCamera = nullptr;

    World          m_world;
    SystemRegistry m_sys;
    PrefabRegistry m_prefabs;

	EntityId m_uiTitleEntity = kInvalidEntity;
};