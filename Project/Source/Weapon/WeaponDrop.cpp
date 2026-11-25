#include "DxLib.h"
#include "WeaponDrop.h"
#include "../FPS/FPS.h"
#include "../Collision/Collision.h"
#include "../GameSetting/GameSetting.h"
#include "../Player/Player.h"
#include <math.h>

// 画像の読み込み状況を、画面の左側に表示する確認用スイッチ（確認できたら0にする）
#define WEAPON_DROP_DEBUG_DISPLAY	(0)

// 画像がない武器の、見た目と判定の大きさ（仮の四角）。画像がある武器も、これより小さくはならない
#define WEAPON_DROP_WIDTH		(64.0f)
#define WEAPON_DROP_HEIGHT		(32.0f)

// 武器画像を表示するときの倍率（全武器共通。武器どうしの大きさの比率は、画像のまま保たれる）
// 画像の大きさを見ながら調整する。1.0で原寸
#define WEAPON_DROP_IMAGE_SCALE	(0.6f)

// 地面に残る時間（秒）
#define WEAPON_DROP_LIFE		(20.0f)
// 消える前に点滅を始める残り時間（秒）
#define WEAPON_DROP_BLINK_TIME	(5.0f)

// 画面の端から、これだけ内側に収める（画面外で倒した敵のドロップを拾えるようにする）
#define WEAPON_DROP_SCREEN_MARGIN	(40.0f)

// 上下にふわふわ揺らす大きさ（ピクセル）と速さ
#define WEAPON_DROP_BOB_HEIGHT	(3.0f)
#define WEAPON_DROP_BOB_SPEED	(4.0f)

// ドロップした武器の画像。nullptrなら仮の四角で表示
static const char* g_DropImagePaths[WEAPON_NUM] =
{
	nullptr,							// ピストル（予備武器なので落とさない）
	"Data/Weapon/Kar98k.png",		// Kar98k
	"Data/Weapon/M1Garand.png",	// M1 Garand
	"Data/Weapon/MP5.png",			// MP5
	"Data/Weapon/M4A1.png",		// M4A1
	"Data/Weapon/XM250.png",		// XM250
};

// 画像がない武器の、仮表示の色
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

// ---- 回復アイテム ----
// 見た目と判定の大きさ（画像がない間は、仮の四角と十字）
#define HEAL_DROP_WIDTH			(40.0f)
#define HEAL_DROP_HEIGHT		(40.0f)
// 画像の倍率
#define HEAL_DROP_IMAGE_SCALE	(1.0f)

// 回復アイテムの画像。実際のファイル名に合わせて書き換える（nullptrなら、仮の四角と十字で表示）
static const char* g_HealImagePath = "Data/Weapon/MedicalKit.png";	// 例: "Data/Item/HealItem.png"

static HealDropData g_HealDrops[HEAL_DROP_MAX] = {};
static bool  g_HealHasImage = false;
static int   g_HealImage = 0;
static float g_HealImageW = 0.0f;
static float g_HealImageH = 0.0f;

// 拾える範囲の大きさ（画像があれば画像の大きさ、なければ仮の四角。最小は仮の四角と同じ）
static void GetHealSize(float* w, float* h)
{
	*w = HEAL_DROP_WIDTH;
	*h = HEAL_DROP_HEIGHT;
	if (g_HealHasImage)
	{
		if (g_HealImageW > *w) *w = g_HealImageW;
		if (g_HealImageH > *h) *h = g_HealImageH;
	}
}

// LoadWeaponDrop が呼ばれた回数（確認用）
static int g_DropLoadCallCount = 0;

// 画像の情報（武器ごと）
static bool  g_DropHasImage[WEAPON_NUM] = {};	// 画像を読み込めたか
static int   g_DropImage[WEAPON_NUM] = {};
static float g_DropScale[WEAPON_NUM] = {};		// 表示する倍率
static float g_DropImageW[WEAPON_NUM] = {};		// 縮小後の画像の幅
static float g_DropImageH[WEAPON_NUM] = {};		// 縮小後の画像の高さ

// 拾える範囲の大きさ（画像があれば画像の大きさ、なければ仮の四角。最小は仮の四角と同じ）
static void GetDropSize(WeaponType type, float* w, float* h)
{
	*w = WEAPON_DROP_WIDTH;
	*h = WEAPON_DROP_HEIGHT;

	if (g_DropHasImage[type])
	{
		if (g_DropImageW[type] > *w) *w = g_DropImageW[type];
		if (g_DropImageH[type] > *h) *h = g_DropImageH[type];
	}
}

