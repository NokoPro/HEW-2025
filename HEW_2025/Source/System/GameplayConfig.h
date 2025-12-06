/**********************************************************************************************
 * @file      GameplayConfig.h
 * @brief     外部ファイルからゲームプレイ調整用パラメータを読み込む機能
 *
 * @author    浅野勇生
 * @date      2025/11/24
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/24 ]
 * 
 *    - [◎] 外部ファイルからの読み込みを実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

class GameplayConfig
{
public:
    bool Load(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open()) return false;
        std::string line;
        while (std::getline(ifs, line))
        {
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            std::stringstream ss(line);
            std::string key, value;
            if (!std::getline(ss, key, ',')) continue;
            if (!std::getline(ss, value, ',')) continue;
            try { m_values[key] = std::stof(value); }
            catch (...) { /* ignore parse errors */ }
        }
        m_path = path; // 保存用に記録
        return true;
    }

    bool Save(const std::string& path = "") const
    {
        std::string outPath = path.empty() ? m_path : path;
        if (outPath.empty()) return false;
        std::ofstream ofs(outPath, std::ios::trunc);
        if (!ofs.is_open()) return false;
        // 並び安定化
        std::vector<std::pair<std::string,float>> list(m_values.begin(), m_values.end());
        std::sort(list.begin(), list.end(), [](auto& a, auto& b){ return a.first < b.first; });
        ofs << "# key,value\n";
        for (auto& kv : list)
        {
            ofs << kv.first << ',' << kv.second << '\n';
        }
        return true;
    }

    float Get(const std::string& key, float def) const
    {
        auto it = m_values.find(key);
        return (it != m_values.end()) ? it->second : def;
    }

    void Set(const std::string& key, float value)
    {
        m_values[key] = value;
    }

private:
    std::unordered_map<std::string, float> m_values;
    std::string m_path; // 最後にロードしたパス
};

class GlobalGameplayConfig
{
public:
    static GlobalGameplayConfig& Instance()
    {
        static GlobalGameplayConfig s; return s;
    }
    void Reload(const std::string& path)
    {
        m_config.Load(path);
    }
    const GameplayConfig& Get() const { return m_config; }
    GameplayConfig& GetMutable() { return m_config; }
    bool Save() const { return m_config.Save(); }
private:
    GameplayConfig m_config;
};
