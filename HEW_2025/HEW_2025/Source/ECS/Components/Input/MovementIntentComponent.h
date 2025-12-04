/*****************************************************************//**
 * @file   MovementIntentComponent.h
 * @brief  入力によって「こう動きたい」が書き込まれるコンポーネント
 *
 * ここには「左に1」「右に0.4」「ジャンプしたい」などの意思だけを入れておく。
 * 実際に速度にするのは別のSystemにしておくと、氷の床とかで調整しやすい。
 *********************************************************************/
#pragma once

struct MovementIntentComponent
{
    bool jump = false;                // 通常ジャンプ
    bool forceJumpRequested = false;  // 強制ジャンプ要求
    bool forceJumpConsumed = false;   // 強制ジャンプ消費済み
    float moveX = 0.0f;               // 移動（必要なら追加）
    bool dash = false;                // ダッシュ（必要なら追加）

    // 向き（1:右, -1:左）
    int facing = 1;

    // ブリンク
    bool blinkRequested = false; // ブリンク要求
    bool blinkConsumed = false;  // ブリンク消費済みフラグ
    float blinkSpeed = 0.0f;     // ブリンク時の速度
    bool isBlinking = false;     // ブリンク中フラグ
};

