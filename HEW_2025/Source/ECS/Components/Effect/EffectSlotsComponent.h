/**********************************************************************************************
 * @file      EffectSlotsComponent.h
 * @brief     エフェクトのプリセットスロットを保持するコンポーネント
 *
 * @author    浅野勇生
 * @date      2025/11/27
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/27 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once

#include "System/AssetManager.h"
#include <DirectXMath.h>

struct EffectParams
{
    // ローカルオフセット（エンティティ基準）
    DirectX::XMFLOAT3 offset{ 0.0f, 0.0f, 0.0f };
    // ローカル回転（度数法）
    DirectX::XMFLOAT3 rotationDeg{ 0.0f, 0.0f, 0.0f };
    // ローカルスケール
    DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };
};

struct EffectSlotsComponent
{
    AssetHandle<EffectRef> onJump;   // ジャンプ時
    AssetHandle<EffectRef> onBlink;  // ブリンク時
    AssetHandle<EffectRef> onDash;   // ダッシュ時
    AssetHandle<EffectRef> onLand;   // 着地時

    // それぞれのスロット用の変換パラメータ
    EffectParams onJumpParams{};
    EffectParams onBlinkParams{};
    EffectParams onDashParams{};
    EffectParams onLandParams{};
};
