/*********************************************************************/
/* @file   PlayerInputSystem.cpp
 * @brief  プレイヤー入力をIntentに落とし込む実装
 * 
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#include "PlayerInputSystem.h"
#include "System/Input.h"   // ←あなたの環境の入力ヘッダ名に合わせてください
#include <cmath> // 追加: 補間用
#include "ECS/Systems/Update/Audio/AudioManagerSystem.h"


// --- 追加: 角度補間関数 ---
namespace 
{
    // 角度の線形補間（度数法、-180～180の範囲で最短経路補間）
    float LerpAngle(float from, float to, float t)
    {
        float diff = std::fmod(to - from + 540.0f, 360.0f) - 180.0f;
        return from + diff * t;
    }
}

void PlayerInputSystem::Update(World& world, float /*dt*/)
{
    // 各プレイヤーのIntent取得
    MovementIntentComponent* intent1 = nullptr;
    MovementIntentComponent* intent2 = nullptr;

	PlayerInputComponent* pic1 = nullptr;
	PlayerInputComponent* pic2 = nullptr;

    EffectComponent* effect1 = nullptr;
    EffectComponent* effect2 = nullptr;
    EffectSlotsComponent* slots1 = nullptr;
    EffectSlotsComponent* slots2 = nullptr;

    world.View<PlayerInputComponent, MovementIntentComponent,EffectComponent,EffectSlotsComponent>(
        [&](EntityId, PlayerInputComponent& pic, MovementIntentComponent& intent,EffectComponent& ef, EffectSlotsComponent& slots) {
            if (pic.playerIndex == 0) 
            {
                intent1 = &intent;
				pic1 = &pic;
				effect1 = &ef;
                slots1 = &slots;
			}
            if (pic.playerIndex == 1)
            {
                intent2 = &intent;
				pic2 = &pic;
				effect2 = &ef;
                slots2 = &slots;
            }
        }
    );

    // 入力初期化・入力反映
    // 入力初期化・入力反映
    world.View<PlayerInputComponent, MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId,
            PlayerInputComponent& pic,
            MovementIntentComponent& intent,
            Rigidbody2DComponent& rig)
        {
            // Intent 初期化
            intent.moveX = 0.0f;
            intent.jump = false;
            intent.dash = false;
            pic.isJumpRequested = false;
            // ブリンク系は MovementApplySystem 側でフラグを見て処理するのでここでは消さない

            // 実入力を MovementIntent に反映
            switch (pic.playerIndex)
            {
            case 0: ReadPlayer0(intent, rig); break;
            case 1: ReadPlayer1(intent, rig); break;
            default: break;
            }

            // moveX が 0 でなければ向きを更新（Facing は StateSystem が使う）
            if (intent.moveX > 0.01f)
            {
                intent.facing = 1;
            }
            else if (intent.moveX < -0.01f)
            {
                intent.facing = -1;
            }
        }
    );


    // --- Rボタンジャンプ（相手に1回だけ） ---
    if (intent2 && !intent2->forceJumpConsumed) {
        if (IsPadTrigger(0, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
            intent2->forceJumpRequested = true;
            intent2->forceJumpConsumed = true;
            pic1->isJumpRequested = true;

            
            if (effect2)
            {
                if (slots2 && slots2->onJump)
                {
                    AudioManager::PlaySE("se_jump_p1", 0.1f);
                    effect2->effect = slots2->onJump;
                }
                effect2->loop = false;
                effect2->playRequested = true;
            }
        }
    }
    if (intent1 && !intent1->forceJumpConsumed) {
        if (IsPadTrigger(1, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
            intent1->forceJumpRequested = true;
            intent1->forceJumpConsumed = true;
			pic2->isJumpRequested = true;
            if (effect1)
            {
                if (slots1 && slots1->onJump)
                {
                    AudioManager::PlaySE("se_jump_p1", 0.1f);
                    effect1->effect = slots1->onJump;
                }
                effect1->loop = false;
                effect1->playRequested = true;
            }
        }
    }

    // --- Lボタンブリンク（相手に1回だけ、今向いている方向に高速移動） ---
    constexpr float BLINK_SPEED = 15.0f; // ブリンク速度（調整可）
    if (intent2) {
        if (IsPadTrigger(0, XINPUT_GAMEPAD_LEFT_SHOULDER)) 
        {
            intent2->blinkRequested = true;
            intent2->blinkSpeed = intent2->facing * BLINK_SPEED;
            // 入力時点ではエフェクトを再生しない。ワンショットフラグのみ設定
            pic1->isBlinkRequested = true;
        }
    }
    if (intent1) 
    {
        if (IsPadTrigger(1, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
            intent1->blinkRequested = true;
            intent1->blinkSpeed = intent1->facing * BLINK_SPEED;
            // 入力時点ではエフェクトを再生しない。ワンショットフラグのみ設定
            pic2->isBlinkRequested = true;
        }
    }

    // 以降のエフェクト再生は MovementApplySystem でブリンクが実行されたタイミングで行う
}


/**
 * 1P入力
 * キーボード: A/D + Space
 * Pad #0     : D-Pad左右 or 左スティックX + Aボタン
 */
void PlayerInputSystem::ReadPlayer0(MovementIntentComponent& outIntent, Rigidbody2DComponent& rig)
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
        if (rig.onGround == true)
        {
            AudioManager::PlaySE("se_jump_p1", 0.1f);
        }
        outIntent.jump = true;
    }
}

/**
 * 2P入力
 * キーボード: ←/→ + ↑
 * Pad #1     : D-Pad左右 or 左スティックX + Aボタン
 */
void PlayerInputSystem::ReadPlayer1(MovementIntentComponent& outIntent, Rigidbody2DComponent& rig)
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
        if (rig.onGround == true)
        {
            AudioManager::PlaySE("se_jump_p1", 0.1f);
        }

        outIntent.jump = true;
    }
}