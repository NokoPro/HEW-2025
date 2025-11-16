/*****************************************************************//**
 * @file   ImGuiLayer.h
 * @brief  ImGUIのレイヤー定義
 * 
 * @author 浅野勇生
 * @date   2025/11/16
 *********************************************************************/
#pragma once
#include <Windows.h>

/**
 * @file ImGuiLayer.h
 * @brief Dear ImGui の初期化・フレーム制御をまとめた薄いラッパー
 * @details
 * - 本レイヤーは IMGUI_ENABLED が定義されていない場合は空実装になります。
 * - 表示するステータスを増やしたい場合は、`DebugSettings` に項目を追加し、
 *   `ImGuiLayer::BeginFrame()` 内のデバッグウィンドウにウィジェットを追加してください。
 * - 入力をImGuiへ渡すには、アプリの `WndProc` で `ImGuiLayer::WndProcHandler()` を呼び出します。
 */
namespace ImGuiLayer
{
    /** @brief ImGui の初期化（有効化時のみ動作） */
    void Init(HWND hWnd);
    /** @brief ImGui の終了処理（有効化時のみ動作） */
    void Shutdown();
    /** @brief 1フレームの開始（有効化時のみ動作） */
    void BeginFrame();
    /** @brief 1フレームの終了と描画（有効化時のみ動作） */
    void EndFrameAndRender();
    /**
     * @brief Win32 メッセージのImGui転送
     * @return メッセージをImGuiが消費した場合 true
     */
    bool WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}
