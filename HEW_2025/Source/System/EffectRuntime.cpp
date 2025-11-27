/*****************************************************************//**
 * @file   EffectRuntime.cpp
 * @brief  Effekseer ベースのエフェクト再生ランタイム実装
 *
 * - Guide_Cpp_Ja.md の「6. アプリケーションでエフェクトを再生する」
 *   「7. カメラと座標系をアプリケーションと一致させる」
 *   「8. 付録: エフェクトの基本的な制御」をベースにした実装
 *********************************************************************/

#include "EffectRuntime.h"
#include "System/DirectX/DirectX.h"

 // Effekseer
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>

#ifndef _DEBUG
#pragma comment(lib, "Effekseer.lib")
//#pragma comment(lib, "EffekseerRendererCommon.lib")
#pragma comment(lib, "EffekseerRendererDX11.lib")
#else
#pragma comment(lib, "Effekseerd.lib")
//#pragma comment(lib, "EffekseerRendererCommond.lib")
#pragma comment(lib, "EffekseerRendererDX11d.lib")
#endif

using namespace DirectX;

// ヘッダ側で using Matrix44 = ::Effekseer::Matrix44; を宣言済み

namespace
{
    // Effekseer のコアオブジェクト
    ::Effekseer::ManagerRef               g_manager;
    ::EffekseerRenderer::RendererRef      g_renderer;
    ::Effekseer::Backend::GraphicsDeviceRef g_graphicsDevice;

    bool g_initialized = false;
    bool g_coordinateLHSet = false;

    // ローカル変換ヘルパー
    ::Effekseer::Vector3D ToEffekseerVector(const XMFLOAT3& v)
    {
        return ::Effekseer::Vector3D(v.x, v.y, v.z);
    }

    // 超シンプルな ASCII → UTF-16 変換（日本語パスを使わない前提）
    std::u16string ToUtf16Ascii(const char* src)
    {
        std::u16string dst;
        if (src == nullptr)
        {
            return dst;
        }

        while (*src != '\0')
        {
            dst.push_back(static_cast<char16_t>(*src));
            ++src;
        }

        return dst;
    }

    Effekseer::Matrix44 g_projEfk{};
    Effekseer::Matrix44 g_viewEfk{};

    // DirectX の転置行列（XMFLOAT4X4）→ Effekseer::Matrix44
    void ConvertXMFloat4x4T_To_Efk(const DirectX::XMFLOAT4X4& srcT,
        Effekseer::Matrix44& dst)
    {
        using namespace DirectX;

        // FollowCameraSystem が返すのは転置済みなので、まず元に戻す
        XMMATRIX m = XMMatrixTranspose(XMLoadFloat4x4(&srcT));

        XMFLOAT4X4 rowMajor{};
        XMStoreFloat4x4(&rowMajor, m);

        for (int r = 0; r < 4; ++r)
        {
            dst.Values[r][0] = rowMajor.m[r][0];
            dst.Values[r][1] = rowMajor.m[r][1];
            dst.Values[r][2] = rowMajor.m[r][2];
            dst.Values[r][3] = rowMajor.m[r][3];
        }
    }
} // namespace

//-------------------------------------------------------------------------
//  Public API
//-------------------------------------------------------------------------

bool EffectRuntime::Initialize()
{
    if (g_initialized)
    {
        return true;
    }

    ID3D11Device* device = GetDevice();
    ID3D11DeviceContext* dc = GetContext();

    if (device == nullptr || dc == nullptr)
    {
        // DirectX 側が初期化されていない
        return false;
    }

    // 6.4. 描画モジュールの作成 (DirectX11)
    // Create a graphics device
    g_graphicsDevice = ::EffekseerRendererDX11::CreateGraphicsDevice(device, dc);
    if (!g_graphicsDevice.Get())
    {
        return false;
    }

    // Create a renderer of effects
    g_renderer = ::EffekseerRendererDX11::Renderer::Create(g_graphicsDevice, 8000);
    if (!g_renderer.Get())
    {
        g_graphicsDevice.Reset();
        return false;
    }

    // 6.3. Manager の作成
    g_manager = ::Effekseer::Manager::Create(8000);
    if (!g_manager.Get())
    {
        g_renderer.Reset();
        g_graphicsDevice.Reset();
        return false;
    }

    // 6.5. 描画モジュールの設定
    g_manager->SetSpriteRenderer(g_renderer->CreateSpriteRenderer());
    g_manager->SetRibbonRenderer(g_renderer->CreateRibbonRenderer());
    g_manager->SetRingRenderer(g_renderer->CreateRingRenderer());
    g_manager->SetTrackRenderer(g_renderer->CreateTrackRenderer());
    g_manager->SetModelRenderer(g_renderer->CreateModelRenderer());

    // 6.5. ローダーの設定
    g_manager->SetTextureLoader(g_renderer->CreateTextureLoader());
    g_manager->SetModelLoader(g_renderer->CreateModelLoader());
    g_manager->SetMaterialLoader(g_renderer->CreateMaterialLoader());
    g_manager->SetCurveLoader(::Effekseer::MakeRefPtr<::Effekseer::CurveLoader>());

    // 6.6. 座標系の設定
    // DirectX11 の左手座標系に合わせて LH にしておく
    g_manager->SetCoordinateSystem(::Effekseer::CoordinateSystem::LH);
    g_coordinateLHSet = true;

    g_initialized = true;
    return true;
}

