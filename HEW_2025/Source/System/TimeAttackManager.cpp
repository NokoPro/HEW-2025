#include "TimeAttackManager.h"
#include <windows.h>
#include <fstream>
#include <iomanip>
#include <sstream>

//--------------------------------------------------------------------------------------------------
// コンストラクタ: 高精度タイマー初期化
//--------------------------------------------------------------------------------------------------
TimeAttackManager::TimeAttackManager()
{
    LARGE_INTEGER f; QueryPerformanceFrequency(&f); m_perfFreq = static_cast<double>(f.QuadPart);
    LARGE_INTEGER c; QueryPerformanceCounter(&c); m_prevCounter = c.QuadPart;
}

//--------------------------------------------------------------------------------------------------
// 内部: 前フレームからのΔt秒を取得
//--------------------------------------------------------------------------------------------------
float TimeAttackManager::sampleDelta()
{
    LARGE_INTEGER c; QueryPerformanceCounter(&c);
    const int64_t now = c.QuadPart;
    const double dt = static_cast<double>(now - m_prevCounter) / m_perfFreq;
    m_prevCounter = now;
    return static_cast<float>(dt);
}

//--------------------------------------------------------------------------------------------------
// Reset
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::Reset()
{
    m_state = State::Ready;
    m_elapsed = 0.0f;
    m_countdownTotal = 0.0f;
    m_countdownRemaining = 0.0f;
    m_dirty = false;
}

//--------------------------------------------------------------------------------------------------
// カウントダウン開始 (秒)
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::StartCountdown(float seconds)
{
    if (seconds <= 0.0f)
    {
        StartRun();
        return;
    }
    m_state = State::Countdown;
    m_countdownTotal = seconds;
    m_countdownRemaining = seconds;
    m_elapsed = 0.0f;
}

//--------------------------------------------------------------------------------------------------
// 計測開始
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::StartRun()
{
    m_state = State::Running;
    m_elapsed = 0.0f;
}

//--------------------------------------------------------------------------------------------------
// クリア通知
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::NotifyClear()
{
    if (m_state == State::Running)
    {
        m_state = State::Cleared;
        if (m_bestTime <= 0.0f || m_elapsed < m_bestTime)
        {
            m_bestTime = m_elapsed;
            m_dirty = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
// 死亡通知
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::NotifyDeath()
{
    if (m_state == State::Running || m_state == State::Countdown)
    {
        m_state = State::Dead;
    }
}

//--------------------------------------------------------------------------------------------------
// Update: 状態に応じて時間を進める
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::Update()
{
    const float dt = sampleDelta();

    switch (m_state)
    {
    case State::Countdown:
        m_countdownRemaining -= dt;
        if (m_countdownRemaining <= 0.0f)
        {
            StartRun();
        }
        break;
    case State::Running:
        m_elapsed += dt;
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------------------------------------------
// FormatTime: 秒→ mm:ss.mmm
//--------------------------------------------------------------------------------------------------
std::string TimeAttackManager::FormatTime(float seconds)
{
    int totalMs = static_cast<int>(seconds * 1000.0f + 0.5f);
    int ms = totalMs % 1000;
    int totalSec = totalMs / 1000;
    int s = totalSec % 60;
    int m = totalSec / 60;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << m << ":" << std::setw(2) << s << "." << std::setw(3) << ms;
    return oss.str();
}

//--------------------------------------------------------------------------------------------------
// LoadRecord: ベストタイムをバイナリから
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::LoadRecord(const std::string& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) return;
    float v = 0.0f; ifs.read(reinterpret_cast<char*>(&v), sizeof(float));
    m_bestTime = (ifs && v > 0.0f) ? v : 0.0f;
}

//--------------------------------------------------------------------------------------------------
// SaveRecord: ベスト更新時のみ保存
//--------------------------------------------------------------------------------------------------
void TimeAttackManager::SaveRecord(const std::string& path)
{
    if (!m_dirty) return;
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) return;
    ofs.write(reinterpret_cast<const char*>(&m_bestTime), sizeof(float));
    m_dirty = false;
}
