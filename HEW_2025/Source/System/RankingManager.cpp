#include "RankingManager.h"
#include "System/TimeAttackManager.h"
#include<Windows.h>

int RankingManager::AddRecord(float time)
{
	m_entries.push_back(time);
	//小さいほうが上位
	std::sort(m_entries.begin(), m_entries.end());

	if (m_entries.size() > MAX_RANK)
	{
		m_entries.resize(MAX_RANK);

	}
	Save(m_filePath);
	return GetRank(time);
}

int RankingManager::GetRank(float time) const
{
	for (size_t i = 0; i < m_entries.size(); i++)
	{
		if (m_entries[i] == time)
		{
			return static_cast<int>(i) + 1;
		}
	}
	return -1;
}

size_t RankingManager::GetCount() const
{
	return m_entries.size();
}

float RankingManager::GetEntry(size_t i) const
{
	return m_entries[i];
}

void RankingManager::Load(const std::string& path)
{
	m_filePath = path;
	Reset();
	std::ifstream ifs(path);
	if (!ifs) {
		return;
	}
	float value;

	while (ifs >> value)
	{
		m_entries.push_back(value);
		if (m_entries.size() > MAX_RANK)
			break;
	}

	std::sort(m_entries.begin(), m_entries.end());
}

void RankingManager::Save(const std::string& path)
{
	if (m_entries.empty()) return;

	std::ofstream ofs(path, std::ios::trunc);
	if (!ofs) return;
	for (auto t : m_entries)
	{
		ofs << t << "\n";
	}
}

void RankingManager::Reset()
{
	m_entries.clear();
}

void RankingManager::ShowRankingMessege() const
{
	if (m_entries.empty())
	{
		MessageBoxA(NULL,"ランキングデータがありません", "Ranking", MB_OK);
		return;
	}
	std::string msg = "===Ranking===\n";
	for (size_t i = 0; i < m_entries.size(); i++)
	{
		char buf[64];
		sprintf_s(buf, "%2zu 位 :%.3f 秒\n", i + 1, m_entries[i]);
		msg += buf;
	}
	MessageBoxA(NULL,msg.c_str(), "Ranking", MB_OK);
}