void EffectRuntime::Shutdown()
{
    g_manager.Reset();
    g_renderer.Reset();
    g_graphicsDevice.Reset();

    g_initialized = false;
    g_coordinateLHSet = false;
}

void EffectRuntime::Update(float dt)
{
    (void)dt; // 現状、Effekseer の UpdateParameter には dt を渡していない

    if (!g_manager.Get())
    {
        return;
    }

    // 6.11. Manager の更新
    ::Effekseer::Manager::UpdateParameter updateParameter;
    g_manager->Update(updateParameter);
}

void EffectRuntime::Render()
{
    if (!g_renderer.Get() || !g_manager.Get())
        return;

    // 前フレームの Update で保存されたカメラ行列を適用
    g_renderer->SetProjectionMatrix(g_projEfk);
    g_renderer->SetCameraMatrix(g_viewEfk);

    g_renderer->BeginRendering();

    Effekseer::Manager::DrawParameter dp;
    dp.ZNear = 0.0f;
    dp.ZFar = 1000.0f;

    g_manager->Draw(dp);

    g_renderer->EndRendering();
}


EffectRuntime::Handle EffectRuntime::Play(const char* pathUtf8, const XMFLOAT3& worldPos, bool loop)
{
    if (!g_manager.Get() || pathUtf8 == nullptr || *pathUtf8 == '\0')
    {
        return -1;
    }

    // 6.8. エフェクト読み込み
    const std::u16string path16 = ToUtf16Ascii(pathUtf8);
    ::Effekseer::EffectRef effect = ::Effekseer::Effect::Create(g_manager, path16.c_str());
    if (!effect.Get())
    {
        return -1;
    }

    // 8.1. 再生
    ::Effekseer::Handle h = g_manager->Play(effect,
        worldPos.x,
        worldPos.y,
        worldPos.z);
    if (h < 0)
    {
        return -1;
    }

    // ループ制御は基本的にエフェクト側の設定で行う。
    // ECS 側では「loop=true の場合は IsFinished を見て自動 Destroy しない」
    // という使い方だけに留めている。
    (void)loop;

    return static_cast<Handle>(h);
}

void EffectRuntime::SetCamera(const DirectX::XMFLOAT4X4& viewT,
    const DirectX::XMFLOAT4X4& projT)
{
    if (!g_renderer.Get() || !g_manager.Get())
        return;

    // DirectX のカメラ行列を Effekseer 行列に変換
    ConvertXMFloat4x4T_To_Efk(viewT, g_viewEfk);
    ConvertXMFloat4x4T_To_Efk(projT, g_projEfk);
}

void EffectRuntime::SetLocation(Handle handle, const XMFLOAT3& worldPos)
{
    if (!g_manager.Get() || handle < 0)
    {
        return;
    }

    // 8.2. SetLocation
    g_manager->SetLocation(static_cast<::Effekseer::Handle>(handle),
        worldPos.x,
        worldPos.y,
        worldPos.z);
}

void EffectRuntime::Stop(Handle handle)
{
    if (!g_manager.Get() || handle < 0)
    {
        return;
    }

    // 8.5. StopEffect
    g_manager->StopEffect(static_cast<::Effekseer::Handle>(handle));
}

bool EffectRuntime::IsFinished(Handle handle)
{
    if (!g_manager.Get() || handle < 0)
    {
        // Manager 自体が無い or 無効ハンドル → 「終わっている」と扱う
        return true;
    }

    // 8.x. Exists を使って終了チェック
    const bool exists = g_manager->Exists(static_cast<::Effekseer::Handle>(handle));
    return !exists;
}
