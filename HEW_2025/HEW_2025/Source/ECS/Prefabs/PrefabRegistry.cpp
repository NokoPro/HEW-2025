/*****************************************************************//**
 * @file   PrefabRegistry.cpp
 * @brief  PrefabRegistry の実装
 *
 * 実体は非常に小さく、名前で検索して登録済みのラムダを呼び出すだけです。
 * 登録されていない名前を Spawn された場合は kInvalidEntity を返します。
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#include "PrefabRegistry.h"

#include <utility>  // std::move

void PrefabRegistry::Register(const std::string& name, SpawnFunc fn)
{
    // 同名があれば上書きします。std::move で中身を移動してコストを下げます。
    m_map[name] = std::move(fn);
}

EntityId PrefabRegistry::Spawn(const std::string& name, World& world, const SpawnParams& params) const
{
    // 名前で検索
    auto it = m_map.find(name);
    if (it == m_map.end())
    {
        // 見つからなければ無効ID
        return kInvalidEntity;
    }

    // 登録されていた生成関数を呼び出す
    return it->second(world, params);
}

bool PrefabRegistry::Has(const std::string& name) const
{
    return (m_map.find(name) != m_map.end());
}
