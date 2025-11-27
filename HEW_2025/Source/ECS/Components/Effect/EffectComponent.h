/*********************************************************************/
/* @file   EffectComponent.h
 * @brief  エフェクト再生用コンポーネント
 *
 * - AssetManager から取得した EffectRef を元に、エフェクトを再生する
 * - 実際の再生は EffectRuntime (Effekseer などのラッパー) に任せる
 *
 * 使用イメージ:
 *   auto& ef = world.Add<EffectComponent>(e);
 *   ef.effect = AssetManager::GetEffect("vfx_dash");
 *   ef.playOnSpawn = true; // 生成時に自動再生
 *
 *   // もしくは:
 *   ef.playRequested = true; // このフレームで再生指示
 *
 * @author 浅野勇生
 * @date   2025/11/27
 *********************************************************************/
#pragma once
#include <cstdint>
#include <string>
#include <DirectXMath.h>
#include "System/AssetManager.h"

 /**
  * @brief エフェクト再生コンポーネント
  */
struct EffectComponent
{
    /// 再生するエフェクト情報（AssetManager::GetEffect で取得）
    AssetHandle<EffectRef> effect;

    /// 直前に再生していたエフェクトのパス（切替検知用）
    std::string lastPath;

    /// 生成されたフレームで自動再生するか
    bool playOnSpawn = true;

    /// ループ再生するか（param1 が "loop" などのときに true にするなど）
    bool loop = false;

    /// 再生完了したときに、このエンティティを自動で破棄するか
    bool autoDestroyEntity = true;

    /// 外から「今フレーム再生してほしい」ときに立てるフラグ
    bool playRequested = false;

    /// 外から停止したいときに立てるフラグ
    bool stopRequested = false;

    /// 今再生中かどうか
    bool playing = false;

    /// ランタイム側のハンドル（Effekseer::Handle 等に相当）
    int nativeHandle = -1;

    /// エフェクトのローカルオフセット（エンティティの位置からの相対座標）
    DirectX::XMFLOAT3 offset{ 0.0f, 0.0f, 0.0f };

    /// エフェクトのローカル回転（度数法、XYZ）
    DirectX::XMFLOAT3 rotationDeg{ 0.0f, 0.0f, 0.0f };

    /// エフェクトのローカルスケール（XYZ）
    DirectX::XMFLOAT3 scale{ 1.0f, 1.0f, 1.0f };
};
