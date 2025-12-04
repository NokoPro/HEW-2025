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

class DeathZoneSystem : public IUpdateSystem
{
public:
    DeathZoneSystem(Collision2DSystem* colSys) : m_colSys(colSys) {}
    void Update(class World& world, float dt) override;
	void GameOverUpdate();
    // 外部(チート/リトライ)用インターフェイス
    void ForceDeath() { m_triggered = true; }
    void ResetDeathFlag() { m_triggered = false; }
    bool IsDead() const { return m_triggered; }
    
    /**
     * @brief デスゾーンの上昇速度を設定する
     * @param speed 毎秒の上昇量(World単位/sec)
     */
    void SetRiseSpeed(float speed) { m_riseSpeed = speed; }
    
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
   
};