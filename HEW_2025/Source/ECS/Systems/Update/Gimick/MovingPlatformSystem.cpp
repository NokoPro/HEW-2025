/*****************************************************************//**
 * @file   MovingPlatformSystem.cpp
 * @brief  移動床を制御するSystem
 * 
 * @author 浅野勇生
 * @date   2025/11/14
 *********************************************************************/
#include "MovingPlatformSystem.h"
#include <DirectXMath.h>
using namespace DirectX;

// 線形補間
static inline XMFLOAT2 Lerp(const XMFLOAT2& a, const XMFLOAT2& b, float t)
{
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

// 2D距離計算
static inline float Distance(const XMFLOAT2& a, const XMFLOAT2& b)
{
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

// 更新処理
void MovingPlatformSystem::Update(World& world, float dt)
{
	// 移動足場を更新
    world.View<TransformComponent, MovingPlatformComponent>(
        [&](EntityId, TransformComponent& tr, MovingPlatformComponent& mp)
        {
            // 待機中
            if (mp.waitTimer > 0.0f)
            {
				// 続きを待つ
                mp.waitTimer -= dt;
                return;
            }

			// 開始点と終了点が同じなら何もしない
            const float dist = Distance(mp.start, mp.end);
            if (dist <= 1e-4f)
                return;

            // t を速度に応じて更新（距離で正規化）
            const float dtNorm = (mp.speed / dist) * dt * (float)mp.dir;
            mp.t += dtNorm;

            // 端に到達したら反転＆待機
            if (mp.t >= 1.0f)
            {
                mp.t = 1.0f;
                mp.dir = -1;
                mp.waitTimer = mp.waitTime;
            }
            else if (mp.t <= 0.0f)
            {
                mp.t = 0.0f;
                mp.dir = +1;
                mp.waitTimer = mp.waitTime;
            }

			// 位置更新
            XMFLOAT2 p = Lerp(mp.start, mp.end, mp.t);
            tr.position.x = p.x;
            tr.position.y = p.y;
        }
    );
}