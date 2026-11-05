#pragma once
#include "DxLib.h"
#include "../UIImage/UIText.h"

#define PLAYER_WIDTH	(46.0f)
#define PLAYER_HEIGHT	(100.0f)
#define PLAYER_RADIUS	(23.0f)

// プレイヤーのアニメーション
#define PLAYER_ANIM_FRAME_MAX (16)

enum PlayerState
{
	PLAYER_STATE_IDLE,
	PLAYER_STATE_MOVE,
	PLAYER_STATE_MOVE_UP,
	PLAYER_STATE_MOVE_DOWN,
	PLAYER_STATE_SHOOT,
	PLAYER_STATE_SHOOT_UP,
	PLAYER_STATE_DEAD,
	PLAYER_STATE_NUM,
};

// プレイヤーのアニメーション関連設定
// 1コマ分のデータ
struct PlayerAnimFrame
{
	int handle;		// 切り出した画像
	int width;		// このコマの幅
	int height;		// このコマの高さ
	int anchorX;	// コマ内での体の中心X座標
};
// アニメーション1種類分のデータ
struct PlayerAnimData
{
	int srcHandle;		// 元画像のハンドル(DerivationGraphの参照元)
	PlayerAnimFrame frames[PLAYER_ANIM_FRAME_MAX];
	int frameNum;			// コマ数
	float frameTime;		// 1コマの表示時間（秒）
	bool loop;				// ループするか
	bool baseFacingLeft;	// 元画像が左向きに描かれているか
};

struct PlayerData
{
	bool active;
	//int bulletIntervalTimer;
	int hp;
	float shootTimer; // 射撃アニメーションを表示する残り時間（秒）
	VECTOR pos;
	VECTOR move; // 移動速度（ピクセル/秒）
	VECTOR aimDir; // 狙う方向（長さ1）
	UITextData* hpText;

	// アニメーション用
	PlayerState state;
	int animFrame;		// 現在のコマ番号
	float animTimer;	// 現在のコマの経過時間（秒）
	bool facingLeft;	// 左向きか（trueなら左向き、falseなら右向き）
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