/*****************************************************************//**
 * @file   PrefabFollowerBlink.h
 * @brief  ブリンク用UIフォロワープレハブ登録
 *
 * プレイヤーに追従しブリンク入力時に表示する "ui_blink" 用スプライトエンティティ。
 * PlayerUISystem が alias="ui_blink" を判定して表示制御。
 */
#pragma once
#include "PrefabRegistry.h"

void RegisterFollowerBlinkPrefab(PrefabRegistry& registry);
