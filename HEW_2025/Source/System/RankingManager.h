/**********************************************************************************************
 * @file      RankingManager.h
 * @brief     クリア時間をランキングにCSVで保存
 *
 * @author    篠原純
 * @author    清水光之介
 * @date      2025/11/25
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/25 ]
 *
 *    - [◎] ベースを実装
 *    - [] ファイルの書き込む情報や読み込む情報を変更
 *
 **********************************************************************************************/

#pragma once
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>



class RankingManager
{
public:
	static RankingManager& Get()
	{
		static RankingManager s_instance;
		return s_instance;
	}

	//クリア後スコア追加
	int AddRecord(float time);
	
	//ランキングが何位かを変えす
	int GetRank(float time)const;
	

	size_t GetCount()const; 
	float GetEntry(size_t i)const; 

	void Load(const std::string& path);
	
	void Save(const std::string& path);
	
	// 指定した順位の時間を返す（1位＝1）
	float GetTimeByRank(int rank) const;

	void Reset(); 

	void ShowRankingMessege()const;
private:
	RankingManager(){}
	~RankingManager(){}
private:
	//ランキング最大値
	static constexpr size_t MAX_RANK = 10;
	//ランキング入れ替えとか
	std::vector<float> m_entries;
	//保存する場所
	std::string m_filePath;

};