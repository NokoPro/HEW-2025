/*****************************************************************//**
 * @file   AssetCatalog.cpp
 * @brief  AssetCatalog の実装
 *
 * CSVから読み込んだ1行をトリムして分解し、AssetDescにして登録します。
 * 文字列キーはすべて小文字化してから格納するので、大文字小文字の違いに
 * 左右されずにアクセスできます。
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#include "AssetCatalog.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

 // 静的メンバ定義
std::unordered_map<std::string, AssetDesc> AssetCatalog::s_aliasToDesc;
std::mutex                                 AssetCatalog::s_mtx;

/**
 * @brief 文字列の前後の空白を取り除く
 */
static std::string Trim(std::string s)
{
    auto is_space =
        [](unsigned char c) -> bool
        {
            return std::isspace(c) != 0;
        };

    // 先頭側の空白除去
    while (!s.empty() && is_space(static_cast<unsigned char>(s.front())))
    {
        s.erase(s.begin());
    }

    // 末尾側の空白除去
    while (!s.empty() && is_space(static_cast<unsigned char>(s.back())))
    {
        s.pop_back();
    }

    return s;
}

/**
 * @brief 文字列をすべて小文字に変換する
 */
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

    // このアセットが持っているすべての別名で登録する
    for (auto alias : desc.aliases)
    {
        alias = ToLower(alias);     // 大文字・小文字を統一
        s_aliasToDesc[alias] = desc;
    }
}

const AssetDesc* AssetCatalog::Find(const std::string& alias)
{
    std::lock_guard<std::mutex> lock(s_mtx);

    const std::string key = ToLower(alias);
    auto it = s_aliasToDesc.find(key);
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

    std::string line;

    // 1行ずつ読んで登録していく
    while (std::getline(ifs, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        // CSVの列（6列）を保持する一時変数
        std::string type;
        std::string path;
        std::string aliasStr;
        std::string scaleStr;
        std::string flipStr;
        std::string prewarmStr;

        // カンマ区切りで取り出す
        std::getline(ss, type, ',');
        std::getline(ss, path, ',');
        std::getline(ss, aliasStr, ',');
        std::getline(ss, scaleStr, ',');
        std::getline(ss, flipStr, ',');
        std::getline(ss, prewarmStr, ',');

        // 前後の空白を削る
        type = Trim(type);
        path = Trim(path);
        aliasStr = Trim(aliasStr);
        scaleStr = Trim(scaleStr);
        flipStr = Trim(flipStr);
        prewarmStr = Trim(prewarmStr);

        // 1行ぶんの記述を組み立てる
        AssetDesc d;
        d.type = type;
        d.path = path;

        // aliases は '|' 区切りで複数並べられる
        {
            std::stringstream as(aliasStr);
            std::string a;
            while (std::getline(as, a, '|'))
            {
                a = Trim(a);
                if (!a.empty())
                {
                    d.aliases.push_back(a);
                }
            }
        }

        // スケールが書かれていれば上書き
        if (!scaleStr.empty())
        {
            d.scale = std::stof(scaleStr);
        }

        // flip が書かれていれば上書き
        if (!flipStr.empty())
        {
            d.flip = std::stoi(flipStr);
        }

        // prewarmStr はこの実装では使っていないが、CSVの列合わせのために読んでおく
        // 将来ここで「起動時に読み込んでおく」処理をしてもよい

        // 出来上がったものを登録
        Register(d);
    }

    return true;
}
