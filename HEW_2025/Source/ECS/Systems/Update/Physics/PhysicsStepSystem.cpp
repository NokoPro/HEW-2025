/*********************************************************************/
/* @file   PhysicsStepSystem.cpp
 * @brief  重力・当たり判定・衝突解決などを行う物理ステップシステム
 * 
 * @author 浅野勇生
 * @date   2025/11/13
 *********************************************************************/
#include "PhysicsStepSystem.h"
#include <algorithm> // std::min, std::max, std::abs

void PhysicsStepSystem::Update(World& world, float dt)
{
    // イベントバッファがあるなら最初に空にしておく
    if (m_eventBuffer)
    {
        m_eventBuffer->Clear();
    }

    // 1) まずはRigidbodyを持ってるものを全部「素で」動かす
    world.View<TransformComponent, Rigidbody2DComponent>(
        [&](EntityId /*e*/, TransformComponent& tr, Rigidbody2DComponent& rb)
        {
            tr.position.x += rb.velocity.x * dt;
            tr.position.y += rb.velocity.y * dt;

            // 動いたので、次のフレームまでは地面にいない扱いにしておく
            // -> 実際に床とぶつかったらまた onGround = true にする
            rb.onGround = false;
        }
    );

    // 2) ここから当たり判定
    // 今はシンプルに「全部と全部」で回す（数が増えたらブロードフェーズを足す）
    world.View<TransformComponent, Collider2DComponent>(
        [&](EntityId e1, TransformComponent& t1, Collider2DComponent& c1)
        {
            world.View<TransformComponent, Collider2DComponent>(
                [&](EntityId e2, TransformComponent& t2, Collider2DComponent& c2)
                {
                    if (e1 >= e2)
                        return; // 同じペアを2回見ないように

                    // レイヤマスクで衝突しないならスキップ
                    if ((c1.hitMask & c2.layer) == 0 &&
                        (c2.hitMask & c1.layer) == 0)
                    {
                        return;
                    }

                    // どっちかがトリガーなら押し戻さずイベントだけ
                    const bool isTriggerPair = c1.isTrigger || c2.isTrigger;

                    // 今回はAABB2Dだけ対応（他の形状はあとで足す）
                    if (c1.shape == ColliderShapeType::AABB2D && c2.shape == ColliderShapeType::AABB2D)
                    {
                        // それぞれにRigidbodyがあるかを取っておく
                        auto* rb1 = world.TryGet<Rigidbody2DComponent>(e1);
                        auto* rb2 = world.TryGet<Rigidbody2DComponent>(e2);

                        if (isTriggerPair)
                        {
                            // AABB同士が重なってたらイベントだけ詰める
                            // 判定だけ同じようにやる
                            const float axMin = t1.position.x + c1.offset.x - c1.aabb.halfX;
                            const float axMax = t1.position.x + c1.offset.x + c1.aabb.halfX;
                            const float ayMin = t1.position.y + c1.offset.y - c1.aabb.halfY;
                            const float ayMax = t1.position.y + c1.offset.y + c1.aabb.halfY;

							// BのAABB（offset含む）
                            const float bxMin = t2.position.x + c2.offset.x - c2.aabb.halfX;
                            const float bxMax = t2.position.x + c2.offset.x + c2.aabb.halfX;
                            const float byMin = t2.position.y + c2.offset.y - c2.aabb.halfY;
                            const float byMax = t2.position.y + c2.offset.y + c2.aabb.halfY;

							// 重なってるか
                            const bool overlapX = (axMin < bxMax) && (axMax > bxMin);
                            const bool overlapY = (ayMin < byMax) && (ayMax > byMin);

                            if (overlapX && overlapY && m_eventBuffer)
                            {
                                m_eventBuffer->Add(e1, e2, true);
                                m_eventBuffer->Add(e2, e1, true);
                            }
                        }
                        else
                        {
                            // 実際にめり込みを解決するパス
                            ResolveAabbVsAabb(e1, t1, c1, rb1, e2, t2, c2, rb2);
                        }
                    }
                }
            );
        }
    );

    // 3) 移動床の処理は「床側が持ってるcurrentDeltaを、当たってる&onGroundなキャラに足す」
    //    本格的にやるときは上のResolveの中で「どの床に乗っているか」を記録しておいて、
    //    ここでまとめて適用する形にするときれい。
    //    今回は土台なのでここまで。
}

