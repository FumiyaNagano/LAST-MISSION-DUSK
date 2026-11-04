// DxLib, Visual Studio 2022でゲーム制作を行っています。プレイヤーが描画されない原因を特定してほしいです。まずPlayer.cppを貼ります。他にスクリプトが必要であれば教えてください。

#include "DxLib.h"
#include "Player.h"
#include "../Input/Input.h"
//#include "../Collision/Collision.h"
//#include "../Score/Score.h"
//#include "../Effect/AnimationEffect.h"
//#include "../Bullet/BulletManager.h"
//#include "../Scene/SceneManager.h"
//#include "../Sound/SoundManager.h"
#include "../Map/Block.h"

// HP表示文字列
#define PLAYER_HP_STRING ("%d")

// プレイヤー設定関連
#define PLAYER_DEFAULT_POS_X (100.0f)
#define PLAYER_DEFAULT_POS_Y (300.0f)
#define PLAYER_MOVE_SPEED (3.0f)

// 弾関連
//#define PLAYER_DEFAULT_SHOT_SPEED	(20.0f)
#define PLAYER_SHOT_LIFE		(600)
#define PLAYER_SHOT_OFFSET_X	(50.0f)
#define PLAYER_SHOT_OFFSET_Y	(57.0f)
#define PLAYER_SHOT_INTERVAL	(10)

// 移動可能エリア
#define PLAYER_MOVE_AREA_LEFT	(0.0f)
#define PLAYER_MOVE_AREA_RIGHT	(1874.0f)
#define PLAYER_MOVE_AREA_TOP	(0.0f)
#define PLAYER_MOVE_AREA_BOTTOM	(980.0f)

// プレイヤーのHP
#define PLAYER_HP (100)

// プレイヤー死亡エフェクトインターバル
#define PLAYER_DEAD_EFFECT_INTERVAL (2)

PlayerData g_PlayerData = { 0 };

void InitPlayer()
{
	g_PlayerData.hp = 0;
	g_PlayerData.pos = VGet(PLAYER_DEFAULT_POS_X, PLAYER_DEFAULT_POS_Y, 0.0f);
	g_PlayerData.move = VGet(0.0f, 0.0f, 0.0f);
	//g_PlayerData.bulletIntervalTimer = 0;
	g_PlayerData.active = false;
}

void LoadPlayer()
{
	g_PlayerData.handle = LoadGraph("Data/Player/PlayerIdle.png");
}

void StartPlayer()
{
	// 生存フラグを立てる
	g_PlayerData.active = true;

	// HPの初期値設定
	g_PlayerData.hp = PLAYER_HP;

	// HPのUI生成
	//char text[256];
	//sprintf_s(text, PLAYER_HP_STRING, g_PlayerData.hp);
	//g_PlayerData.hpText = CreateUIText(60.0f, 900.0f, text);
}

void StepPlayer()
{
	/*if (g_PlayerData.bulletIntervalTimer > 0)
	{
		g_PlayerData.bulletIntervalTimer--;
	}*/

	VECTOR dir = VGet(0.0f, 0.0f, 0.0f);

	if (IsInputKey(KEY_W)) dir.y -= 1.0f;
	if (IsInputKey(KEY_S)) dir.y += 1.0f;
	if (IsInputKey(KEY_A)) dir.x -= 1.0f;
	if (IsInputKey(KEY_D)) dir.x += 1.0f;

	if (VSquareSize(dir) > 0.0f)
	{
		dir = VNorm(dir);
	}
	g_PlayerData.move = VScale(dir, PLAYER_MOVE_SPEED);
}

void UpdatePlayer()
{
	if (!g_PlayerData.active) return;

	// X方向の移動
	VECTOR nextPos = VAdd(g_PlayerData.pos, g_PlayerData.move);

	if (!CheckBlockCollision(
		nextPos.x,
		g_PlayerData.pos.y,
		PLAYER_WIDTH,
		PLAYER_HEIGHT))
	{
		g_PlayerData.pos.x = nextPos.x;
	}

	if (!CheckBlockCollision(
		g_PlayerData.pos.x,
		nextPos.y,
		PLAYER_WIDTH,
		PLAYER_HEIGHT))
	{
		g_PlayerData.pos.y = nextPos.y;
	}

	// 画面外への移動制限
	if (g_PlayerData.pos.x < PLAYER_MOVE_AREA_LEFT)
		g_PlayerData.pos.x = PLAYER_MOVE_AREA_LEFT;

	if (g_PlayerData.pos.x > PLAYER_MOVE_AREA_RIGHT)
		g_PlayerData.pos.x = PLAYER_MOVE_AREA_RIGHT;

	if (g_PlayerData.pos.y < PLAYER_MOVE_AREA_TOP)
		g_PlayerData.pos.y = PLAYER_MOVE_AREA_TOP;

	if (g_PlayerData.pos.y > PLAYER_MOVE_AREA_BOTTOM)
		g_PlayerData.pos.y = PLAYER_MOVE_AREA_BOTTOM;
}

void DrawPlayer()
{
	if (!g_PlayerData.active) return;

	if (g_PlayerData.active == true)
	{
		DrawGraph((int)g_PlayerData.pos.x, (int)g_PlayerData.pos.y, g_PlayerData.handle, TRUE);
	}
}

void FinPlayer()
{
	DeleteGraph(g_PlayerData.handle);
}

PlayerData* GetPlayer()
{
	return &g_PlayerData;
}

void PlayerDead()
{
	// 死亡
	g_PlayerData.active = false;

	// 死亡エフェクト
	//StartAnimationEffect(PLAYER_DEAD, g_PlayerData.pos.x, g_PlayerData.pos.y, PLAYER_DEAD_EFFECT_INTERVAL);

	// 死亡SE
	//PlaySE(SE_PLAYER_DEAD);
}