/*****************************************************************//**
 * @file   ModelComponent.h
 * @brief  3Dモデル描画に必要な情報を保持するコンポーネント
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <string>
#include "System/AssetManager.h"

 /**
  * @brief 3Dモデルを描画するための情報
  * alias には AssetCatalog/AssetManager に登録した名前を入れておく
  * （例: "player", "stage01" など）
  */
struct ModelRendererComponent
{
    AssetHandle<Model> model;  ///< 描画するモデル
    bool visible = true;
};