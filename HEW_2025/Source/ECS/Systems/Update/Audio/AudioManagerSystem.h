/**********************************************************************************************
 * @file      AudioManagerSystem.h
 * @brief     オーディオを管理するシステム
 *
 * - AssetManager で CSV 登録 (type="audio") されたエイリアスからのロード対応
 * - GetAudio(alias) → AudioClip.path を使って LoadAudioAlias() で WAV 読み込み
 * - メモリ解放用 Shutdown() を追加
 * - SoundData.audioData のリーク対策
 * - PlaySE 引数タイポ修正
 **********************************************************************************************/
#pragma once
#include <string>
#include <xaudio2.h>
#include <map>



class AudioManager
{
public:
    static bool Initialize();
    static void Shutdown();                // 追加: 解放処理

    // 直接パス指定ロード (既存)
    static bool LoadAudio(const std::string& name,const std::wstring filepath);
    // エイリアス指定ロード (AssetManager 経由)
    static bool LoadAudioAlias(const std::string& alias); // alias をそのまま name として登録

    static void PlayBGM(const std::string& name, bool loop);
    static void StopBGM();
    static void SetBGMVolume(float vol);
    static void PlaySE(const std::string& name, float volume); // typo 修正

private:
    AudioManager() = delete; //インスタンス禁止
    ~AudioManager() = delete;

    static IXAudio2* xAudio;
    static IXAudio2MasteringVoice* masterVoice;
    static IXAudio2SourceVoice* bgmVoice;

    struct SoundData {
        WAVEFORMATEX wfx{};
        XAUDIO2_BUFFER buffer{};
        BYTE* audioData = nullptr; // 動的確保 → Shutdown で解放
    };
    static std::map<std::string, SoundData> sounds;

    struct SEVoice {
        IXAudio2SourceVoice* voice = nullptr;
        bool playing = false;
    };

    static std::map<std::string, SEVoice> seVoices;
};
