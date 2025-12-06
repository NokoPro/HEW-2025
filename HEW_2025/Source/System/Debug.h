#pragma once
#include <string>
#include <vector>
#include <mutex>

// ImGuiが有効な場合のみインクルード
#if defined(_DEBUG) || defined(IMGUI_ENABLED)
#include "imgui.h"
#endif

enum class LogLevel
{
    Info,
    Warning,
    Error
};

/**
 * @class Debug
 * @brief デバッグ用のログ出力機能を提供します。
 * @details
 *  - ログは内部バッファに保存され、ImGuiウィンドウに表示できます。
 *  - スレッドセーフです。
 */
class Debug
{
public:
    /**
     * @brief ログメッセージを追加します。
     * @param level ログレベル (Info, Warning, Error)
     * @param message ログメッセージ
     */
    static void Log(LogLevel level, const std::string& message);

    /**
     * @brief ImGuiウィンドウを描画します。
     * @details
     *  - この関数はImGuiのフレーム内 (ImGui::Begin/Endの間) で呼び出す必要があります。
     *  - IMGUI_ENABLED が定義されている場合のみ有効です。
     */
    static void DrawImGuiLogWindow();

private:
    struct LogEntry
    {
        LogLevel level;
        std::string message;
    };

    static std::vector<LogEntry> s_logBuffer;
    static std::mutex s_mutex;
};
