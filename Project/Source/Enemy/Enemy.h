#pragma once
#include "DxLib.h"
#include "../Weapon/Weapon.h"
#include "../Animation/Animation.h"

// 同時に存在できる敵の最大数
#define ENEMY_MAX (64)

// 敵の種類
enum EnemyType
{
	ENEMY_TYPE_SOLDIER,
	ENEMY_TYPE_NUM,
};

// 敵の見た目の状態
enum EnemyState
{
	ENEMY_STATE_IDLE,	// 待機
	ENEMY_STATE_MOVE,	// 移動
	ENEMY_STATE_SHOOT,	// 射撃
	ENEMY_STATE_NUM,
};

// 敵1体分のデータ
struct EnemyData
{
	bool active;
	EnemyType type;
	VECTOR pos;				// 当たり判定矩形の左上
	VECTOR velocity;		// 移動速度（ピクセル/秒）
	float width;
	float height;
	int hp;
	int maxHp;
	int contactDamage;		// プレイヤーに接触したときのダメージ
	float moveSpeed;		// 接近する速さ（ピクセル/秒）
	float stopDistance;		// プレイヤーとの距離がこれ以下になると止まる（ピクセル）
	float aimSpread;		// 照準のばらつき（ラジアン）
	WeaponState gun;		// 持っている武器と弾数・リロード状態（倒すとドロップする）
	bool facingLeft;		// 左向きか（画像は右向きが基準）
	EnemyState state;
	AnimPlayer anim;
	float shootTimer;		// 射撃アニメーションを表示する残り時間（秒）
	float hitFlashTimer;	// 被弾の白点滅の残り時間（秒）
};