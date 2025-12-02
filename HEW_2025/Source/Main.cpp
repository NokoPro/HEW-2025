/*****************************************************************//**
 * @file   Main.cpp
 * @brief  ECS を用いた 3D ベースプログラムのエントリポイントおよび
 *         DirectX / 入力 / ImGui / ゲーム本体の初期化・メインループを管理する。
 *
 * @author 浅野勇生
 * @date   2025/10/20 - 制作開始、ECSの組み込み
 * @date   2025/10/21 - Player移動、カメラ追従、かんたんな物理挙動作成
 * @date   2025/10/22 - OBBとRayを使った当たり判定の作成。斜めの床とか作って判定取れるようになった
 * @date   2025/10/23 - カメラを回転できるようにした
 * @date   2025/10/24 - モデルのパスを外部ファイルから読み込めるように。
 *                      Excelのツールを用いてcsvファイルを生成可能に
 * @date   2025/10/26 - PadによるTPS視点＆操作作成
 * @date   2025/10/28 - 二人プレイ機能作成
 *
 * @update 2025/10/29 - Git共有
 * @update 2025/11/20 - a版制作開始！
 * @update 2025/11/27 - a版vr1完成！
 *
 * @author 浅野勇生
 * @author 奥田修也
 * @author 川谷優真
 * @author 篠原純
 * @author 清水光之介
 * @author 土本蒼翔
 * @author 堤翔
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
#include "System/EffectRuntime.h"

/**
 * @brief ウィンドウプロシージャの宣言。
 * @details Win32 メッセージを処理する関数。定義はファイル後半。
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/**
 * @brief アプリケーションのエントリポイント（Win32 用）。
 *
 * @param hInst     インスタンスハンドル
 * @param hPrevInst 旧インスタンスハンドル（未使用）
 * @param lpCmdLine コマンドライン文字列（未使用）
 * @param nCmdShow  ウィンドウ表示状態
 *
 * @return プロセスの終了コード
 */
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
    // メモリリーク検出を有効化（デバッグビルドのみ）
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // ------------------------------
    // ウィンドウクラスの登録
    // ------------------------------
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "DX11_ECS_Class";
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, "RegisterClassEx failed", "Error", MB_OK);
        return 0;
    }

    // ------------------------------
    // ウィンドウサイズ調整＆作成
    // ------------------------------
    RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    const DWORD style = WS_CAPTION | WS_SYSMENU;
    const DWORD exStyle = WS_EX_OVERLAPPEDWINDOW;

    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    HWND hWnd = CreateWindowEx(
        exStyle,
        wc.lpszClassName,
        APP_TITLE,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInst,
        nullptr
    );

    if (!hWnd)
    {
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // ------------------------------
    // DirectX / 入力などの初期化
    // ------------------------------
    if (FAILED(Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT)))
    {
        Uninit();
        DestroyWindow(hWnd);
        UnregisterClass(wc.lpszClassName, hInst);
        return 0;
    }

    // ImGui 初期化（有効化時のみ）
    ImGuiLayer::Init(hWnd);

	EffectRuntime::Initialize();

    // ゲーム(ECS)側の初期化
    Game_Init(hWnd, SCREEN_WIDTH, SCREEN_HEIGHT);

    // ------------------------------
    // 高精度タイマーによるフレーム制御の準備
    // ------------------------------
    timeBeginPeriod(1);  // Sleep の精度を向上（1ms 単位）

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    const double perfFreq = static_cast<double>(freq.QuadPart);  ///< パフォーマンスカウンタの1秒あたりのカウント数

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    int64_t prevCounter = counter.QuadPart;                      ///< 前フレームのカウンタ値

    const double fixedDt = kFixedDt;                             ///< 論理的な固定ステップ(秒)
    const double targetFrameTicks = perfFreq * fixedDt;          ///< 1フレームあたりの理想カウント数

    // FPS 計測用
    double fpsAccumSec = 0.0;                                    ///< FPS 計測用の経過秒数
    int    fpsFrames = 0;                                        ///< 計測期間内のフレーム数
    float  displayedFps = static_cast<float>(kTargetFps);        ///< 画面に表示する FPS 値

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    // ------------------------------
    // メインループ
    // ------------------------------
    while (true)
    {
        // Windows メッセージ処理
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

        // 現在カウンタ値の取得
        LARGE_INTEGER cur;
        QueryPerformanceCounter(&cur);
        int64_t now = cur.QuadPart;
        int64_t deltaTicks = now - prevCounter;

        // 目標フレーム時間より短ければ Sleep で残り時間を消化する
        if (deltaTicks < static_cast<int64_t>(targetFrameTicks))
        {
            // 残り時間 [秒]
            const double remaining = (targetFrameTicks - deltaTicks) / perfFreq;

            if (remaining > 0.0)
            {
                // ミリ秒に変換し、少しマージンを引いて Sleep
                DWORD ms = static_cast<DWORD>(remaining * 1000.0 - 0.25);
                if (ms > 0)
                {
                    Sleep(ms);
                }
            }

            // フレーム境界まで達していないので再チェック
            continue;
        }

        // 今フレームを基準とする
        prevCounter = now;

        // 実際の経過時間を秒に変換
        const double realDt = static_cast<double>(deltaTicks) / perfFreq;

        // FPS 計測
        fpsAccumSec += realDt;
        ++fpsFrames;

        if (fpsAccumSec >= 1.0)
        {
            displayedFps = static_cast<float>(fpsFrames / fpsAccumSec);
            fpsAccumSec -= 1.0;
            fpsFrames = 0;
        }

        // デバッグ表示用 FPS を更新
        DebugSettings::Get().fpsValue = displayedFps;

        // ImGui の有効 / 無効切り替え（F5）
        if (IsKeyTrigger(VK_F5))
        {
            DebugSettings::Get().imguiEnabled = !DebugSettings::Get().imguiEnabled;
        }

        // ------------------------------
        // フレーム処理
        // ------------------------------

        // ImGui フレーム開始
        ImGuiLayer::BeginFrame();

        // 入力更新 & ゲーム更新（固定ステップ1回分）
        // ※ 実際には fixedDt を渡す Update ループなどに拡張可能
        UpdateInput();
        Game_Update();

        // 描画処理
        BeginDrawDirectX();
        Draw();

        RenderTarget* rtv = GetDefaultRTV();
        DepthStencil* dsv = GetDefaultDSV();
        SetRenderTargets(1, &rtv, dsv);

        // ImGui 描画＆Present まで
        ImGuiLayer::EndFrameAndRender();
        EndDrawDirectX();
    }

    // 高精度タイマー設定を元に戻す
    timeEndPeriod(1);

    // ------------------------------
    // 終了処理
    // ------------------------------
    Game_Uninit();
    ImGuiLayer::Shutdown();

    Uninit();
    DestroyWindow(hWnd);
    UnregisterClass(wc.lpszClassName, hInst);

    return static_cast<int>(msg.wParam);
}

