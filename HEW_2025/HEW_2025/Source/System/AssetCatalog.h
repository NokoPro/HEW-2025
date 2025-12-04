/*****************************************************************//**
 * @file   AssetCatalog.h
 * @brief  CSV ベースのアセット台帳
 *
 * Data.csv から
 *   type, path, aliases, scale, flip, preload, group, tags, param1, param2, notes
 * を読み取り、エイリアス → アセット情報の対応表として保持する。
 *
 * AssetManager はここからパスやスケール等を引き当てる。
 *
 * @author  浅野勇生
 * @date    2025/11/24
 *********************************************************************/
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

 /**
  * @brief CSV 1 行分のアセット情報
  */
struct AssetDesc
{
    std::string type;                   ///< アセット種別（model / texture / audio / effect ...）
    std::string path;                   ///< 実際のファイルパス

    std::vector<std::string> aliases;   ///< エイリアス一覧（小文字で保持）
    float       scale = 1.0f;          ///< モデルスケール
    int         flip = 0;             ///< モデルフリップ（Model::Flip 用）

    bool        preload = false;        ///< 起動時プリロードフラグ

    std::string              group;     ///< 論理グループ名（ui / bgm / se / vfx / stage など）
    std::vector<std::string> tags;      ///< タグ一覧（検索・分類用）

    std::string param1;                 ///< 拡張パラメータ1（用途自由）
    std::string param2;                 ///< 拡張パラメータ2（用途自由）

    std::string notes;                  ///< 制作者向けメモ
};

/**
 * @brief Data.csv を元にしたアセット台帳
 *
 * - Data.csv を読み込んで AssetDesc に展開
 * - aliases の各要素をキーにしてハッシュマップに登録
 * - AssetManager からは Find(alias) で参照される
 */
class AssetCatalog
{
public:
    /// すべての登録を破棄
    static void Clear();

    /// 1 レコード分を登録（同じ AssetDesc を複数 alias に張り付ける）
    static void Register(const AssetDesc& desc);

    /// エイリアスから台帳を検索（なければ nullptr）
    static const AssetDesc* Find(const std::string& alias);

    /**
     * @brief CSV を読み込んで台帳を構築
     * @param csvPath CSV ファイルパス
     * @return 成功なら true
     *
     * サポートする形式:
     *   旧: type,path,aliases,scale,flip,prewarm
     *   新: type,path,aliases,scale,flip,preload,group,tags,param1,param2,notes
     *
     * 列数が足りない場合は残りをデフォルト値で埋める。
     */
    static bool LoadCsv(const std::string& csvPath);

    /// 台帳を走査（プリロード処理などで使用する想定）
    template <class F>
    static void ForEach(const F& fn)
    {
        std::lock_guard<std::mutex> lock(s_mtx);
        for (auto& kv : s_aliasToDesc)
        {
            fn(kv.second);
        }
    }

private:
    static std::unordered_map<std::string, AssetDesc> s_aliasToDesc; ///< alias → AssetDesc
    static std::mutex                                 s_mtx;         ///< スレッドセーフ用
};
