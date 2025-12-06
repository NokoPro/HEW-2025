/*****************************************************************//**
 * @file   ModelRenderSystem.h
 * @brief  Transform と ModelRenderer を組み合わせて3Dモデルを描画するシステム
 *
 * - 各エンティティの TransformComponent で位置・回転・スケールを取得し、
 *   ModelRendererComponent に登録されている Model を描画します。
 * - ビュー・プロジェクション行列はカメラシステムから受け取る設計です。
 * - 照明やカメラ位置は ShaderList の静的関数経由で設定します。
 * - 2025/11/14 レイヤーに基づきソートしてから描画する
 * @author 浅野勇生
 * @date   2025/11/8
 *********************************************************************/
#pragma once

#include "../IRenderSystem.h"
#include <DirectXMath.h>
#include <vector>
#include "System/Model.h"
#include "System/DirectX/Texture.h" 
#include "System/AssetManager.h"


 /**
  * @class ModelRenderSystem
  * @brief TransformComponent + ModelRendererComponent を描画するレンダリングシステム
  * @details
  * - 各エンティティごとにワールド行列を計算してシェーダへ送信。
  * - WVP（World, View, Projection）は配列としてまとめて送っています。
  * - 照明設定やカメラ位置も ShaderList の静的関数を通じて共有。
  */
class ModelRenderSystem final : public IRenderSystem
{
public:
    /**
     * @brief ビュー行列・プロジェクション行列を設定する
     * @param V ビュー行列（転置済みを想定）
     * @param P プロジェクション行列（転置済みを想定）
     */
    void SetViewProj(const DirectX::XMFLOAT4X4& V, const DirectX::XMFLOAT4X4& P)
    {
        m_V = V;
        m_P = P;
    }

    /**
     * @brief すべての ModelRendererComponent を描画する
     * @param world ECSのWorld
     */
    void Render(const class World& world) override;

    /**
     * @brief シーン開始時などに一度だけ呼び出すデフォルトライティング設定
     * @param camY カメラ高さ（光源設定用）
     * @param camRadius カメラ距離（カメラ位置設定用）
     */
    static void ApplyDefaultLighting(float camY, float camRadius);

private:
    DirectX::XMFLOAT4X4 m_V{}; ///< ビュー行列（転置済み）
    DirectX::XMFLOAT4X4 m_P{}; ///< プロジェクション行列（転置済み）

private:
    /**
     * @brief ソートと描画のためのモデル一時情報.
     */
    struct SortableModel
    {
        // ソート用キー
        int layer; // <ModelRendererComponent::layer

        // 描画用データ
        DirectX::XMFLOAT4X4 world; // ワールド行列(Transpose済み)
        Model* model;              // モデルハンドル
        AssetHandle<Texture> overrideTexture; // keep shared_ptr alive for the frame
    };

    // 描画リスト(毎フレーム使いまわす)
    std::vector<SortableModel>m_modelList;
};




