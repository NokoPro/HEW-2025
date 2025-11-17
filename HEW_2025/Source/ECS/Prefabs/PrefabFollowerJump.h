/*****************************************************************//**
 * @file   PrefabFollowerJump.h
 * @brief  ジャンプ用UIフォロワープレハブ登録
 *
 * プレイヤーに追従しジャンプ入力時に表示する "ui_jump" 用スプライトエンティティ。
 * PlayerUISystem が alias="ui_jump" を判定して表示制御。
 */
#pragma once
#include "PrefabRegistry.h"

void RegisterFollowerJumpPrefab(PrefabRegistry& registry);
