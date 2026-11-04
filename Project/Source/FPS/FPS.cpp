#include "DxLib.h"
#include "FPS.h"

// 目標FPS（0以下で上限なし）
#define FPS_TARGET (240)
// deltaTimeの上限（秒）。処理落ち時の巨大な値を抑える
#define FPS_DELTA_TIME_MAX (0.05f)
// FPS表示を更新する間隔（マイクロ秒）
#define FPS_MEASURE_INTERVAL (500000LL)

// 前フレーム開始時の時刻（マイクロ秒）
LONGLONG g_FPSPrevCount = 0;
// 前フレームからの経過時間（秒）
float g_FPSDeltaTime = 0.0f;

// FPS計測用
LONGLONG g_FPSMeasureStartCount = 0;
int g_FPSFrameCount = 0;
float g_FPSDisplayValue = 0.0f;

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
		g_FPSDeltaTime = FPS_MEASURE_INTERVAL;
	}

	// 一定間隔ごとにFPSを算出（毎フレーム計算すると表示がちらつくため）
	g_FPSFrameCount++;
	LONGLONG elapsed = now - g_FPSMeasureStartCount;

	if (elapsed >= FPS_MEASURE_INTERVAL);
}
