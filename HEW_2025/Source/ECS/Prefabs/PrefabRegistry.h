/*****************************************************************//**
 * @file   PrefabRegistry.h
 * @brief  プレハブ（エンティティ生成レシピ）を名前で登録・生成するレジストリ
 *
 * Scene や System からは「名前」と「生成時のパラメータ」を渡すだけで
 * 中で決めておいた生成関数を呼び出してくれます。
 *
 * 例：
 *   registry.Register("Player", [](World& w, const SpawnParams& sp){ ... });
 *   EntityId e = registry.Spawn("Player", world, params);
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#pragma once

#include <functional>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>

#include "../World.h"   // EntityId, World

 /**
  * @class PrefabRegistry
  * @brief 名前→生成関数 の対応を持っておくための小さなレジストリ
  * @details
  * - ゲーム側の「こういう敵を出したい」を名前にしておけます
  * - 実際のコンポーネント付与は登録側のラムダに閉じ込められます
  * - 同名で登録した場合は後からの登録で上書きされます
  */
class PrefabRegistry
{
public:
    /**
     * @brief プレハブ生成時に渡す標準パラメータ
     * @details
     * エディタやイベント側から渡しやすいように最低限の項目だけを定義しています。
     * 必要に応じてこの構造体を拡張するか、ラムダ内で別のWorld.Get～をしても構いません。
     */
    struct SpawnParams
    {
        DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f }; ///< 生成位置（ワールド座標）
        DirectX::XMFLOAT3 rotationDeg{ 0.0f, 0.0f, 0.0f }; ///< 生成回転（度数法）
        DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f }; ///< 生成スケール
        int                padIndex = -1;                 ///< どのプレイヤー入力に紐づけるか（-1なら未指定）
    };

    /**
     * @brief プレハブ生成関数の型
     * @details
     * World と SpawnParams を受け取って、生成した EntityId を返す関数です。
     * 失敗した場合や生成しない場合は kInvalidEntity を返してください。
     */
    using SpawnFunc = std::function<EntityId(World&, const SpawnParams&)>;

public:
    /**
     * @brief デフォルトコンストラクタ
     */
    PrefabRegistry() = default;

    /**
     * @brief プレハブ（生成関数）を登録する
     * @param name 識別用の名前（例："Player" "EnemyA" "GaugeUI"）
     * @param fn 実際にWorldに対してエンティティを作る関数
     * @details
     * 同じ name がすでにある場合は上書きします。
     */
    void Register(const std::string& name, SpawnFunc fn);

    /**
     * @brief 名前からプレハブをスポーンする
     * @param name 登録時に指定した名前
     * @param world 生成先のWorld
     * @param params 生成パラメータ（位置など）
     * @return 生成されたエンティティID。見つからなければ kInvalidEntity
     */
    EntityId Spawn(const std::string& name, World& world, const SpawnParams& params) const;

    /**
     * @brief 指定した名前のプレハブが登録済みかどうか
     * @param name 調べたい名前
     * @return 登録済みならtrue
     */
    bool Has(const std::string& name) const;

private:
    std::unordered_map<std::string, SpawnFunc> m_map; ///< 名前→生成関数 のテーブル
};
