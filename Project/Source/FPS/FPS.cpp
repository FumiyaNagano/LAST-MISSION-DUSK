#include "DxLib.h"
#include "FPS.h"
#include "../Player/Player.h"

// 目標FPS（0以下で上限なし）
#define FPS_TARGET (60)
// deltaTimeの上限（秒）。処理落ち時の巨大な値を抑える
#define FPS_DELTA_TIME_MAX (0.05f)
// FPS表示を更新する間隔（マイクロ秒）
#define FPS_MEASURE_INTERVAL (500000LL)

// 前フレーム開始時の時刻（マイクロ秒）
static LONGLONG g_FPSPrevCount = 0;
// 前フレームからの経過時間（秒）
static float g_FPSDeltaTime = 0.0f;

// FPS計測用
static LONGLONG g_FPSMeasureStartCount = 0;
static int g_FPSFrameCount = 0;
static float g_FPSDisplayValue = 0.0f;

void InitFPS()
{
	g_FPSPrevCount = GetNowHiPerformanceCount();
	g_FPSDeltaTime = 0.0f;
	g_FPSMeasureStartCount = g_FPSPrevCount;
	g_FPSFrameCount = 0;
	g_FPSDisplayValue = 0.0f;
}

void UpdateFPS()
{
	LONGLONG now = GetNowHiPerformanceCount();

	// 経過時間を秒に変換
	g_FPSDeltaTime = (now - g_FPSPrevCount) / 1000000.0f;
	g_FPSPrevCount = now;

	if (g_FPSDeltaTime > FPS_DELTA_TIME_MAX)
	{
		g_FPSDeltaTime = FPS_DELTA_TIME_MAX;
	}

	// 一定間隔ごとにFPSを算出（毎フレーム計算すると表示がちらつくため）
	g_FPSFrameCount++;
	LONGLONG elapsed = now - g_FPSMeasureStartCount;

	if (elapsed >= FPS_MEASURE_INTERVAL)
	{
		g_FPSDisplayValue = g_FPSFrameCount * 1000000.0f / elapsed;
		g_FPSFrameCount = 0;
		g_FPSMeasureStartCount = now;
	}
}

void DrawFPS()
{
	DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS; %.1f", g_FPSDisplayValue);
}

void FPSWait()
{
	if (FPS_TARGET <= 0) return;

	// 1フレームにかけたい時間（マイクロ秒）
	const LONGLONG targetFrameTime = 1000000LL / FPS_TARGET;
	LONGLONG waitUntil = g_FPSPrevCount + targetFrameTime;

	// 残り時間が多いうちはSleepで待つ（CPU負荷軽減）
	while (waitUntil - GetNowHiPerformanceCount() > 2000) Sleep(1);
}

float GetDeltaTime()
{
	return g_FPSDeltaTime;
}