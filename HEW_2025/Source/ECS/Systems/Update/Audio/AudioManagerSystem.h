#pragma once
#include <string>
#include <xaudio2.h>
#include <map>



class AudioManager
{
public:
    static bool Initialize();
    static bool LoadAudio(const std::string& name,const std::wstring filepath);
    static void PlayBGM(const std::string& name, bool loop);
    static void StopBGM();
    static void SetBGMVolume(float vol);
    static void PlaySE(const std::string& naem, float volume);

private:
    AudioManager() = delete; //インスタンス禁止
    ~AudioManager() = delete;

    static IXAudio2* xAudio;
    static IXAudio2MasteringVoice* masterVoice;

    struct SoundData {
        WAVEFORMATEX wfx{};
        XAUDIO2_BUFFER buffer{};
        BYTE* audioData = nullptr;
    };
    static IXAudio2SourceVoice* bgmVoice;
    static std::map<std::string, SoundData> sounds;

    struct SEVoice {
        IXAudio2SourceVoice* voice = nullptr;
        bool playing = false;
    };

    static std::map<std::string, SEVoice> seVoices;
};
