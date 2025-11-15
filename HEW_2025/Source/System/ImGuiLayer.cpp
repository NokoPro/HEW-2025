#include "ImGuiLayer.h"
#include "DirectX/DirectX.h"
#include "DebugSettings.h"

// We guard all ImGui usage behind IMGUI_ENABLED so the project builds without the library.
#ifdef IMGUI_ENABLED

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")

#endif // IMGUI_ENABLED

namespace ImGuiLayer
{
    void Init(HWND hWnd)
    {
#ifdef IMGUI_ENABLED
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
#ifdef IMGUI_ENABLED
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif
    }

    void BeginFrame()
    {
#ifdef IMGUI_ENABLED
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Simple debug window hook using DebugSettings
        if (DebugSettings::Get().imguiEnabled)
        {
            ImGui::Begin("Debug Settings");
            ImGui::Checkbox("Magma Enabled (F3)", &DebugSettings::Get().magmaEnabled);
            ImGui::SliderFloat("Magma Speed Scale", &DebugSettings::Get().magmaSpeedScale, 0.0f, 3.0f, "%.2f");
            ImGui::Checkbox("Infinite Jump (F4)", &DebugSettings::Get().infiniteJump);
            ImGui::End();
        }
#endif
    }

    void EndFrameAndRender()
    {
#ifdef IMGUI_ENABLED
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
    }
}
