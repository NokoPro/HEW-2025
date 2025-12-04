/**********************************************************************************************
/* @file      StageSelectScene.cpp
/* @brief	  ステージセレクトシーン
/*
/* @author    堤翔
/* @author	  奥田修也
/*
/* @date      2025 / 11 / 25
/*
/* @author    川谷優真 (追記)
 * @date      2025/12/02
/* =============================================================================================
/*  Progress Log  - 進捗ログ
/* ---------------------------------------------------------------------------------------------
/*  [ @date 2025/11/25 ]
/*
/*    - [◎]ゲームステージシーンへの遷移
/*	  - []	タイトルＵＩ表示
/*
/**********************************************************************************************/
#include <DirectXMath.h>// 座標計算用:川谷(追記)
#include "StageSelectScene.h"
#include "GameScene.h" // 遷移先
#include "Scene/SceneAPI.h"

// ECSコンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// ECSプレハブ
#include "ECS/Prefabs/PrefabFloor.h"
#include "ECS/Prefabs/PrefabPlayer.h"

// アセット
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h" // ShaderList::SetL用

using namespace DirectX;

// 便利なマクロ
#define IS_DECIDE (IsKeyTrigger(VK_SPACE) || IsKeyTrigger(VK_RETURN) || IsPadTrigger(0, XINPUT_GAMEPAD_A))
#define IS_CANCEL (IsKeyTrigger(VK_BACK)  || IsPadTrigger(0, XINPUT_GAMEPAD_B))
#define IS_RIGHT  (IsKeyTrigger(VK_RIGHT) || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_RIGHT) || GetPadLX(0) > 0.5f)
#define IS_LEFT   (IsKeyTrigger(VK_LEFT)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_LEFT)  || GetPadLX(0) < -0.5f)
#define IS_UP     (IsKeyTrigger(VK_UP)    || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_UP)    || GetPadLY(0) > 0.5f)
#define IS_DOWN   (IsKeyTrigger(VK_DOWN)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_DOWN)  || GetPadLY(0) < -0.5f)

StageSelectScene::StageSelectScene()
{
    Initialize();
}

StageSelectScene::~StageSelectScene()
{
}

