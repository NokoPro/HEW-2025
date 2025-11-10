/*****************************************************************//**
 * @file   SystemRegistry.h
 * @brief  UpdateŒn‚ÆRenderŒnSystem‚ğ•ª—£‚µ‚Ä“o˜^EÀs‚·‚éƒŒƒWƒXƒgƒŠƒNƒ‰ƒX
 *
 * ECS(World)ã‚Å“®ì‚·‚é‘SSystem‚ğŠÇ—‚µ‚Ü‚·B
 * AddUpdate<T>(), AddRender<T>() ‚Å“o˜^‚µA
 * Tick()/Render() ‚ÅÀs‚³‚ê‚Ü‚·B
 *
 * “o˜^‡‚ª‚»‚Ì‚Ü‚ÜÀs‡‚É‚È‚è‚Ü‚·B
 *
 * @author ó–ì—E¶
 * @date   2025/11/11
 *********************************************************************/
#pragma once

#include <memory>
#include <vector>
#include <type_traits>

#include "IUpdateSystem.h"
#include "IRenderSystem.h"

class World;

/**
 * @class SystemRegistry
 * @brief UpdateŒnERenderŒn‚ÌSystem‚ğ“Š‡ŠÇ—‚·‚éƒNƒ‰ƒX
 */
class SystemRegistry
{
public:
	~SystemRegistry();

	//====================================================================
	// System “o˜^
	//====================================================================

	template <class T, class... Args>
	T& AddUpdate(Args&&... args)
	{
		static_assert(std::is_base_of<IUpdateSystem, T>::value, "T must inherit IUpdateSystem");

		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		T& ref = *ptr;
		m_updates.emplace_back(std::move(ptr));
		return ref;
	}

	template <class T, class... Args>
	T& AddRender(Args&&... args)
	{
		static_assert(std::is_base_of<IRenderSystem, T>::value, "T must inherit IRenderSystem");

		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		T& ref = *ptr;
		m_renders.emplace_back(std::move(ptr));
		return ref;
	}

	//====================================================================
	// Às§Œä
	//====================================================================

	/**
	 * @brief “o˜^‚³‚ê‚½‘SUpdateƒVƒXƒeƒ€‚ğ‡‚ÉÀs‚·‚é
	 * @param world ECS‚ÌWorld
	 * @param dt Œo‰ßŠÔi•bj
	 */
	void Tick(World& world, float dt);

	/**
	 * @brief “o˜^‚³‚ê‚½‘SRenderƒVƒXƒeƒ€‚ğ‡‚ÉÀs‚·‚é
	 * @param world ECS‚ÌWorld
	 */
	void Render(World& world);

	//====================================================================
	// Œ^‚Å’T‚·ƒwƒ‹ƒp
	//====================================================================

	template <class T>
	T* GetUpdate()
	{
		for (auto& s : m_updates)
		{
			if (auto* p = dynamic_cast<T*>(s.get()))
			{
				return p;
			}
		}
		return nullptr;
	}

	template <class T>
	const T* GetUpdate() const
	{
		for (auto& s : m_updates)
		{
			if (auto* p = dynamic_cast<const T*>(s.get()))
			{
				return p;
			}
		}
		return nullptr;
	}

	template <class T>
	T* GetRender()
	{
		for (auto& s : m_renders)
		{
			if (auto* p = dynamic_cast<T*>(s.get()))
			{
				return p;
			}
		}
		return nullptr;
	}

	template <class T>
	const T* GetRender() const
	{
		for (auto& s : m_renders)
		{
			if (auto* p = dynamic_cast<const T*>(s.get()))
			{
				return p;
			}
		}
		return nullptr;
	}

	template <class T, class... Args>
	T& GetOrAddUpdate(Args&&... args)
	{
		if (auto* p = GetUpdate<T>())
		{
			return *p;
		}
		return AddUpdate<T>(std::forward<Args>(args)...);
	}

	template <class T, class... Args>
	T& GetOrAddRender(Args&&... args)
	{
		if (auto* p = GetRender<T>())
		{
			return *p;
		}
		return AddRender<T>(std::forward<Args>(args)...);
	}

private:
	std::vector<std::unique_ptr<IUpdateSystem>> m_updates;
	std::vector<std::unique_ptr<IRenderSystem>> m_renders;
};
