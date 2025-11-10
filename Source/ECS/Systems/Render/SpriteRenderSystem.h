/*****************************************************************//**
 * @file   SpriteRenderSystem.h
 * @brief  Sprite2DComponent を持つエンティティを描画するSystem
 *
 * @author 浅野勇生
 * @date   2025/11/11
 *********************************************************************/
#pragma once
#include "ECS/Systems/IRenderSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

#include "System/Sprite.h"
#include "System/AssetManager.h"

 /**
  * @class SpriteRenderSystem
  * @brief Sprite2DComponent を持っているものをすべて描画する
  */
class SpriteRenderSystem : public IRenderSystem
{
public:
    void Render(const World& world) override
    {
        using namespace DirectX;

        world.View<TransformComponent, Sprite2DComponent>(
            [&](EntityId, const TransformComponent& tr, const Sprite2DComponent& sp)
            {
                auto hTex = AssetManager::GetTexture(sp.alias);
                if (!hTex)
                {
                    return;
                }

                // 位置とサイズをセット
                Sprite::SetOffset(XMFLOAT2(tr.position.x, tr.position.y));
                Sprite::SetSize(XMFLOAT2(sp.width, sp.height));

                // ビュー・プロジェクションはSpriteが静的に持ってるので、
                // ここでは設定しなくてOK（必要なら最初の1回だけSetView/SetProjectionする）
                Sprite::SetTexture(hTex.get());
                Sprite::Draw();
            }
        );
    }
};
