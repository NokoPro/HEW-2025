#pragma once

#include <string>

// 前方宣言
class World;
class PrefabRegistry;

namespace DirectX
{
    struct XMFLOAT3;
}

/**
 * @class StageLoader
 * @brief TiledエディタからエクスポートされたJSON形式のステージファイルを読み込むクラス
 */
class StageLoader
{
public:
    /**
     * @brief ステージファイルを読み込み、エンティティを生成する
     * @param filePath ステージファイルのパス (JSON)
     * @param world エンティティを生成する先のWorld
     * @param prefabs PrefabRegistryインスタンス
     * @return 読み込みと生成に成功した場合はtrue
     */
    bool Load(const std::string& filePath, World& world, PrefabRegistry& prefabs);

private:
    // Tiledの座標をゲームの座標に変換するヘルパー関数などをここに追加できます。
    // 例: void ConvertTiledPosition(float tiledX, float tiledY, DirectX::XMFLOAT3& outPosition);
};
