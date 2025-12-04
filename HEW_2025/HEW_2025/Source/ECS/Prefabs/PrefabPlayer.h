/*****************************************************************//**
 * @file   PrefabPlayer.h
 * @brief  プレイヤー用プレハブの登録関数を提供するヘッダ
 *
 * TestSceneでベタ書きしていたプレイヤー生成処理を
 * PrefabRegistryに閉じ込めて再利用できるようにします。
 *
 * @author 浅野勇生
 * @date   2025/11/12
 *********************************************************************/
#pragma once

#include "PrefabRegistry.h"

 /**
  * @brief プレイヤーのプレハブをレジストリに登録する
  * @details
  *  この関数をどこかの初期化タイミングで呼んでおくと、
  *  registry.Spawn("Player", world, params);
  *  でプレイヤーを生成できるようになります。
  *
  *  生成されるプレイヤーは以下のコンポーネントを持ちます。
  *  - TransformComponent
  *  - PlayerInputComponent
  *  - MovementIntentComponent
  *  - Rigidbody2DComponent
  *  - ModelRendererComponent
  *  - Collider2DComponent
  *
  *  また、PrefabRegistry::SpawnParams の
  *  position / rotationDeg / scale / padIndex を反映します。
  *
  * @param registry 登録先のレジストリ
  */
void RegisterPlayerPrefab(PrefabRegistry& registry);
