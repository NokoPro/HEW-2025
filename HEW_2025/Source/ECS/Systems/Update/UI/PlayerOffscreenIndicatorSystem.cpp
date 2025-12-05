/**********************************************************************************************
 * @file      PlayerOffscreenIndicatorSystem.cpp
 * @brief     画面下端インジケータUIの更新システム実装
 *
 * アクティブカメラから画面下端のワールド座標を取得し、対象プレイヤーが画面下端より下に
 * いる場合にインジケータUIを表示します。UIはプレイヤーのXに追従し、Yは下端に固定します。
 *
 * @author    浅野勇生
 * @date      2025/12/5
 **********************************************************************************************/
#include "PlayerOffscreenIndicatorSystem.h"
#include "ECS/Components/UI/OffscreenIndicatorComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/Camera3DComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Render/Sprite2DComponent.h"

namespace {
    /**
     * @brief アクティブカメラの中心Yと正射影半高さを取得
     */
    bool GetActiveCamera(World& world, TransformComponent*& camTr, Camera3DComponent*& cam)
    {
        camTr = nullptr; cam = nullptr;
        world.View<ActiveCameraTag, TransformComponent, Camera3DComponent>(
            [&](EntityId, ActiveCameraTag&, TransformComponent& tr, Camera3DComponent& c)
            {
                camTr = &tr;
                cam   = &c;
            });
        return (camTr != nullptr && cam != nullptr);
    }
}

void PlayerOffscreenIndicatorSystem::Update(World& world, float /*dt*/)
{
    TransformComponent* camTr = nullptr;
    Camera3DComponent* cam = nullptr;
    if (!GetActiveCamera(world, camTr, cam)) return;

    const float camCenterY = camTr->position.y;
    const float halfOrthoH = cam->orthoHeight * 0.5f;
    const float bottomY = camCenterY - halfOrthoH;

    world.View<OffscreenIndicatorComponent, TransformComponent, Sprite2DComponent>(
        [&](EntityId e, OffscreenIndicatorComponent& ind, TransformComponent& uiTr, Sprite2DComponent& sprite)
        {
            if (ind.targetId == kInvalidEntity || !world.Has<TransformComponent>(ind.targetId))
            {
                ind.visible = false;
                sprite.visible = false;
                return;
            }

            const auto& targetTr = world.Get<TransformComponent>(ind.targetId);
            const bool isBelow = (targetTr.position.y < bottomY);
            ind.visible = isBelow;
            sprite.visible = ind.visible;

            if (ind.visible)
            {
                uiTr.position.x = targetTr.position.x + ind.offset.x;
                uiTr.position.y = bottomY + ind.bottomMargin + ind.offset.y;
                uiTr.position.z = targetTr.position.z + ind.offset.z;
            }
        }
    );
}
