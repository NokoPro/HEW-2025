/**********************************************************************************************
 * @file      GameStateComponent.h
 * @brief     ゲームシーンの状態を保持するコンポーネント
 *
 * シーン全体の進行状態（カウントダウン／プレイ中／ゴール後演出／ゲームオーバー）を
 * 単一エンティティに紐づけて管理します。状態ごとの経過時間や、ゴール後にリザルトへ
 * 遷移するまでの猶予秒数を保持します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#pragma once
#include <cstdint>

/**
 * @enum GamePlayState
 * @brief ゲーム進行の状態を表す列挙
 * - Countdown: カウントダウン演出中（プレイ系の更新は限定）
 * - Running: 通常プレイ中
 * - PostGoal: ゴール後の演出猶予中（入力停止、落下継続など）
 * - GameOver: ゲームオーバー演出中
 */
enum class GamePlayState : uint8_t
{
    Countdown,
    Running,
    PostGoal,
    GameOver
};

/**
 * @struct GameStateComponent
 * @brief シーンのゲーム状態と、その補助情報を保持
 */
struct GameStateComponent
{
    /** 現在のゲーム状態 */
    GamePlayState state{ GamePlayState::Countdown };

    /** 現在の状態に入ってからの経過時間（秒） */
    float elapsedInStateSec{ 0.0f };

    /**
     * ゴール後（PostGoal）からリザルトへ遷移するまでの猶予時間（秒）
     * PostGoal中は演出のみ進行し、猶予経過後にシーン側で遷移します。
     */
    float postGoalDelaySec{ 1.5f };

    /** ゴール到達時刻（必要に応じて利用） */
    float goalReachedTimeSec{ 0.0f };
};
