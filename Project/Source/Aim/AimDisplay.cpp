#include "DxLib.h"
#include "AimDisplay.h"
#include "../GameSetting/GameSetting.h"
#include "../Map/Block.h"
#include "../Enemy/EnemyManager.h"
#include <math.h>

// レーザー：終点を探すときの刻み幅（ピクセル）。ブロック（75px）より十分に小さくする
#define AIM_LASER_STEP			(10.0f)
// レーザーを、この長さごとに区切って、先へいくほど薄くする
#define AIM_LASER_SEGMENT_LEN	(100.0f)
#define AIM_LASER_ALPHA_START	(190)
#define AIM_LASER_ALPHA_MIN		(50)
#define AIM_LASER_ALPHA_STEP	(14)

// クロスヘア（ピクセル）
#define AIM_RING_RADIUS			(14.0f)
#define AIM_TICK_INNER			(8.0f)
#define AIM_TICK_OUTER			(22.0f)
#define AIM_DOT_RADIUS			(2.0f)

// リロードの進み具合を、クロスヘアの下に出すバー
#define AIM_RELOAD_BAR_WIDTH	(44)
#define AIM_RELOAD_BAR_HEIGHT	(5)
#define AIM_RELOAD_BAR_OFFSET_Y	(30)

VECTOR AimFindEnd(VECTOR origin, VECTOR dir, float probeSize)
{
	float maxLen = sqrtf((float)SCREEN_WIDTH * SCREEN_WIDTH + (float)SCREEN_HEIGHT * SCREEN_HEIGHT);
	VECTOR last = origin;

	for (float t = AIM_LASER_STEP; t <= maxLen; t += AIM_LASER_STEP)
	{
		VECTOR p = VGet(origin.x + dir.x * t, origin.y + dir.y * t, 0.0f);

		// 画面の端
		if (p.x < 0.0f || p.x > SCREEN_WIDTH || p.y < 0.0f || p.y > SCREEN_HEIGHT) break;

		// ブロック（弾と同じ大きさの矩形で調べる）
		if (CheckBlockCollision(p.x - probeSize * 0.5f, p.y - probeSize * 0.5f, probeSize, probeSize)) break;

		last = p;
	}
	return last;
}

void DrawAimLaser(VECTOR muzzle, VECTOR dir, float probeSize, bool dimmed)
{
	VECTOR end = AimFindEnd(muzzle, dir, probeSize);

	float dx = end.x - muzzle.x;
	float dy = end.y - muzzle.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len < 1.0f) return;

	unsigned int color = dimmed ? GetColor(170, 170, 170) : GetColor(255, 60, 60);

	// 先へいくほど薄くなるように、区切って描く
	int alpha = dimmed ? 90 : AIM_LASER_ALPHA_START;
	for (float start = 0.0f; start < len; start += AIM_LASER_SEGMENT_LEN)
	{
		float stop = start + AIM_LASER_SEGMENT_LEN;
		if (stop > len) stop = len;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawLineAA(muzzle.x + dx * (start / len), muzzle.y + dy * (start / len),
			muzzle.x + dx * (stop / len), muzzle.y + dy * (stop / len), color, 2.0f);

		alpha -= AIM_LASER_ALPHA_STEP;
		if (alpha < AIM_LASER_ALPHA_MIN) alpha = AIM_LASER_ALPHA_MIN;
	}

	// 終点（弾が壁や画面の端に当たる位置）に、小さな点
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, dimmed ? 110 : 220);
	DrawCircleAA(end.x, end.y, 4.0f, 16, color, TRUE);

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 敵の矩形の中に、点があるか
static bool IsOverEnemy(VECTOR p)
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		const EnemyData* e = GetEnemy(i);
		if (e == nullptr || !e->active) continue;

		if (p.x >= e->pos.x && p.x <= e->pos.x + e->width &&
			p.y >= e->pos.y && p.y <= e->pos.y + e->height)
		{
			return true;
		}
	}
	return false;
}

// 黒い縁取りつきの線（明るい背景でも暗い背景でも見えるようにする）
static void DrawLineOutlined(float x1, float y1, float x2, float y2, unsigned int color)
{
	DrawLineAA(x1, y1, x2, y2, GetColor(0, 0, 0), 4.0f);
	DrawLineAA(x1, y1, x2, y2, color, 2.0f);
}

void DrawCrosshair(VECTOR mouse, float reloadProgress)
{
	bool reloading = reloadProgress >= 0.0f;

	unsigned int color = GetColor(255, 255, 255);
	if (reloading)					color = GetColor(255, 220, 0);
	else if (IsOverEnemy(mouse))	color = GetColor(255, 60, 60);

	float x = mouse.x;
	float y = mouse.y;

	// 外側の円
	DrawCircleAA(x, y, AIM_RING_RADIUS, 32, GetColor(0, 0, 0), FALSE, 4.0f);
	DrawCircleAA(x, y, AIM_RING_RADIUS, 32, color, FALSE, 2.0f);

	// 上下左右の短い線
	DrawLineOutlined(x - AIM_TICK_OUTER, y, x - AIM_TICK_INNER, y, color);
	DrawLineOutlined(x + AIM_TICK_INNER, y, x + AIM_TICK_OUTER, y, color);
	DrawLineOutlined(x, y - AIM_TICK_OUTER, x, y - AIM_TICK_INNER, color);
	DrawLineOutlined(x, y + AIM_TICK_INNER, x, y + AIM_TICK_OUTER, color);

	// 中心の点
	DrawCircleAA(x, y, AIM_DOT_RADIUS + 1.5f, 12, GetColor(0, 0, 0), TRUE);
	DrawCircleAA(x, y, AIM_DOT_RADIUS, 12, color, TRUE);

	// リロード中は、進み具合のバーを下に出す
	if (reloading)
	{
		int bx = (int)x - AIM_RELOAD_BAR_WIDTH / 2;
		int by = (int)y + AIM_RELOAD_BAR_OFFSET_Y;
		int filled = (int)(AIM_RELOAD_BAR_WIDTH * (reloadProgress > 1.0f ? 1.0f : reloadProgress));

		DrawBox(bx - 1, by - 1, bx + AIM_RELOAD_BAR_WIDTH + 1, by + AIM_RELOAD_BAR_HEIGHT + 1, GetColor(0, 0, 0), TRUE);
		DrawBox(bx, by, bx + filled, by + AIM_RELOAD_BAR_HEIGHT, GetColor(255, 220, 0), TRUE);
	}
}