/*****************************************************************//**
 * @file   SceneAPI.h
 * @brief  シーン切り替えをどこからでも呼べる共通アクセサ
 *
 * 将来的に Scene 内や各 System から直接シーン切り替えしたい要求に対応するための
 * シンプルなユーティリティ。 Game.cpp 内の SceneManager インスタンスへアクセスします。
 *
 * 利用例:
 *   #include "Scene/SceneAPI.h"
 *   // シーン変更
 *   ChangeScene<TestStageScene>();
 *   // 現在シーン取得
 *   Scene* sc = CurrentScene();
 *
 * 注意:
 *  - 非スレッドセーフ（メインスレッドのみで利用想定）
 *  - 追加のライフサイクル管理は行わず、SceneManager::Change に委譲
 */
#pragma once

#include "SceneManager.h"

// SceneManager グローバルインスタンス取得 (Game.cpp で定義)
SceneManager& GetSceneManager();

// テンプレートで直接シーン変更
template <class T, class... Args>
inline void ChangeScene(Args&&... args)
{
    GetSceneManager().Change<T>(std::forward<Args>(args)...);
}

// 現在アクティブなシーンを取得
inline Scene* CurrentScene()
{
    return GetSceneManager().Current();
}
