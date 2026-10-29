#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "FPS/FPS.h"
#include "Input/Input.h"
#include "Scene/SceneManager.h"
#include "UIImage/UIText.h"
#include "BackGround/BackGround.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// ウィンドウモードON
	ChangeWindowMode(TRUE);

	// 画面解像度の設定
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

	if (DxLib_Init() == -1)		// DxLib初期化処理
	{
		return -1;
	}

	// ウィンドウサイズ設定
	SetWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	// 透過色の設定
	SetTransColor(TRANS_COLOR_R, TRANS_COLOR_G, TRANS_COLOR_B);

	// 描画先を裏画面へ
	SetDrawScreen(DX_SCREEN_BACK);

	// 入力システム初期化
	InitInput();

	// UIはゲーム起動と同時に初期化、ロード
	//InitUIText();

	//InitFPS();

	while (ProcessMessage() >= 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE)) break;

		// FPS制御を更新
		// UpdateFPS();

		SceneManagerUpdate();

		// FPS値描画
		// DrawFPS();

		// 入力を更新
		UpdateInput();

		// 1フレームが速すぎた時の待機時間
		// FPSWait();

		// 画面フリップ
		ScreenFlip();
	}

	// 入力システム終了
	FinInput();

	// UI終了
	//FinUIText();

	// DxLib使用の終了処理
	DxLib_End();

	return 0;
}