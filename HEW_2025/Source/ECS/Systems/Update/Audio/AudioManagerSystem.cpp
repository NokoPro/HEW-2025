#include "AudioManagerSystem.h"
#include <iostream>
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <vector>

#pragma comment(lib, "winmm.lib")

void Msg(const std::string& s)
{
    //MessageBoxA(nullptr, s.c_str(), "DEBUG", MB_OK);
}

bool AudioManagerSystem::Initialize()
{
    HRESULT hr = XAudio2Create(&xAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) {
        std::cerr << "XAudio2 初期化失敗\n";
        return false;
    }

    hr = xAudio->CreateMasteringVoice(&masterVoice);
    if (FAILED(hr)) {
        std::cerr << "MasteringVoice 作成失敗\n";
        return false;
    }

    return true;
}

AudioManagerSystem::~AudioManagerSystem()
{
    for (auto& p : sounds) {
        delete[] p.second.audioData;
    }
    if (bgmVoice) bgmVoice->DestroyVoice();
    if (masterVoice) masterVoice->DestroyVoice();
    if (xAudio) xAudio->Release();
}

bool AudioManagerSystem::LoadBGM(const std::string& name, const std::wstring& filepath)
{
    HMMIO hmmio = mmioOpenW(const_cast<LPWSTR>(filepath.c_str()), nullptr, MMIO_READ | MMIO_ALLOCBUF);
    if (!hmmio) {
        std::wcerr << L"[ERROR] WAV ファイル開けない: " << filepath << std::endl;
        return false;
    }

    MMCKINFO ck{};
    ck.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, &ck, nullptr, MMIO_FINDRIFF) != MMSYSERR_NOERROR) {
        std::wcerr << L"[ERROR] RIFF チャンク見つからない: " << filepath << std::endl;
        mmioClose(hmmio, 0);
        return false;
    }

    MMCKINFO fmtck{};
    fmtck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if (mmioDescend(hmmio, &fmtck, &ck, 0) != MMSYSERR_NOERROR) {
        std::wcerr << L"[ERROR] fmt チャンク見つからない: " << filepath << std::endl;
        mmioClose(hmmio, 0);
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

    std::cout << "[DEBUG] WAV 読み込み成功: " << name
        << " " << wfx.nChannels << "ch "
        << wfx.nSamplesPerSec << "Hz "
        << wfx.wBitsPerSample << "bit\n";

    return true;
}

void AudioManagerSystem::PlayBGM(const std::string& name, bool loop)
{
    Msg("PlayBGM START");

    if (bgmVoice) {
        Msg("Destroy old voice");
        bgmVoice->DestroyVoice();
        bgmVoice = nullptr;
    }

    auto it = sounds.find(name);
    if (it == sounds.end()) {
        Msg("ERROR: sound not found: " + name);
        return;
    }

    auto& data = it->second;
    data.buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    HRESULT hr = xAudio->CreateSourceVoice(&bgmVoice, &data.wfx);
    if (FAILED(hr)) {
        Msg("ERROR: CreateSourceVoice FAILED: 0x" + std::to_string(hr));
        return;
    }

    hr = bgmVoice->SubmitSourceBuffer(&data.buffer);
    if (FAILED(hr)) {
        Msg("ERROR: SubmitSourceBuffer FAILED: 0x" + std::to_string(hr));
        return;
    }

    hr = bgmVoice->Start(0);
    if (FAILED(hr)) {
        Msg("ERROR: Start FAILED: 0x" + std::to_string(hr));
        return;
    }

    Msg("PLAY STARTED SUCCESSFULLY");
}

void AudioManagerSystem::StopBGM()
{
    if (bgmVoice) bgmVoice->Stop();
}

void AudioManagerSystem::SetBGMVolume(float vol)
{
    if (bgmVoice)
        bgmVoice->SetVolume(vol);
}
