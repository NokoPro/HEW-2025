/*****************************************************************//**
 * @file   EffectRuntime.h
 * @brief  Effekseer ベースのエフェクト再生ランタイム
 *
 * - DirectX11 で初期化されたデバイスに対して Effekseer を構築する
 * - 毎フレーム Update / Render を呼び出すことでエフェクトを更新・描画する
 * - EffectSystem / EffectComponent からは Play / SetLocation / Stop 等だけを叩く
 *
 * 使い方（ざっくり）:
 *   // DirectX 初期化後
 *   EffectRuntime::Initialize();
 *
 *   // 毎フレーム:
 *   EffectRuntime::Update(dt);
 *   EffectRuntime::Render(projMatrixEfk, cameraMatrixEfk);
 *
 *   // 終了時:
 *   EffectRuntime::Shutdown();
 *
 *   // ECS 側（EffectSystem）から:
 *   auto handle = EffectRuntime::Play(path.c_str(), pos, loop);
 *   EffectRuntime::SetLocation(handle, pos);
 *
 * @author  浅野勇生
 * @date    2025/11/27
 *********************************************************************/
#pragma once

#include <cstdint>
#include <DirectXMath.h>

// Effekseer の Matrix44 の前方宣言（ヘッダでの依存を最小化）
namespace Effekseer { struct Matrix44; }

namespace EffectRuntime
{
    /// Effekseer::Handle 相当。外側では int として扱ってよい。
    using Handle = std::int32_t;

    /**
     * @brief Effekseer ランタイム初期化
     * @return 成功したら true
     *
     * - 事前に InitDirectX() で DirectX11 初期化が終わっていること
     * - 複数回呼んでも 2 回目以降は何もしない
     */
    bool Initialize();

    /**
     * @brief ランタイム終了処理
     *
     * - アプリ終了時に 1 度だけ呼び出す
     */
    void Shutdown();

    /**
     * @brief 毎フレームの更新
     * @param dt 経過時間[秒]（現在は Effekseer::Manager に対しては未使用）
     *
     * - エミッタ寿命やパーティクルを進めるために毎フレーム呼び出す
     */
    void Update(float dt);

    /**
     * @brief エフェクト描画
     * @param projection Effekseer の投影行列（Matrix44）
     * @param camera     Effekseer のカメラ行列（Matrix44）
     *
     * - Guide_Cpp_Ja.md の「7. カメラと座標系をアプリケーションと一致させる」で
     *   紹介されている Matrix44 をそのまま渡す想定
     * - DirectX(XMMATRIX) → Effekseer::Matrix44 の変換は、呼び出し側で行う
     */
    // Effekseer の行列型に名前を合わせる（エイリアス）
    using Matrix44 = ::Effekseer::Matrix44;
    void Render();

    void SetCamera(const DirectX::XMFLOAT4X4& viewT,
        const DirectX::XMFLOAT4X4& projT);

    /**
     * @brief エフェクトを再生する
     * @param pathUtf8  再生する .efkefc へのパス（UTF-8 / ASCII 前提）
     * @param worldPos  再生開始位置（ワールド座標）
     * @param loop      ループさせたいか（現在は「自動破棄しない」判定にのみ使用）
     * @return 再生インスタンスのハンドル（失敗時は -1）
     */
    Handle Play(const char* pathUtf8, const DirectX::XMFLOAT3& worldPos, bool loop);

    /**
     * @brief 再生中インスタンスの位置を更新する
     */
    void SetLocation(Handle handle, const DirectX::XMFLOAT3& worldPos);

    /**
     * @brief 再生中インスタンスを停止する
     */
    void Stop(Handle handle);

    /**
     * @brief インスタンスが再生終了済みかどうか
     * @return true なら「もう再生されていない」
     */
    bool IsFinished(Handle handle);

    // 追加: 回転とスケール制御API
    void SetRotationDeg(Handle handle, const DirectX::XMFLOAT3& rotationDeg);
    void SetScale(Handle handle, const DirectX::XMFLOAT3& scale);

    // 位置・回転（度）・スケールを一括適用
    void SetTransform(Handle handle,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& rotationDeg,
        const DirectX::XMFLOAT3& scale);

    // パスを指定して「読み込むだけ」のプリロード
    // すでにキャッシュ済みなら何もしない。
    bool Preload(const char* pathUtf8);

} // namespace EffectRuntime
