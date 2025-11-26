/*****************************************************************//**
 * @file   AssetManager.h
 * @brief  モデル・テクスチャ・オーディオ等のアセット取得窓口
 *
 * - AssetCatalog（CSV 台帳）からパス・スケール等を引く
 * - 3D モデルとテクスチャは shared_ptr をキャッシュ
 * - Audio / Effect は「パス情報を持つだけの軽量構造体」として返す
 * - スレッドセーフのため、キャッシュには mutex を使用
 *
 * @author  浅野勇生
 * @date    2025/11/24
 *********************************************************************/
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

class Model;
class Texture;
struct AssetDesc;

/**
 * @brief アセット共通ハンドル
 *
 * - shared_ptr を包んだだけの軽量ラッパ
 * - `if (handle) { ... }` で存在チェック可能
 * - `handle->メンバ` で中身にアクセス
 */
template <class T>
class AssetHandle
{
public:
    AssetHandle() = default;
    explicit AssetHandle(std::shared_ptr<T> ptr)
        : m_ptr(std::move(ptr))
    {
    }

    T* operator->() const
    {
        return m_ptr.get();
    }

    T& operator*() const
    {
        return *m_ptr;
    }

    // 既存コード互換: get()/ref() を提供
    T* get() const
    {
        return m_ptr.get();
    }
    T& ref() const
    {
        return *m_ptr;
    }

    explicit operator bool() const
    {
        return static_cast<bool>(m_ptr);
    }

    std::shared_ptr<T> Get() const
    {
        return m_ptr;
    }

private:
    std::shared_ptr<T> m_ptr{};
};

/**
 * @brief オーディオクリップ情報
 *
 * 実ファイルパスと、CSV から取れるメタ情報をまとめておく。
 * AudioManagerSystem 側は path さえ分かれば WAV を読める想定。
 */
struct AudioClip
{
    std::string              path;
    std::string              group;
    std::vector<std::string> tags;
    std::string              param1; ///< 例: デフォルト音量など
    std::string              param2; ///< 例: デフォルトピッチなど
};

/**
 * @brief エフェクト参照情報（Effekseer など）
 */
struct EffectRef
{
    std::string              path;
    std::string              group;
    std::vector<std::string> tags;
    std::string              param1; ///< 例: ループ/1shot 等
    std::string              param2; ///< 例: spawn 間隔など
};

class AssetManager
{
public:
    /// モデル解決結果
    struct Resolved
    {
        std::string path;   ///< 実パス
        float       scale;  ///< スケール
        int         flip;   ///< Model::Flip にキャストして使う
    };

public:
    static void Init();
    static void Shutdown();

    /// ホットリロード用（現状はダミー）
    static void UpdateHotReload(float dt);

    /// モデル用：エイリアス or パス → 実パス + scale + flip の解決
    static Resolved ResolveModel(const std::string& aliasOrPath);

    /// テクスチャ用：エイリアス or パス → 実パス
    static std::string ResolveTexturePath(const std::string& aliasOrPath);

    /// オーディオ用：エイリアス or パス → 実パス（type=="audio" を優先）
    static std::string ResolveAudioPath(const std::string& aliasOrPath);

    /// エフェクト用：エイリアス or パス → 実パス（type=="effect" を優先）
    static std::string ResolveEffectPath(const std::string& aliasOrPath);

    /// モデル取得（キャッシュ付き）
    static AssetHandle<Model> GetModel(const std::string& aliasOrPath);

    /// テクスチャ取得（キャッシュ付き）
    static AssetHandle<Texture> GetTexture(const std::string& aliasOrPath);

    /// オーディオ取得（軽量な AudioClip 構造体を返す）
    static AssetHandle<AudioClip> GetAudio(const std::string& aliasOrPath);

    /// エフェクト取得（軽量な EffectRef 構造体を返す）
    static AssetHandle<EffectRef> GetEffect(const std::string& aliasOrPath);

private:
    static std::shared_ptr<Model>   LoadModelByPath(const std::string& path, float scale, int flip);
    static std::shared_ptr<Texture> LoadTextureByPath(const std::string& path);

private:
    static std::unordered_map<std::string, std::weak_ptr<Model>>   s_modelCache;
    static std::unordered_map<std::string, std::weak_ptr<Texture>> s_texCache;
    static std::unordered_map<std::string, std::shared_ptr<AudioClip>> s_audioCache;
    static std::mutex                                               s_mtxModel;
    static std::mutex                                               s_mtxTex;
    static std::mutex                                               s_mtxAudio;

};
