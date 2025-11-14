#pragma once
#include <string>
#include <xaudio2.h>
#include <map>

class AudioManagerSystem
{
public:
    static AudioManagerSystem& Instance()
    {
        static AudioManagerSystem inst;
        return inst;
    }

    bool Initialize();
    bool LoadBGM(const std::string& name, const std::wstring& filepath);
    void PlayBGM(const std::string& name, bool loop);
    void StopBGM();
    void SetBGMVolume(float vol);
    void PlaySE(const std::string& name, float volume );


private:
    AudioManagerSystem() = default;
    ~AudioManagerSystem();

    IXAudio2* xAudio = nullptr;
    IXAudio2MasteringVoice* masterVoice = nullptr;

    struct SoundData {
        WAVEFORMATEX wfx{};
        XAUDIO2_BUFFER buffer{};
        BYTE* audioData = nullptr;
    };

    IXAudio2SourceVoice* bgmVoice = nullptr;
    std::map<std::string, SoundData> sounds;

    struct SEVoice
    {
        IXAudio2SourceVoice* voice = nullptr;
        bool playing = false;
    };

    std::map<std::string, SEVoice> seVoices; // äe SE ÇÃçƒê∂èÛë‘Çï€éù
};
