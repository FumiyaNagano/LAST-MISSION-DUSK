#include "DxLib.h"
#include "BulletManager.h"
#include "../Map/Block.h"
#include "../FPS/FPS.h"
#include "../GameSetting/GameSetting.h"
#include <math.h>

// 弾の画像
#define BULLET_IMAGE_PLAYER		("Data/Bullet/PlayerBullet.png")

// 画面外に出てから消すまでの余白（ピクセル）
#define BULLET_OUT_MARGIN		(50.0f)

// 1回の当たり判定で進める最大距離（ピクセル）。ブロック（75px）より十分に小さくする
#define BULLET_STEP_MAX_DISTANCE (30.0f)

BulletData g_BulletData[BULLET_MAX] = { 0 };
int g_BulletHandle = -1;

void InitBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		g_BulletData[i].active = false;
	}
}

void LoadBullet()
{
	g_BulletHandle = LoadGraph(BULLET_IMAGE_PLAYER);
}

void UpdateBullet()
{
	float dt = GetDeltaTime();

	for (int i = 0; i < BULLET_MAX; i++)
	{
		BulletData* b = &g_BulletData[i];
		if (!b->active) continue;

		b->prevPos = b->pos;

		// このフレームの移動量を、小さな区間に分けて進める
		VECTOR move = VScale(b->velocity, dt);
		int steps = (int)ceilf(VSize(move) / BULLET_STEP_MAX_DISTANCE);
		if (steps < 1) steps = 1;
		VECTOR stepMove = VScale(move, 1.0f / steps);

		for (int s = 0; s < steps; s++)
		{
			b->pos = VAdd(b->pos, stepMove);

			// ブロックに当たったら消す（弾の半径ぶんの四角形で判定）
			if (CheckBlockCollision(
				b->pos.x - b->radius,
				b->pos.y - b->radius,
				b->radius * 2.0f,
				b->radius * 2.0f))
			{
				b->active = false;
				break;
			}
		}
		if (!b->active) continue;

		b->life -= dt;

		// 寿命切れ
		if (b->life <= 0.0f)
		{
			b->active = false;
			continue;
		}

		// 画面外
		if (b->pos.x < -BULLET_OUT_MARGIN || b->pos.x > SCREEN_WIDTH + BULLET_OUT_MARGIN ||
			b->pos.y < -BULLET_OUT_MARGIN || b->pos.y > SCREEN_HEIGHT + BULLET_OUT_MARGIN)
		{
			b->active = false;
			continue;
		}
	}
}

void DrawBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		BulletData* b = &g_BulletData[i];
		if (!b->active) continue;

		if (g_BulletHandle != -1)
		{
			// 画像は右向き前提。進行方向に合わせて回転する
			double angle = atan2((double)b->velocity.y, (double)b->velocity.x);
			DrawRotaGraphF(b->pos.x, b->pos.y, 1.0f, angle, g_BulletHandle, TRUE);
		}
		else
		{
			// 画像が読み込めていない時の代用
			DrawCircleAA(b->pos.x, b->pos.y, b->radius, 16, GetColor(255, 220, 80), TRUE);
		}
	}
}

void FinBullet()
{
	if (g_BulletHandle != -1)
	{
		DeleteGraph(g_BulletHandle);
		g_BulletHandle = -1;
	}
}

void FireBullet(BulletOwner owner, VECTOR pos, VECTOR dir, float speed, float life, float radius, int damage)
{
	// 空いている枠を探して使う（全部埋まっていたら発射しない）
	for (int i = 0; i < BULLET_MAX; i++)
	{
		BulletData* b = &g_BulletData[i];
		if (b->active) continue;

		b->active = true;
		b->owner = owner;
		b->pos = pos;
		b->prevPos = pos;
		b->velocity = VScale(dir, speed);
		b->life = life;
		b->radius = radius;
		b->damage = damage;
		return;
	}
}

BulletData* GetBullet(int index)
{
	if (index < 0 || index >= BULLET_MAX) return nullptr;
	return &g_BulletData[index];
}