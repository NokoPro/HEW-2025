/**********************************************************************************************
 * @file      ResultScene.cpp
 * @brief     リザルトシーンの実装。UI生成・更新、入力処理、レンダリングを担当。
 * @authors   堤翔 / 奥田修也 / 篠原純 / 清水光之介
 * @date      2025/11/25
 *
 * @details
 *  - プレハブ登録（結果UI、タイマー、ランキングUI）
 *  - ランキング読込
 *  - 必要なシステム登録（フォローカメラ、UI描画、スプライト描画、ランキング制御）
 *  - 固定エンティティ（背景やボタン、ランキングUI）の生成
 *  - カメラ設定（Fixedモードの正射影）
 *  - Update でシステム Tick と UI/入力更新
 *  - Draw で VP 行列の適用と描画実行
 **********************************************************************************************/
#include "ResultScene.h"
#include "GameScene.h"
#include "Scene/TitleScene.h"
#include "Scene/StageSelectScene.h"

// ECSコンポーネント
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

// システム
#include "ECS/Systems/Render/ResultTimerSystem.h"
#include "ECS/Prefabs/PrefabTimer.h"
#include "ECS/Prefabs/PrefabResultTimer.h"
#include "ECS/Systems/Update/Ranking/ResultRankingSystem.h"
#include "ECS/Systems/Update/Core/FollowCameraSystem.h"
#include "ECS/Prefabs/PrefabResultRankingUI.h"
#include "ECS/Prefabs/PrefabResultUi.h"

// アセット
#include "System/AssetManager.h"
#include "System/DirectX/ShaderList.h"
#include "System/RankingManager.h"

#define IS_DECIDE (IsKeyTrigger(VK_SPACE) || IsKeyTrigger(VK_RETURN)                    || IsPadTrigger(0, XINPUT_GAMEPAD_A))
#define IS_CANCEL (IsKeyTrigger(VK_BACK)  || IsKeyTrigger(VK_ESCAPE)                    || IsPadTrigger(0, XINPUT_GAMEPAD_B))
#define IS_RIGHT  (IsKeyTrigger(VK_RIGHT) || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_RIGHT) || GetPadLX(0) > 0.5f)
#define IS_LEFT   (IsKeyTrigger(VK_LEFT)  || IsPadTrigger(0, XINPUT_GAMEPAD_DPAD_LEFT)  || GetPadLX(0) < -0.5f)

ResultScene::ResultScene()
{
    // 0. プレハブ登録
    RegisterResultUiPrefab(m_prefabs);
    RegisterResultTimerPrefab(m_prefabs);
    RegisterResultRankingUIPrefab(m_prefabs);

    // ランキングの読込
    const std::string rankingPath = "Assets/Save/ranking.json";
    RankingManager::Get().Load(rankingPath);

    // 2. System登録
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();
    // ResultUiRenderSystem を廃止し、スプライト描画に一本化
    m_drawSprite   = &m_sys.AddRender<SpriteRenderSystem>();
    m_sys.AddUpdate<ResultRankingSystem>();

    // 4. 固定エンティティ生成
    // 背景
    {
        PrefabRegistry::SpawnParams sp;
        sp.position   = { 0.0f, 0.0f, 2.0f };
        sp.scale      = { 55.0f, 55.0f, 55.0f };
        sp.modelAlias = "tex_resultbackground";
        m_prefabs.Spawn("ResultUI", m_world, sp);
    }
    
    // 3位
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 18.5f, -0.5f, 0.0f };
        sp.scale      = { 15.0f, 11.0f, 1.0f };
        sp.modelAlias = "tex_3st";
        m_prefabs.Spawn("ResultUI", m_world, sp);
    }
    // 2位
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 18.5f, 6.f, 0.0f };
        sp.scale = { 15.0f, 11.0f, 1.0f };
        sp.modelAlias = "tex_2st";
        m_prefabs.Spawn("ResultUI", m_world, sp);
    }
    // 1位
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { 18.5f, 12.5f, 0.0f };
        sp.scale = { 15.0f, 11.0f, 1.0f };
        sp.modelAlias = "tex_1st";
        m_prefabs.Spawn("ResultUI", m_world, sp);
    }
    // セレクト画面にもどるUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position   = { 0.f, -10.0f, 3.0f };
        sp.scale      = { 1.0f, 1.0f, 1.0f };
        sp.modelAlias = "tex_relrystage";
        m_entSelectStageButton = m_prefabs.Spawn("ResultUI", m_world, sp);
        if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entSelectStageButton))
        {
            spr->alias   = sp.modelAlias;
            spr->visible = true;
            spr->layer   = 10;
            spr->width   = 18.0f;
            spr->height  = 18.0f;
        }
    }
    // リトライUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -20.f, -10.0f, 3.0f };
        sp.scale      = { 1.0f, 1.0f, 1.0f };
        sp.modelAlias = "tex_selectstage";
        m_entRetryButton = m_prefabs.Spawn("ResultUI", m_world, sp);
        if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entRetryButton))
        {
            spr->alias   = sp.modelAlias;
            spr->visible = true;
            spr->layer   = 10;
            spr->width   = 18.0f;
            spr->height  = 18.0f;
        }
    }
    // ランキングUI
    {
        PrefabRegistry::SpawnParams sp;
        sp.position = { -4.0f, -1.0f, 0.0f };
        sp.scale    = { 0.5f, 0.5f, 0.5f };
        m_prefabs.Spawn("ResultRankingUI", m_world, sp);
    }

    // 5. カメラ生成
    {
        EntityId cam     = m_world.Create();
        m_world.Add<ActiveCameraTag>(cam);
        auto& tr         = m_world.Add<TransformComponent>(cam);
        tr.position      = { 0.0f, 0.0f, -10.0f };
        auto& c3d        = m_world.Add<Camera3DComponent>(cam);
        c3d.mode         = Camera3DComponent::Mode::Fixed;
        c3d.orthoHeight  = 720.0f;
        c3d.aspect       = 1280.0f / 720.0f;
        c3d.nearZ        = 0.1f;
        c3d.farZ         = 300.0f;
        c3d.fovY         = 20.0f;
        c3d.orthoHeight  = 39.5f; // 実ゲーム用に上書き（要調整）
        c3d.sideFixedX   = (c3d.orthoHeight * c3d.aspect) * 0.5f;
        c3d.sideLookAtX  = 0.0f;
        c3d.lookAtOffset = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
    }
}

