#include "Debug.h"

// 静的メンバー変数の実体を定義
std::vector<Debug::LogEntry> Debug::s_logBuffer;
std::mutex Debug::s_mutex;

void Debug::Log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_logBuffer.push_back({ level, message });
}

#if defined(_DEBUG) || defined(IMGUI_ENABLED)
void Debug::DrawImGuiLogWindow()
{
    if (ImGui::Begin("Debug Log"))
    {
        // ログを逆順に表示（最新のものが上に来るように）
        std::lock_guard<std::mutex> lock(s_mutex);
        for (auto it = s_logBuffer.rbegin(); it != s_logBuffer.rend(); ++it)
        {
            ImVec4 color;
            switch (it->level)
            {
            case LogLevel::Warning:
                color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // 黄色
                break;
            case LogLevel::Error:
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); // 赤色
                break;
            default:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
                break;
            }
            ImGui::TextColored(color, "%s", it->message.c_str());
        }

        if (ImGui::Button("Clear"))
        {
            s_logBuffer.clear();
        }
    }
    ImGui::End();
}
#else
// ImGuiが無効な場合は何もしない
void Debug::DrawImGuiLogWindow() {}
#endif
