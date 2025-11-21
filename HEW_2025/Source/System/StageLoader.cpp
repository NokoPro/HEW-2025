#include "StageLoader.h"
#include "ECS/World.h"
#include "ECS/Prefabs/PrefabRegistry.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Gimick/MovingPlatformComponent.h"
#include "System/Debug.h"

#include <fstream>
#include "libs/nlohmann/json.hpp"
#include <DirectXMath.h>

using json = nlohmann::json;

template<typename T>
T GetPropertyValue(const json& properties, const std::string& name, T defaultValue)
{
    for (const auto& prop : properties)
    {
        if (prop.contains("name") && prop["name"] == name && prop.contains("value"))
        {
            try { return prop["value"].get<T>(); } catch (...) { return defaultValue; }
        }
    }
    return defaultValue;
}

bool StageLoader::Load(const std::string& filePath, World& world, PrefabRegistry& prefabs)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        Debug::Log(LogLevel::Error, "Failed to open stage file: " + filePath);
        return false;
    }

    json data;
    try { file >> data; }
    catch (json::parse_error& e)
    {
        Debug::Log(LogLevel::Error, "Failed to parse stage file: " + std::string(e.what()));
        return false;
    }

    const int mapTilesY = data.value("height", 0);              // タイル数(縦)
    const int mapTilesX = data.value("width", 0);               // タイル数(横)
    const float tilePixW = data.value("tilewidth", 16);          // ピクセル幅
    const float tilePixH = data.value("tileheight", 16);         // ピクセル高

    // ゲーム内では 1 タイル = 1.0f ワールド単位とする
    const float worldTileW = 2.0f;
    const float worldTileH = 2.0f;
    const float pxToWorldX = worldTileW / tilePixW; // ピクセル→ワールド変換係数
    const float pxToWorldY = worldTileH / tilePixH;
    const float mapHeightWorld = static_cast<float>(mapTilesY) * worldTileH; // 反転用

    if (!data.contains("layers"))
    {
        Debug::Log(LogLevel::Warning, "Stage file has no layers: " + filePath);
        return true;
    }

    for (const auto& layer : data["layers"])
    {
        if (!layer.contains("type")) continue;

        // ------------------ Object Layer ------------------
        if (layer["type"] == "objectgroup")
        {
            std::vector<json> sortedObjects;
            if (layer.contains("objects"))
                for (const auto& object : layer["objects"]) sortedObjects.push_back(object);

            // 下から上 / 左から右
            std::sort(sortedObjects.begin(), sortedObjects.end(), [](const json& a, const json& b) {
                const float ay = a.value("y", 0.0f);
                const float by = b.value("y", 0.0f);
                if (ay != by) return ay > by; // Tiledは下へ増えるので大きい方を先
                const float ax = a.value("x", 0.0f);
                const float bx = b.value("x", 0.0f);
                return ax < bx;
            });

            for (const auto& object : sortedObjects)
            {
                std::string type = object.value("type", "");
                if (type.empty()) continue;

                PrefabRegistry::SpawnParams sp;

                const float tiledPX = object.value("x", 0.0f);      // px
                const float tiledPY = object.value("y", 0.0f);      // px (上から下へ増える)
                const float objPixW = object.value("width", 0.0f);  // px
                const float objPixH = object.value("height", 0.0f); // px

                // 幅高さ→ワールド単位
                float wWorld = objPixW * pxToWorldX;
                float hWorld = objPixH * pxToWorldY;

                if (type == "StaticBlock" || type == "MovingPlatform")
                {
                    sp.scale.x = 1.0f;
                    sp.scale.y = 1.0f;
                }
                else
                {
                    sp.scale.x = (wWorld > 0.0f) ? wWorld : worldTileW;
                    sp.scale.y = (hWorld > 0.0f) ? hWorld : worldTileH;
                }
                sp.scale.z = 1.0f;

                // オブジェクト中心: Tiledは左上原点。Yは反転して上方向が+になるよう mapHeightWorld - (...)
                float centerXWorld;
                float centerYWorld;
                if (objPixW == 0.0f && objPixH == 0.0f)
                {
                    // Point object: その地点をタイル中心扱い
                    centerXWorld = tiledPX * pxToWorldX + sp.scale.x * 0.5f;
                    centerYWorld = mapHeightWorld - (tiledPY * pxToWorldY) - sp.scale.y * 0.5f;
                }
                else
                {
                    centerXWorld = (tiledPX * pxToWorldX) + wWorld * 0.5f;
                    centerYWorld = mapHeightWorld - (tiledPY * pxToWorldY) - hWorld * 0.5f;
                }

                sp.position.x = centerXWorld;
                sp.position.y = centerYWorld;
                sp.position.z = 0.0f;

                if (object.contains("properties"))
                {
                    const auto& properties = object["properties"];
                    sp.padIndex = GetPropertyValue<int>(properties, "padIndex", 0);
                    sp.modelAlias = GetPropertyValue<std::string>(properties, "modelAlias", "");
                }

                EntityId entity = prefabs.Spawn(type, world, sp);

                // MovingPlatform: start/end座標も同じ変換
                if (entity != kInvalidEntity && type == "MovingPlatform" && world.Has<MovingPlatformComponent>(entity) && object.contains("properties"))
                {
                    const auto& properties = object["properties"];
                    auto& mc = world.Get<MovingPlatformComponent>(entity);

                    float rawStartX = GetPropertyValue<float>(properties, "startX", sp.position.x / pxToWorldX) * pxToWorldX;
                    float rawStartY = GetPropertyValue<float>(properties, "startY", (mapHeightWorld - sp.position.y) / pxToWorldY) * pxToWorldY;
                    float rawEndX   = GetPropertyValue<float>(properties, "endX", sp.position.x / pxToWorldX) * pxToWorldX;
                    float rawEndY   = GetPropertyValue<float>(properties, "endY", (mapHeightWorld - sp.position.y) / pxToWorldY) * pxToWorldY;

                    mc.start.x = rawStartX * pxToWorldX; // 再度pxToWorldX掛けて確定
                    mc.start.y = mapHeightWorld - (rawStartY * pxToWorldY);
                    mc.end.x   = rawEndX * pxToWorldX;
                    mc.end.y   = mapHeightWorld - (rawEndY * pxToWorldY);
                    mc.speed   = GetPropertyValue<float>(properties, "speed", 1.0f);
                }
            }
        }
        // ------------------ Tile Layer ------------------
        else if (layer["type"] == "tilelayer")
        {
            std::string layerType = GetPropertyValue<std::string>(layer.value("properties", json::array()), "type", "");
            if (layerType.empty()) continue;
            if (!layer.contains("data")) continue;

            const auto& tiles = layer["data"];
            const int width = layer.value("width", 0);
            int index = 0;
            for (const auto& tileGid : tiles)
            {
                if (tileGid.is_number() && tileGid.get<int>() > 0)
                {
                    PrefabRegistry::SpawnParams sp;
                    const int tileX = index % width;
                    const int tileY = index / width;

                    sp.scale.x = 1.0f;
                    sp.scale.y = 1.0f;
                    sp.scale.z = 1.0f;

                    // Tiledの(0,0)は左上。ゲームはY上向きなので反転: mapHeightWorld - tileY - 0.5
                    sp.position.x = tileX * worldTileW + 0.5f * worldTileW;
                    sp.position.y = mapHeightWorld - tileY * worldTileH - 0.5f * worldTileH;
                    sp.position.z = 0.0f;

                    prefabs.Spawn(layerType, world, sp);
                }
                ++index;
            }
        }
    }

    Debug::Log(LogLevel::Info, "Stage loaded successfully: " + filePath);
    return true;
}
