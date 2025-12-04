/*****************************************************************//**
 * @file   World.h
 * @brief  ECS全体を管理するWorldクラスの定義
 *
 * エンティティの生成・破棄、コンポーネントの追加や取得、
 * そして複数コンポーネントをまとめて走査(View)する機能を提供します。
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once

#include "ECS.h"
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <memory>
#include <utility>
#include <cassert>

 /**
  * @brief 各型T専用のコンポーネントストレージ
  * @tparam T 管理したいコンポーネントの型
  *
  * エンティティIDをキーにして、そのエンティティが持つTを1つだけ保持します。
  */
template <class T>
struct ComponentStorage final : public IComponentStorage
{
	/// エンティティIDをキーとしたコンポーネントのマップ
	std::unordered_map<EntityId, T> data;

	void Remove(EntityId e) override
	{
		data.erase(e);
	}

	bool Has(EntityId e) const override
	{
		return (data.find(e) != data.end());
	}
};

/**
 * @class World
 * @brief ECSの中核クラス
 * @details
 * - エンティティIDの発行
 * - コンポーネントの追加/削除/取得
 * - 特定コンポーネントを持つエンティティの一括走査(View)
 * をまとめて行うクラスです。
 */
class World
{
public:
	World()
	{
	}

	//==================================================================
	// エンティティ管理
	//==================================================================

	/**
	 * @brief 新しいエンティティを生成する
	 * @return 生成されたエンティティID
	 */
	EntityId Create()
	{
		if (m_next == kInvalidEntity)
		{
			m_next = 1;
		}
		return m_next++;
	}

	/**
	 * @brief 指定したエンティティを破棄する
	 * @param e 破棄するエンティティID
	 * @details
	 * 登録されているすべてのコンポーネントストレージから、このエンティティに紐づくデータを削除します。
	 */
	void Destroy(EntityId e)
	{
		for (auto& kv : m_storages)
		{
			kv.second->Remove(e);
		}
	}

	//==================================================================
	// コンポーネント操作
	//==================================================================

	template <class T, class... Args>
	T& Add(EntityId e, Args &&...args)
	{
		auto& s = storage<T>();
		auto it = s.find(e);

		if (it == s.end())
		{
			it = s.emplace(e, T{ std::forward<Args>(args)... }).first;
		}
		else
		{
			it->second = T{ std::forward<Args>(args)... };
		}

		return it->second;
	}

	template <class T>
	bool Has(EntityId e) const
	{
		const auto* s = find<T>();
		return (s && s->count(e) != 0);
	}

	template <class T>
	T& Get(EntityId e)
	{
		auto* s = find<T>();
		assert(s && "Get<T>: storage not found");
		auto it = s->find(e);
		assert(it != s->end() && "Get<T>: entity doesn't have T");
		return it->second;
	}

	template <class T>
	const T& Get(EntityId e) const
	{
		const auto* s = find<T>();
		assert(s && "Get<T> const: storage not found");
		auto it = s->find(e);
		assert(it != s->end() && "Get<T> const: entity doesn't have T");
		return it->second;
	}

	template <class T>
	T* TryGet(EntityId e)
	{
		auto* s = find<T>();
		if (!s)
		{
			return nullptr;
		}
		auto it = s->find(e);
		if (it == s->end())
		{
			return nullptr;
		}
		return &it->second;
	}

	template <class T>
	const T* TryGet(EntityId e) const
	{
		const auto* s = find<T>();
		if (!s)
		{
			return nullptr;
		}
		auto it = s->find(e);
		if (it == s->end())
		{
			return nullptr;
		}
		return &it->second;
	}

	template <class T>
	void Remove(EntityId e)
	{
		auto* s = find<T>();
		if (s)
		{
			s->erase(e);
		}
	}

	//==================================================================
	// View処理
	//==================================================================

	template <class A, class... Rest, class Fn>
	void View(Fn&& fn)
	{
		auto* sa = find<A>();
		if (!sa)
		{
			return;
		}

		for (auto& kv : *sa)
		{
			const EntityId e = kv.first;
			if (has_all_rest<Rest...>(e))
			{
				fn(e, Get<A>(e), Get<Rest>(e)...);
			}
		}
	}

	template <class A, class... Rest, class Fn>
	void View(Fn&& fn) const
	{
		const auto* sa = find<A>();
		if (!sa)
		{
			return;
		}

		for (const auto& kv : *sa)
		{
			const EntityId e = kv.first;
			if (has_all_rest<Rest...>(e))
			{
				fn(e, Get<A>(e), Get<Rest>(e)...);
			}
		}
	}

private:
	//==================================================================
	// 内部ヘルパ
	//==================================================================

	template <class T>
	std::unordered_map<EntityId, T>& storage()
	{
		ComponentType key = std::type_index(typeid(T));
		auto it = m_storages.find(key);

		if (it == m_storages.end())
		{
			auto ptr = std::unique_ptr<IComponentStorage>(new ComponentStorage<T>());
			it = m_storages.emplace(key, std::move(ptr)).first;
		}

		return static_cast<ComponentStorage<T> *>(it->second.get())->data;
	}

	template <class T>
	std::unordered_map<EntityId, T>* find()
	{
		ComponentType key = std::type_index(typeid(T));
		auto it = m_storages.find(key);
		if (it == m_storages.end())
		{
			return nullptr;
		}
		return &static_cast<ComponentStorage<T> *>(it->second.get())->data;
	}

	template <class T>
	const std::unordered_map<EntityId, T>* find() const
	{
		ComponentType key = std::type_index(typeid(T));
		auto it = m_storages.find(key);
		if (it == m_storages.end())
		{
			return nullptr;
		}
		return &static_cast<const ComponentStorage<T> *>(it->second.get())->data;
	}

	template <class... Ts>
	typename std::enable_if<sizeof...(Ts) == 0, bool>::type
		has_all_rest(EntityId) const
	{
		return true;
	}

	template <class T1, class... Ts>
	bool has_all_rest(EntityId e) const
	{
		return Has<T1>(e) && has_all_rest<Ts...>(e);
	}

private:
	/// 各コンポーネント型に対応するストレージ
	std::unordered_map<ComponentType, std::unique_ptr<IComponentStorage>> m_storages;

	/// 次に割り当てるエンティティID
	EntityId m_next = kInvalidEntity;
};
