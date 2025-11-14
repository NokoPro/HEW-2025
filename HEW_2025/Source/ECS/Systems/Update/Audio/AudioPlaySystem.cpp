#include "ECS/Systems/Update/Audio/AudioPlaySystem.h"
#include "ECS/Systems/Update/Audio/AudioManagerSystem.h"
#include <iostream>
#include <fstream>

void AudioPlaySystem::Update(World& world, float dt)
{
    static bool started = false;
    if (started) return;

    auto& audio = AudioManagerSystem::Instance();

    audio.Initialize(); // Å©ïKóv
  
    if (!audio.LoadBGM("MainBGM", L"C:/HAL/HEW2.5D/HEW_2025/Assets/Audio/BGMHEW.wav"))
        return;

    audio.PlayBGM("MainBGM", true);
    audio.SetBGMVolume(0.8f);

    started = true;
}
