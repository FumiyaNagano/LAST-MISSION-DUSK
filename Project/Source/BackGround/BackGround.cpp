#include "DxLib.h"
#include "BackGround.h"

#define BACK_GROUND_WIDTH	(1920)
#define BACK_GROUND_HEIGHT	(1080)

// 背景用のデータ
struct BackGroundData
{
	int handle;
	float posX;
	float posY;
	float moveX;
	float moveY;
};
BackGroundData g_BackGroundData[BACK_GROUND_LAYER_MAX] = { 0 };

void InitBackGround()
{
	for (int i = 0; i < BACK_GROUND_LAYER_MAX; i++)
	{
		g_BackGroundData[i].handle = 0;
		g_BackGroundData[i].posX = 0.0f;
		g_BackGroundData[i].posY = 0.0f;
		g_BackGroundData[i].moveX = 0.0f;
		g_BackGroundData[i].moveY = 0.0f;
	}
}

void LoadBackGround(const char* path, BackGroundLayer layer)
{
	g_BackGroundData[layer].handle = LoadGraph(path);
}

void StepBackGround()
{
}

void UpdateBackGround()
{
	
}

void DrawBackGround()
{
	for (int i = 0; i < BACK_GROUND_LAYER_MAX; i++)
	{
		DrawGraph((int)g_BackGroundData[i].posX, (int)g_BackGroundData[i].posY, (int)g_BackGroundData[i].handle, TRUE);
	}
}

void FinBackGround()
{
	for (int i = 0; i < BACK_GROUND_LAYER_MAX; i++)
	{
		DeleteGraph(g_BackGroundData[i].handle);
	}
}

void SetBackGroundMove(float moveX, float moveY, BackGroundLayer layer)
{
	g_BackGroundData[layer].moveX = moveX;
	g_BackGroundData[layer].moveY = moveY;
}
