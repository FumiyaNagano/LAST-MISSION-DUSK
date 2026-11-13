#include "DxLib.h"
#include "WeaponDrop.h"
#include "../FPS/FPS.h"
#include "../Collision/Collision.h"
#include "../GameSetting/GameSetting.h"
#include "../Player/Player.h"
#include <math.h>

// 見た目と判定の大きさ
#define WEAPON_DROP_WIDTH		(64.0f)
#define WEAPON_DROP_HEIGHT		(32.0f)

// 地面に残る時間（秒）
#define WEAPON_DROP_LIFE		(20.0f)
// 消える前に点滅を始める残り時間（秒）
#define WEAPON_DROP_BLINK_TIME	(5.0f)

// 画面の端から、これだけ内側に収める（画面外で倒した敵のドロップを拾えるようにする）
#define WEAPON_DROP_SCREEN_MARGIN	(40.0f)

// 上下にふわふわ揺らす大きさ（ピクセル）と速さ
#define WEAPON_DROP_BOB_HEIGHT	(3.0f)
#define WEAPON_DROP_BOB_SPEED	(4.0f)

// 仮表示の色（武器ごと）。画像ができたら置き換える
static const int g_DropColors[WEAPON_NUM][3] =
{
	{ 150, 150, 150 },	// ピストル
	{ 150, 100,  50 },	// Kar98k
	{ 120, 140,  70 },	// M1 Garand
	{ 130, 130, 140 },	// MP5
	{  70, 100, 160 },	// M4A1
	{ 190,  60,  60 },	// XM250
};

static WeaponDropData g_Drops[WEAPON_DROP_MAX] = {};

void InitWeaponDrop()
{
	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		g_Drops[i].active = false;
	}
}

void StartWeaponDrop()
{
	InitWeaponDrop();
}

WeaponDropData* SpawnWeaponDrop(WeaponType type, int magazine, int reserve, VECTOR pos, bool pickable)
{
	if (type < 0 || type >= WEAPON_NUM) return nullptr;

	// 空きを探す。なければ、残り時間が最も短いものを使う
	WeaponDropData* slot = nullptr;
	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		if (!g_Drops[i].active)
		{
			slot = &g_Drops[i];
			break;
		}
		if (slot == nullptr || g_Drops[i].life < slot->life)
		{
			slot = &g_Drops[i];
		}
	}

	// 画面の外に落ちないように、位置を画面の内側へ収める
	if (pos.x < WEAPON_DROP_SCREEN_MARGIN) pos.x = WEAPON_DROP_SCREEN_MARGIN;
	if (pos.x > SCREEN_WIDTH - WEAPON_DROP_SCREEN_MARGIN) pos.x = SCREEN_WIDTH - WEAPON_DROP_SCREEN_MARGIN;
	if (pos.y < WEAPON_DROP_SCREEN_MARGIN) pos.y = WEAPON_DROP_SCREEN_MARGIN;
	if (pos.y > SCREEN_HEIGHT - WEAPON_DROP_SCREEN_MARGIN) pos.y = SCREEN_HEIGHT - WEAPON_DROP_SCREEN_MARGIN;

	slot->active = true;
	slot->type = type;
	slot->magazine = magazine;
	slot->reserve = reserve;
	slot->pos = pos;
	slot->life = WEAPON_DROP_LIFE;
	slot->age = 0.0f;
	slot->pickable = pickable;
	return slot;
}

WeaponDropData* GetWeaponDrop(int index)
{
	if (index < 0 || index >= WEAPON_DROP_MAX) return nullptr;
	return &g_Drops[index];
}

void UpdateWeaponDrop()
{
	float dt = GetDeltaTime();
	const PlayerData* p = GetPlayer();

	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		WeaponDropData* d = &g_Drops[i];
		if (!d->active) continue;

		d->age += dt;
		d->life -= dt;
		if (d->life <= 0.0f)
		{
			d->active = false;
			continue;
		}

		// プレイヤーの当たり判定矩形と、ドロップの矩形が重なっているか
		bool touching = p->active && CheckRectRect(
			p->pos.x, p->pos.y, PLAYER_WIDTH, PLAYER_HEIGHT,
			d->pos.x - WEAPON_DROP_WIDTH * 0.5f, d->pos.y - WEAPON_DROP_HEIGHT * 0.5f,
			WEAPON_DROP_WIDTH, WEAPON_DROP_HEIGHT);

		// 捨てた直後のドロップは、プレイヤーが一度離れるまで拾えない
		if (!d->pickable)
		{
			if (!touching) d->pickable = true;
			continue;
		}

		if (!touching) continue;

		// 拾う。拾う処理の中で今の武器が捨てられ、同じ配列に新しいドロップが作られるため、
		// 先にこの枠を空けてから呼び、拾えなかった場合は元に戻す
		WeaponDropData copy = *d;
		d->active = false;
		if (!PlayerPickupWeapon(copy.type, copy.magazine, copy.reserve))
		{
			*d = copy;
		}
	}
}

void DrawWeaponDrop()
{
	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		const WeaponDropData* d = &g_Drops[i];
		if (!d->active) continue;

		// 消える直前は点滅
		if (d->life < WEAPON_DROP_BLINK_TIME && ((int)(d->life * 8.0f)) % 2 == 0) continue;

		float bob = sinf(d->age * WEAPON_DROP_BOB_SPEED) * WEAPON_DROP_BOB_HEIGHT;
		int x1 = (int)(d->pos.x - WEAPON_DROP_WIDTH * 0.5f);
		int y1 = (int)(d->pos.y - WEAPON_DROP_HEIGHT * 0.5f + bob);
		int x2 = x1 + (int)WEAPON_DROP_WIDTH;
		int y2 = y1 + (int)WEAPON_DROP_HEIGHT;

		// TODO: 画像ができたらここを置き換える（今は仮の四角）
		const int* c = g_DropColors[d->type];
		DrawBox(x1, y1, x2, y2, GetColor(c[0], c[1], c[2]), TRUE);
		DrawBox(x1, y1, x2, y2, GetColor(255, 255, 255), FALSE);

		// 武器名と、残りの総弾数
		DrawString(x1, y1 - 18, GetWeaponDef(d->type)->name, GetColor(255, 255, 255));
		DrawFormatString(x1 + 4, y1 + 8, GetColor(255, 255, 255), "%d", d->magazine + d->reserve);
	}
}