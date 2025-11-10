#include "Input.h"
#include <Xinput.h>

//--- グローバル変数
BYTE g_keyTable[256];
BYTE g_oldTable[256];

// --- 内部状態 ---
static POINT s_prevCursor{ 0,0 };
static POINT s_deltaCursor{ 0,0 };
static int   s_wheelDelta = 0; // 120単位で積む

// Pad の内部状態
struct PadState {
    XINPUT_STATE now{};
    XINPUT_STATE prev{};
    bool connected = false;
    float lx = 0, ly = 0, rx = 0, ry = 0, lt = 0, rt = 0;
};
static PadState g_pad[kPadMax];

static float NormalizeThumb(SHORT v, SHORT deadzone)
{
    int iv = (int)v;
    int ad = (int)deadzone;
    if (iv > 32767) iv = 32767;
    if (iv < -32768) iv = -32768;

    if (iv >= ad)   return  (iv - ad) / (32767.0f - ad);
    if (iv <= -ad)   return  (iv + ad) / (32768.0f - ad);
    return 0.0f;
}
static float NormalizeTrigger(BYTE v, BYTE deadzone = 30)
{
    if (v <= deadzone) return 0.0f;
    return (v - deadzone) / (255.0f - deadzone);
}

HRESULT InitInput()
{
    GetKeyboardState(g_keyTable);
    // Pad 初回取得
    for (int i = 0; i < kPadMax; ++i) {
        ZeroMemory(&g_pad[i], sizeof(PadState));
        XINPUT_STATE st{};
        g_pad[i].connected = (XInputGetState(i, &st) == ERROR_SUCCESS);
        if (g_pad[i].connected) { g_pad[i].now = st; }
    }
    return S_OK;
}

void UpdateInput()
{
    // --- Keyboard
    memcpy_s(g_oldTable, sizeof(g_oldTable), g_keyTable, sizeof(g_keyTable));
    GetKeyboardState(g_keyTable);

    // --- Pad
    for (int i = 0; i < kPadMax; ++i) {
        g_pad[i].prev = g_pad[i].now;
        XINPUT_STATE st{};
        g_pad[i].connected = (XInputGetState(i, &st) == ERROR_SUCCESS);
        if (g_pad[i].connected) {
            g_pad[i].now = st;
            const XINPUT_GAMEPAD& gp = st.Gamepad;

            g_pad[i].lx = NormalizeThumb((SHORT)gp.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            g_pad[i].ly = NormalizeThumb((SHORT)gp.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
            g_pad[i].rx = NormalizeThumb((SHORT)gp.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            g_pad[i].ry = NormalizeThumb((SHORT)gp.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
            g_pad[i].lt = NormalizeTrigger(gp.bLeftTrigger);
            g_pad[i].rt = NormalizeTrigger(gp.bRightTrigger);
        }
        else {
            ZeroMemory(&g_pad[i].now, sizeof(XINPUT_STATE));
            g_pad[i].lx = g_pad[i].ly = g_pad[i].rx = g_pad[i].ry = g_pad[i].lt = g_pad[i].rt = 0.0f;
        }
    }
}


void UninitInput()
{
}
// --- Pad API 実装
bool IsPadConnected(int idx) { return 0 <= idx && idx < kPadMax ? g_pad[idx].connected : false; }
bool IsPadPress(int idx, PadButton b) { return IsPadConnected(idx) && (g_pad[idx].now.Gamepad.wButtons & b) != 0; }
bool IsPadTrigger(int idx, PadButton b) { return IsPadConnected(idx) && ((g_pad[idx].now.Gamepad.wButtons & b) && !(g_pad[idx].prev.Gamepad.wButtons & b)); }
bool IsPadRelease(int idx, PadButton b) { return IsPadConnected(idx) && (!(g_pad[idx].now.Gamepad.wButtons & b) && (g_pad[idx].prev.Gamepad.wButtons & b)); }
float GetPadLX(int idx) { return IsPadConnected(idx) ? g_pad[idx].lx : 0.0f; }
float GetPadLY(int idx) { return IsPadConnected(idx) ? g_pad[idx].ly : 0.0f; }
float GetPadRX(int idx) { return IsPadConnected(idx) ? g_pad[idx].rx : 0.0f; }
float GetPadRY(int idx) { return IsPadConnected(idx) ? g_pad[idx].ry : 0.0f; }
float GetPadLT(int idx) { return IsPadConnected(idx) ? g_pad[idx].lt : 0.0f; }
float GetPadRT(int idx) { return IsPadConnected(idx) ? g_pad[idx].rt : 0.0f; }


bool IsKeyPress(BYTE key) { return g_keyTable[key] & 0x80; }
bool IsKeyTrigger(BYTE key) { return (g_keyTable[key] ^ g_oldTable[key]) & g_keyTable[key] & 0x80; }
bool IsKeyRelease(BYTE key) { return (g_keyTable[key] ^ g_oldTable[key]) & g_oldTable[key] & 0x80; }
bool IsKeyRepeat(BYTE) { return false; }

// --- 新規追加 ---
bool  IsMouseDownR() { return (GetKeyState(VK_RBUTTON) & 0x8000) != 0; }
POINT GetMouseDelta() { return s_deltaCursor; }

int ConsumeMouseWheelDelta() 
{
    int v = s_wheelDelta;
    s_wheelDelta = 0;
    return v;
}

void Input_OnMouseWheel(short delta)
{
    s_wheelDelta += (int)delta; // 120/ノッチ
}
