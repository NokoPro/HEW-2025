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
#include "TimeAttackManager.h"
#include "GameplayConfig.h" // 追加: 外部調整値

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

#include "Debug.h" // Debug.hをインクルード

/**
 * @brief ImGui の初期化
 * @details IMGUI_ENABLED かつ backends がインクルードされている時のみ処理を実行
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

        Debug::DrawImGuiLogWindow();

        auto& ds = DebugSettings::Get();
        if (ds.imguiEnabled)
        {
            ImGui::Begin("Debug Settings");

            if (ImGui::CollapsingHeader("Debug Config"))
            {
                ImGui::Checkbox("Magma Enabled", &ds.magmaEnabled);
                ImGui::SliderFloat("Magma Speed Scale", &ds.magmaSpeedScale, 0.0f, 3.0f, "%.2f");
                ImGui::Checkbox("Infinite Jump", &ds.infiniteJump);
                ImGui::Separator();
                ImGui::Checkbox("God Mode", &ds.godMode);
                ImGui::SliderFloat("Player Speed", &ds.playerSpeed, 0.0f, 3.0f, "%.2f");
                ImGui::Checkbox("Show Collision (F2)", &ds.showCollision);
                ImGui::Checkbox("Show FPS", &ds.fpsEnabled);
                if (ds.fpsEnabled) ImGui::Text("FPS: %.1f", ds.fpsValue);
                ImGui::Separator();
            }


            // Gameplay tunables (CSV連携)
            auto& gcfg = GlobalGameplayConfig::Instance().GetMutable();
            float groundAccel = gcfg.Get("groundAccel", 40.0f);
            float airAccel = gcfg.Get("airAccel", 10.0f);
            float maxSpeedX = gcfg.Get("maxSpeedX", 6.0f);
            float jumpSpeed = gcfg.Get("jumpSpeed", 15.5f);
            float blinkUpImpulse = gcfg.Get("blinkUpImpulse", 2.5f);

            if (ImGui::CollapsingHeader("Gameplay Config"))
            {
                ImGui::Text("(Editable, saved to CSV)");
                if (ImGui::SliderFloat("Ground Accel", &groundAccel, 0.0f, 120.0f, "%.2f")) gcfg.Set("groundAccel", groundAccel);
                if (ImGui::SliderFloat("Air Accel", &airAccel, 0.0f, 60.0f, "%.2f")) gcfg.Set("airAccel", airAccel);
                if (ImGui::SliderFloat("Max Speed X", &maxSpeedX, 0.0f, 20.0f, "%.2f")) gcfg.Set("maxSpeedX", maxSpeedX);
                if (ImGui::SliderFloat("Jump Speed", &jumpSpeed, 0.0f, 50.0f, "%.2f")) gcfg.Set("jumpSpeed", jumpSpeed);
                if (ImGui::SliderFloat("Blink Up Impulse", &blinkUpImpulse, 0.0f, 20.0f, "%.2f")) gcfg.Set("blinkUpImpulse", blinkUpImpulse);
                if (ImGui::Button("Save Gameplay CSV"))
                {
                    GlobalGameplayConfig::Instance().Save();
                }
                ImGui::SameLine();
                if (ImGui::Button("Reload Gameplay CSV"))
                {
                    GlobalGameplayConfig::Instance().Reload("Assets/Config/gameplay.csv");
                }
            }
            ImGui::Separator();

            // Game Timer (legacy)
            ImGui::Text("Legacy Time: %.2f s", ds.gameElapsedSeconds);
            auto& ta = TimeAttackManager::Get();
            ImGui::Separator();
            ImGui::Text("Time Attack");
            switch (ta.GetState())
            {
            case TimeAttackManager::State::Ready: ImGui::TextColored(ImVec4(0.7f,0.7f,0.7f,1), "State: READY"); break;
            case TimeAttackManager::State::Countdown: ImGui::TextColored(ImVec4(1,1,0,1), "State: COUNTDOWN"); break;
            case TimeAttackManager::State::Running: ImGui::TextColored(ImVec4(0,1,0,1), "State: RUNNING"); break;
            case TimeAttackManager::State::Cleared: ImGui::TextColored(ImVec4(0,0.6f,1,1), "State: CLEARED"); break;
            case TimeAttackManager::State::Dead: ImGui::TextColored(ImVec4(1,0,0,1), "State: DEAD"); break;
            }
            if (ta.GetState() == TimeAttackManager::State::Countdown)
                ImGui::Text("Countdown: %.2f", ta.GetCountdownRemaining());
            ImGui::Text("Current: %s", TimeAttackManager::FormatTime(ta.GetElapsed()).c_str());
            if (ta.HasBestTime()) ImGui::Text("Best: %s", TimeAttackManager::FormatTime(ta.GetBestTime()).c_str());
            else ImGui::Text("Best: --:--.--");
            if (ImGui::Button("Restart Run")) { ta.Reset(); ta.StartCountdown(3.0f); ds.gameCleared=false; ds.gameDead=false; ds.gameTimerRunning=false; ds.gameElapsedSeconds=0.0f; }
            ImGui::Separator();
            ImGui::Text("Cheats");
            if (ImGui::Button("Cheat: Force Clear"))
            {
                ds.gameDead = false; ds.gameCleared = true; ds.gameTimerRunning = false;
                if (ta.GetState() == TimeAttackManager::State::Countdown) { ta.StartRun(); }
                ta.NotifyClear();
            }
            if (ImGui::Button("Cheat: Force Game Over"))
            {
                ds.gameCleared = false; ds.gameDead = true; ds.gameTimerRunning = false;
                if (ta.GetState() == TimeAttackManager::State::Countdown) { ta.NotifyDeath(); }
                else if (ta.GetState() == TimeAttackManager::State::Running) { ta.NotifyDeath(); }
            }
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
