/*****************************************************************//**
 * @file   SceneManager.h
 * @brief  単一アクティブなシーンを保持・切り替えするマネージャ
 *
 * - Change<T>() でシーンを差し替えます（古いシーンは破棄）
 * - Update()/Draw() を呼ぶと現在のシーンにそのまま委譲します
 * - 現在のシーンを取得したい場合は Current() を使います
 *
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#pragma once

#include <memory>
#include <type_traits>
#include "Scene.h"

 /**
  * @class SceneManager
  * @brief 今アクティブなシーンを1つだけ持つシンプルなマネージャ
  */
class SceneManager
{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    SceneManager() = default;

    /**
     * @brief デフォルトデストラクタ
     */
    ~SceneManager() = default;

    /**
     * @brief シーンを切り替える（古いシーンは破棄）
     * @tparam T 新しく生成するシーンの型（Sceneを継承していること）
     * @tparam Args シーンのコンストラクタに渡す引数型
     * @param args シーンのコンストラクタに渡す引数
     */
    template <class T, class... Args>
    void Change(Args&&... args)
    {
        static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");
        m_current.reset(new T(std::forward<Args>(args)...));
    }

    /**
     * @brief 現在のシーンを更新する
     */
    void Update()
    {
        if (m_current)
        {
            m_current->Update();
        }
    }

    /**
     * @brief 現在のシーンを描画する
     */
    void Draw()
    {
        if (m_current)
        {
            m_current->Draw();
        }
    }

    /**
     * @brief 現在のシーンへのポインタを取得する
     * @return 現在アクティブなScene。なければnullptr。
     */
    Scene* Current() const
    {
        return m_current.get();
    }

private:
    std::unique_ptr<Scene> m_current; ///< 現在アクティブなシーン
};
