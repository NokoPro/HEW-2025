/*****************************************************************//**
 * @file   GameStateSystem.cpp
 * @brief  ゲームシーンの状態（カウントダウン/プレイ/ゴール後/ゲームオーバー）を管理するUpdateシステムの実装
 *
 * 本システムは単一の状態エンティティに `GameStateComponent` を付与し、
 * 経過時間や外部トリガ（ゴール/死亡）に応じて状態を遷移させます。
 * ゴール時は入力と水平移動を停止し、重力を維持して自然落下させます。
 *********************************************************************/
#include "GameStateSystem.h"
#include "System/TimeAttackManager.h"
#include "ECS/ECS.h"
#include "ECS/Components/Input/MovementIntentComponent.h"
#include "ECS/Components/Physics/Rigidbody2DComponent.h"

/**
 * @brief 状態保持用のエンティティを確保（未作成なら作成）
 * @param world ECSのWorld
 */
void GameStateSystem::ensureStateEntity(World& world)
{
    if (m_stateEntity == kInvalidEntity)
    {
        m_stateEntity = world.Create();
        world.Add<GameStateComponent>(m_stateEntity);
    }
}

/**
 * @brief シーン開始時の初期化。カウントダウン状態へ設定。
 * @param world ECSのWorld
 */
void GameStateSystem::Initialize(World& world)
{
    ensureStateEntity(world);
    auto& st = world.Get<GameStateComponent>(m_stateEntity);
    st.state = GamePlayState::Countdown;
    st.elapsedInStateSec = 0.0f;
    st.postGoalDelaySec = 1.5f;
}

/**
 * @brief 現在のゲームプレイ状態を取得
 * @param world ECSのWorld（状態エンティティを参照）
 * @return 現在の状態（未初期化時はCountdown）
 */
GamePlayState GameStateSystem::GetState(const World& world) const
{
    if (m_stateEntity != kInvalidEntity && world.Has<GameStateComponent>(m_stateEntity))
    {
        return world.Get<GameStateComponent>(m_stateEntity).state;
    }
    return GamePlayState::Countdown;
}

/**
 * @brief PostGoal（ゴール後演出）状態の猶予が終了したかを判定
 * @param world ECSのWorld
 * @return 猶予時間を過ぎていれば true
 */
bool GameStateSystem::IsPostGoalFinished(const World& world) const
{
    if (m_stateEntity != kInvalidEntity && world.Has<GameStateComponent>(m_stateEntity))
    {
        const auto& st = world.Get<GameStateComponent>(m_stateEntity);
        return st.state == GamePlayState::PostGoal && st.elapsedInStateSec >= st.postGoalDelaySec;
    }
    return false;
}

/**
 * @brief ゴール時トリガ。PostGoal状態へ遷移し、入力と水平移動を停止（落下は継続）。
 * @param world ECSのWorld
 */
void GameStateSystem::OnGoal(World& world)
{
    ensureStateEntity(world);
    auto& st = world.Get<GameStateComponent>(m_stateEntity);
    st.state = GamePlayState::PostGoal;
    st.elapsedInStateSec = 0.0f;

    // 空中ゴール時も自然落下させるため、横移動とジャンプ入力を停止し、重力は維持
    world.View<MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId e, MovementIntentComponent& intent, Rigidbody2DComponent& rb)
        {
            intent.moveX = 0.0f;
            intent.jump = false;
            intent.forceJumpRequested = false;
            intent.blinkRequested = false;
            intent.isBlinking = false;
            // 水平速度を止め、垂直はそのまま（重力により落下）
            rb.velocity.x = 0.0f;
            rb.useGravity = true;
        }
    );
}

/**
 * @brief 死亡時トリガ。GameOver状態へ遷移。
 * @param world ECSのWorld
 */
void GameStateSystem::OnDeath(World& world)
{
    ensureStateEntity(world);
    auto& st = world.Get<GameStateComponent>(m_stateEntity);
    st.state = GamePlayState::GameOver;
    st.elapsedInStateSec = 0.0f;
}

/**
 * @brief 状態のフレーム更新。経過時間を進め、必要に応じて自動遷移を行う。
 * @param world ECSのWorld
 * @param dt フレーム経過時間（秒）
 */
void GameStateSystem::Update(World& world, float dt)
{
    ensureStateEntity(world);
    auto& st = world.Get<GameStateComponent>(m_stateEntity);
    st.elapsedInStateSec += dt;

    switch (st.state)
    {
    case GamePlayState::Countdown:
        // 計測のRunning開始と同期してプレイ開始
        if (TimeAttackManager::Get().GetState() == TimeAttackManager::State::Running)
        {
            st.state = GamePlayState::Running;
            st.elapsedInStateSec = 0.0f;
        }
        break;
    case GamePlayState::Running:
        // プレイ中は外部トリガで遷移
        break;
    case GamePlayState::PostGoal:
        // ゴール後は猶予のみ進める（遷移はScene側）
        break;
    case GamePlayState::GameOver:
        // ゲームオーバー演出（遷移はScene/DeathSystem側）
        break;
    }
}
