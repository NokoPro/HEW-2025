/*
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
#include "Game.h" // GetSceneManager() 宣言取得
#include "Scene/SceneManager.h"
#include "Scene/TestStageScene.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"
#include "ECS/Components/Physics/Collider2DComponent.h"
#include "ECS/Components/Render/ModelComponent.h"
#include "ECS/Tag/Tag.h"

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

// Forward declare global accessor if header didn’t provide (avoid unresolved symbol in some TU order)
class SceneManager; SceneManager& GetSceneManager();

namespace ImGuiLayer
{
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

    void Shutdown()
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_WIN32) && defined(IMGUI_HAS_DX11)
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif
    }

    // --- Entity 情報表示ヘルパ ---
    static void DrawPlayerList(TestStageScene* scene)
    {
        if (!scene) { ImGui::TextDisabled("Scene not available"); return; }
        World& w = scene->GetWorld();

        int count = 0;
        // Player タグ付きのみ列挙
        w.View<TagPlayer, TransformComponent>([&](EntityId e, const TagPlayer&, TransformComponent& tr)
        {
            ++count;
            ImGui::PushID((int)e);
            char label[64];
            snprintf(label, sizeof(label), "Player Entity %d", (int)e);
            if (ImGui::TreeNode(label))
            {
                ImGui::Text("Position: (%.2f, %.2f, %.2f)", tr.position.x, tr.position.y, tr.position.z);
                ImGui::Text("Rotation: (%.1f, %.1f, %.1f)", tr.rotationDeg.x, tr.rotationDeg.y, tr.rotationDeg.z);
                ImGui::Text("Scale: (%.2f, %.2f, %.2f)", tr.scale.x, tr.scale.y, tr.scale.z);

                if (w.Has<PlayerInputComponent>(e))
                {
                    const auto& inp = w.Get<PlayerInputComponent>(e);
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.7f,0.9f,1,1), "PlayerInputComponent");
                    ImGui::Text("playerIndex: %d", inp.playerIndex);
                    ImGui::Text("jumpReq: %d blinkReq: %d", inp.isJumpRequested?1:0, inp.isBlinkRequested?1:0);
                }
                if (w.Has<MovementIntentComponent>(e))
                {
                    const auto& mv = w.Get<MovementIntentComponent>(e);
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.7f,0.9f,1,1), "MovementIntentComponent");
                    ImGui::Text("moveX: %.2f facing: %d", mv.moveX, mv.facing);
                    ImGui::Text("jump:%d force:%d consumed:%d", mv.jump?1:0, mv.forceJumpRequested?1:0, mv.forceJumpConsumed?1:0);
                    ImGui::Text("blinkReq:%d speed:%.2f isBlink:%d", mv.blinkRequested?1:0, mv.blinkSpeed, mv.isBlinking?1:0);
                }
                if (w.Has<Rigidbody2DComponent>(e))
                {
                    const auto& rb = w.Get<Rigidbody2DComponent>(e);
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.7f,0.9f,1,1), "Rigidbody2DComponent");
                    ImGui::Text("vel:(%.2f, %.2f) onGround:%d", rb.velocity.x, rb.velocity.y, rb.onGround?1:0);
                }
                if (w.Has<Collider2DComponent>(e))
                {
                    const auto& col = w.Get<Collider2DComponent>(e);
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.7f,0.9f,1,1), "Collider2DComponent");
                    ImGui::Text("layer:%d hitMask:%d", col.layer, col.hitMask);
                    ImGui::Text("half:(%.2f, %.2f) offset:(%.2f, %.2f)", col.aabb.halfX, col.aabb.halfY, col.offset.x, col.offset.y);
                }
                if (w.Has<ModelRendererComponent>(e))
                {
                    const auto& mr = w.Get<ModelRendererComponent>(e);
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(1,0.85f,0.4f,1), "ModelRendererComponent");
                    ImGui::Text("visible:%d layer:%d", mr.visible?1:0, mr.layer);
                    ImGui::Text("localOffset:(%.2f,%.2f,%.2f)", mr.localOffset.x, mr.localOffset.y, mr.localOffset.z);
                    ImGui::Text("localScale:(%.2f,%.2f,%.2f)", mr.localScale.x, mr.localScale.y, mr.localScale.z);
                }
                if (w.Has<ModelAnimationComponent>(e))
                {
                    const auto& anim = w.Get<ModelAnimationComponent>(e);
                    ImGui::TextColored(ImVec4(1,0.85f,0.4f,1), "ModelAnimationComponent");
                    ImGui::Text("animeNo:%d loop:%d speed:%.2f playReq:%d", anim.animeNo, anim.loop?1:0, anim.speed, anim.playRequested?1:0);
                }
                if (w.Has<ModelAnimationStateComponent>(e))
                {
                    const auto& st = w.Get<ModelAnimationStateComponent>(e);
                    ImGui::TextColored(ImVec4(1,0.85f,0.4f,1), "ModelAnimationStateComponent");
                    ImGui::Text("current:%d requested:%d", (int)st.current, (int)st.requested);
                }
                if (w.Has<ModelAnimationTableComponent>(e))
                {
                    const auto& tbl = w.Get<ModelAnimationTableComponent>(e);
                    if (ImGui::TreeNode("AnimationTable"))
                    {
                        for (size_t i=0;i<tbl.table.size();++i)
                        {
                            const auto& d = tbl.table[i];
                            ImGui::Text("[%zu] animeNo:%d loop:%d speed:%.2f", i, d.animeNo, d.loop?1:0, d.speed);
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        });
        if (count == 0)
        {
            ImGui::TextDisabled("No player entities");
        }
    }

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

            // =====================
            // Entity Header (新規)
            // =====================
            if (ImGui::CollapsingHeader("Entities"))
            {
                ImGui::TextDisabled("Player entities only");
                SceneManager& sm = GetSceneManager();
                auto* current = sm.Current();
                TestStageScene* testScene = dynamic_cast<TestStageScene*>(current);
                DrawPlayerList(testScene);
            }

            ImGui::End();
        }
#endif
    }

    void EndFrameAndRender()
    {
#if defined(IMGUI_ENABLED) && defined(IMGUI_HAS_CORE) && defined(IMGUI_HAS_DX11)
        ImGui::Render();
#  if defined(IMGUI_HAS_DX11)
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#  endif
#endif
    }

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
