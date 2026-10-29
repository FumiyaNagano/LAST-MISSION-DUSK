#pragma once

enum InputKey
{
	KEY_W = (1 << 0),
	KEY_S = (1 << 1),
	KEY_A = (1 << 2),
	KEY_D = (1 << 3),
	KEY_LCLICK = (1 << 4),
};

void InitInput();
void UpdateInput();
void DrawInput();
void FinInput();

bool IsInputKey(InputKey key);
bool IsTriggerKey(InputKey key);