void InitWeaponDrop()
{
	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		g_Drops[i].active = false;
	}
	for (int i = 0; i < HEAL_DROP_MAX; i++)
	{
		g_HealDrops[i].active = false;
	}
}

void LoadWeaponDrop()
{
	g_DropLoadCallCount++;

	for (int t = 0; t < WEAPON_NUM; t++)
	{
		g_DropHasImage[t] = false;
		if (g_DropImagePaths[t] == nullptr) continue;

		int handle = LoadGraph(g_DropImagePaths[t]);
		if (handle == -1)
		{
			printfDx("画像の読み込みに失敗: %s\n", g_DropImagePaths[t]);
			continue;
		}

		int w = 0, h = 0;
		GetGraphSize(handle, &w, &h);

		float scale = WEAPON_DROP_IMAGE_SCALE;

		g_DropImage[t] = handle;
		g_DropScale[t] = scale;
		g_DropImageW[t] = (float)w * scale;
		g_DropImageH[t] = (float)h * scale;
		g_DropHasImage[t] = true;
	}

	// 回復アイテムの画像
	g_HealHasImage = false;
	if (g_HealImagePath != nullptr)
	{
		int handle = LoadGraph(g_HealImagePath);
		if (handle == -1)
		{
			printfDx("画像の読み込みに失敗: %s\n", g_HealImagePath);
		}
		else
		{
			int w = 0, h = 0;
			GetGraphSize(handle, &w, &h);
			g_HealImage = handle;
			g_HealImageW = (float)w * HEAL_DROP_IMAGE_SCALE;
			g_HealImageH = (float)h * HEAL_DROP_IMAGE_SCALE;
			g_HealHasImage = true;
		}
	}
}

void StartWeaponDrop()
{
	InitWeaponDrop();
}

