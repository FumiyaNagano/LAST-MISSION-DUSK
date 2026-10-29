#include "DxLib.h"
// #include "../../Player/Player.h"
#include "../../BackGround/BackGround.h"
#include "../../Scene/SceneManager.h"
// #include "../../Bullet/BulletManager.h"
// #include "../../Enemy/EnemyManager.h"
// #include "../../Sound/SoundManager.h"
// #include "../../Collision/Collision.h"
// #include "../../Effect/Effect.h"
// #include "../../UIImage/UIImage.h"
// #include "../../UIImage/UIText.h"
// #include "../../Score/Score.h"
#include "../../GameSetting/GameSetting.h"

// 全てのスクリプトの参考元 : STG課題

void InitPlayScene()
{
	InitBackGround();
}

void LoadPlayScene()
{
	LoadBackGround("Data/BackGround/The1stOriginalGameBG.png", BACK_GROUND_LAYER_3);
}

void StartPlayScene()
{
}

void StepPlayScene()
{
	StepBackGround();
}

void UpdatePlayScene()
{
	UpdateBackGround();
}

void DrawPlayScene()
{
	DrawBackGround();
}

void FinPlayScene()
{
	FinBackGround();
}