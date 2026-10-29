#pragma once

// 背景レイヤー（数字が多いほど前面）
enum BackGroundLayer
{
	BACK_GROUND_LAYER_1,
	BACK_GROUND_LAYER_2,
	BACK_GROUND_LAYER_3,
	BACK_GROUND_LAYER_MAX,
};

// 関数のプロトタイプ宣言
void InitBackGround();
void LoadBackGround(const char* path, BackGroundLayer layer);
void StepBackGround();
void UpdateBackGround();
void DrawBackGround();
void FinBackGround();

// 背景の移動量設定
void SetBackGroundMove(float moveX, float moveY, BackGroundLayer layer);
