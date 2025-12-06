/**********************************************************************************************
 * @file      PlayerStateComponent.h
 * @brief     物理・ゲームルールを踏まえた「現在の状態」を保持するコンポーネント
 *
 * @author    浅野勇生
 * @date      2025/12/2
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/2 ]
 * 
 *    - [◎] PlayerStateComponent を実装
 *    - [◎] enum class PlayerLocomotionState を実装
 *
 **********************************************************************************************/
#pragma once

 // プレイヤーの移動状態
enum class PlayerLocomotionState
{
    None,       // アニメ停止用（今のIdle代わり）
    Idle,       // Idleクリップを用意したらここを使う
    Walk,
    Jump,
    Fall,
    Land,
    Dash,
    Blink,
    GoalPose,
};

// プレイヤーの向き
enum class PlayerFacingState
{
    Right,
    Left,
    Forward,    // 正面（ゴール演出など）
};

struct PlayerStateComponent
{
    // 現在の状態
    PlayerLocomotionState m_locomotion = PlayerLocomotionState::None;
    PlayerFacingState     m_facing = PlayerFacingState::Right;

    // 前フレームの状態（遷移検知用）
    PlayerLocomotionState m_prevLocomotion = PlayerLocomotionState::None;
    PlayerFacingState     m_prevFacing = PlayerFacingState::Right;

    // -----------------------------------------------------------------------------
    // 相手コマンド実行フラグ（1フレーム限定）
    //  - 相手の R / L ボタンによって「このプレイヤー」が強制ジャンプ／ブリンク
    //    させられたフレームだけ true にする
    // -----------------------------------------------------------------------------
    bool m_remoteCommandJumpThisFrame = false;   ///< 強制ジャンプが実行されたフレーム
    bool m_remoteCommandBlinkThisFrame = false;  ///< 強制ブリンクが実行されたフレーム
};
