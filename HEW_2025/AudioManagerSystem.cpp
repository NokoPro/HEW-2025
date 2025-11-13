#include "AudioManagerSystem.h"
#include <fstream>
#include <iostream>
#include <thread>   // ★追加
#include <chrono>   // ★追加

using Microsoft::WRL::ComPtr;

//----------------------------------------------
// WAVファイル読み込み関数
//----------------------------------------------
static bool LoadWaveFile(const std::wstring& filePath, AudioManagerSystem::SoundData& outData)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        std::wcerr << L"[Audio] ファイルが開けません: " << filePath << std::endl;
        return false;
    }

    // RIFFヘッダ確認
    char riff[4];
    file.read(riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0)
        return false;

    file.seekg(4, std::ios::cur); // ファイルサイズスキップ
    char wave[4];
    file.read(wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0)
        return false;

    WAVEFORMATEX wfx = {};
    std::vector<BYTE> audioBuffer;

    while (file)
    {
        char chunkId[4];
        DWORD chunkSize = 0;
        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), sizeof(DWORD));

        if (strncmp(chunkId, "fmt ", 4) == 0)
        {
            file.read(reinterpret_cast<char*>(&wfx), chunkSize);
        }
        else if (strncmp(chunkId, "data", 4) == 0)
        {
            audioBuffer.resize(chunkSize);
            file.read(reinterpret_cast<char*>(audioBuffer.data()), chunkSize);
        }
        else
        {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    if (audioBuffer.empty())
    {
        std::wcerr << L"[Audio] データチャンクが見つかりません: " << filePath << std::endl;
        return false;
    }

    outData.buffer = std::move(audioBuffer);
    outData.format = wfx;

    return true;
}

//----------------------------------------------
// 初期化
//----------------------------------------------
bool AudioManagerSystem::Init()
{
    HRESULT hr = XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        std::cerr << "[Audio] XAudio2の初期化に失敗しました。" << std::endl;
        return false;
    }

    hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);
    if (FAILED(hr))
    {
        std::cerr << "[Audio] MasterVoice作成失敗。" << std::endl;
        return false;
    }

    std::cout << "[Audio] AudioManagerSystem 初期化完了" << std::endl;
    return true;
}

//----------------------------------------------
// 更新（フェードなどを将来的に実装）
//----------------------------------------------
void AudioManagerSystem::Update()
{
    // 今後フェード制御などを入れる場合に使用
}

//----------------------------------------------
// 終了処理
//----------------------------------------------
void AudioManagerSystem::Release()
{
    StopBGM();

    if (m_masterVoice)
    {
        m_masterVoice->DestroyVoice();
        m_masterVoice = nullptr;
    }

    m_xAudio2.Reset();

    m_bgmData.clear();
    m_seData.clear();

    std::cout << "[Audio] AudioManagerSystem 解放完了" << std::endl;
}

//----------------------------------------------
// WAV読み込み
//----------------------------------------------
bool AudioManagerSystem::LoadBGM(const std::string& name, const std::wstring& filePath)
{
    SoundData data;
    if (!LoadWaveFile(filePath, data)) return false;
    m_bgmData[name] = std::move(data);
    return true;
}

bool AudioManagerSystem::LoadSE(const std::string& name, const std::wstring& filePath)
{
    SoundData data;
    if (!LoadWaveFile(filePath, data)) return false;
    m_seData[name] = std::move(data);
    return true;
}

//----------------------------------------------
// BGM再生
//----------------------------------------------
void AudioManagerSystem::PlayBGM(const std::string& name, bool loop)
{
    StopBGM();

    auto it = m_bgmData.find(name);
    if (it == m_bgmData.end()) return;

    const auto& data = it->second;

    HRESULT hr = m_xAudio2->CreateSourceVoice(&m_bgmVoice, &data.format);
    if (FAILED(hr)) return;

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = static_cast<UINT32>(data.buffer.size());
    buffer.pAudioData = data.buffer.data();
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    m_bgmVoice->SubmitSourceBuffer(&buffer);
    m_bgmVoice->SetVolume(m_bgmVolume);
    m_bgmVoice->Start(0);
}

//----------------------------------------------
// BGM停止
//----------------------------------------------
void AudioManagerSystem::StopBGM()
{
    if (m_bgmVoice)
    {
        m_bgmVoice->Stop(0);
        m_bgmVoice->DestroyVoice();
        m_bgmVoice = nullptr;
    }
}

//----------------------------------------------
// 効果音再生
//----------------------------------------------
void AudioManagerSystem::PlaySE(const std::string& name)
{
    auto it = m_seData.find(name);
    if (it == m_seData.end()) return;

    const auto& data = it->second;

    IXAudio2SourceVoice* seVoice = nullptr;
    HRESULT hr = m_xAudio2->CreateSourceVoice(&seVoice, &data.format);
    if (FAILED(hr)) return;

    XAUDIO2_BUFFER buffer = {};
    buffer.AudioBytes = static_cast<UINT32>(data.buffer.size());
    buffer.pAudioData = data.buffer.data();
    buffer.LoopCount = 0;

    seVoice->SubmitSourceBuffer(&buffer);
    seVoice->SetVolume(m_seVolume);
    seVoice->Start(0);

    // 自動破棄スレッド
    std::thread([seVoice]()
        {
            XAUDIO2_VOICE_STATE state;
            do {
                seVoice->GetState(&state);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } while (state.BuffersQueued > 0);
            seVoice->DestroyVoice();
        }).detach();
}

//----------------------------------------------
// 音量設定
//----------------------------------------------
void AudioManagerSystem::SetBGMVolume(float volume)
{
    m_bgmVolume = volume;
    if (m_bgmVoice)
        m_bgmVoice->SetVolume(volume);
}

void AudioManagerSystem::SetSEVolume(float volume)
{
    m_seVolume = volume;
}