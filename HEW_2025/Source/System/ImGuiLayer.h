#pragma once
#include <Windows.h>

namespace ImGuiLayer
{
    // Initialize Dear ImGui (no-op if IMGUI_ENABLED is not defined)
    void Init(HWND hWnd);
    // Shutdown Dear ImGui (no-op if IMGUI_ENABLED is not defined)
    void Shutdown();
    // Begin a new frame (no-op if IMGUI_ENABLED is not defined)
    void BeginFrame();
    // Render and present ImGui draw data (no-op if IMGUI_ENABLED is not defined)
    void EndFrameAndRender();
}