ResultScene::~ResultScene() = default;

/**
 * @brief 結果シーンの更新。システム Tick、UI 見た目更新、入力処理。
 */
void ResultScene::Update()
{
    const float dt = 1.0f / 60.0f;
    m_sys.Tick(m_world, dt);
    UpdateUI();
    UpdateInput();
}

/**
 * @brief 入力処理。キャンセル/左右切替/決定の反応とシーン遷移。
 */
void ResultScene::UpdateInput()
{
    if (IS_CANCEL)
    {
        ChangeScene<StageSelectScene>();
        return;
    }

    if (IS_LEFT || IS_RIGHT)
    {
        m_menuItem = (m_menuItem == MenuItem::Retry) ? MenuItem::StageSelect : MenuItem::Retry;
    }

    if (IS_DECIDE)
    {
        if (m_menuItem == MenuItem::Retry)
        {
            ChangeScene<StageSelectScene>();
        }
        else
        {
            ChangeScene<GameScene>(1, Difficulty::Normal);
        }
    }
}

/**
 * @brief UI の見た目を更新。選択中ボタンのサイズを変更。
 */
void ResultScene::UpdateUI()
{
    constexpr float kBaseW = 18.0f;
    constexpr float kBaseH = 18.0f;
    constexpr float kSelW  = 22.0f;
    constexpr float kSelH  = 22.0f;

    // どちらのボタンも常に表示
    if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entRetryButton))        spr->visible = true;
    if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entSelectStageButton)) spr->visible = true;

    // リトライボタン
    if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entRetryButton))
    {
        const bool selected = (m_menuItem == MenuItem::Retry);
        spr->width  = selected ? kSelW : kBaseW;
        spr->height = selected ? kSelH : kBaseH;
    }

    // ステージセレクトボタン
    if (auto* spr = m_world.TryGet<Sprite2DComponent>(m_entSelectStageButton))
    {
        const bool selected = (m_menuItem == MenuItem::StageSelect);
        spr->width  = selected ? kSelW : kBaseW;
        spr->height = selected ? kSelH : kBaseH;
    }
}

/**
 * @brief 描画処理。3D/2D の VP を設定してレンダリング。
 */
void ResultScene::Draw()
{
    if (m_followCamera)
    {
        const auto& V = m_followCamera->GetView();
        const auto& P = m_followCamera->GetProj();
        if (m_drawSprite)   m_drawSprite->SetViewProj(V, P);
        ShaderList::SetL(V, P);
    }
    m_sys.Render(m_world);
}

