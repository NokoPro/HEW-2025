/**********************************************************************************************
 * @file      PrefabOffscreenIndicatorP2.h
 * @brief     2P用 画面下端インジケータUIのプレハブ登録API
 *
 * 2P用の見た目（テクスチャ別名）で初期化されるインジケータUIを生成します。
 * 対象プレイヤーの設定は生成後に `OffscreenIndicatorComponent.targetId` を外部から設定します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Prefabs/PrefabRegistry.h"

/**
 * @brief 2P用インジケータプレハブをレジストリへ登録
 */
void RegisterOffscreenIndicatorP2Prefab(PrefabRegistry& reg);
