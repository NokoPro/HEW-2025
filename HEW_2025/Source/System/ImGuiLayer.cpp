/*****************************************************************//**
 * @file   ImGuiLayer.cpp
 * @brief  ImGui レイヤー実装
 * 
 * - Dear ImGui の初期化・フレーム制御をまとめた薄いラッパー実装です。
 * - 本レイヤーは IMGUI_ENABLED が定義されていない場合は空実装になります。
 * - 表示するステータスを増やしたい場合は、`DebugSettings` に項目を追加し、
 * -   `ImGuiLayer::BeginFrame()` 内のデバッグウィンドウにウィジェットを追加してください。
 * - 入力をImGuiへ渡すには、アプリの `WndProc` で `ImGuiLayer::WndProcHandler()` を呼び出します。
 * 
 * @author 浅野勇生
 * @date   2025/11/16
 *********************************************************************/
#include "ImGuiLayer.h"
#include "DirectX/DirectX.h"
#include "DebugSettings.h"

// ImGui 有効化オプション
#ifdef IMGUI_ENABLED

#  if __has_include("../../libs/imgui/imgui.h")
#    include "../../libs/imgui/imgui.h"
#    define IMGUI_HAS_CORE 1
#  elif __has_include("imgui.h")
#    include "imgui.h"
#    define IMGUI_HAS_CORE 1
#  endif

#  if __has_include("../../libs/imgui/backends/imgui_impl_win32.h")
#    include "../../libs/imgui/backends/imgui_impl_win32.h"
#    define IMGUI_HAS_WIN32 1
#  endif

#  if __has_include("../../libs/imgui/backends/imgui_impl_dx11.h")
#    include "../../libs/imgui/backends/imgui_impl_dx11.h"
#    define IMGUI_HAS_DX11 1
#  endif

// Forward declare the Win32 WndProc handler
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // IMGUI_ENABLED

/**
 * @brief ImGui の初期化
 * @details IMGUI_ENABLED かつ backends が組み込まれているときのみ実処理を行う
 */
namespace ImGuiLayer
{
    /**
     * @brief ImGui を初期化します。
     * @param hWnd アプリケーションの Win32 ウィンドウハンドル。
     * @details 次の条件が満たされている場合のみ初期化処理を行います:
     *          IMGUI_ENABLED, IMGUI_HAS_CORE, IMGUI_HAS_WIN32, IMGUI_HAS_DX11。
     *          ImGui コンテキストの作成、キーボードナビの有効化、
     *          `ImGui_ImplWin32_Init` と `ImGui_ImplDX11_Init` によるバックエンド初期化を行います。
     *          DX11 のデバイス/コンテキストは `DirectX/DirectX.h` の `GetDevice()` および `GetContext()` から取得します。
     * @note 対応する `Shutdown()` を必ず呼び出してリソースを解放してください。
     * @warning 複数回呼び出す再初期化は想定していません。
     */
    void Init(HWND hWnd)
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_WIN32) && defined(IMGUI_HAS_DX11)
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX11_Init(GetDevice(), GetContext());
#endif
    }

    /** @brief ImGui の終了処理 */
    void Shutdown()
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_WIN32) && defined(IMGUI_HAS_DX11)
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif
    }

    /**
     * @brief 1フレームの開始
     * @details ここにデバッグウィンドウ項目を追加することで表示ステータスを増やせます。
     * 例:
     *  - Checkbox("God Mode", &DebugSettings::Get().godMode);
     *  - SliderFloat("Player Speed", &DebugSettings::Get().playerSpeed, 0, 20);
     */
    void BeginFrame()
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_WIN32) && defined(IMGUI_HAS_DX11)
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

		// --- デバッグウィンドウ ---
        if (DebugSettings::Get().imguiEnabled)
        {
            ImGui::Begin("Debug Settings");
            ImGui::Checkbox("Magma Enabled (F3)", &DebugSettings::Get().magmaEnabled);
            ImGui::SliderFloat("Magma Speed Scale", &DebugSettings::Get().magmaSpeedScale, 0.0f, 3.0f, "%.2f");
            ImGui::Checkbox("Infinite Jump (F4)", &DebugSettings::Get().infiniteJump);
            ImGui::Separator();
            ImGui::Checkbox("God Mode", &DebugSettings::Get().godMode);
            ImGui::SliderFloat("Player Speed", &DebugSettings::Get().playerSpeed, 0.0f, 3.0f, "%.2f");
            ImGui::Checkbox("Show Collision (F2)", &DebugSettings::Get().showCollision);
            ImGui::End();
        }
#endif
    }

    /** @brief ImGui の描画と発行 */
    void EndFrameAndRender()
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_DX11)
        ImGui::Render();
#  if defined(IMGUI_HAS_DX11)
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#  endif
#endif
    }

    /** @brief Win32 メッセージを ImGui へ転送 */
    bool WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
#if defined(IMGUI_ENABLED)
        return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) != 0;
#else
        (void)hWnd; (void)msg; (void)wParam; (void)lParam;
        return false;
#endif
    }
}
