/**********************************************************************************************
 * @file      PrefabOffscreenIndicatorFaceP1.h
 * @brief     1P用 画面下端インジケータ上物（アイコン / HI / GO）プレハブ登録API
 *
 * 吹き出しとは別エンティティとして、上に重ねて表示する UI を生成します。
 * - OffscreenIndicatorComponent : 位置追従・表示制御
 * - OffscreenIndicatorFaceComponent : アイコン / HI / GO の切り替え制御
 *
 * 対象プレイヤーは生成後に OffscreenIndicatorComponent.targetId を外部から設定します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Prefabs/PrefabRegistry.h"

 /**
  * @brief 1P用インジケータ上物プレハブをレジストリへ登録
  */
void RegisterOffscreenIndicatorFaceP1Prefab(PrefabRegistry& reg);
