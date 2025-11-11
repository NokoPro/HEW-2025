#pragma once

#include <DirectXMath.h>

struct ParallaxComponent
{
    // Parallax factor: 0 = background (doesn't move with camera), 1 = foreground (moves fully with camera)
    float factor = 0.2f;

    // Stored base/world position (set once when component is first processed)
    DirectX::XMFLOAT3 basePos{ 0.0f, 0.0f, 0.0f };
    bool baseSet = false;
};
