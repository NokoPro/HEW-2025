#include "ECS/Systems/Update/Audio/AudioPlaySystem.h"
#include "ECS/Systems/Update/Audio/AudioManagerSystem.h"
#include <iostream>

void AudioPlaySystem::Update(World& world, float dt)
{
    static bool bgmStarted = false; // 一度だけ再生するためのフラグ
    if (!bgmStarted)
    {
        auto& audio = AudioManagerSystem::Instance();

        // BGMファイルの相対パス（プロジェクトの実行フォルダからの相対）
        // 実行時のカレントディレクトリが「HEW_2025\」なら下記でOK
        std::wstring bgmPath = L"Assets/Audio/BGM.wav";

        // 1? WAVファイルをロード
        if (!audio.LoadBGM("MainBGM", bgmPath))
        {
            std::wcerr << L"[AudioPlaySystem] BGMの読み込みに失敗: " << bgmPath << std::endl;
            return;
        }

        // 2? ループ再生でBGMを再生
        audio.PlayBGM("MainBGM", true); // true = ループ再生
        audio.SetBGMVolume(0.8f);       // 音量（0.0～1.0）

        std::cout << "[AudioPlaySystem] BGM再生開始" << std::endl;
        bgmStarted = true;
    }
}
