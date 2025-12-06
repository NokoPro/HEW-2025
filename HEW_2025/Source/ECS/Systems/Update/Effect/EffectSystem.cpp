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

    // エンティティのTransformに対して、EffectComponentのローカルオフセットを回転させて加算
    XMFLOAT3 ComputeEffectWorldPos(const TransformComponent& tr, const EffectComponent& efc)
    {
        // エンティティの回転（度数法）をラジアンに変換
        const float pitch = XMConvertToRadians(tr.rotationDeg.x);
        const float yaw   = XMConvertToRadians(tr.rotationDeg.y);
        const float roll  = XMConvertToRadians(tr.rotationDeg.z);

        // ローカルオフセットにエンティティのローカルスケールを適用（必要に応じて）
        XMFLOAT3 scaledOffset{
            efc.offset.x * tr.scale.x,
            efc.offset.y * tr.scale.y,
            efc.offset.z * tr.scale.z
        };

        // 行列でローカル→ワールド方向への回転を適用
        XMMATRIX R = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
        XMVECTOR off = XMLoadFloat3(&scaledOffset);
        XMVECTOR offRot = XMVector3Transform(off, R);

        XMFLOAT3 pos = GetWorldPosition(tr);
        XMFLOAT3 offRotF{};
        XMStoreFloat3(&offRotF, offRot);

        return XMFLOAT3{
            pos.x + offRotF.x,
            pos.y + offRotF.y,
            pos.z + offRotF.z
        };
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
                    const XMFLOAT3 worldPos = ComputeEffectWorldPos(tr, efc);

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

                    auto handle = EffectRuntime::Play(ref.path.c_str(), worldPos, loop);
                    if (handle >= 0)
                    {
                        efc.nativeHandle = handle;
                        efc.playing = true;
                        efc.lastPath = ref.path;
                        // 位置・回転・スケールを一括反映
                        EffectRuntime::SetTransform(efc.nativeHandle, worldPos, efc.rotationDeg, efc.scale);
                        // フォールバック（見えない場合に備えて個別設定も併用）
                        EffectRuntime::SetLocation(efc.nativeHandle, worldPos);
                        EffectRuntime::SetRotationDeg(efc.nativeHandle, efc.rotationDeg);
                        EffectRuntime::SetScale(efc.nativeHandle, efc.scale);
                    }
                }
            }

            // 2. 再生中のエフェクトに対する処理
            if (efc.playing && efc.nativeHandle >= 0)
            {
                const XMFLOAT3 worldPos = ComputeEffectWorldPos(tr, efc);
                EffectRuntime::SetTransform(efc.nativeHandle, worldPos, efc.rotationDeg, efc.scale);
                // フォールバック（見えない場合に備えて個別設定も併用）
                EffectRuntime::SetLocation(efc.nativeHandle, worldPos);
                EffectRuntime::SetRotationDeg(efc.nativeHandle, efc.rotationDeg);
                EffectRuntime::SetScale(efc.nativeHandle, efc.scale);

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
