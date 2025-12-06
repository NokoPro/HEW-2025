#pragma once
#include "System/AssetManager.h"

struct DeathTextureOverrideComponent
{
    AssetHandle<Texture> texture; // 強制描画テクスチャ
    bool enabled = false;         // 有効化フラグ
};