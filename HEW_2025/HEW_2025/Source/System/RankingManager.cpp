#include "RankingManager.h"
#include "System/TimeAttackManager.h"
#include "libs/nlohmann/json.hpp"
#include <fstream>
#include <algorithm>
#include <Windows.h>

using json = nlohmann::json;

int RankingManager::AddRecord(float time)
{
    if (time <= 0.0f)
    {
        return -1;
    }

    // 追加して昇順維持（最大件数制限）
    m_times.push_back(time);
    std::sort(m_times.begin(), m_times.end());
    if (m_times.size() > MAX_RANK)
    {
        m_times.resize(MAX_RANK);
    }

    return GetRank(time);
}

int RankingManager::GetRank(float time) const
{
    for (size_t i = 0; i < m_times.size(); i++)
    {
        if (m_times[i] == time)
        {
            return static_cast<int>(i) + 1;
        }
    }
    return -1;
}

void RankingManager::Submit(float seconds)
{
    if (seconds <= 0.0f)
    {
        return;
    }

    m_times.push_back(seconds);
    std::sort(m_times.begin(), m_times.end());
    if (m_times.size() > MAX_RANK)
    {
        m_times.resize(MAX_RANK);
    }
}

std::vector<float> RankingManager::GetTop(size_t count) const
{
    std::vector<float> out;
    out.reserve((std::min)(count, m_times.size()));
    for (size_t i = 0; i < m_times.size() && i < count; i++)
    {
        out.push_back(m_times[i]);
    }
    return out;
}

size_t RankingManager::GetCount() const
{
    return m_times.size();
}

float RankingManager::GetEntry(size_t i) const
{
    return m_times[i];
}

void RankingManager::Load(const std::string& path)
{
    m_filePath = path;

    m_times.clear();
    std::ifstream ifs(path, std::ios::in);
    if (!ifs) return;

    json j;
    ifs >> j;
    if (!j.is_object()) return;

    if (j.contains("times") && j["times"].is_array())
    {
        for (auto& v : j["times"])
        {
            if (v.is_number())
                m_times.push_back(v.get<float>());
        }
        std::sort(m_times.begin(), m_times.end());
        if (m_times.size() > MAX_RANK) m_times.resize(MAX_RANK);
    }
}

void RankingManager::Save(const std::string& path) const
{
    // 次回の AddRecord のためにパスを覚えておく
    const_cast<RankingManager*>(this)->m_filePath = path;

    json j;
    j["times"] = m_times;

    std::ofstream ofs(path, std::ios::out | std::ios::trunc);
    if (!ofs) return;
    ofs << j.dump(2);
}

float RankingManager::GetTimeByRank(int rank) const
{
    if (rank <= 0 || rank > static_cast<int>(m_times.size()))
    {
        return -1.0f;
    }
    return m_times[rank - 1];
}

void RankingManager::Reset()
{
    m_times.clear();
}

void RankingManager::ShowRankingMessege() const
{
    if (m_times.empty())
    {
        MessageBoxA(NULL, "ランキングデータがありません", "Ranking", MB_OK);
        return;
    }
    std::string msg = "===Ranking===\n";
    for (size_t i = 0; i < m_times.size(); i++)
    {
        char buf[64];
        sprintf_s(buf, "%2zu 位 :%.3f 秒\n", i + 1, m_times[i]);
        msg += buf;
    }
    MessageBoxA(NULL, msg.c_str(), "Ranking", MB_OK);
}
