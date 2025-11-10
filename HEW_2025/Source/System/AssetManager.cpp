/*****************************************************************//**
 * @file   AssetManager.cpp
 * @brief  AssetManager の実装
 *
 * モデルとテクスチャを、エイリアス名→実パスに解決して読み込み、
 * キャッシュに置くところまでを担当します。
 * 同じアセットを何度も読み込まないようになっています。
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#include "AssetManager.h"
#include "AssetCatalog.h"
#include "Model.h"
#include "DirectX/Texture.h"

std::unordered_map<std::string, std::weak_ptr<Model>>   AssetManager::s_modelCache;
std::unordered_map<std::string, std::weak_ptr<Texture>> AssetManager::s_texCache;
std::mutex AssetManager::s_mtxModel;
std::mutex AssetManager::s_mtxTex;

void AssetManager::Init()
{
    // 今は特にやることなし。
    // 将来的に AssetCatalog のリロードや監視スレッドを起動するならここ。
}

void AssetManager::Shutdown()
{
    // 2つのキャッシュをそれぞれロックしてからクリアする
    std::lock_guard<std::mutex> lk1(s_mtxModel);
    std::lock_guard<std::mutex> lk2(s_mtxTex);

    s_modelCache.clear();
    s_texCache.clear();
}

AssetManager::Resolved AssetManager::ResolveModel(const std::string& aliasOrPath)
{
    // まず台帳を引いてみる
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        AssetManager::Resolved r;
        r.path = d->path;
        r.scale = d->scale;
        r.flip = d->flip;
        return r;
    }

    // 台帳に無ければそのままパス扱い
    return { aliasOrPath, 1.0f, 0 };
}

std::string AssetManager::ResolveTexturePath(const std::string& aliasOrPath)
{
    if (auto d = AssetCatalog::Find(aliasOrPath))
    {
        return d->path;
    }

    // 登録されていなければ引数をそのままパスと見なす
    return aliasOrPath;
}

AssetHandle<Model> AssetManager::GetModel(const std::string& aliasOrPath)
{
    // エイリアスを実パスに変換
    const auto resolved = ResolveModel(aliasOrPath);

    // 1. まずキャッシュを確認
    {
        std::lock_guard<std::mutex> lock(s_mtxModel);

        auto it = s_modelCache.find(resolved.path);
        if (it != s_modelCache.end())
        {
            // weak_ptr → shared_ptr に戻す
            if (auto sp = it->second.lock())
            {
                return { sp };
            }
        }
    }

    // 2. キャッシュに無いのでロードする
    auto sp = LoadModelByPath(resolved.path, resolved.scale, resolved.flip);

    // 3. 成功したらキャッシュに保存
    if (sp)
    {
        std::lock_guard<std::mutex> lock(s_mtxModel);
        s_modelCache[resolved.path] = sp;
    }

    return { sp };
}

AssetHandle<Texture> AssetManager::GetTexture(const std::string& aliasOrPath)
{
    // エイリアスを実パスに変換
    const std::string path = ResolveTexturePath(aliasOrPath);

    // 1. キャッシュを先に見る
    {
        std::lock_guard<std::mutex> lock(s_mtxTex);

        auto it = s_texCache.find(path);
        if (it != s_texCache.end())
        {
            if (auto sp = it->second.lock())
            {
                return { sp };
            }
        }
    }

    // 2. 実際にロード
    auto sp = LoadTextureByPath(path);

    // 3. 読めたらキャッシュに記録
    if (sp)
    {
        std::lock_guard<std::mutex> lock(s_mtxTex);
        s_texCache[path] = sp;
    }

    return { sp };
}

std::shared_ptr<Model> AssetManager::LoadModelByPath(const std::string& path, float scale, int flip)
{
    auto m = std::make_shared<Model>();

    // flip はプロジェクト側の Model::Flip に合わせてキャストする
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
        // 読み込みに失敗したらnullptrにしておくと後でわかりやすい
        return nullptr;
    }

    return tex;
}

void AssetManager::UpdateHotReload(float /*dt*/)
{
    // TODO: 将来的にファイルの更新時刻を見て再ロードする処理を書けるようにしておく
}
