/**********************************************************************************************
 * @file      CountdownUIComponent.h
 * @brief     カウントダウンUIコンポーネント
 *
 * カウントダウン数字(3/2/1)とSTARTの表示を管理します。数字は拡大＋回転演出、
 * STARTは拡大演出のみを行います。拡大は `Sprite2DComponent.width/height` を直接更新します。
 *
 * 各パラメータはプレハブ初期化または外部から調整できます。
 * - digitsAlias/startAlias: 使用するテクスチャのエイリアス
 * - digitBaseWidth/Height, startBaseWidth/Height: 拡大演出の基準サイズ
 * - digitPopScaleFrom/To, startPopScaleFrom/To: 拡大の開始/終了倍率
 * - digitPopDuration, startPopDuration: 拡大演出時間
 * - digitSpinSpeedDeg: 数字の回転速度（度/秒）
 * - digit3/2/1AnimTime, currentStartAnimTime: 演出経過時間（数字ごとに独立）
 **********************************************************************************************/
#pragma once
#include <DirectXMath.h>
#include <string>
#include "ECS/World.h"

struct CountdownUIComponent
{
    /** ベース位置（ワールド座標）とスケール */
    DirectX::XMFLOAT3 position{ 35.0f,20.0f,-50.0f };
    DirectX::XMFLOAT3 scale{ 8.0f,8.0f,1.0f };

    /** 使用するテクスチャエイリアス（数字/START） */
    std::string digitsAlias;  ///< 数字スプライトシートのエイリアス
    std::string startAlias;   ///< START用テクスチャのエイリアス

    /** スプライトシートの1セルサイズ（情報用） */
    int cellWidth = 128;
    int cellHeight = 128;

    /** 各スプライトのエンティティID（3/2/1/START） */
    EntityId sprite3{ kInvalidEntity };
    EntityId sprite2{ kInvalidEntity };
    EntityId sprite1{ kInvalidEntity };
    EntityId spriteStart{ kInvalidEntity };

    /** START表示の残り秒数とトリガフラグ（Countdown→Running遷移時に使用） */
    float startShowTime = 0.0f;
    bool startTriggered = false;

    /** 数字の回転速度（度/秒）と拡大演出の設定 */
    float digitSpinSpeedDeg = 0.0f;   ///< 数字の回転速度（Z回り）
    float digitPopScaleFrom = 0.5f;     ///< 数字の拡大開始倍率
    float digitPopScaleTo   = 8.0f;     ///< 数字の拡大終了倍率
    float digitPopDuration  = 0.45f;    ///< 数字の拡大演出時間（秒）

    /** STARTの拡大演出設定（回転は行わない） */
    float startPopScaleFrom = 0.5f;     ///< STARTの拡大開始倍率
    float startPopScaleTo   = 1.5f;     ///< STARTの拡大終了倍率
    float startPopDuration  = 0.5f;     ///< STARTの拡大演出時間（秒）

    /** 数字ごとのアニメ経過時間（表示開始でリセット） */
    float digit3AnimTime = 0.0f;        ///< 「3」の演出経過時間
    float digit2AnimTime = 0.0f;        ///< 「2」の演出経過時間
    float digit1AnimTime = 0.0f;        ///< 「1」の演出経過時間
    float currentStartAnimTime = 0.0f;  ///< STARTの演出経過時間

    /** 拡大演出の基準サイズ（Sprite2DComponent.width/height の基準値） */
    float digitBaseWidth  = 16.0f;      ///< 数字の基準幅
    float digitBaseHeight = 16.0f;      ///< 数字の基準高
    float startBaseWidth  = 40.0f;      ///< STARTの基準幅
    float startBaseHeight = 40.0f;      ///< STARTの基準高
};