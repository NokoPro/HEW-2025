#ifndef __INPUT_H__
#define __INPUT_H__

#include <Windows.h>
#undef max
#undef min

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

// 同時接続Pad最大数
static const int kPadMax = 4;

// XInputのボタンマスクをそのまま使う
using PadButton = WORD;

// Pad系API
bool IsPadConnected(int idx);                 // idx: 0..3
bool IsPadPress(int idx, PadButton button);   // 押下中
bool IsPadTrigger(int idx, PadButton button); // 今フレームで押した
bool IsPadRelease(int idx, PadButton button); // 今フレームで離した

// 軸・トリガ（デッドゾーン込みで -1..+1 / 0..1）
float GetPadLX(int idx); // 左スティックX
float GetPadLY(int idx); // 左スティックY（上:+, 下:- にしたい場合は符号反転で調整）
float GetPadRX(int idx); // 右スティックX
float GetPadRY(int idx); // 右スティックY
float GetPadLT(int idx); // 左トリガ 0..1
float GetPadRT(int idx); // 右トリガ 0..1


HRESULT InitInput();
void UninitInput();
void UpdateInput();

bool IsKeyPress(BYTE key);
bool IsKeyTrigger(BYTE key);
bool IsKeyRelease(BYTE key);
bool IsKeyRepeat(BYTE key);

bool IsMouseDownR();
POINT GetMouseDelta();

// ホイールの増分を取得して 0 にリセット
int  ConsumeMouseWheelDelta();

// WndProc から呼ぶ用：ホイールの生Δを積む（120 単位）
void Input_OnMouseWheel(short delta);
#endif // __INPUT_H__