#pragma once
#include <xaudio2.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include <vector>

//----------------------------------------------
// AudioManagerSystem
//  DirectX(XAudio2)を利用したサウンド管理クラス
//----------------------------------------------
class AudioManagerSystem
{
public:
    struct SoundData // ← public に移動！（外部関数からアクセスできるように）
    {
        std::vector<BYTE> buffer;
        WAVEFORMATEX format = {};
    };

    static AudioManagerSystem& Instance()
    {
        static AudioManagerSystem instance;
        return instance;
    }

    // 初期化・更新・解放
    bool Init();
    void Update();
    void Release();

    // BGM操作
    void PlayBGM(const std::string& name, bool loop = true);
    void StopBGM();
    void SetBGMVolume(float volume);

    // 効果音(SE)操作
    void PlaySE(const std::string& name);
    void SetSEVolume(float volume);

    // 音声データ読み込み
    bool LoadBGM(const std::string& name, const std::wstring& filePath);
    bool LoadSE(const std::string& name, const std::wstring& filePath);

private:
    AudioManagerSystem() = default;
    ~AudioManagerSystem() = default;

    // コピー禁止
    AudioManagerSystem(const AudioManagerSystem&) = delete;
    AudioManagerSystem& operator=(const AudioManagerSystem&) = delete;

private:
    // XAudio2本体
    Microsoft::WRL::ComPtr<IXAudio2> m_xAudio2;
    IXAudio2MasteringVoice* m_masterVoice = nullptr;

    // BGM・SE用マップ
    std::unordered_map<std::string, SoundData> m_bgmData;
    std::unordered_map<std::string, SoundData> m_seData;

    IXAudio2SourceVoice* m_bgmVoice = nullptr;
    float m_bgmVolume = 1.0f;
    float m_seVolume = 1.0f;
};