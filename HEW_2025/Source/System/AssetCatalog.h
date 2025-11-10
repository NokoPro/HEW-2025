/*****************************************************************//**
 * @file   AssetCatalog.h
 * @brief  アセットのエイリアス名を実ファイル情報に解決する台帳
 *
 * CSVなどから「type, path, aliases, scale, flip, prewarm」のような
 * 行を読み込み、aliases のどの名前でも同じアセットにアクセスできるようにします。
 * AssetManager からはこのカタログを経由して実パス・スケールなどを取得します。
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

 /**
  * @brief アセット1件ぶんのメタ情報
  * @details
  * - type  : "model" / "texture" など、アセットの種類
  * - path  : 実ファイルへのパス（キャッシュキーにもなる）
  * - aliases : このアセットを引くときに使える別名（小文字推奨・複数OK）
  * - scale : モデルにだけ意味のあるスケール
  * - flip  : モデルにだけ意味のある反転指定（Model::Flipをint化したもの）
  */
struct AssetDesc
{
    std::string              type;    ///< アセット種別（"model" や "texture"）
    std::string              path;    ///< 実ファイルパス
    std::vector<std::string> aliases; ///< このアセットを引ける別名一覧
    float                    scale = 1.0f; ///< モデル用の基準スケール
    int                      flip = 0;    ///< モデル用の反転フラグ
};

/**
 * @brief エイリアス名 → AssetDesc をスレッドセーフに管理する台帳
 * @details
 * - Register() で1件ずつ登録
 * - LoadCsv() でまとめて登録
 * - Find() でエイリアスから引く（なければnullptr）
 */
class AssetCatalog
{
public:
    /**
     * @brief 登録されているすべてのアセット情報をクリアする
     */
    static void Clear();

    /**
     * @brief 1件のアセットを登録する
     * @param desc 登録したいアセット情報
     * @details
     * desc.aliases に入っているすべての別名で同じdescが引けるようになります。
     * 同じ名前がすでにある場合は上書きします。
     */
    static void Register(const AssetDesc& desc);

    /**
     * @brief エイリアス名からアセット情報を探す
     * @param alias 検索したい名前（大文字・小文字は区別しない）
     * @return 見つかればその情報、なければnullptr
     */
    static const AssetDesc* Find(const std::string& alias);

    /**
     * @brief CSVファイルからまとめて登録するユーティリティ
     * @param csvPath 読み込むCSVのパス
     * @return 読み込めたらtrue
     *
     * フォーマット:
     *   type,path,aliases,scale,flip,prewarm
     * 例:
     *   model,Assets/Model/Player.fbx,player|p,1.0,0,1
     *   texture,Assets/UI/Icon.png,icon|icon_btn,,,0
     */
    static bool LoadCsv(const std::string& csvPath);

private:
    static std::unordered_map<std::string, AssetDesc> s_aliasToDesc; ///< 小文字化した別名→アセット情報
    static std::mutex                                 s_mtx;         ///< 登録・検索を守るためのmutex
};
