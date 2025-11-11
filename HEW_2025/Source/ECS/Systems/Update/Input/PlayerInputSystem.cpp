/*********************************************************************/
/* @file   PlayerInputSystem.cpp
 * @brief  プレイヤー入力をIntentに落とし込む実装
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "PlayerInputSystem.h"
#include "System/Input.h"   // ←あなたの環境の入力ヘッダ名に合わせてください

void PlayerInputSystem::Update(World& world, float /*dt*/)
{
    world.View<PlayerInputComponent, MovementIntentComponent>(
        [&](EntityId,
            const PlayerInputComponent& pic,
            MovementIntentComponent& intent)
        {
            // 毎フレーム初期化
            intent.moveX = 0.0f;
            intent.jump = false;
            intent.dash = false;

            switch (pic.playerIndex)
            {
            case 0: ReadPlayer0(intent); break;
            case 1: ReadPlayer1(intent); break;
            default: break;
            }
        }
    );
}

/**
 * 1P入力
 * キーボード: A/D + Space
 * Pad #0     : D-Pad左右 or 左スティックX + Aボタン
 */
void PlayerInputSystem::ReadPlayer0(MovementIntentComponent& outIntent)
{
    // --- キーボード ---
    const bool keyLeft = IsKeyPress('A');
    const bool keyRight = IsKeyPress('D');
    const bool keyJump = IsKeyTrigger(VK_SPACE);

    // --- パッド0番 ---
    const bool padLeft = IsPadPress(0, XINPUT_GAMEPAD_DPAD_LEFT);
    const bool padRight = IsPadPress(0, XINPUT_GAMEPAD_DPAD_RIGHT);
    const bool padJump = IsPadTrigger(0, XINPUT_GAMEPAD_A);
    const float lx = GetPadLX(0);   // -1.0 ～ +1.0

    // 左右：キーボード優先→パッド→スティック
    if (keyLeft || padLeft)
    {
        outIntent.moveX = -1.0f;
    }
    else if (keyRight || padRight)
    {
        outIntent.moveX = 1.0f;
    }
    else
    {
        if (lx > 0.25f)       outIntent.moveX = lx;
        else if (lx < -0.25f) outIntent.moveX = lx;
    }

    // ジャンプ：Space or Pad A
    if (keyJump || padJump)
    {
        outIntent.jump = true;
    }
}

/**
 * 2P入力
 * キーボード: ←/→ + ↑
 * Pad #1     : D-Pad左右 or 左スティックX + Aボタン
 */
void PlayerInputSystem::ReadPlayer1(MovementIntentComponent& outIntent)
{
    // --- キーボード ---
    const bool keyLeft = IsKeyPress(VK_LEFT);
    const bool keyRight = IsKeyPress(VK_RIGHT);
    const bool keyJump = IsKeyTrigger(VK_UP);

    // --- パッド1番 ---
    const bool padLeft = IsPadPress(1, XINPUT_GAMEPAD_DPAD_LEFT);
    const bool padRight = IsPadPress(1, XINPUT_GAMEPAD_DPAD_RIGHT);
    const bool padJump = IsPadTrigger(1, XINPUT_GAMEPAD_A);
    const float lx = GetPadLX(1);

    if (keyLeft || padLeft)
    {
        outIntent.moveX = -1.0f;
    }
    else if (keyRight || padRight)
    {
        outIntent.moveX = 1.0f;
    }
    else
    {
        if (lx > 0.25f)       outIntent.moveX = lx;
        else if (lx < -0.25f) outIntent.moveX = lx;
    }

    if (keyJump || padJump)
    {
        outIntent.jump = true;
    }
}