void StageSelectScene::Initialize()
{
    // 1. システム登録
// カメラシステムを登録（Update用）
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

    // 3Dモデル描画システム:川谷(追記)
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>(); // 先に描画

    // UI描画システム（Render用）
    m_drawSprite = &m_sys.AddRender<SpriteRenderSystem>(); // 後に描画(UIを重ねるため)

	// Prefab登録:川谷(追記)
	RegisterFloorPrefab(m_prefabs);
	RegisterPlayerPrefab(m_prefabs);

    // 2. カメラ生成
    {
        EntityId cam = m_world.Create();
        m_world.Add<ActiveCameraTag>(cam);

		// Transformコンポーネント追加:川谷(追記)
        m_world.Add<TransformComponent>(cam);

        auto& c3d = m_world.Add<Camera3DComponent>(cam);
        c3d.mode = Camera3DComponent::Mode::Orbit; // Fixed(平衡投影)モード

		// 斜め上から見下ろす視点設定:川谷(追記)
        c3d.targetX = 0.0f; 
        c3d.targetY = 0.0f; 
        c3d.targetZ = 0.0f;

		c3d.orbitDistance    =  35.0f; // カメラまでの距離:川谷(追加)
		c3d.orbitPitchDeg    =  30.0f; // 見下ろす角度:川谷(追加)
		c3d.orbitYawDeg      = -90.0f; // 横回転:川谷(追加)

		// ズーム具合:縦に何メートル分映すか:川谷(追記)
        c3d.fovY = 45.0f;
        c3d.aspect = 1280.0f / 720.0f; // アスペクト比も設定推奨
        c3d.nearZ = 0.1f;
        c3d.farZ = 100.0f;

    }

	// ステージ環境構築:川谷(追記)
    InitStages();

    // プレイヤー生成:川谷(追記)
    {
        PrefabRegistry::SpawnParams sp;
        sp.padIndex = 0;
		sp.modelAlias = "mdl_2Pplayer";

		// 初期位置計算:川谷(追記)
        if (!m_stagePoints.empty())
        {
            sp.position = m_stagePoints[0];
            sp.position.y += 1.0f; // 床に埋まらないように
        }
		sp.rotationDeg = { 0.0f, 135.0f, 0.0f }; // カメラの方を向く角度:川谷(追記)
        sp.scale = { 1.0f, 1.0f, 1.0f };

        m_playerEntity = m_prefabs.Spawn("Player", m_world, sp);

        // アニメーションシステムが無い場面なので、シェーダを静適用に上書き
        if (auto* mr = m_world.TryGet<ModelRendererComponent>(m_playerEntity))
        {
            if (mr->model)
            {
                mr->model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
            }
        }
	}

    // 3. UIエンティティ生成

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

// ステージ環境の配置:川谷(追記)
void StageSelectScene::InitStages()
{
    m_stagePoints.clear();

    // 座標定義(左、中央、右)
	m_stagePoints.push_back({ -8.0f, 0.0f, 0.0f });
    m_stagePoints.push_back({  0.0f, 0.0f, 0.0f });
    m_stagePoints.push_back({  8.0f, 0.0f, 0.0f });

    for (const auto& pos : m_stagePoints)
    {
        // 床
        {
            PrefabRegistry::SpawnParams sp;
            sp.position = pos;
            sp.scale = { 2.0f, 1.0f, 2.0f };
            sp.modelAlias = "mdl_ground";
            m_prefabs.Spawn("Floor", m_world, sp);
        }

        // 塔
        {
            EntityId tower = m_world.Create();
            auto& tr = m_world.Add<TransformComponent>(tower);
            tr.position = { pos.x, pos.y + 1.5f, pos.z + 1.5f };
            tr.scale = { 0.8f, 3.0f, 0.8f };

			auto& mr = m_world.Add<ModelRendererComponent>(tower);
            mr.model = AssetManager::GetModel("mdl_ground"); // 代用モデル
            mr.visible = true;
			mr.layer = 0;


            if (mr.model)
            {
                mr.model->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
                mr.model->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
            }
        }
    }
}

// プレイヤー移動関数:川谷(追記)
void StageSelectScene::SyncPlayerPosition()
{
    // m_currentStageは1～3なので-1して配列アクセス
    int idx = m_currentStage - 1;
    if (idx >= 0 && idx < (int)m_stagePoints.size())
    {
        if (auto* tr = m_world.TryGet<TransformComponent>(m_playerEntity))
        {
			DirectX::XMFLOAT3 targetPos = m_stagePoints[idx];
            targetPos.y += 1.0f;
            tr->position = targetPos;
        }
    }
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
		// 川谷(追記)
        bool changed = false;

        if (IS_RIGHT)
        {
            m_currentStage++;
            if (m_currentStage > m_maxStages) m_currentStage = 1;
			changed = true; // :川谷(追記)
        }
        else if (IS_LEFT)
        {
            m_currentStage--;
            if (m_currentStage < 1) m_currentStage = m_maxStages;
			changed = true; // :川谷(追記)
        }

        // 変更があったらプレイヤー位置を更新
        if (changed)
        {
            SyncPlayerPosition();
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
    if(auto * sp = m_world.TryGet<Sprite2DComponent>(m_entStageIcon))
    {

        // 画像切り替え
        sp->alias = "tex_ui_stage" + std::to_string(m_currentStage);

        if (m_state == State::SelectStage)
        {
            sp->visible = true;
            sp->width = 420.0f;
            sp->height = 120.0f;

            if (auto* tr = m_world.TryGet<TransformComponent>(m_entStageIcon))
            {
                // UI座標系: (0,0)が画面中央
                // Y=50.0f くらいにして、キャラの少し上に表示
                tr->position = { 0.0f, 120.0f, 0.0f };
            }
        }
        else
        {
            sp->width  = 420.0f;
            sp->height = 120.0f;
        }
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

            // 画面中央へ配置
            if (auto* tr = m_world.TryGet<TransformComponent>(m_entDiffIcon))
            {
                // UI座標系では(0, 0)が画面中央
                tr->position = { 0.0f, 0.0f, -1.0f };
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
    // 3Dモデル描画
    if (m_followCamera && m_drawModel)
    {
        const auto& V = m_followCamera->GetView();
		const auto& P = m_followCamera->GetProj();

        m_drawModel->SetViewProj(V, P);
        ShaderList::SetL(V, P); // ライティングも3D基準
    }

    // 2Dスプライト描画
    if (m_drawSprite)
    {
        // 2Dスプライト描画
        DirectX::XMFLOAT4X4 view2D;
        DirectX::XMStoreFloat4x4(&view2D, DirectX::XMMatrixIdentity());

        DirectX::XMFLOAT4X4 proj2D;
        DirectX::XMStoreFloat4x4(&proj2D, DirectX::XMMatrixOrthographicLH(1280.0f, 720.0f, 0.0f, 100.0f));

        m_drawSprite->SetViewProj(view2D, proj2D);
    }

    // 描画実行
    m_sys.Render(m_world);
}