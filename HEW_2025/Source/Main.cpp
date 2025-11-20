/*****************************************************************//**
 * @file   Main.cpp
 * @brief  ECSを用いた3Dベースプログラム
 * 
 * @author 浅野勇生
 * @date   2025/10/20 - 制作開始、ECSの組み込み
 * @date   2025/10/21 - Player移動、カメラ追従、かんたんな物理挙動作成
 * @date   2025/10/22 - OBBとRayを使った当たり判定の作成。斜めの床とか作って判定取れるようになった
 * @date   2025/10/23 - カメラを回転できるようにした
 * @date   2025/10/24 - モデルのパスを外部ファイルから読み込めるように。Excelのツールを用いてcsvファイルを生成可能に
 * @date   2025/10/26 - PadによるTPS視点＆操作作成
 * @date   2025/10/28 - 二人プレイ機能作成
 * 
 * @update 2025/10/29 - Git共有
 * 
 * @update 2025/11/20 - a版制作開始
 *
 * @author 浅野勇生
 * @author 奥田修也
 * @author 川谷優真
 * @author 篠原純
 * @author 清水光之介
 * @author 土本蒼翔
 * @author 堤翔
 * mas
 *********************************************************************/
#include "Main.h"
#include <memory>
#include <crtdbg.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "System/Defines.h"
#include "System/Input.h"
#include "System/DirectX/ShaderList.h"
#include "System/Geometory.h"
#include "System/Sprite.h"
#include "System/AssetCatalog.h"
#include "System/AssetManager.h"
#include "System/ImGuiLayer.h"
#include "System/DebugSettings.h"
#include "System/DirectX/DirectX.h"

 // 追加：ゲーム本体
#include "Game.h"

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // --- 以下はあなたの元のコードと同じ ---
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "DX11_ECS_Class";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, "RegisterClassEx failed", "Error", MB_OK);
        return 0;
    }

    RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    const DWORD style = WS_CAPTION | WS_SYSMENU;
    const DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    HWND hWnd = CreateWindowEx(
        exStyle, wc.lpszClassName, APP_TITLE, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInst, nullptr
    );
    if (!hWnd)
    {
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // DXや入力の初期化
    if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)))
    {
        Uninit();
        DestroyWindow(hWnd);
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    // ImGui初期化（有効化時のみ）
    ImGuiLayer::Init(hWnd);

    // ここでゲーム(ECS)側も初期化
    Game_Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT);

    timeBeginPeriod(1);
    const double targetMs = 1000.0 / static_cast<double>(fFPS);
    DWORD lastTick = timeGetTime();

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    for (;;)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        const DWORD now = timeGetTime();
        const double elapsed = static_cast<double>(now - lastTick);

        if (elapsed >= targetMs)
        {
            // F5でUI表示切り替え（キー入力はInput更新前でも問題なしのためここで）
            if (IsKeyTrigger(VK_F5))
            {
                DebugSettings::Get().imguiEnabled = !DebugSettings::Get().imguiEnabled;
            }

            // フレーム開始（ImGui → 3D描画 → ImGui描画 → Present）
            ImGuiLayer::BeginFrame();

            // 入力とゲーム更新
            Update();

            // バックバッファのクリアとシーン描画
            BeginDrawDirectX();
            Draw();

            // 念のためデフォルトのレンダーターゲットに戻してからImGui描画
            {
                RenderTarget* rtv = GetDefaultRTV();
                DepthStencil* dsv = GetDefaultDSV();
                SetRenderTargets(1, &rtv, dsv);
            }
            // ImGui描画を行ってからPresent
            ImGuiLayer::EndFrameAndRender();
            EndDrawDirectX();

            lastTick = now;
        }
        else
        {
            const DWORD sleepMs = static_cast<DWORD>(targetMs - elapsed);
            if (sleepMs > 0)
            {
                Sleep(sleepMs);
            }
        }
    }

    timeEndPeriod(1);

    // ゲーム側終了
    Game_Uninit();

    // ImGui終了
    ImGuiLayer::Shutdown();

    Uninit();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, hInst);
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Give ImGui a chance to consume the event first
    if (ImGuiLayer::WndProcHandler(hWnd, message, wParam, lParam))
    {
        return 1;
    }

    switch (message)
    {
    case WM_MOUSEWHEEL:
        Input_OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT Init(HWND hWnd, UINT width, UINT height)
{
    if (FAILED(InitDirectX(hWnd, width, height, false)))
    {
        return E_FAIL;
    }

    if (FAILED(InitInput()))
    {
        return E_FAIL;
    }
    ShaderList::Init();
    Geometory::Init();
    Sprite::Init();

    AssetCatalog::Clear();
    AssetCatalog::LoadCsv("Assets/Data.csv");
    AssetManager::Init();

    return S_OK;
}

void Uninit()
{
    Sprite::Uninit();
    Geometory::Uninit();
    ShaderList::Uninit();
    UninitInput();
    UninitDirectX();
}

void Update()
{
    // 入力更新
    UpdateInput();

    // ゲーム(ECS)の更新
    Game_Update();
}

void Draw()
{
    // ゲーム(ECS)の描画のみ。Presentはメインループ側で行う。
    Game_Draw();
}