// AABB vs AABB の衝突解決
void PhysicsStepSystem::ResolveAabbVsAabb(EntityId a, TransformComponent& ta, Collider2DComponent& ca, Rigidbody2DComponent* ra,
    EntityId b, TransformComponent& tb, Collider2DComponent& cb, Rigidbody2DComponent* rb)
{
    // AのAABB（offset含む）
    const float axMin = ta.position.x + ca.offset.x - ca.aabb.halfX;
    const float axMax = ta.position.x + ca.offset.x + ca.aabb.halfX;
    const float ayMin = ta.position.y + ca.offset.y - ca.aabb.halfY;
    const float ayMax = ta.position.y + ca.offset.y + ca.aabb.halfY;

    // BのAABB（offset含む）
    const float bxMin = tb.position.x + cb.offset.x - cb.aabb.halfX;    
    const float bxMax = tb.position.x + cb.offset.x + cb.aabb.halfX;
    const float byMin = tb.position.y + cb.offset.y - cb.aabb.halfY;
    const float byMax = tb.position.y + cb.offset.y + cb.aabb.halfY;

	// 重なってるか
	const bool overlapX = (axMin < bxMax) && (axMax > bxMin);   // X方向
	const bool overlapY = (ayMin < byMax) && (ayMax > byMin);   // Y方向

    if (!overlapX || !overlapY)
        return;

    // 重なり量
    const float overlapYAmount = std::min(ayMax, byMax) - std::max(ayMin, byMin);
    const float overlapXAmount = std::min(axMax, bxMax) - std::max(axMin, bxMin);

    // どちらが固定物か
    const bool aStatic = ca.isStatic || ca.isTrigger;
    const bool bStatic = cb.isStatic || cb.isTrigger;

    // Y方向優先で最小の押し戻しを行う
    if (overlapYAmount <= overlapXAmount)
    {
        // Aの中心(オフセット込み)がBの中心より上ならAは上にいる
        const float aCenterY = ta.position.y + ca.offset.y;
        const float bCenterY = tb.position.y + cb.offset.y;
        const float dir = (aCenterY >= bCenterY) ? 1.0f : -1.0f; // 1: A above B -> push apart along +y for A

		// Y方向の解決
        if (!aStatic && bStatic)
        {
            // Bが静的ならAを全部動かす
            ta.position.y += dir * overlapYAmount;
            if (ra) 
            {
                ra->velocity.y = 0.0f;
                // onGround = true になる条件: Aが上でBが床なら
                if (dir > 0.0f) ra->onGround = true;
            }
        }
        else if (aStatic && !bStatic)
        {
            // Aが静的ならBを動かす
            tb.position.y -= dir * overlapYAmount; // move opposite direction
            if (rb) rb->velocity.y = 0.0f;
        }
        else if (!aStatic && !bStatic)
        {
            // 両方動的 -> 半分ずつ
            ta.position.y += dir * (overlapYAmount * 0.5f);
            tb.position.y -= dir * (overlapYAmount * 0.5f);
            if (ra) ra->velocity.y = 0.0f;
            if (rb) rb->velocity.y = 0.0f;
        }
        else
        {
            // 両方静的なら何もしない
        }

        // Y方向の解決後、最終的にどちらが上にいるかを見て onGround をセットする
        if (ra)
        {
            ra->onGround = (!aStatic && ((ta.position.y + ca.offset.y) > (tb.position.y + cb.offset.y)));
        }
        if (rb)
        {
            rb->onGround = (!bStatic && ((tb.position.y + cb.offset.y) > (ta.position.y + ca.offset.y)));
        }
    }
    else
    {
        // 横方向の解決（X優先）
        const float dir = (ta.position.x + ca.offset.x >= tb.position.x + cb.offset.x) ? 1.0f : -1.0f; // 1: A is right of B -> push A right

		// X方向の解決
        if (!aStatic && bStatic)
        {
            ta.position.x += dir * overlapXAmount;
            if (ra) ra->velocity.x = 0.0f;
        }
        else if (aStatic && !bStatic)
        {
            tb.position.x -= dir * overlapXAmount;
            if (rb) rb->velocity.x = 0.0f;
        }
        else if (!aStatic && !bStatic)
        {
            ta.position.x += dir * (overlapXAmount * 0.5f);
            tb.position.x -= dir * (overlapXAmount * 0.5f);
            if (ra) ra->velocity.x = 0.0f;
            if (rb) rb->velocity.x = 0.0f;
        }
    }

    // イベントも出しておく
    if (m_eventBuffer)
    {
        m_eventBuffer->Add(a, b, false);
        m_eventBuffer->Add(b, a, false);
    }
}
