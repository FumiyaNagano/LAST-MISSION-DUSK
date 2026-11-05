#pragma once
#include "DxLib.h"

// 同時に存在できる弾の最大数
#define BULLET_MAX (1028)

// 弾の持ち主（当たり判定で、誰の弾かを区別するため）
enum BulletOwner
{
	BULLET_OWNER_PLAYER,
	BULLET_OWNER_ENEMY,
};

// 弾1発分のデータ
struct BulletData
{
	bool active;
	BulletOwner owner;
	VECTOR pos;
	VECTOR velocity;
	float life;
	float radius;
};