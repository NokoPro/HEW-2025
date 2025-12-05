/*****************************************************************//**
 * @file   ModelComponent.h
 * @brief  3Dモデル描画に必要な情報を保持するコンポーネント
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include <string>
#include <array>
#include "System/AssetManager.h"
#include "System/Model.h"

 /**
  * @brief 3Dモデルを描画するための情報
  * alias には AssetCatalog/AssetManager に登録した名前を入れておく
  * （例: "player", "stage01" など）
  */
struct ModelRendererComponent
{
    AssetHandle<Model> model;  ///< 描画するモデル
    AssetHandle<Texture> baseTexture;      // 1P/2Pの色違い用
    AssetHandle<Texture> overrideTexture;  // 表情用（なければ baseTexture or モデルデフォルト）
    bool visible = true;

    DirectX::XMFLOAT3 localOffset{ 0.f,0.f,0.f };
	DirectX::XMFLOAT3 localRotationDeg{ 0.f,0.f,0.f };
	DirectX::XMFLOAT3 localScale{ 1.f,1.f,1.f };

    /**
     * @brief 描画レイヤー(優先度)
     * @details
     * この値でソートされます。
     * 数値が小さいほど「奥」に、大きいほど手前に描画されます。
     * (例:3D背景 = -20, 3Dキャラ = 0, 2Dエフェクト = 10)
     * カメラに近いもの(Zが小さいもの)が手前に描画されます(Zバッファによる深度テスト)。.
     */
    int layer = 0;
};

/**
 * @brief 3Dモデルアニメーション制御用コンポーネント
 * @details
 * - 他のシステム（入力／状態遷移など）が animeNo / loop / speed を設定し、
 *   playRequested を true にする。
 * - ModelAnimationSystem がそれを監視して Model::Play を呼び、毎フレーム Step する。
 */
struct ModelAnimationComponent
{
    /// 再生したいアニメ番号（Model::AnimeNo と同じ意味。-1 なら何もしない）
    int   animeNo = -1;

    /// ループ再生するか
    bool  loop = true;

    /// 再生速度（Model::Play の speed にそのまま渡す）
    float speed = 1.0f;

    /// true のフレームに Model::Play() を呼ぶ
    bool  playRequested = false;
};

/**
 * @brief 3Dモデルアニメーション制御用コンポーネント
 * @details
 * - 他のシステム（入力／状態遷移など）が animeNo / loop / speed を設定し、
 *   playRequested を true にする。
 * - ModelAnimationSystem がそれを監視して Model::Play を呼び、毎フレーム Step する。
 */
struct ModelAnimationClipDesc
{
    int   animeNo = -1;   ///< Model::AnimeNo 相当
    bool  loop = true;
    float speed = 1.0f;
};

/// モデルアニメーションの論理状態
enum class ModelAnimState
{
    None = 0,
    Idle,
    Walk,
    Run,
    RunLeft,
    RunRight,
    Jump,
    Fall,
    Land,
    Blink,
    Goal,
	Death,

    Count,  // 配列サイズ用
};


/// 「ステート → アニメクリップ」の対応テーブル
struct ModelAnimationTableComponent
{
    std::array<ModelAnimationClipDesc, static_cast<size_t>(ModelAnimState::Count)> table;
};

/// 「今どの論理状態か」を管理するコンポーネント
struct ModelAnimationStateComponent
{
    ModelAnimState current = ModelAnimState::None;   ///< 現在適用中
    ModelAnimState requested = ModelAnimState::None;   ///< 他のシステムが書き込むターゲット状態
};

/// 他のシステムから簡単に呼ぶ用ヘルパー
inline void RequestModelAnimation(ModelAnimationStateComponent& state,
    ModelAnimState next)
{
    // 無駄な書き換えを減らしてデバッグしやすくする
    if (state.requested != next)
    {
        state.requested = next;
    }
}