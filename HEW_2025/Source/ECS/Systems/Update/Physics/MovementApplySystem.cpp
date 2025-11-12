#include "MovementApplySystem.h"
#include <algorithm>  // std::clamp

void MovementApplySystem::Update(World& world, float dt)
{
    // Intent + Rigidbody を持ってるやつだけ対象
    world.View<MovementIntentComponent, Rigidbody2DComponent>(
        [&](EntityId e,
            MovementIntentComponent& intent,
            Rigidbody2DComponent& rb)
        {
            // 1. いまこのキャラが地面にいるかどうかで横移動の効き方を変える
            //    rb.onGround は前のフレームの衝突でセットされる想定
            const bool onGround = rb.onGround;

            // 2. このエンティティのコライダーがあれば摩擦を読む（なければ1.0扱い）
            float friction = 1.0f;
            if (auto* col = world.TryGet<Collider2DComponent>(e))
            {
                friction = col->material.friction;
                // 0～1の間に収めておく
                friction = std::clamp(friction, 0.0f, 1.0f);
            }

            // 3. 入力からターゲット速度を作る
            //    moveX が -1～1なので、これに最大速度を掛けるだけ
            const float targetVelX = intent.moveX * m_maxSpeedX;

            // 実際に使う加速の強さ（地上・空中で変える）
            const float accel = onGround ? m_groundAccel : m_airAccel;

            // 現在速度をターゲットに寄せていく
            // 加速方向だけ決めて足す
            float vx = rb.velocity.x;

            // ターゲットより左なら右に加速、右なら左に加速
            const float toTarget = targetVelX - vx;
            // どれくらい動かすか（加速量）
            float add = accel * dt;

            // 氷っぽくしたいなら摩擦で弱める
            add *= friction;

            // 目標への差分が加速量より小さいなら一気に目標へ
            if (std::abs(toTarget) <= add)
            {
                vx = targetVelX;
            }
            else
            {
                // まだ届かないので、符号だけ合わせて一歩近づける
                vx += (toTarget > 0.0f) ? add : -add;
            }


            // --- 着地したらブリンク・ジャンプ消費状態をリセット ---
            if (rb.onGround)
            {
                intent.forceJumpConsumed = false;
                intent.blinkConsumed = false;
                intent.isBlinking = false; // ブリンク終了
            }

            // --- 強制ジャンプ ---
            if (intent.forceJumpRequested)
            {
                rb.velocity.y = m_jumpSpeed;
                intent.forceJumpRequested = false;
                intent.forceJumpConsumed = true; // ★ジャンプ消費済みにする
            }
            // --- 通常ジャンプ ---
            else if (intent.jump && onGround)
            {
                rb.velocity.y = m_jumpSpeed;
                rb.onGround = false;
            }
            else
            {
                if (rb.useGravity)
                {
                    rb.velocity.y += m_gravity * dt;
                }
            }

            // 5. 今フレームぶんためてた外力を加える
            //    風や移動床がここに力を積んでおけば勝手に反映される
            vx += rb.accumulatedForce.x * dt;
            rb.velocity.y += rb.accumulatedForce.y * dt;

            // 使い終わったのでクリア
            rb.accumulatedForce = { 0.0f, 0.0f };

            // 6. 計算した横速度を戻す
            rb.velocity.x = vx;

            // --- ブリンク処理 ---
            if (intent.blinkRequested)
            {
                rb.velocity.x = intent.blinkSpeed;
                intent.blinkRequested = false;
                intent.blinkSpeed = 0.0f;
                intent.isBlinking = true; // ブリンク開始
            }

        }
    );
}
