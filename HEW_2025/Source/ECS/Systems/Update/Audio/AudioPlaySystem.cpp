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
    auto& audio = AudioManagerSystem::Instance();
    auto& P1SE = AudioManagerSystem::Instance();
    auto& P2SE = AudioManagerSystem::Instance();
    bool onGround1 = false;
    bool onGround2 = false;

   

    if (!started)
    {

        audio.Initialize();
        P1SE.Initialize();
        P2SE.Initialize();

        if (!audio.LoadBGM("MainBGM", L"Assets/Audio/BGM.wav"))
            return;

        if (!P1SE.LoadBGM("JumpSE", L"Assets/Audio/jump.wav"))  // SEもロード
            return;
        if (!P2SE.LoadBGM("JumpSE1", L"Assets/Audio/jump1.wav"))  // SEもロード
            return;

        audio.PlayBGM("MainBGM", true);
        audio.SetBGMVolume(0.1f);

        started = true;
    }

    // auto& audio = AudioManagerSystem::Instance();
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
            P1SE.PlaySE("JumpSE", 0.8f); // 音量 0.8

        }
    }
   if (onSE2)
    {
       if (intent2->jump)
       {
           onSE2 = false;
           P2SE.PlaySE("JumpSE1", 0.8f); // 音量 0.8

       }
    }
   
    
   
   


   
}