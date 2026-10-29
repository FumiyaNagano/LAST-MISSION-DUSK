#include "DxLib.h"
#include "Input.h"

// 入力ビットフラグ
int g_InputState = 0;
// 前回の入力ビット
int g_PrevInputState = 0;

void InitInput()
{
	g_InputState = 0;
	g_PrevInputState = 0;
}

void UpdateInput()
{
	// 前回の入力を覚えておく
	g_PrevInputState = g_InputState;

	// 入力状態をクリア
	g_InputState = 0;

	// 入力状態をビットフラグで設定
	if (CheckHitKey(KEY_INPUT_W))
	{
		g_InputState |= KEY_W;
	}
	if (CheckHitKey(KEY_INPUT_S))
	{
		g_InputState |= KEY_S;
	}
	if (CheckHitKey(KEY_INPUT_A))
	{
		g_InputState |= KEY_A;
	}
	if (CheckHitKey(KEY_INPUT_D))
	{
		g_InputState |= KEY_D;
	}
}

void DrawInput()
{
}

void FinInput()
{
}

bool IsInputKey(InputKey key)
{
	return g_InputState & key;
}

bool IsTriggerKey(InputKey key)
{
	return (g_InputState & key) && !(g_PrevInputState & key);
}