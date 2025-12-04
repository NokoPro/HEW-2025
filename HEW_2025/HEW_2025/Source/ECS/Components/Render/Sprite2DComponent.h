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
#include <DirectXMath.h>

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

    /**
     * @brief 描画レイヤー(優先度)
     * @details.
     * この値でソートされます。
     * 数値が小さいほど「奥」に、大きいほど「手前」に描画されます。
     * (例:背景 = -10, キャラ = 0, UI = 100)
     * *同じレイヤー内では、TransformComponent::position.zが小さいものが
     * 手前に描画されます(Zソート)。
     */
    int layer = 0;

    /**
     * @brief 描画有効フラグ
     * @details true のときのみ描画処理とテクスチャ取得を行います。
     */
    bool visible = true;

	DirectX::XMFLOAT2 uvOffset = { 0.0f, 0.0f }; ///< UVオフセット
	DirectX::XMFLOAT2 uvSize = { 1.0f, 1.0f };   ///< UVサイズ
};







