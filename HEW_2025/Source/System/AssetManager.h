/*****************************************************************//**
 * @file   AssetManager.h
 * @brief  名前（エイリアス）でモデル・テクスチャを取得するキャッシュ付きローダ
 *
 * - AssetCatalog に登録されたエイリアス名を実パスに解決してロードします。
 * - ロードされた実体は shared_ptr で保持し、以後はキャッシュから取り出します。
 * - モデルは台帳にあった scale / flip をロード時に適用します。
 * - スレッドセーフにするため、モデル用・テクスチャ用で別々のmutexを持ちます。
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

 /**
  * @brief 前方宣言（実体はプロジェクト側にある）
  */
class Model;
class Texture;

/**
 * @brief アセットを安全に受け取るための薄いハンドル
 * @tparam T モデルやテクスチャの型
 *
 * shared_ptr を中に持っているだけなので、コピーしてもコストは小さいです。
 */
template <class T>
struct AssetHandle
{
    std::shared_ptr<T> ptr;     ///< 実体（shared_ptrで共有）

    /**
     * @brief 生ポインタ取得
     */
    T* get() const
    {
        return ptr.get();
    }

    /**
     * @brief 参照を取得（存在している前提で使うとき用）
     */
    T& ref() const
    {
        return *ptr;
    }

    /**
     * @brief -> で中身にアクセスできるようにする
     */
    T* operator->() const
    {
        return ptr.get();
    }

    /**
     * @brief if (handle) の形で存在チェックできるようにする
     */
    explicit operator bool() const
    {
        return static_cast<bool>(ptr);
    }
};

/**
 * @brief 名前（エイリアス or パス）からアセットを取得する静的マネージャ
 * @details
 * - GetModel("player") のように呼ぶだけで、初回はロード・2回目以降はキャッシュから返します。
 * - 内部キャッシュは path で引くので、エイリアスは一度実パスに解決してから使います。
 * - Init()/Shutdown() はアプリの開始・終了で呼んでください。
 */
class AssetManager
{
public:
    /**
     * @brief 起動時に一度だけ呼ぶ初期化処理
     */
    static void Init();

    /**
     * @brief 終了時に呼ぶ破棄処理（キャッシュをクリア）
     */
    static void Shutdown();

    /**
     * @brief モデルを取得する
     * @param aliasOrPath 台帳に登録されている名前、または直接のファイルパス
     * @return 読み込み済みモデルへのハンドル（失敗時は空）
     */
    static AssetHandle<Model> GetModel(const std::string& aliasOrPath);

    /**
     * @brief テクスチャを取得する
     * @param aliasOrPath 台帳に登録されている名前、または直接のファイルパス
     * @return 読み込み済みテクスチャへのハンドル（失敗時は空）
     */
    static AssetHandle<Texture> GetTexture(const std::string& aliasOrPath);

    /**
     * @brief 開発中のホットリロード用に1フレームごとに呼ぶ想定のダミー
     * @param dt 経過時間（現状未使用）
     */
    static void UpdateHotReload(float dt);

private:
    /**
     * @brief モデルキャッシュ（path → weak_ptr<Model>）
     *
     * weak_ptr にしているのは、誰も使っていないアセットは自然に消えるようにするためです。
     */
    static std::unordered_map<std::string, std::weak_ptr<Model>> s_modelCache;

    /**
     * @brief テクスチャキャッシュ（path → weak_ptr<Texture>）
     */
    static std::unordered_map<std::string, std::weak_ptr<Texture>> s_texCache;

    /**
     * @brief モデルキャッシュ用の排他オブジェクト
     */
    static std::mutex s_mtxModel;

    /**
     * @brief テクスチャキャッシュ用の排他オブジェクト
     */
    static std::mutex s_mtxTex;

    /**
     * @brief 実パスからモデルを読み込む内部関数
     * @param path ファイルパス
     * @param scale モデル全体に掛けるスケール
     * @param flip 反転フラグ（Model::Flip にキャストして使う）
     */
    static std::shared_ptr<Model> LoadModelByPath(const std::string& path, float scale, int flip);

    /**
     * @brief 実パスからテクスチャを読み込む内部関数
     * @param path ファイルパス
     */
    static std::shared_ptr<Texture> LoadTextureByPath(const std::string& path);

    /**
     * @brief モデル用のエイリアス解決結果
     */
    struct Resolved
    {
        std::string path;  ///< 実際にロードするパス
        float       scale = 1.0f; ///< 台帳に書かれていたスケール
        int         flip = 0;    ///< 台帳に書かれていた反転フラグ
    };

    /**
     * @brief モデルのエイリアスを実パスに変換する
     * @param aliasOrPath エイリアス名またはそのままパス
     * @return 解決結果（登録されていない場合は入力文字列をそのままpathに入れる）
     */
    static Resolved ResolveModel(const std::string& aliasOrPath);

    /**
     * @brief テクスチャのエイリアスを実パスに変換する
     * @param aliasOrPath エイリアス名またはそのままパス
     * @return 実パス
     */
    static std::string ResolveTexturePath(const std::string& aliasOrPath);
};