/**
 * @brief ウィンドウプロシージャ。
 *
 * @details
 *  - まず ImGui にメッセージを渡し、消費された場合はそこで処理を終了します。
 *  - マウスホイールや WM_DESTROY 等の最低限のメッセージのみをローカルで処理し、
 *    その他は DefWindowProc にフォールバックします。
 *
 * @param hWnd   ウィンドウハンドル
 * @param message メッセージID
 * @param wParam  メッセージ固有の追加情報
 * @param lParam  メッセージ固有の追加情報
 *
 * @return 処理結果（LRESULT）
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // 先に ImGui にメッセージを渡し、消費された場合はここで終了
    if (ImGuiLayer::WndProcHandler(hWnd, message, wParam, lParam))
    {
        return 1;
    }

    switch (message)
    {
    case WM_MOUSEWHEEL:
    {
        const int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        Input_OnMouseWheel(wheelDelta);
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * @brief DirectX / 入力 / シェーダ / スプライト / アセット等の初期化関数。
 *
 * @param hWnd   作成済みウィンドウハンドル
 * @param width  クライアント領域の幅
 * @param height クライアント領域の高さ
 *
 * @retval S_OK   初期化成功
 * @retval E_FAIL いずれかの初期化に失敗
 */
HRESULT Init(HWND hWnd, UINT width, UINT height)
{
    // DirectX 初期化
    if (FAILED(InitDirectX(hWnd, width, height, false)))
    {
        return E_FAIL;
    }

    // 入力初期化
    if (FAILED(InitInput()))
    {
        return E_FAIL;
    }

    // シェーダ・ジオメトリ・スプライトの初期化
    ShaderList::Init();
    Geometory::Init();
    Sprite::Init();

    // アセットカタログ・マネージャ初期化
    AssetCatalog::Clear();
    AssetCatalog::LoadCsv("Assets/Data.csv");
    AssetManager::Init();

    return S_OK;
}

/**
 * @brief Init で確保・初期化した各種システムの終了処理をまとめて行う。
 */
void Uninit()
{
    Sprite::Uninit();
    Geometory::Uninit();
    ShaderList::Uninit();

    UninitInput();
    EffectRuntime::Shutdown();
    UninitDirectX();

}

/**
 * @brief ゲームの更新処理（ラッパ関数）。
 *
 * @details
 *  現在は Main ループから直接 `UpdateInput()` と `Game_Update()` を呼び出しているため
 *  使用していないが、将来的にメインループと責務を分ける場合のために残している。
 */
void Update()
{
    // 入力更新
    UpdateInput();

    // ゲーム(ECS)の更新
    Game_Update();
}

/**
 * @brief 描画処理（ラッパ関数）。
 *
 * @details
 *  Present はメインループ側（`EndDrawDirectX()`）で行うため、
 *  ここではゲームシーンの描画のみを行う。
 */
void Draw()
{
    // ゲーム(ECS)の描画のみ。Present はメインループ側で行う。
    Game_Draw();
}
