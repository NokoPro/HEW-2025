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

// Add PhysicsStepSystem include
#include "ECS/Systems/Update/Physics/PhysicsStepSystem.h"

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
    m_groundModel = AssetManager::GetModel("mdl_ground");
    if (m_groundModel)
    {
        m_groundModel->SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
        m_groundModel->SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));
    }

    //
    // 2. System登録（順番が重要）
    //
    // 2-1 入力 → Intent
    m_sys.AddUpdate<PlayerInputSystem>();
    // 2-2 Intent → Rigidbody（加速・ジャンプ・重力）
    m_sys.AddUpdate<MovementApplySystem>();
    // 2-2.5 Rigidbody の速度を位置に反映する物理ステップ（重力で変化した速度を位置へ適用）
    m_sys.AddUpdate<PhysicsStepSystem>(&m_colBuf);
    // 2-3 当たり判定（めり込み解消＆イベント）
    m_sys.AddUpdate<Collision2DSystem>(&m_colBuf);
    // 2-4 カメラ（最終位置を見たいので最後）
    m_followCamera = &m_sys.AddUpdate<FollowCameraSystem>();

    // 2-5 描画
    m_drawModel = &m_sys.AddRender<ModelRenderSystem>();
    
    m_debugCollision = &m_sys.AddRender<CollisionDebugRenderSystem>();

    //
    // 3. 床を1枚つくる（静的コライダー）
    //
    {
        EntityId ground = m_world.Create();

        // 位置は画面のちょい下あたり
        auto& tr = m_world.Add<TransformComponent>(
            ground,
            XMFLOAT3{ 0.0f, -5.0f, 0.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 20.0f, 1.0f, 1.0f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(ground);
        mr.model = m_groundModel;

        mr.localScale = {1.f,1.f,1.f};

        mr.visible = true;

        auto& col = m_world.Add<Collider2DComponent>(ground);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_GROUND;
        col.hitMask = Physics::LAYER_PLAYER;
        col.isStatic = true;    // ←動かないやつ
    }

    {
        EntityId ground = m_world.Create();

        // 位置は画面のちょい下あたり
        auto& tr = m_world.Add<TransformComponent>(
            ground,
            XMFLOAT3{ 30.0f, 2.0f, 0.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 20.0f, 1.0f, 1.0f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(ground);
        mr.model = m_groundModel;

        mr.localScale = { 1.f,1.f,1.f };

        mr.visible = true;

        auto& col = m_world.Add<Collider2DComponent>(ground);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_GROUND;
        col.hitMask = Physics::LAYER_PLAYER;
        col.isStatic = true;    // ←動かないやつ
    }

    {
        EntityId ground = m_world.Create();

        // 位置は画面のちょい下あたり
        auto& tr = m_world.Add<TransformComponent>(
            ground,
            XMFLOAT3{ 0.0f, 25.0f, 0.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 0.5f, 50.0f, 1.0f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(ground);
        mr.model = m_groundModel;

        mr.localScale = { 1.f,1.f,1.f };

        mr.visible = true;

        auto& col = m_world.Add<Collider2DComponent>(ground);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_GROUND;
        col.hitMask = Physics::LAYER_PLAYER;
        col.isStatic = true;    // ←動かないやつ
    }

    {
        EntityId ground = m_world.Create();

        // 位置は画面のちょい下あたり
        auto& tr = m_world.Add<TransformComponent>(
            ground,
            XMFLOAT3{ 18.0f, 25.0f, 0.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 1.0f, 50.0f, 1.0f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(ground);
        mr.model = m_groundModel;

        mr.localScale = { 1.f,1.f,1.f };

        mr.visible = true;

        auto& col = m_world.Add<Collider2DComponent>(ground);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_GROUND;
        col.hitMask = Physics::LAYER_PLAYER;
        col.isStatic = true;    // ←動かないやつ
    }

    {
        EntityId ground = m_world.Create();

        // 位置は画面のちょい下あたり
        auto& tr = m_world.Add<TransformComponent>(
            ground,
            XMFLOAT3{ -18.0f, 25.0f, 0.0f },
            XMFLOAT3{ 0.0f, 0.0f, 0.0f },
            XMFLOAT3{ 1.0f, 50.0f, 1.0f }
        );

        auto& mr = m_world.Add<ModelRendererComponent>(ground);
        mr.model = m_groundModel;

        mr.localScale = { 1.f,1.f,1.f };

        mr.visible = true;

        auto& col = m_world.Add<Collider2DComponent>(ground);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = tr.scale.x;
        col.aabb.halfY = tr.scale.y;
        col.layer = Physics::LAYER_GROUND;
        col.hitMask = Physics::LAYER_PLAYER;
        col.isStatic = true;    // ←動かないやつ
    }


    //
    // 3. プレイヤー生成（カメラのターゲット）
    //
    {
        m_playerEntity = m_world.Create();

        // Transform
        auto& tr = m_world.Add<TransformComponent>(
            m_playerEntity,
            DirectX::XMFLOAT3{ -10.0f, 0.0f, 0.0f },   // ← z を 0 にした
            DirectX::XMFLOAT3{ 0.0f, 180.0f, 0.0f },
            DirectX::XMFLOAT3{ 0.06f, 0.06f, 0.06f }
        );

        // 入力の対象にする（1P）
        auto& inp = m_world.Add<PlayerInputComponent>(m_playerEntity);
        inp.playerIndex = 0;   // ←2Pを試すときはここを1にするだけ

        // 入力の結果を受け取る箱
        m_world.Add<MovementIntentComponent>(m_playerEntity);

        // 物理ボディ
        auto& rb = m_world.Add<Rigidbody2DComponent>(m_playerEntity);
        rb.useGravity = true;

        auto& mr = m_world.Add<ModelRendererComponent>(m_playerEntity);
        mr.model = m_playerModel;
        mr.visible = true;

        // 当たり（プレイヤーの当たり判定）
        auto& col = m_world.Add<Collider2DComponent>(m_playerEntity);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = 1.5f;
        col.aabb.halfY = 1.5f;
        col.layer = Physics::LAYER_PLAYER;
        col.hitMask = Physics::LAYER_GROUND;
        col.isStatic = false;
		col.offset = XMFLOAT2{ 0.0f, 1.5f }; // 足元に合わせる
    }

    {
        m_playerEntity2 = m_world.Create();

        // Transform
        auto& tr = m_world.Add<TransformComponent>(
            m_playerEntity2,
            DirectX::XMFLOAT3{ 10.0f, 0.0f, 0.0f },   // ← z を 0 にした
            DirectX::XMFLOAT3{ 0.0f, 180.0f, 0.0f },
            DirectX::XMFLOAT3{ 0.06f, 0.06f, 0.06f }
        );

        // 入力の対象にする（1P）
        auto& inp = m_world.Add<PlayerInputComponent>(m_playerEntity2);
        inp.playerIndex = 1;   // ←2Pを試すときはここを1にするだけ

        // 入力の結果を受け取る箱
        m_world.Add<MovementIntentComponent>(m_playerEntity2);

        // 物理ボディ
        auto& rb = m_world.Add<Rigidbody2DComponent>(m_playerEntity2);
        rb.useGravity = true;

        auto& mr = m_world.Add<ModelRendererComponent>(m_playerEntity2);
        mr.model = m_playerModel;
        mr.visible = true;

        // 当たり（プレイヤーの当たり判定）
        auto& col = m_world.Add<Collider2DComponent>(m_playerEntity2);
        col.shape = ColliderShapeType::AABB2D;
        col.aabb.halfX = 1.5f;
        col.aabb.halfY = 1.5f;
        col.layer = Physics::LAYER_PLAYER;
        col.hitMask = Physics::LAYER_GROUND;
        col.isStatic = false;
        col.offset = XMFLOAT2{ 0.0f, 1.5f }; // 足元に合わせる
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
            XMFLOAT3{ 0.0f, 0.f, -16.0f },
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
        cam.farZ = 10.0f;

        // サイドビュー用の固定値
        cam.scrollSpeed = 0.5f;   // 好きな速度に調整
        cam.followOffsetY = 1.5f;
        cam.followMarginY = 0.5f;
        cam.sideFixedX = 0.0f;
        cam.sideFixedZ = -15.0f;
        cam.sideLookAtX = 0.0f;
        cam.lookAtOffset = XMFLOAT3{ 0.0f, -5.5f, 0.0f };
        cam.orthoHeight = 20.0f;
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
    if (m_followCamera)
    {
        const auto& V = m_followCamera->GetView();
        const auto& P = m_followCamera->GetProj();

        if (m_drawModel)
            m_drawModel->SetViewProj(V, P);
        if (m_debugCollision)
            m_debugCollision->SetViewProj(V, P);
    }

    m_sys.Render(m_world);
}
