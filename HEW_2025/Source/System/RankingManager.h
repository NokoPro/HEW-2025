/**********************************************************************************************
 * @file      RankingManager.h
 * @brief     クリア時間をランキングにJSONで保存/読込
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
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

class RankingManager
{
public:
    static RankingManager& Get()
    {
        static RankingManager s_instance;
        return s_instance;
    }

    // クリア後スコア追加（保存も行う）
    int AddRecord(float time);

    // ランキングが何位かを返す（見つからなければ -1）
    int GetRank(float time) const;

    // ランキングにタイムを追加, 昇順で保持, 最大6件まで（保存はしない）
    void Submit(float seconds);

    // 上位N件取得（既定:6）
    std::vector<float> GetTop(size_t count = 6) const;

    size_t GetCount() const;
    float GetEntry(size_t i) const;

    // JSON 読込/保存（times 配列）
    void Load(const std::string& path);
    void Save(const std::string& path) const;

    // 指定した順位の時間を返す（1位＝1）
    float GetTimeByRank(int rank) const;

    void Reset();

    void ShowRankingMessege() const;

private:
    RankingManager() {}
    ~RankingManager() {}

private:
    // ランキング最大件数
    static constexpr size_t MAX_RANK = 6;

    // 昇順(小さい = 速い)
    std::vector<float> m_times;

    // 直近の保存/読込パス（AddRecord の保存先に利用）
    std::string m_filePath;
};