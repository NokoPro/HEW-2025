/**********************************************************************************************
 * @file      PrefabOffscreenIndicatorP1.h
 * @brief     1P用 画面下端インジケータUIのプレハブ登録API
 *
 * 吹き出しベースのインジケータUIを生成します。見た目や基本パラメータは本プレハブで初期化し、
 * 対象プレイヤーの設定は生成後に `OffscreenIndicatorComponent.targetId` を外部から設定します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Prefabs/PrefabRegistry.h"

/**
 * @brief 1P用インジケータプレハブをレジストリへ登録
 */
void RegisterOffscreenIndicatorP1Prefab(PrefabRegistry& reg);
