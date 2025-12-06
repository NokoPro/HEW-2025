/*****************************************************************//**
 * @file   AssetCatalog.cpp
 * @brief  AssetCatalog 実装
 *
 * Data.csv を読み込み、エイリアスごとの AssetDesc を構築する。
 *
 * 旧 6 列形式と、新 11 列形式のどちらも読み込めるようにしてある。
 *
 * @author  浅野勇生
 * @date    2025/11/24
 *********************************************************************/
#include "AssetCatalog.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

 // 静的メンバ定義
std::unordered_map<std::string, AssetDesc> AssetCatalog::s_aliasToDesc;
std::mutex                                 AssetCatalog::s_mtx;

// 文字列トリム（前後の空白を削除）
static std::string Trim(std::string s)
{
    auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    s.erase(std::remove_if(s.begin(), s.end(), isSpace), s.end());
    return s;
}

// 小文字化
static std::string ToLower(std::string s)
{
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char c) -> char
        {
            return static_cast<char>(std::tolower(c));
        });
    return s;
}

void AssetCatalog::Clear()
{
    std::lock_guard<std::mutex> lock(s_mtx);
    s_aliasToDesc.clear();
}

void AssetCatalog::Register(const AssetDesc& desc)
{
    std::lock_guard<std::mutex> lock(s_mtx);

    // 各エイリアスで同じ AssetDesc を共有登録
    for (auto alias : desc.aliases)
    {
        alias = ToLower(alias);
        if (alias.empty())
        {
            continue;
        }
        s_aliasToDesc[alias] = desc;
    }
}

const AssetDesc* AssetCatalog::Find(const std::string& alias)
{
    std::lock_guard<std::mutex> lock(s_mtx);

    const std::string key = ToLower(alias);
    auto              it = s_aliasToDesc.find(key);
    if (it == s_aliasToDesc.end())
    {
        return nullptr;
    }
    return &it->second;
}

bool AssetCatalog::LoadCsv(const std::string& csvPath)
{
    std::ifstream ifs(csvPath);
    if (!ifs)
    {
        return false;
    }

    Clear();

    std::string line;

    // 1 行ずつ読み込み
    while (std::getline(ifs, line))
    {
        if (line.empty())
        {
            continue;
        }
        // コメント行をスキップ
        if (line[0] == '#' ||
            (line.size() >= 2 && line[0] == '/' && line[1] == '/'))
        {
            continue;
        }

        std::stringstream        ss(line);
        std::vector<std::string> cols;
        std::string              col;

        while (std::getline(ss, col, ','))
        {
            cols.push_back(Trim(col));
        }

        // type / path は必須
        if (cols.size() < 2)
        {
            continue;
        }

        auto getCol = [&](size_t idx) -> std::string
            {
                if (idx < cols.size())
                {
                    return cols[idx];
                }
                return {};
            };

        AssetDesc d;

        d.type = getCol(0);
        d.path = getCol(1);

        std::string aliasesStr = getCol(2);
        std::string scaleStr = getCol(3);
        std::string flipStr = getCol(4);
        std::string preloadStr = getCol(5);
        std::string groupStr = getCol(6);
        std::string tagsStr = getCol(7);
        d.param1 = getCol(8);
        d.param2 = getCol(9);
        d.notes = getCol(10);

        // aliases 分解
        if (!aliasesStr.empty())
        {
            std::stringstream as(aliasesStr);
            std::string       a;
            while (std::getline(as, a, '|'))
            {
                a = Trim(ToLower(a));
                if (!a.empty())
                {
                    d.aliases.push_back(a);
                }
            }
        }

        // scale
        if (!scaleStr.empty())
        {
            d.scale = std::stof(scaleStr);
        }

        // flip
        if (!flipStr.empty())
        {
            d.flip = std::stoi(flipStr);
        }

        // preload / prewarm
        if (!preloadStr.empty())
        {
            // "0" 以外は true と扱う（旧 prewarm も 0/1 想定）
            d.preload = (preloadStr != "0");
        }

        // group
        d.group = groupStr;

        // tags 分解
        if (!tagsStr.empty())
        {
            std::stringstream ts(tagsStr);
            std::string       t;
            while (std::getline(ts, t, '|'))
            {
                t = Trim(ToLower(t));
                if (!t.empty())
                {
                    d.tags.push_back(t);
                }
            }
        }

        // 実際の登録
        Register(d);
    }

    return true;
}
