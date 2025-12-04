/**********************************************************************************************
 * @file      AudioPlaySystem.cpp
 * @brief     PlayerInputComponentの入力に応じてSEを再生するシステム
 *
 * @author    浅野勇生
 * @date      2025/11/24
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/24 ]
 * 
 *    - [◎] AssetManagerからの再生を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#include "ECS/Systems/Update/Audio/AudioPlaySystem.h"
#include "ECS/Systems/Update/Audio/AudioManagerSystem.h"

#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Input/PlayerInputComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"

void AudioPlaySystem::Update(World& world, float dt)
{
    static bool started = false;

    if (!started)
    {
        AudioManager::Initialize();

        // ===== CSV alias だけを使う =====
        AudioManager::LoadAudioAlias("bgm_main");
        AudioManager::PlayBGM("bgm_main", true);
        AudioManager::SetBGMVolume(0.1f);

        // SE
        AudioManager::LoadAudioAlias("se_jump_p1");
        AudioManager::LoadAudioAlias("se_jump_p2");

        started = true;
    }

    bool onGround1 = false;
    bool onGround2 = false;

    MovementIntentComponent* intent1 = nullptr;
    MovementIntentComponent* intent2 = nullptr;

    world.View<PlayerInputComponent, MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId, const PlayerInputComponent& pic, MovementIntentComponent& intent, Rigidbody2DComponent& rb)
        {
            if (pic.playerIndex == 0) {
                intent1 = &intent;
                onGround1 = rb.onGround;
            }
            else if (pic.playerIndex == 1) {
                intent2 = &intent;
                onGround2 = rb.onGround;
            }
        }
    );

    if (onGround1) onSE1 = true;
    if (onGround2) onSE2 = true;

    if (onSE1 && intent1 && intent1->jump)
    {
        onSE1 = false;
        AudioManager::PlaySE("se_jump_p1", 0.1f);
    }

    if (onSE2 && intent2 && intent2->jump)
    {
        onSE2 = false;
        AudioManager::PlaySE("se_jump_p2", 0.1f);
    }
}