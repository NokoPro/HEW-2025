/**********************************************************************************************
 * @file      AudioPlaySystem.h
 * @brief     外部ファイルからオーディオ読み込みなどを組み込み
 *
 * @author    浅野勇生
 * @date      2025/11/24
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/24 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#pragma once
#include"ECS/World.h"
#include"ECS/Systems/IUpdateSystem.h"
#include"ECS/Systems/Update/Audio/AudioManagerSystem.h"

class AudioPlaySystem final : public IUpdateSystem
{
public:
    void Update(World& world, float dt) override;
private:
    bool onSE1 = false;
    bool onSE2 = false;
};  
