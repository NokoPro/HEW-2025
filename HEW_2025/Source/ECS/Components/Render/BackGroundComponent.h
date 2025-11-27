/*****************************************************************//**
 * @file   BackGroundComponent.h
 * @brief  背景描画のための情報を保持するコンポーネント
 *
 * @author 土本蒼翔
 * @date   2025/11/25
 *********************************************************************/
#pragma once
#include <string>

 /**
   * @brief スプライトを描画するための情報
   * @details
   * textureId は AssetManager などから取得したIDを入れる想定です。
   * UV矩形やカラーなどは、必要になったらフィールドを追加してください。
   */
struct BackGroundComponent
{
    std::string alias;           ///< 使用するテクスチャID
    float width = 1920.0f;         ///< 表示幅（ピクセルベース想定）
    float height = 1080.0f;        ///< 表示高さ
    float originX = 0.5f;        ///< 原点X(0～1) 0.5なら中央
    float originY = 0.5f;        ///< 原点Y(0～1)

    /**
     * @brief 描画レイヤー(優先度)
     * @details.
     * この値でソートされます。
     * 数値が小さいほど「奥」に、大きいほど「手前」に描画されます。
     * (例:背景 = -10, キャラ = 0, UI = 100)
     * *同じレイヤー内では、TransformComponent::position.zが小さいものが
     * 手前に描画されます(Zソート)。
     */
    int layer = -10;

    /**
     * @brief 描画有効フラグ
     * @details true のときのみ描画処理とテクスチャ取得を行います。
     */
    bool visible = true;
};
