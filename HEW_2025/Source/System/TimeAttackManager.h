/**********************************************************************************************
 * @file      TimeAttackManager.h
 * @brief     タイムアタック用の計測とベスト記録管理を行うシングルトン
 *
 * @author    浅野勇生
 * @date      2025/11/23
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/11/23 ]
 * 
 *    - [◎] ベースを実装
 *    - [] ファイルの書き込む情報や読み込む情報を変更
 *
 **********************************************************************************************/
#pragma once
#include <string>
#include <cstdint>

/**
 * @class TimeAttackManager
 * @brief タイムアタック用の計測とベスト記録管理を行うシングルトン。
 * @details
 *  - 高精度タイマー(QueryPerformanceCounter)でカウントダウンおよび走行時間を計測。
 *  - 状態遷移: Ready -> Countdown -> Running -> (Cleared | Dead) -> Reset。
 *  - クリア時にベストタイムを自動更新 & 保存フラグ (m_dirty) セット。
 *  - ファイルI/Oは単純なバイナリ浮動小数 (ロード/セーブ) のみ。
 *  - 将来的拡張: セグメント分割 / ゴースト / リトライ統計 などを追加しやすい最小構成。
 */
class TimeAttackManager
{
public:
    /**
     * @enum State
     * @brief タイムアタックの進行状態。
     */
    enum class State
    {
        Ready,      /**< 待機 (カウントダウン未開始) */
        Countdown,  /**< カウントダウン中 */
        Running,    /**< 計測中 */
        Cleared,    /**< クリア (記録確定) */
        Dead        /**< ゲームオーバー */
    };

    /**
     * @brief シングルトンインスタンス取得。
     * @return TimeAttackManager 参照。
     */
    static TimeAttackManager& Get()
    {
        static TimeAttackManager s_instance; return s_instance;
    }

    /** @name 制御API */
    ///@{
    /** @brief 状態と内部カウンタを初期化 (ベストは保持)。 */
    void Reset();
    /** @brief カウントダウンを開始。seconds<=0 の場合は即 Running へ。 */
    void StartCountdown(float seconds);
    /** @brief 計測を強制開始 (外部イベント用)。 */
    void StartRun();
    /** @brief クリア通知。Running 中のみ Cleared へ遷移しベスト更新判定。 */
    void NotifyClear();
    /** @brief 死亡通知。Running/Countdown 中なら Dead へ遷移。 */
    void NotifyDeath();
    /** @brief 毎フレーム更新。内部でdeltaを測り状態に応じて加算/減算。 */
    void Update();
    ///@}

    /** @name 取得API */
    ///@{
    State GetState() const { return m_state; }
    float GetElapsed() const { return m_elapsed; }
    float GetCountdownRemaining() const { return (m_state == State::Countdown) ? m_countdownRemaining : 0.0f; }
    float GetBestTime() const { return m_bestTime; }
    bool  HasBestTime() const { return m_bestTime > 0.0f; }
    ///@}

    /**
     * @brief 秒を mm:ss.mmm 形式にフォーマット。
     * @param seconds 経過秒。
     * @return 整形文字列 (例: "01:23.456")。
     */
    static std::string FormatTime(float seconds);

    /** @name 永続化 */
    ///@{
    /** @brief ベストタイムをロード。存在しない場合は無視。 */
    void LoadRecord(const std::string& path);
    /** @brief ベストタイムを保存。ベスト更新時(m_dirty=true)のみ書き込み。 */
    void SaveRecord(const std::string& path);
    ///@}

private:
    /** @brief コンストラクタ。シングルトン隠蔽。 */
    TimeAttackManager();

    //=== 高精度時間計測 ===//
    double  m_perfFreq     = 0.0;   /**< QueryPerformanceFrequency の値 */
    int64_t m_prevCounter  = 0;     /**< 前フレームのカウンタ */

    //=== 状態 & 計測値 ===//
    State m_state               = State::Ready; /**< 現在の状態 */
    float m_countdownTotal      = 0.0f;         /**< カウントダウン総時間 */
    float m_countdownRemaining  = 0.0f;         /**< カウントダウン残り */
    float m_elapsed             = 0.0f;         /**< Running 中の経過時間 */
    float m_bestTime            = 0.0f;         /**< ベスト記録 (0=未登録) */
    bool  m_dirty               = false;        /**< ベスト更新フラグ */

    /** @brief フレーム間Δt取得。 */
    float sampleDelta();
};
