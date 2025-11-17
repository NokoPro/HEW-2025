#include "AudioManagerSystem.h"
#include <iostream>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <vector>
#include <thread>   
#include <chrono>   

#pragma comment(lib, "winmm.lib")

void Msg(const std::string& s)
{
    MessageBoxA(nullptr, s.c_str(), "DEBUG", MB_OK);
}

//＝＝＝＝＝static＝＝＝＝＝
IXAudio2* AudioManager::xAudio = nullptr;
IXAudio2MasteringVoice* AudioManager::masterVoice = nullptr;
IXAudio2SourceVoice* AudioManager::bgmVoice = nullptr;

std::map<std::string, AudioManager::SoundData> AudioManager::sounds ;
std::map<std::string, AudioManager::SEVoice> AudioManager::seVoices ;
//＝＝＝＝＝＝＝＝＝＝＝＝＝


bool AudioManager::Initialize()
{

    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr))
    {
        std::cerr << "XAudio2 初期化失敗\n";
        return false;
    }
    hr = xAudio->CreateMasteringVoice(&masterVoice);
    if (FAILED(hr))
    {
        std::cerr << "MasteringVoice 作成失敗\n";
        return false;
    }

    return true;
}

bool AudioManager::LoadAudio(const std::string& name, const std::wstring filepath)
{
    HMMIO hmmio = mmioOpenW(const_cast<LPWSTR>(filepath.c_str()), nullptr, MMIO_READ | MMIO_ALLOCBUF);
    if (!hmmio) {
        std::wcerr << L"[ERROR] WAV ファイル開けない: " << filepath << std::endl;
        return false;
    }
    MMCKINFO ck{};
    ck.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, &ck, nullptr, MMIO_FINDRIFF) != MMSYSERR_NOERROR)
    {
        std::wcerr << L"[ERROR] RIFF チャンク見つからない：" << filepath << std::endl;
        mmioClose(hmmio, 0);
        return false;
    }
    MMCKINFO fmtck{};
    fmtck.ckid = mmioFOURCC('f', 'm', 't', 'c');
    if (mmioDescend(hmmio, &fmtck, &ck, 0) != MMSYSERR_NOERROR) {
        std::wcerr << L"[ERROR] fmt チャンクが見つからない：" << filepath << std::endl;
        return false;
    }




    WAVEFORMATEX wfx{};
    if (mmioRead(hmmio, reinterpret_cast<HPSTR>(&wfx), sizeof(WAVEFORMATEX)) != sizeof(WAVEFORMATEX)) {
        std::wcerr << L"[ERROR] WAVEFORMATEX 読み込み失敗\n";
        mmioClose(hmmio, 0);
        return false;
    }

    

    mmioAscend(hmmio, &fmtck, 0);

    MMCKINFO datack{};
    datack.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmio, &datack, &ck, MMIO_FINDCHUNK) != MMSYSERR_NOERROR) {
        std::wcerr << L"[ERROR] data チャンク見つからない\n";
        mmioClose(hmmio, 0);
        return false;
    }
    std::vector<BYTE> buffer(datack.cksize);
    if (mmioRead(hmmio, reinterpret_cast<HPSTR>(buffer.data()), datack.cksize) != datack.cksize) {
        std::wcerr << L"[ERROR] WAV データ読み込み失敗\n";
        mmioClose(hmmio, 0);
        return false;
    }

    mmioClose(hmmio, 0);

    SoundData data;
    data.wfx = wfx;
    data.audioData = new BYTE[buffer.size()];
    memcpy(data.audioData, buffer.data(), buffer.size());

    data.buffer.AudioBytes = buffer.size();
    data.buffer.pAudioData = data.audioData;
    data.buffer.Flags = XAUDIO2_END_OF_STREAM;
    data.buffer.LoopCount = 0;

    sounds[name] = data;

    return true;
}

void AudioManager::PlayBGM(const std::string& name, bool loop)
{
    if (bgmVoice)
    {
        bgmVoice->DestroyVoice();
        bgmVoice = nullptr;
    }

    auto it = sounds.find(name);
    if (it == sounds.end()) return;
    auto& data = it->second;
    data.buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    if (FAILED(xAudio->CreateSourceVoice(&bgmVoice, &data.wfx))) return;
    if (FAILED(bgmVoice->SubmitSourceBuffer(&data.buffer))) return;

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

void AudioManager::PlaySE(const std::string& name, float volume)
{
    auto it = sounds.find(name);
    if (it == sounds.end()) return;

    auto& seState = seVoices[name];
    if (seState.playing)return;

    auto& data = it->second;

    IXAudio2SourceVoice* seVoice = nullptr;
    if (FAILED(xAudio->CreateSourceVoice(&seVoice, &data.wfx))) return;

    seVoice->SetVolume(volume);
    seVoice->SubmitSourceBuffer(&data.buffer);
    seVoice->Start(0);

    seState.voice = seVoice;
    seState.playing = true;

    std::thread([&seState]()
        {
            XAUDIO2_VOICE_STATE state{};
            do {
                seState.voice->GetState(&state);
                Sleep(1);
            
            } while (state.BuffersQueued > 0);

            seState.voice->DestroyVoice();
            seState.voice = nullptr;
            seState.playing = false;

        }).detach();
}
