#pragma once

// Centralized runtime debug toggles used by systems. Simple singleton.
struct DebugSettings
{
    // Toggle rising/kill of magma (DeathZone)
    bool magmaEnabled = true;
    // Allow jump even when not grounded
    bool infiniteJump = false;
    // Placeholder for future UI
    bool imguiEnabled = false;
    // Scale for magma rising speed (1.0 = default)
    float magmaSpeedScale = 1.0f;

    static DebugSettings& Get()
    {
        static DebugSettings s_instance{};
        return s_instance;
    }

private:
    DebugSettings() = default;
};
