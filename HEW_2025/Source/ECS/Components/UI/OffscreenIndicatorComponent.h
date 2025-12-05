/**********************************************************************************************
 * @file      OffscreenIndicatorComponent.h
 * @brief     画面外（下側）に出たプレイヤーを示すインジケータUIの設定を保持するコンポーネント
 *
 * 対象プレイヤーの X 軸に追従しつつ、画面下端に沿って表示するインジケータの表示制御や
 * 位置オフセット、下端からのマージン量などを保持します。1P/2Pで独立して利用できます。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/ECS.h"
#include <DirectXMath.h>

/**
 * @struct OffscreenIndicatorComponent
 * @brief 画面下端インジケータの表示制御・追従設定
 */
struct OffscreenIndicatorComponent
{
    /** 追従対象となるプレイヤーのエンティティID */
    EntityId targetId{ kInvalidEntity };

    /** 現在の表示状態（System が制御） */
    bool     visible{ false };

    /** 画面下端からの上方向マージン（ワールド座標単位） */
    float    bottomMargin{ 0.5f };

    /** UIの追加オフセット（X/Y/Z） */
    DirectX::XMFLOAT3 offset{ 0.0f, 0.0f, 0.0f };
};
