/**********************************************************************************************
 * @file      AudioManagerSystem.cpp
 * @brief     オーディオを管理するシステム
 *
 * @author    浅野勇生
 * @date      2025/11/24
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/24 ]
 * 
 *    - [◎] 〇〇を実装
 *    - [] △△のバグ調査中
 *
 **********************************************************************************************/
#include "AudioManagerSystem.h"
#include "System/AssetManager.h"

#include <iostream>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <vector>
#include <thread>

#pragma comment(lib, "winmm.lib")

IXAudio2* AudioManager::xAudio = nullptr;
IXAudio2MasteringVoice* AudioManager::masterVoice = nullptr;
IXAudio2SourceVoice* AudioManager::bgmVoice = nullptr;

std::map<std::string, AudioManager::SoundData> AudioManager::sounds;
std::map<std::string, AudioManager::SEVoice> AudioManager::seVoices;

bool AudioManager::Initialize()
{
    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) { std::cerr << "XAudio2 init failed\n"; return false; }
    hr = xAudio->CreateMasteringVoice(&masterVoice);
    if (FAILED(hr)) { std::cerr << "MasteringVoice failed\n"; return false; }
    return true;
}

void AudioManager::Shutdown()
{
    if (bgmVoice) { bgmVoice->DestroyVoice(); bgmVoice = nullptr; }

    for (auto& kv : seVoices)
    {
        if (kv.second.voice) kv.second.voice->DestroyVoice();
    }
    seVoices.clear();

    for (auto& kv : sounds)
    {
        delete[] kv.second.audioData;
        kv.second.audioData = nullptr;
    }
    sounds.clear();

    if (masterVoice) masterVoice->DestroyVoice();
    if (xAudio) xAudio->Release();
}

//-----------------------------------------
// WAV load (direct path, with cache check)
//-----------------------------------------
bool AudioManager::LoadAudio(const std::string& name, const std::wstring filepath)
{
    // ==== キャッシュ済みならスキップ ====
    if (sounds.find(name) != sounds.end())
        return true;

    HMMIO hmmio = mmioOpenW(const_cast<LPWSTR>(filepath.c_str()), nullptr,
        MMIO_READ | MMIO_ALLOCBUF);
    if (!hmmio) { std::wcerr << L"[ERROR] open WAV: " << filepath << std::endl; return false; }

    MMCKINFO ck{}; ck.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, &ck, nullptr, MMIO_FINDRIFF) != MMSYSERR_NOERROR)
    {
        mmioClose(hmmio, 0); return false;
    }

    MMCKINFO fmtck{}; fmtck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &fmtck, &ck, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
    {
        mmioClose(hmmio, 0); return false;
    }

    WAVEFORMATEX wfx{};
    mmioRead(hmmio, reinterpret_cast<HPSTR>(&wfx), sizeof(WAVEFORMATEX));
    mmioAscend(hmmio, &fmtck, 0);

    MMCKINFO datack{}; datack.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &datack, &ck, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
    {
        mmioClose(hmmio, 0); return false;
    }

    std::vector<BYTE> buffer(datack.cksize);
    mmioRead(hmmio, reinterpret_cast<HPSTR>(buffer.data()), datack.cksize);
    mmioClose(hmmio, 0);

    SoundData data;
    data.wfx = wfx;
    data.audioData = new BYTE[buffer.size()];
    memcpy(data.audioData, buffer.data(), buffer.size());

    data.buffer.AudioBytes = (UINT32)buffer.size();
    data.buffer.pAudioData = data.audioData;
    data.buffer.Flags = XAUDIO2_END_OF_STREAM;
    data.buffer.LoopCount = 0;

    sounds[name] = data;
    return true;
}

//-----------------------------------------
// Load by alias (CSV → path → WAV load)
//-----------------------------------------
bool AudioManager::LoadAudioAlias(const std::string& alias)
{
    // ---- すでにキャッシュ済みなら OK ----
    if (sounds.find(alias) != sounds.end())
        return true;

    auto clipHandle = AssetManager::GetAudio(alias);
    if (!clipHandle) return false;

    const std::string& path = clipHandle->path;
    if (path.empty()) return false;

    std::wstring wpath(path.begin(), path.end());
    return LoadAudio(alias, wpath);
}

//-----------------------------------------
// Play BGM
//-----------------------------------------
void AudioManager::PlayBGM(const std::string& name, bool loop)
{
    if (bgmVoice) { bgmVoice->DestroyVoice(); bgmVoice = nullptr; }

    auto it = sounds.find(name);
    if (it == sounds.end()) return;
    auto& d = it->second;

    d.buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    if (FAILED(xAudio->CreateSourceVoice(&bgmVoice, &d.wfx))) return;
    bgmVoice->SubmitSourceBuffer(&d.buffer);
    bgmVoice->Start(0);
}

void AudioManager::StopBGM()
{
    if (bgmVoice) bgmVoice->Stop();
}

void AudioManager::SetBGMVolume(float vol)
{
    if (bgmVoice) bgmVoice->SetVolume(vol);
}

//-----------------------------------------
// Play SE
//-----------------------------------------
void AudioManager::PlaySE(const std::string& name, float volume)
{
    auto it = sounds.find(name);
    if (it == sounds.end()) return;

    auto& data = it->second;
    auto& state = seVoices[name];

    if (state.playing) return;

    IXAudio2SourceVoice* voice = nullptr;
    if (FAILED(xAudio->CreateSourceVoice(&voice, &data.wfx))) return;

    voice->SetVolume(volume);
    voice->SubmitSourceBuffer(&data.buffer);
    voice->Start(0);

    state.voice = voice;
    state.playing = true;

    std::thread([&state]() {
        XAUDIO2_VOICE_STATE s{};
        do {
            state.voice->GetState(&s);
            Sleep(1);
        } while (s.BuffersQueued > 0);

        state.voice->DestroyVoice();
        state.voice = nullptr;
        state.playing = false;
        }).detach();
}