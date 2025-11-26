/*****************************************************************//**
 * @file   AssetManager.cpp
 * @brief  AssetManager 実装
 *
 * - AssetCatalog からパス・スケール等を解決
 * - Model / Texture は shared_ptr キャッシュ
 * - Audio / Effect はパス情報を含む小さい構造体を返すだけ
 *
 * @author  浅野勇生
 * @date    2025/11/24
 *********************************************************************/
#include "AssetManager.h"
#include "AssetCatalog.h"
#include "Model.h"
#include "DirectX/Texture.h"

#include <cassert>

 // 静的メンバ定義
std::unordered_map<std::string, std::weak_ptr<Model>>   AssetManager::s_modelCache;
std::unordered_map<std::string, std::weak_ptr<Texture>> AssetManager::s_texCache;
std::mutex                                               AssetManager::s_mtxModel;
std::mutex                                               AssetManager::s_mtxTex;

// ==== Audio 用キャッシュ ====
std::unordered_map<std::string, std::shared_ptr<AudioClip>> AssetManager::s_audioCache;
std::mutex                                                   AssetManager::s_mtxAudio;

void AssetManager::Init()
{
    // 現状特になし
    // 将来的に AssetCatalog::LoadCsv() をここから叩く案もあり
}

void AssetManager::Shutdown()
{
    std::lock_guard<std::mutex> lk1(s_mtxModel);
    std::lock_guard<std::mutex> lk2(s_mtxTex);

    s_modelCache.clear();
    s_texCache.clear();
}

AssetManager::Resolved AssetManager::ResolveModel(const std::string& aliasOrPath)
{
    // まず CSV 台帳を引く
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        Resolved r;
        r.path = d->path;
        r.scale = d->scale;
        r.flip = d->flip;
        return r;
    }

    // 登録がない場合：そのままパス扱い
    Resolved r;
    r.path = aliasOrPath;
    r.scale = 1.0f;
    r.flip = 0;
    return r;
}

std::string AssetManager::ResolveTexturePath(const std::string& aliasOrPath)
{
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        // type が空 or "texture" のものを優先
        if (d->type.empty() || d->type == "texture")
        {
            return d->path;
        }
    }

    // 登録がない / 種別違い：そのままパス扱い
    return aliasOrPath;
}

std::string AssetManager::ResolveAudioPath(const std::string& aliasOrPath)
{
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        if (d->type == "audio")
        {
            return d->path;
        }
    }
    return aliasOrPath;
}

std::string AssetManager::ResolveEffectPath(const std::string& aliasOrPath)
{
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        if (d->type == "effect")
        {
            return d->path;
        }
    }
    return aliasOrPath;
}

AssetHandle<Model> AssetManager::GetModel(const std::string& aliasOrPath)
{
    // 1. CSV から path / scale / flip を解決
    const auto resolved = ResolveModel(aliasOrPath);

    // 2. まずキャッシュを確認
    {
        std::lock_guard<std::mutex> lock(s_mtxModel);

        auto it = s_modelCache.find(resolved.path);
        if (it != s_modelCache.end())
        {
            if (auto sp = it->second.lock())
            {
                return AssetHandle<Model>(sp);
            }
        }
    }

    // 3. キャッシュに無ければロード
    auto sp = LoadModelByPath(resolved.path, resolved.scale, resolved.flip);

    // 4. ロード成功時のみキャッシュに登録
    if (sp)
    {
        std::lock_guard<std::mutex> lock(s_mtxModel);
        s_modelCache[resolved.path] = sp;
    }

    return AssetHandle<Model>(sp);
}

AssetHandle<Texture> AssetManager::GetTexture(const std::string& aliasOrPath)
{
    // 1. パス解決
    const std::string path = ResolveTexturePath(aliasOrPath);

    // 2. キャッシュ確認
    {
        std::lock_guard<std::mutex> lock(s_mtxTex);

        auto it = s_texCache.find(path);
        if (it != s_texCache.end())
        {
            if (auto sp = it->second.lock())
            {
                return AssetHandle<Texture>(sp);
            }
        }
    }

    // 3. ロード
    auto sp = LoadTextureByPath(path);

    // 4. キャッシュ登録
    if (sp)
    {
        std::lock_guard<std::mutex> lock(s_mtxTex);
        s_texCache[path] = sp;
    }

    return AssetHandle<Texture>(sp);
}

AssetHandle<AudioClip> AssetManager::GetAudio(const std::string& aliasOrPath)
{
    {
        std::lock_guard<std::mutex> lock(s_mtxAudio);
        auto it = s_audioCache.find(aliasOrPath);
        if (it != s_audioCache.end())
        {
            return AssetHandle<AudioClip>(it->second);
        }
    }

    // まだキャッシュされていないので、新規に AudioClip を組み立てる
    AudioClip clip{};

    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        if (d->type == "audio")
        {
            clip.path = d->path;
            clip.group = d->group;
            clip.tags = d->tags;
            clip.param1 = d->param1;
            clip.param2 = d->param2;
        }
        else
        {
#if defined(_DEBUG)
            // type ミスマッチの警告（CSV 側の type 設定ミスに気づけるように）
            std::string msg = "[AssetManager::GetAudio] alias '" + aliasOrPath +
                "' is not type 'audio' (type='" + d->type + "')\n";
            OutputDebugStringA(msg.c_str());
#endif
        }
    }

    // CSV に存在しない / type 不一致などで path が取れなかった場合は
    // aliasOrPath をそのままパスとして扱う
    if (clip.path.empty())
    {
        clip.path = aliasOrPath;
    }

    // ここで shared_ptr に包み、キャッシュに登録
    auto sp = std::make_shared<AudioClip>(std::move(clip));
    {
        std::lock_guard<std::mutex> lock(s_mtxAudio);
        // 同時アクセスで別スレッドが先に登録している可能性もあるので二重チェック
        auto it = s_audioCache.find(aliasOrPath);
        if (it == s_audioCache.end())
        {
            s_audioCache.emplace(aliasOrPath, sp);
        }
        else
        {
            // すでに存在していたらそちらを返す
            sp = it->second;
        }
    }

    return AssetHandle<AudioClip>(sp);
}

AssetHandle<EffectRef> AssetManager::GetEffect(const std::string& aliasOrPath)
{
    EffectRef ef{};

    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        if (d->type == "effect")
        {
            ef.path = d->path;
            ef.group = d->group;
            ef.tags = d->tags;
            ef.param1 = d->param1;
            ef.param2 = d->param2;
        }
    }

    if (ef.path.empty())
    {
        ef.path = aliasOrPath;
    }

    return AssetHandle<EffectRef>(std::make_shared<EffectRef>(std::move(ef)));
}

std::shared_ptr<Model> AssetManager::LoadModelByPath(const std::string& path, float scale, int flip)
{
    auto m = std::make_shared<Model>();

    // flip は enum Model::Flip にキャストして渡す
    if (!m->Load(path.c_str(), scale, static_cast<Model::Flip>(flip)))
    {
        return nullptr;
    }

    return m;
}

std::shared_ptr<Texture> AssetManager::LoadTextureByPath(const std::string& path)
{
    auto tex = std::make_shared<Texture>();

    if (FAILED(tex->Create(path.c_str())))
    {
        // 読み込み失敗時は nullptr を返す
        return nullptr;
    }

    return tex;
}

void AssetManager::UpdateHotReload(float /*dt*/)
{
    // TODO:
    //   将来的にファイル更新時の自動リロードをここから実装する
}
