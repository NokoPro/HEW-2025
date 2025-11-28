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
#include "GameScene.h" // 遷移先
#include "Scene/SceneAPI.h"

// ECSコンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// アセット
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h" // ShaderList::SetL用

// 便利なマクロ
#define IS_DECIDE (IsKeyTrigger(VK_SPACE) || IsKeyTrigger(VK_RETURN) || IsPadTrigger(0, XINPUT_GAMEPAD_A))
#define IS_CANCEL (IsKeyTrigger(VK_BACK)  || IsPadTrigger(0, XINPUT_GAMEPAD_B))
#define IS_RIGHT  (IsKeyTrigger(VK_RIGHT) || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_RIGHT) || GetPadLX(0) > 0.5f)
#define IS_LEFT   (IsKeyTrigger(VK_LEFT)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_LEFT)  || GetPadLX(0) < -0.5f)
#define IS_UP     (IsKeyTrigger(VK_UP)    || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_UP)    || GetPadLY(0) > 0.5f)
#define IS_DOWN   (IsKeyTrigger(VK_DOWN)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)  || GetPadLY(0) < -0.5f)

StageSelectScene::StageSelectScene()
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

    // ステージ選択アイコン (位置調整: 中央0,0基準)
    {
        m_entStageIcon = m_world.Create();
        // 中央より少し上 (Yプラス方向が上なら +50)
        m_world.Add<TransformComponent>(m_entStageIcon, DirectX::XMFLOAT3{ 0.0f, 50.0f, 0.0f });
        auto& sp = m_world.Add<Sprite2DComponent>(m_entStageIcon);
        sp.alias = "tex_ui_stage1";
        sp.width = 400.0f;
        sp.height = 100.0f;
        sp.layer = 0;
    }

    // 難易度選択アイコン (位置調整: 中央より下)
    {
        m_entDiffIcon = m_world.Create();
        m_world.Add<TransformComponent>(m_entDiffIcon, DirectX::XMFLOAT3{ 0.0f, -100.0f, 0.0f });
        auto& sp = m_world.Add<Sprite2DComponent>(m_entDiffIcon);
        sp.alias = "tex_ui_diff_normal";
        sp.width = 300.0f;
        sp.height = 80.0f;
        sp.layer = 0;
        sp.visible = false;
    }
}

StageSelectScene::~StageSelectScene()
{
}

void StageSelectScene::Update()
{
    // システム全体の更新 (これで FollowCameraSystem::Update が呼ばれる)
    m_sys.Tick(m_world, 1.0f / 60.0f);

    UpdateUI();
    UpdateInput();
}

void StageSelectScene::UpdateInput()
{
    // ... (入力ロジックは変更なし) ...
    if (m_state == State::SelectStage)
    {
        if (IS_RIGHT)
        {
            m_currentStage++;
            if (m_currentStage > m_maxStages) m_currentStage = 1;
        }
        else if (IS_LEFT)
        {
            m_currentStage--;
            if (m_currentStage < 1) m_currentStage = m_maxStages;
        }

        if (IS_DECIDE)
        {
            m_state = State::SelectDifficulty;
        }
    }
    else if (m_state == State::SelectDifficulty)
    {
        if (IS_RIGHT || IS_UP)
        {
            int d = (int)m_currentDiff + 1;
            if (d > (int)Difficulty::Hard) d = (int)Difficulty::Easy;
            m_currentDiff = (Difficulty)d;
        }
        else if (IS_LEFT || IS_DOWN)
        {
            int d = (int)m_currentDiff - 1;
            if (d < (int)Difficulty::Easy) d = (int)Difficulty::Hard;
            m_currentDiff = (Difficulty)d;
        }

        if (IS_DECIDE)
        {
            ChangeScene<GameScene>(m_currentStage, m_currentDiff);
        }
        else if (IS_CANCEL)
        {
            m_state = State::SelectStage;
        }
    }
}

void StageSelectScene::UpdateUI()
{
    // ... (UI更新ロジックは変更なし) ...
    // テクスチャエイリアスの切り替え等
    if (auto* sp = m_world.TryGet<Sprite2DComponent>(m_entStageIcon))
    {
        if (m_state == State::SelectStage)
        {
            sp->visible = true;
            sp->width = 420.0f;
            sp->height = 120.0f;
        }
        else
        {
            sp->width = 400.0f;
            sp->height = 100.0f;
        }
        sp->alias = "tex_ui_stage" + std::to_string(m_currentStage);
    }

    if (auto* sp = m_world.TryGet<Sprite2DComponent>(m_entDiffIcon))
    {
        if (m_state == State::SelectDifficulty)
        {
            sp->visible = true;
            sp->width = 320.0f;
            sp->height = 100.0f;
            switch (m_currentDiff)
            {
            case Difficulty::Easy:   sp->alias = "tex_ui_diff_easy"; break;
            case Difficulty::Normal: sp->alias = "tex_ui_diff_normal"; break;
            case Difficulty::Hard:   sp->alias = "tex_ui_diff_hard"; break;
            }
        }
        else
        {
            sp->visible = false;
        }
    }
}

void StageSelectScene::Draw()
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