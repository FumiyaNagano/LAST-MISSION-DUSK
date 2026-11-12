#include "Weapon.h"

static const WeaponDef g_WeaponDefs[WEAPON_NUM] =
{
	//  名前         モード            間隔          弾速     寿命   半径  ダメ 弾倉 予備弾                  リロード SE
	{ "Pistol",    FIRE_MODE_SEMI, 0.25f,        1200.0f, 10.0f, 6.0f, 10,  12,  WEAPON_AMMO_INFINITE, 1.0f, SE_PLAYER_SHOT },	// 予備武器
	{ "Kar98k",    FIRE_MODE_SEMI, 1.0f,         1400.0f, 10.0f, 5.0f,  5,   5,  45,                   2.0f, SE_PLAYER_SHOT },	// 5発 x 10個
	{ "M1 Garand", FIRE_MODE_SEMI, 0.3f,         1300.0f, 10.0f, 5.0f,  5,   8,  72,                   3.0f, SE_PLAYER_SHOT },	// 8発 x 10個
	{ "MP5",       FIRE_MODE_AUTO, 1.0f / 13.0f,  900.0f, 10.0f, 4.0f,  3,  30, 120,                   2.0f, SE_PLAYER_SHOT },	// 30発 x 5個、13発/秒
	{ "M4A1",      FIRE_MODE_AUTO, 1.0f / 11.0f, 1000.0f, 10.0f, 5.0f,  4,  30, 120,                   3.0f, SE_PLAYER_SHOT },	// 30発 x 5個、11発/秒
	{ "XM250",     FIRE_MODE_AUTO, 0.1f,         1100.0f, 10.0f, 6.0f,  7, 100, 200,                   5.0f, SE_PLAYER_SHOT },	// 100発 x 3個、10発/秒
};

const WeaponDef* GetWeaponDef(WeaponType type)
{
	if (type < 0 || type >= WEAPON_NUM)
	{
		return &g_WeaponDefs[WEAPON_DEFAULT];
	}
	return &g_WeaponDefs[type];
}

static int ClampInt(int value, int minValue, int maxValue)
{
	if (value < minValue) return minValue;
	if (value > maxValue) return maxValue;
	return value;
}

void WeaponStateInit(WeaponState* s, WeaponType type, int magazine, int reserve)
{
	if (type < 0 || type >= WEAPON_NUM)
	{
		type = WEAPON_DEFAULT;
	}
	const WeaponDef* def = GetWeaponDef(type);

	s->type = type;
	s->magazine = ClampInt(magazine, 0, def->magazineSize);

	if (def->reserveMax == WEAPON_AMMO_INFINITE || reserve == WEAPON_AMMO_INFINITE)
	{
		s->reserve = WEAPON_AMMO_INFINITE;
	}
	else
	{
		s->reserve = ClampInt(reserve, 0, def->reserveMax);
	}

	s->reloadTimer = 0.0f;
	s->intervalTimer = 0.0f;
}

void WeaponStateInitFull(WeaponState* s, WeaponType type)
{
	const WeaponDef* def = GetWeaponDef(type);
	WeaponStateInit(s, type, def->magazineSize, def->reserveMax);
}

// リロード完了：予備弾から弾倉へ補充する
static void FinishReload(WeaponState* s)
{
	const WeaponDef* def = GetWeaponDef(s->type);
	int need = def->magazineSize - s->magazine;

	if (s->reserve == WEAPON_AMMO_INFINITE)
	{
		s->magazine += need;
	}
	else
	{
		int take = (need < s->reserve) ? need : s->reserve;
		s->magazine += take;
		s->reserve -= take;
	}
}

bool WeaponStartReload(WeaponState* s)
{
	const WeaponDef* def = GetWeaponDef(s->type);

	if (s->reloadTimer > 0.0f) return false;				// すでにリロード中
	if (s->magazine >= def->magazineSize) return false;		// 弾倉が満タン
	if (s->reserve == 0) return false;						// 予備弾がない

	if (def->reloadTime <= 0.0f)
	{
		FinishReload(s);	// 時間0の武器は即座に完了
		return true;
	}

	s->reloadTimer = def->reloadTime;
	return true;
}

void WeaponUpdate(WeaponState* s, float dt)
{
	// 発射間隔。端数の持ち越しは最大1フレーム分まで（リロード中に溜まって連射しすぎないように）
	s->intervalTimer -= dt;
	if (s->intervalTimer < -dt)
	{
		s->intervalTimer = -dt;
	}

	// リロードを進める
	if (s->reloadTimer > 0.0f)
	{
		s->reloadTimer -= dt;
		if (s->reloadTimer <= 0.0f)
		{
			s->reloadTimer = 0.0f;
			FinishReload(s);
		}
	}

	// 弾倉が空なら自動でリロード
	if (s->magazine <= 0)
	{
		WeaponStartReload(s);
	}
}

bool WeaponCanFire(const WeaponState* s)
{
	return s->reloadTimer <= 0.0f && s->magazine > 0 && s->intervalTimer <= 0.0f;
}

bool WeaponTryFire(WeaponState* s)
{
	if (!WeaponCanFire(s)) return false;

	s->magazine--;
	s->intervalTimer += GetWeaponDef(s->type)->interval;
	return true;
}

bool WeaponIsReloading(const WeaponState* s)
{
	return s->reloadTimer > 0.0f;
}

bool WeaponIsOutOfAmmo(const WeaponState* s)
{
	return s->magazine <= 0 && s->reserve == 0;
}

float WeaponReloadProgress(const WeaponState* s)
{
	const WeaponDef* def = GetWeaponDef(s->type);
	if (s->reloadTimer <= 0.0f || def->reloadTime <= 0.0f) return 0.0f;
	return 1.0f - s->reloadTimer / def->reloadTime;
}