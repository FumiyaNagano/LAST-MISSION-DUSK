#pragma once
#include "DxLib.h"
#include "../Weapon/Weapon.h"

// 同時に存在できる敵の最大数
#define ENEMY_MAX (64)

// 敵の種類
enum EnemyType
{
	ENEMY_TYPE_SOLDIER,
	ENEMY_TYPE_NUM,
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
	WeaponState gun;		// 持っている武器と弾数・リロード状態（倒すとドロップする）
	float hitFlashTimer;	// 被弾の白点滅の残り時間（秒）
};