#include "ParallaxSystem.h"
#include "ECS/World.h"
#include "ECS/Components/Render/ParallaxComponent.h"
#include "ECS/Components/Physics/TransformComponent.h"
#include "ECS/Components/Core/ActiveCameraTag.h"
#include "ECS/Components/Core/Camera3DComponent.h"

#include <DirectXMath.h>
#include <unordered_map>

using namespace DirectX;

void ParallaxSystem::Update(World& world, float dt)
{
    // Find active camera transform position (use first active camera)
    XMFLOAT3 camPos{ 0.0f, 0.0f, 0.0f };
    bool camFound = false;

    world.View<ActiveCameraTag, Camera3DComponent, TransformComponent>(
        [&](EntityId /*e*/, ActiveCameraTag&, Camera3DComponent& cam, TransformComponent& tr)
        {
            camPos = tr.position;
            camFound = true;
        }
    );

    static XMFLOAT3 prevCamPos = camPos;
    if (!camFound)
    {
        prevCamPos = camPos;
        return;
    }

    // camera movement delta since last frame
    XMFLOAT3 delta{
        camPos.x - prevCamPos.x,
        camPos.y - prevCamPos.y,
        camPos.z - prevCamPos.z
    };

    // For all entities with ParallaxComponent and TransformComponent, apply offset
    world.View<ParallaxComponent, TransformComponent>(
        [&](EntityId /*e*/, ParallaxComponent& par, TransformComponent& tr)
        {
            if (!par.baseSet)
            {
                par.basePos = tr.position;
                par.baseSet = true;
            }

            // Parallax formula: newPos = basePos + (camPos - baseCam) * factor
            // We keep baseCam implicit by accumulating delta each frame
            tr.position.x = par.basePos.x + (camPos.x * par.factor);
            tr.position.y = par.basePos.y + (camPos.y * par.factor);
            tr.position.z = par.basePos.z + (camPos.z * par.factor);
        }
    );

    prevCamPos = camPos;
}
