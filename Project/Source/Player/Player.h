#pragma once
#include "../UIImage/UIText.h"

#define PLAYER_WIDTH	(46.0f)
#define PLAYER_HEIGHT	(100.0f)
#define PLAYER_RADIUS	(23.0f)

struct PlayerData
{
	bool active;
	int handle;
	//int bulletIntervalTimer;
	int hp;
	float posX;
	float posY;
	float moveX;
	float moveY;
	UITextData* hpText;
};

// 関数のプロトタイプ宣言
void InitPlayer();
void LoadPlayer();
void StartPlayer();
void StepPlayer();
void UpdatePlayer();
void DrawPlayer();
void FinPlayer();

// 取得関数
PlayerData* GetPlayer();

// 当たり判定
void PlayerHitEnemy();
void PlayerHitEnemyBullet();