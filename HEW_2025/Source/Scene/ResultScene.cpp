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

ResultScene::ResultScene()
{
    // 1. システム登録
    // カメラシステムを登録（Update用）
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

    // UI描画システム（Render用）
    m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>();

    // 2. カメラ生成
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
        c3d.farZ = 100.0f;
    }

    // 3. UIエンティティ生成
    // 背景
    {
        EntityId e = m_world.Create();
        // 原点(0,0)中心に表示したいので位置は0,0でOK (画面サイズ1280x720なら中央に来る)
        // ※座標系注意: Transform{0,0} = 画面中央
        m_world.Add<TransformComponent>(e, DirectX::XMFLOAT3{ 0.0f, 0.0f, 10.0f });
        auto& sp = m_world.Add<Sprite2DComponent>(e);
        sp.alias = "tex_select_bg";
        sp.width = 1280.0f;
        sp.height = 720.0f;
        sp.layer = -10;
    }
    

    // リトライアイコン (位置調整: 中央より下)
    {
        m_entRetryButton = m_world.Create();
        m_world.Add<TransformComponent>(m_entRetryButton, DirectX::XMFLOAT3{ 0.0f, -100.0f, 0.0f });
        auto& sp = m_world.Add<Sprite2DComponent>(m_entRetryButton);
        sp.alias = "tex_ui_diff_normal";
        sp.width = 300.0f;
        sp.height = 80.0f;
        sp.layer = 0;
        sp.visible = false;
    }
}

ResultScene::~ResultScene()
{

}

void ResultScene::Update()
{
    // システム全体の更新 (これで FollowCameraSystem::Update が呼ばれる)
    m_sys.Tick(m_world, 1.0f / 60.0f);
	// シーン変更
	if (IsKeyPress(VK_ESCAPE))
	{
		ChangeScene<StageSelectScene>();
	}
}

void ResultScene::Draw()
{
    // 修正: 手計算をやめ、FollowCameraSystem から行列をもらう
    if (m_followCamera)
    {
        const auto& V = m_followCamera->GetView();
        const auto& P = m_followCamera->GetProj();

        // 描画システムに行列をセット
        if (m_drawSprite)
        {
            m_drawSprite->SetViewProj(V, P);
            // 他の描画システムがあれば同様にセット
        }

        // 必要に応じてシェーダ側にもセット（ライト等）
        ShaderList::SetL(V, P);
    }

    // 描画実行
    m_sys.Render(m_world);
}
