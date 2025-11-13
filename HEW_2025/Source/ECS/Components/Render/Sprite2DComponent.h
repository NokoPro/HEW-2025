/*****************************************************************//**
 * @file   Sprite2DComponent.h
 * @brief  2Dスプライト描画のための情報を保持するコンポーネント
 *
 * @author 浅野勇生
 * @author 川谷優真
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <string>

 /**
  * @brief スプライトを描画するための情報
  * @details
  * textureId は AssetManager などから取得したIDを入れる想定です。
  * UV矩形やカラーなどは、必要になったらフィールドを追加してください。
  */
struct Sprite2DComponent
{
    std::string alias;           ///< 使用するテクスチャID
    float width = 64.0f;         ///< 表示幅（ピクセルベース想定）
    float height = 64.0f;        ///< 表示高さ
    float originX = 0.5f;        ///< 原点X(0～1) 0.5なら中央
    float originY = 0.5f;        ///< 原点Y(0～1)
};
