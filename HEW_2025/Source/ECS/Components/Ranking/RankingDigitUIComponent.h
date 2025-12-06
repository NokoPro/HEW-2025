/**********************************************************************************************
 * @file      RankingDigitUIComponent.h
 * @brief     ランキング用タイム表示の桁指定コンポーネント
 */
#pragma once

struct RankingDigitUIComponent
{
    // 0..5 (最大6行)
    int row = 0;

    enum class Type
    {
        MinTens,      // 分10の位
        MinOnes,      // 分1の位
        Separator,    // コロン（:）
        SecTens,      // 秒10の位
        SecOnes,      // 秒1の位
        Dot,          // 小数点（.)
        MsTens,       // 1/100秒10の位
        MsOnes,       // 1/100秒1の位
    } type = Type::MinTens;
};