void FinWeaponDrop()
{
	for (int t = 0; t < WEAPON_NUM; t++)
	{
		if (g_DropHasImage[t])
		{
			DeleteGraph(g_DropImage[t]);
			g_DropHasImage[t] = false;
		}
	}

	if (g_HealHasImage)
	{
		DeleteGraph(g_HealImage);
		g_HealHasImage = false;
	}
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

HealDropData* SpawnHealDrop(int amount, VECTOR pos)
{
	// 空きを探す。なければ、残り時間が最も短いものを使う
	HealDropData* slot = nullptr;
	for (int i = 0; i < HEAL_DROP_MAX; i++)
	{
		if (!g_HealDrops[i].active)
		{
			slot = &g_HealDrops[i];
			break;
		}
		if (slot == nullptr || g_HealDrops[i].life < slot->life)
		{
			slot = &g_HealDrops[i];
		}
	}

	// 画面の外に落ちないように、位置を画面の内側へ収める
	if (pos.x < WEAPON_DROP_SCREEN_MARGIN) pos.x = WEAPON_DROP_SCREEN_MARGIN;
	if (pos.x > SCREEN_WIDTH - WEAPON_DROP_SCREEN_MARGIN) pos.x = SCREEN_WIDTH - WEAPON_DROP_SCREEN_MARGIN;
	if (pos.y < WEAPON_DROP_SCREEN_MARGIN) pos.y = WEAPON_DROP_SCREEN_MARGIN;
	if (pos.y > SCREEN_HEIGHT - WEAPON_DROP_SCREEN_MARGIN) pos.y = SCREEN_HEIGHT - WEAPON_DROP_SCREEN_MARGIN;

	slot->active = true;
	slot->pos = pos;
	slot->amount = amount;
	slot->life = WEAPON_DROP_LIFE;
	slot->age = 0.0f;
	return slot;
}

HealDropData* GetHealDrop(int index)
{
	if (index < 0 || index >= HEAL_DROP_MAX) return nullptr;
	return &g_HealDrops[index];
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
		float w = 0.0f, h = 0.0f;
		GetDropSize(d->type, &w, &h);
		bool touching = p->active && CheckRectRect(
			p->pos.x, p->pos.y, PLAYER_WIDTH, PLAYER_HEIGHT,
			d->pos.x - w * 0.5f, d->pos.y - h * 0.5f, w, h);

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

	// ---- 回復アイテム ----
	for (int i = 0; i < HEAL_DROP_MAX; i++)
	{
		HealDropData* d = &g_HealDrops[i];
		if (!d->active) continue;

		d->age += dt;
		d->life -= dt;
		if (d->life <= 0.0f)
		{
			d->active = false;
			continue;
		}

		float w = 0.0f, h = 0.0f;
		GetHealSize(&w, &h);
		bool touching = p->active && CheckRectRect(
			p->pos.x, p->pos.y, PLAYER_WIDTH, PLAYER_HEIGHT,
			d->pos.x - w * 0.5f, d->pos.y - h * 0.5f, w, h);

		if (!touching) continue;

		// 拾う。HPが満タンのときは拾えず、地面に残る（あとで減ったときに拾える）
		if (PlayerHeal(d->amount))
		{
			d->active = false;
		}
	}
}

void DrawWeaponDrop()
{
#if WEAPON_DROP_DEBUG_DISPLAY
	DrawFormatString(0, 240, GetColor(255, 255, 0), "WeaponDrop load calls: %d", g_DropLoadCallCount);
	for (int t = 1; t < WEAPON_NUM; t++)
	{
		DrawFormatString(0, 240 + t * 20, GetColor(255, 255, 0), "%s image: %s  (%s)",
			GetWeaponDef((WeaponType)t)->name,
			g_DropHasImage[t] ? "OK" : "NONE",
			g_DropImagePaths[t] ? g_DropImagePaths[t] : "no path");
	}
#endif

	for (int i = 0; i < WEAPON_DROP_MAX; i++)
	{
		const WeaponDropData* d = &g_Drops[i];
		if (!d->active) continue;

		// 消える直前は点滅
		if (d->life < WEAPON_DROP_BLINK_TIME && ((int)(d->life * 8.0f)) % 2 == 0) continue;

		float w = 0.0f, h = 0.0f;
		GetDropSize(d->type, &w, &h);

		float bob = sinf(d->age * WEAPON_DROP_BOB_SPEED) * WEAPON_DROP_BOB_HEIGHT;
		float cx = d->pos.x;
		float cy = d->pos.y + bob;

		if (g_DropHasImage[d->type])
		{
			// 武器の画像（中心を合わせて描く）
			DrawRotaGraphF(cx, cy, (double)g_DropScale[d->type], 0.0, g_DropImage[d->type], TRUE);
		}
		else
		{
			// 画像がない武器は、色つきの四角で代用
			int x1 = (int)(cx - w * 0.5f);
			int y1 = (int)(cy - h * 0.5f);
			const int* c = g_DropColors[d->type];
			DrawBox(x1, y1, x1 + (int)w, y1 + (int)h, GetColor(c[0], c[1], c[2]), TRUE);
			DrawBox(x1, y1, x1 + (int)w, y1 + (int)h, GetColor(255, 255, 255), FALSE);
		}

		// 武器名（上）と、残りの総弾数（下）
		int tx = (int)(cx - w * 0.5f);
		DrawString(tx, (int)(cy - h * 0.5f) - 18, GetWeaponDef(d->type)->name, GetColor(255, 255, 255));
		DrawFormatString(tx, (int)(cy + h * 0.5f) + 2, GetColor(255, 255, 255), "%d", d->magazine + d->reserve);
	}

	// ---- 回復アイテム ----
	for (int i = 0; i < HEAL_DROP_MAX; i++)
	{
		const HealDropData* d = &g_HealDrops[i];
		if (!d->active) continue;

		// 消える直前は点滅
		if (d->life < WEAPON_DROP_BLINK_TIME && ((int)(d->life * 8.0f)) % 2 == 0) continue;

		float w = 0.0f, h = 0.0f;
		GetHealSize(&w, &h);

		float bob = sinf(d->age * WEAPON_DROP_BOB_SPEED) * WEAPON_DROP_BOB_HEIGHT;
		float cx = d->pos.x;
		float cy = d->pos.y + bob;

		if (g_HealHasImage)
		{
			DrawRotaGraphF(cx, cy, (double)HEAL_DROP_IMAGE_SCALE, 0.0, g_HealImage, TRUE);
		}
		else
		{
			// 画像がないときは、緑の四角に白い十字で代用
			int x1 = (int)(cx - w * 0.5f);
			int y1 = (int)(cy - h * 0.5f);
			int mx = (int)cx;
			int my = (int)cy;
			DrawBox(x1, y1, x1 + (int)w, y1 + (int)h, GetColor(40, 160, 70), TRUE);
			DrawBox(x1, y1, x1 + (int)w, y1 + (int)h, GetColor(255, 255, 255), FALSE);
			DrawBox(mx - 3, my - 12, mx + 3, my + 12, GetColor(255, 255, 255), TRUE);
			DrawBox(mx - 12, my - 3, mx + 12, my + 3, GetColor(255, 255, 255), TRUE);
		}

		// 回復量
		DrawFormatString((int)(cx - w * 0.5f), (int)(cy - h * 0.5f) - 18, GetColor(180, 255, 180), "+%d", d->amount);
	}
}

bool WeaponDropHasImage(WeaponType type)
{
	if (type < 0 || type >= WEAPON_NUM) return false;
	return g_DropHasImage[type];
}