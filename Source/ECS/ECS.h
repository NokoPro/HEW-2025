/*****************************************************************//**
 * @file   ECS.h
 * @brief  ECSの基礎となる型定義とインターフェース
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once

#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <memory>

 /**
  * @brief エンティティID型
  * @details
  * エンティティは単なる整数IDとして扱います。
  * 0 は無効値として予約しておきます。
  */
using EntityId = std::uint32_t;

/// 無効エンティティ定数
constexpr EntityId kInvalidEntity = 0;

/**
 * @brief コンポーネントの型IDを表すためのエイリアス
 * @details
 * 実装では std::type_index をキーにして、型ごとのストレージをひとまとめに管理します。
 */
using ComponentType = std::type_index;

/**
 * @brief 各コンポーネントストレージ共通のインターフェース
 * @details
 * これを基底として、型Tごとの実ストレージを生成していきます。
 */
struct IComponentStorage
{
	virtual ~IComponentStorage() = default;

	/**
	 * @brief 指定したエンティティのコンポーネントを削除する
	 * @param e 対象のエンティティID
	 */
	virtual void Remove(EntityId e) = 0;

	/**
	 * @brief 指定したエンティティがこのストレージに含まれているか
	 * @param e エンティティID
	 * @return 含まれていれば true
	 */
	virtual bool Has(EntityId e) const = 0;
};
