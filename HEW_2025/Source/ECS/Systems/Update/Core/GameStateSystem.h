/**********************************************************************************************
 * @file      GameStateSystem.h
 * @brief     GameStateComponent を管理する Update システムの宣言
 *
 * ゲームシーンの状態（Countdown/Running/PostGoal/GameOver）を一元管理し、
 * 経過時間の更新や外部トリガに応じた状態遷移を行います。PostGoal中は入力を停止し、
 * 重力による自然落下のみ許可します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 *
 * =============================================================================================
 *  Progress Log  - 進捗ログ
 * ---------------------------------------------------------------------------------------------
 *  [ @date 2025/12/5 ]
 *    - [◎] シーン状態管理の基本実装（カウントダウン、プレイ、ゴール後、ゲームオーバー）
 *    - [◎] ゴール時の入力停止と落下継続処理（OnGoal）
 *
 **********************************************************************************************/
#pragma once
#include "ECS/World.h"
#include "ECS/Components/Core/GameStateComponent.h"
#include "ECS/Systems/IUpdateSystem.h"

/**
 * @class GameStateSystem
 * @brief シーンのゲーム状態を管理する Update システム
 *
 * - 状態エンティティに `GameStateComponent` を保持
 * - `Update()` で経過時間を進め、必要な自動遷移を行う
 * - `OnGoal()`／`OnDeath()` の外部イベントで状態遷移をトリガ
 */
class GameStateSystem : public IUpdateSystem
{
public:
    /**
     * @brief 初期化。状態エンティティの準備と初期状態（Countdown）の設定
     * @param world ECS の World
     */
    void Initialize(World& world);

    /**
     * @brief フレーム更新。経過時間を進め、必要な自動遷移を行う
     * @param world ECS の World
     * @param dt 経過時間（秒）
     */
    void Update(World& world, float dt) override;

    /**
     * @brief ゴール到達イベント。PostGoalへ遷移し、入力停止＋落下継続をセット
     * @param world ECS の World
     */
    void OnGoal(World& world);

    /**
     * @brief 死亡イベント。GameOverへ遷移
     * @param world ECS の World
     */
    void OnDeath(World& world);

    /**
     * @brief 現在状態の取得
     * @param world ECS の World
     * @return `GamePlayState`（未初期化時は Countdown）
     */
    GamePlayState GetState(const World& world) const;

    /**
     * @brief PostGoal猶予が終了したかの判定
     * @param world ECS の World
     * @return 猶予時間経過なら true
     */
    bool IsPostGoalFinished(const World& world) const;

private:
    /**
     * @brief 状態保持用エンティティ（単一）のID
     */
    EntityId m_stateEntity{};

    /**
     * @brief 状態エンティティを確保（未作成なら作成）
     * @param world ECS の World
     */
    void ensureStateEntity(World& world);
};
