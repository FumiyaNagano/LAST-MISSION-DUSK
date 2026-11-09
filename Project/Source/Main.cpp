#include "DxLib.h"
#include "GameSetting/GameSetting.h"
#include "FPS/FPS.h"
#include "Player/Player.h"
#include "Input/Input.h"
#include "Scene/SceneManager.h"
#include "Sound/SoundManager.h"
#include "UIImage/UIText.h"
#include "BackGround/BackGround.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	// ウィンドウモードON
	ChangeWindowMode(TRUE);

	// 画面解像度の設定
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_DEPTH);

	SetWaitVSyncFlag(TRUE);

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

	LoadSound();

	// UIはゲーム起動と同時に初期化、ロード
	//InitUIText();

	InitFPS();

	while (ProcessMessage() >= 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE)) break;

		// FPS制御を更新
		UpdateFPS();

		// 入力を更新
		UpdateInput();

		ClearDrawScreen();

		SceneManagerUpdate();

		// FPS値描画
		DrawFPS();

		// デバッグ用
		DrawFormatString(0, 60, GetColor(255, 255, 255), "HP: %d  inv: %.2f",
			GetPlayer()->hp, GetPlayer()->invincibleTimer);



		// 画面フリップ
		ScreenFlip();

		// 1フレームが速すぎた時の待機時間
		FPSWait();
	}

	// 入力システム終了
	FinInput();

	FinSound();

	// UI終了
	//FinUIText();

	// DxLib使用の終了処理
	DxLib_End();

	return 0;
}