#include "ECS/Systems/Update/Audio/AudioPlaySystem.h"
#include "ECS/Systems/Update/Audio/AudioManagerSystem.h"
#include"ECS/Components/Input/MovementIntentComponent.h"
#include"ECS/Components/Input/PlayerInputComponent.h"
#include"ECS/Components/Physics/Rigidbody2DComponent.h"
#include <iostream>
#include <fstream>

void AudioPlaySystem::Update(World& world, float dt)
{
   


    static bool started = false;
  
    bool onGround1 = false;
    bool onGround2 = false;

   

    if (!started)
    {
        AudioManager::Initialize();
        AudioManager::LoadAudio("MainBGM", L"Assets/Audio/BGM.wav");
        AudioManager::PlayBGM("MainBGM", true);
        AudioManager::LoadAudio("JumpP1", L"Assets/Audio/jump.wav");
        AudioManager::LoadAudio("JumpP2", L"Assets/Audio/jump1.wav");
        AudioManager::SetBGMVolume(0.1f);
    

        started = true;
    }

    // 各プレイヤーのIntent取得
    MovementIntentComponent* intent1 = nullptr;
    MovementIntentComponent* intent2 = nullptr;
    
    world.View<PlayerInputComponent, MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId, const PlayerInputComponent& pic, MovementIntentComponent& intent, Rigidbody2DComponent& rb) {
            if (pic.playerIndex == 0) {
                intent1 = &intent;
                onGround1 = rb.onGround;
            }
            if (pic.playerIndex == 1) {
                intent2 = &intent;
                onGround2 = rb.onGround;
            }
        }
    );
    if (onGround1)
    {
        onSE1 = true;
    }
    if (onGround2)
    {
        onSE2 = true;
    }

    if (onSE1)
    {
        if (intent1->jump)
        {
            onSE1 = false;
           
            AudioManager::PlaySE("JumpP1", 0.1f);
        }
    }
   if (onSE2)
    {
       if (intent2->jump)
       {
           onSE2 = false;
           
           AudioManager::PlaySE("JumpP2", 0.1f);
       }
    }
   
}