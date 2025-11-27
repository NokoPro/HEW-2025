#include "EffectSystem.h"

using namespace DirectX;

namespace
{
    XMFLOAT3 GetWorldPosition(const TransformComponent& tr)
    {
        // ここは現状の TransformComponent の実装に合わせて調整
        // 例えば:
        return tr.position;  // position が XMFLOAT3 の場合
    }
}

void EffectSystem::Update(World& world, float dt)
{
    // まずランタイム側を更新
    EffectRuntime::Update(dt);

    world.View<TransformComponent, EffectComponent>(
        [&](EntityId e, const TransformComponent& tr, EffectComponent& efc)
        {
            // EffectRef が無ければ何もしない
            if (!efc.effect)
            {
                return;
            }

            const auto& ref = *efc.effect; // EffectRef

            // A) エフェクト切替検知: lastPath と異なるアセットが指定されたら、再生中なら停止→再生し直し
            if (!ref.path.empty())
            {
                if (efc.playing && efc.lastPath != ref.path)
                {
                    // 再生中エフェクトを止め、次に再生を要求
                    efc.stopRequested = true;
                    efc.playRequested = true;
                }
                // lastPath を更新（この時点ではまだ Play 前の可能性もあるが、次回検知のために保持）
                efc.lastPath = ref.path;
            }

            // 1. 再生開始条件チェック
            if (!efc.playing)
            {
                bool shouldPlay = false;

                // 初回フレームの自動再生
                if (efc.playOnSpawn)
                {
                    shouldPlay = true;
                    efc.playOnSpawn = false; // 一度きり
                }

                // 外部からの再生リクエスト
                if (efc.playRequested)
                {
                    shouldPlay = true;
                    efc.playRequested = false;
                }

                if (shouldPlay)
                {
                    const XMFLOAT3 pos = GetWorldPosition(tr);

                    // ループフラグは param1 から決める or efc.loop を直接見る
                    bool loop = efc.loop;
                    if (!ref.param1.empty())
                    {
                        // 例: param1 が "loop" ならループ扱いにする等
                        if (ref.param1 == "loop")
                        {
                            loop = true;
                        }
                    }

                    auto handle = EffectRuntime::Play(ref.path.c_str(), pos, loop);
                    if (handle >= 0)
                    {
                        efc.nativeHandle = handle;
                        efc.playing = true;
                        efc.lastPath = ref.path;
                    }
                }
            }

            // 2. 再生中のエフェクトに対する処理
            if (efc.playing && efc.nativeHandle >= 0)
            {
                const XMFLOAT3 pos = GetWorldPosition(tr);
                EffectRuntime::SetLocation(efc.nativeHandle, pos);

                // 停止リクエスト
                if (efc.stopRequested)
                {
                    EffectRuntime::Stop(efc.nativeHandle);
                    efc.stopRequested = false;
                    efc.playing = false;
                    efc.nativeHandle = -1;
                    return;
                }

                // ワンショットかつ再生終了したら片付け
                if (!efc.loop && EffectRuntime::IsFinished(efc.nativeHandle))
                {
                    efc.playing = false;
                    efc.nativeHandle = -1;

                    if (efc.autoDestroyEntity)
                    {
                        world.Destroy(e);
                    }
                }
            }
        });
}
