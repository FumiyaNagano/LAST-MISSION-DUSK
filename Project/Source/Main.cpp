#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "FPS/FPS.h"
#include "Input/Input.h"
#include "Scene/SceneManager.h"
#include "BackGround/BackGround.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	ChangeWindowMode(TRUE);

	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

	if (DxLib_Init() == -1)
	{
		return -1;
	}

	SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	SetTransColor(TRANS_COLOR_R, TRANS_COLOR_G, TRANS_COLOR_B);

	SetDrawScreen(DX_SCREEN_BACK);

	//InitInput();



	//InitFPS();

	while (ProcessMessage() >= 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE)) break;

		ClearDrawScreen();

		SceneManagerUpdate();

		ScreenFlip();
	}

	FinScene();

	DxLib_End();

	return 0;
}