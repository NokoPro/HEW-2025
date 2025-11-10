/*****************************************************************//**
 * @file   TestScene.cpp
 * @brief  ECSを試すためのテストシーン実装
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "TestScene.h"
#include "ECS/Systems/SystemRegistry.h"
#include "ECS/World.h"

#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"

#include "ECS/Systems/Render/ModelRenderSystem.h"
#include "ECS/Systems/Render/SpriteRenderSystem.h"

#include "System/CameraHelper.h"
#include "System/DirectX/ShaderList.h"
#include "System/Defines.h"
#include "System/CameraMath.h"

#include <cstdio>
#include <DirectXMath.h>

using namespace DirectX;

//----------------------------------------------------------
// コンストラクタ
//----------------------------------------------------------
TestScene::TestScene()
{
    //
    // 1. アセット取得
    //
    m_playerModel = AssetManager::GetModel("mdl_slime");
    if (m_playerModel)
    {
        m_playerModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
        m_playerModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
    }

    //
    // 2. システム登録
    //
    // 先にカメラを更新したいのでUpdate側に乗せる
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();
    // そのあと描画
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();

    //
    // 3. プレイヤー生成（カメラのターゲット）
    //
    {
        m_playerEntity = m_world.Create();

        // Transform
        auto& tr = m_world.Add<TransformComponent>(
            m_playerEntity,
            DirectX::XMFLOAT3{ 0.0f, 0.0f, 2.0f },   // ← z を 0 にした
            DirectX::XMFLOAT3{ 0.0f, 180.0f, 0.0f },
            DirectX::XMFLOAT3{ 0.06f, 0.06f, 0.06f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(m_playerEntity);
        mr.model = m_playerModel;
        mr.visible = true;
    }

    //
    // 4. カメラエンティティ生成
    //
    {
        EntityId camEnt = m_world.Create();

        // カメラとして認識させるタグ
        m_world.Add<ActiveCameraTag>(camEnt);

        // 位置
        auto& tr = m_world.Add<TransformComponent>(
            camEnt,
            XMFLOAT3{ 0.0f, 2.75f, -6.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 1.0f, 1.0f, 1.0f }
        );

        // カメラ本体（制御情報もここに統合した版）
        auto& cam = m_world.Add<Camera3DComponent>(camEnt);
        cam.mode = Camera3DComponent::Mode::SideScroll;     // ←今回のモード
        cam.target = m_playerEntity;                          // 追従先

        // 基本パラメータ
        cam.aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        cam.nearZ = 0.1f;
        cam.farZ = 1000.0f;

        // サイドビュー用の固定値
        cam.scrollSpeed = 0.5f;   // 好きな速度に調整
        cam.followOffsetY = 1.5f;
        cam.followMarginY = 0.5f;
        cam.sideFixedX = 0.0f;
        cam.sideFixedZ = -15.0f;
        cam.sideLookAtX = 0.0f;
        cam.lookAtOffset = XMFLOAT3{ 0.0f, 1.5f, 0.0f };
    }

    //
    // 5. ライト（前と同じ）
    //
    ModelRenderSystem::ApplyDefaultLighting(2.75f, 6.0f);
}


//----------------------------------------------------------
// デストラクタ
//----------------------------------------------------------
TestScene::~TestScene()
{
    // World側にUninitがあればここで呼ぶ
    m_world.Destroy(1); // 今回は適当、必要なら全破棄ループを書く
    // m_world.Uninit(); を作ってるならそれでもOK
}

//----------------------------------------------------------
// 毎フレーム更新
//----------------------------------------------------------
void TestScene::Update()
{
    // 今はとくにUpdateするものがなければ固定dtで回すだけでもいい
    const float dt = 1.0f / 60.0f;
    m_sys.Tick(m_world, dt);
}

//----------------------------------------------------------
// 毎フレーム描画
//----------------------------------------------------------
void TestScene::Draw()
{
    if (m_followCamera && m_drawModel)
    {
        m_drawModel->SetViewProj(m_followCamera->GetView(), m_followCamera->GetProj());
    }

    m_sys.Render(m_world);
}
