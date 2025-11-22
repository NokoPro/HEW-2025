/*****************************************************************//**
 * @file   DebugSettings.h
 * @brief  デバッグ用ランタイム設定定義
 * 
 * @author 浅野勇生
 * @date   2025/11/16
 *********************************************************************/
#pragma once

/**
 * @brief デバッグ用のランタイム設定を一元管理するシングルトン
 * @details
 * - 各Update/Renderシステムは必要に応じて本設定を参照して挙動を切り替えます。
 * - ImGuiのデバッグウィンドウからも編集できます（`imguiEnabled` がONのとき）。
 * - 項目を追加したい場合は、ここにフィールドを追加し、必要なシステムで参照/適用、ImGui側にウィジェットを追加してください。
 */
struct DebugSettings
{
    /** @brief マグマ（DeathZone）の上昇/当たりの有効・無効 */
    bool magmaEnabled = true;

    /** @brief 地上判定なしでもジャンプできるチート（無限ジャンプ） */
    bool infiniteJump = false;

    /** @brief デバッグUI（ImGui）の表示ON/OFF（F5で切り替え） */
    bool imguiEnabled = false;

    /** @brief マグマの上昇速度スケール（1.0で標準、0.0で停止、>1.0で加速） */
    float magmaSpeedScale = 1.0f;

    /** @brief ゴッドモード（被ダメージ無効化、Deathゾーン無効など） */
    bool godMode = false;

    /** @brief プレイヤーの移動速度スケール（1.0で標準、0.0で停止、>1.0で加速） */
    float playerSpeed = 1.0f;

    /** @brief 当たり判定の可視化を表示する（F2と連動） */
    bool showCollision = true;

    /** @brief FPS表示ON/OFF */
    bool fpsEnabled = true;
    /** @brief 最新のFPS値 */
    float fpsValue = 0.0f;

    // --- Game Timer ---
    bool  gameTimerRunning = false;   // 計測中か
    bool  gameCleared      = false;   // クリア済みか
    bool  gameDead         = false;   // 死亡(ゲームオーバー)か
    float gameElapsedSeconds = 0.0f;  // 経過秒

    /**
     * @brief シングルトン取得
     * @return DebugSettings の唯一のインスタンス参照
     */
    static DebugSettings& Get()
    {
        static DebugSettings s_instance{};
        return s_instance;
    }

private:
    DebugSettings() = default;
};
