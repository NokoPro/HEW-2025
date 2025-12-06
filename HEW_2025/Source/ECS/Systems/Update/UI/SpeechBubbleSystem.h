/**********************************************************************************************
 * @file      SpeechBubbleSystem.h
 * @brief     吹き出しUI（背景＋コンテンツ）の更新システム宣言
 *
 * アクティブカメラの視野とプレイヤー位置を用いて、画面下端に固定される吹き出し背景と
 * その上のコンテンツ（キャラアイコン/HI/GO）の表示・位置を更新します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"

class SpeechBubbleSystem : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
};
