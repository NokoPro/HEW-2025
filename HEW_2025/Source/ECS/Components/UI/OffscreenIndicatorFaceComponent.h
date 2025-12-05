/**********************************************************************************************
 * @file      OffscreenIndicatorFaceComponent.h
 * @brief     インジケータ吹き出し上のアイコン / HI / GO 切り替え用コンポーネント
 *
 * 対象プレイヤーの入力（ジャンプ / ブリンク）を監視し、
 * 一定時間だけ HI / GO テクスチャに差し替えます。
 * 時間が切れたら常時表示用アイコンに戻します。
 *
 * 位置の追従や表示 / 非表示は OffscreenIndicatorComponent +
 * PlayerOffscreenIndicatorSystem 側で行う想定です。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include <string>

 /**
  * @struct OffscreenIndicatorFaceComponent
  * @brief インジケータ上物（アイコン / HI / GO）の状態管理
  *
  * 同一エンティティには以下のコンポーネントを併用する想定です。
  * - TransformComponent
  * - Sprite2DComponent
  * - OffscreenIndicatorComponent（位置追従・表示制御）
  */
struct OffscreenIndicatorFaceComponent
{
    /// 表示モード
    enum class Mode
    {
        Icon,   ///< 通常状態（キャラクターアイコン）
        Hi,     ///< ジャンプ入力後の HI 表示
        Go      ///< ブリンク入力後の GO 表示
    };

    // --- 時間設定（秒） ---
    float hiDuration = 2.0f;  ///< HI を表示しておく時間
    float goDuration = 2.0f;  ///< GO を表示しておく時間

    // --- 内部タイマー（System が制御） ---
    float hiTimer = 0.0f;     ///< >0 の間 HI 表示候補
    float goTimer = 0.0f;     ///< >0 の間 GO 表示候補（HI より優先）

    Mode currentMode = Mode::Icon;  ///< 現在の表示モード

    // --- テクスチャ別名 ---
    std::string aliasIcon;   ///< 通常時アイコン用テクスチャ別名
    std::string aliasHi;     ///< HI 表示用テクスチャ別名
    std::string aliasGo;     ///< GO 表示用テクスチャ別名
};
