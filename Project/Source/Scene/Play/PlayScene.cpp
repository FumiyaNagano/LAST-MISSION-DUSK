#include "DxLib.h"
#include "../../Player/Player.h"
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
#include "../../Map/MapChip.h"
#include "../../Map/Block.h"

// 全てのスクリプトの参考元 : STG課題

void InitPlayScene()
{
	InitBackGround();

	InitPlayer();

	InitBlock();
}

void LoadPlayScene()
{
	LoadBackGround("Data/BackGround/The1stOriginalGameBG.png", BACK_GROUND_LAYER_3);

	LoadPlayer();

	LoadBlock();

	LoadMapChipData();
}

void StartPlayScene()
{
	StartPlayer();

	CreateMap();
}

void StepPlayScene()
{
	StepBackGround();

	StepPlayer();

	StepBlock();
}

void UpdatePlayScene()
{
	UpdateBackGround();

	UpdatePlayer();
}

void DrawPlayScene()
{
	DrawBackGround();

	DrawBlock();

	DrawPlayer();
}

void FinPlayScene()
{
	FinBackGround();

	FinPlayer();

	FinBlock();
}