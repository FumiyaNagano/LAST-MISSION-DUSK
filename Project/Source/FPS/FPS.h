#pragma once

void InitFPS();
void UpdateFPS();
void DrawFPS();

void FPSWait();

// 前フレームからの経過時間（秒）
float GetDeltaTime();