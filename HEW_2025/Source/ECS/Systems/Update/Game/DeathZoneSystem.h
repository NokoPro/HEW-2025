/*********************************************************************/
/* @file   DeathZoneSystem.h
 * @brief  死亡ゾーンシステム
 * * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#pragma once
#include "ECS/Systems/IUpdateSystem.h"
#include "ECS/World.h"
#include "ECS/Systems/Update/Physics/Collision2DSystem.h"
#include "System/GameCommon.h" // Difficulty

class DeathZoneSystem : public IUpdateSystem
{
public:
    DeathZoneSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(class World& world, float dt) override;
	void GameOverUpdate(World& world);
    // 外部(チート/リトライ)用インターフェイス
    void ForceDeath() { m_triggered = true; }
    void ResetDeathFlag() { m_triggered = false; }
    bool IsDead() const { return m_triggered; }
    
    /**
     * @brief デスゾーンの上昇速度を設定する
     * @param speed 毎秒の上昇量(World単位/sec)
     */
    void SetRiseSpeed(float speed) { m_riseSpeed = speed; }

    // 遅延シーン遷移: GameScene 側のフレーム最後で拾って実行する
    bool HasPendingSceneChange() const { return m_requestSceneChange; }
    bool PendingContinue() const { return m_sceneSwitch; }
    int  PendingStageNo() const { return m_pendingStageNo; }
    Difficulty PendingDifficulty() const { return m_pendingDifficulty; }
    void ClearPendingSceneChange() { m_requestSceneChange = false; }
    
private:
    Collision2DSystem* m_colSys = nullptr;  /// 死亡ゾーンの当たり判定システム
    bool m_triggered = false;               /// 死亡ゾーンに触れたかどうか
    float m_riseSpeed = 1.0f;               /// 上昇速度（デフォルト値）

    //追加 堤翔
    const float m_warningDistance = 25.0f;    ///  デスゾーンとプレイヤーの間の距離
    bool m_warningActive = false;              ///  警告処理が実行されているか
    float m_dis;                               ///  計算格納用
    float m_oldDis = m_warningDistance * m_warningDistance;                            ///  古い計算格納用
    bool m_sceneSwitch = true;             /// シーン切り替え

    // Death後の上端追従移動用
    bool m_moveToScreenTop = false;        /// デス後、上端を画面上端へ移動するか
    float m_moveSpeed = 10.0f;             /// 線形移動速度(World単位/秒)
    float m_cameraTopOffset = 20.0f;       /// 画面上端の推定オフセット(カメラ中心からの+Y)

    // 画面埋め演出の時間を統一するための情報
    float m_fillDurationSec = 1.2f;        /// 画面が埋まるまでの所要時間（秒）
    float m_fillRemainSec = 0.0f;          /// 残り時間（秒）
    float m_targetScreenTopY = 0.0f;       /// デス時の画面上端Y（ワールド座標）
    float m_startTopY = 0.0f;              /// デス時のデスゾーン上端Y

    // デス後、上がり始めまでのバッファ時間（生存キャラ演出用）
    float m_postDeathDelaySec = 0.8f;      /// 上昇開始までの遅延時間（秒）
    float m_delayRemainSec = 0.0f;         /// 遅延の残り時間（秒）
    bool  m_inDelay = false;               /// 遅延中フラグ

    // 遅延実行するシーン遷移の情報
    bool m_requestSceneChange = false;
    int  m_pendingStageNo = 0;
    Difficulty m_pendingDifficulty = Difficulty::Normal;
};