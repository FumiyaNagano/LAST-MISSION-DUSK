#include "DxLib.h"
#include "EnemyManager.h"
#include "../FPS/FPS.h"
#include "../Bullet/BulletManager.h"
#include "../Collision/Collision.h"
#include "../GameSetting/GameSetting.h"

// 画面外に出てから消すまでの余白（ピクセル）
#define ENEMY_OUT_MARGIN		(200.0f)

// 被弾時の白点滅の長さ（秒）
#define ENEMY_HIT_FLASH_TIME	(0.1f)

// 敵の種類ごとの設定
struct EnemyDef
{
	float width;
	float height;
	int hp;
	float speed;			// 左へ進む速さ（ピクセル/秒）。0なら動かない
	int contactDamage;
	WeaponType weapon;
};

static const EnemyDef g_EnemyDefs[ENEMY_TYPE_NUM] =
{
	//  幅      高さ    HP  速度  接触ダメ 武器
	{ 46.0f, 100.0f, 30, 0.0f, 10, WEAPON_PISTOL },	// ソルジャー
};

static EnemyData g_EnemyData[ENEMY_MAX] = {};

// 敵を倒したときの処理
static void KillEnemy(EnemyData* e)
{
	e->active = false;

	// TODO: 次のステップで、ここから e->gun.type の武器をドロップする
}

void InitEnemy()
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_EnemyData[i].active = false;
	}
}

void LoadEnemy()
{
	// TODO: 敵の画像ができたらここで読み込む
}

void StartEnemy()
{
	InitEnemy();

	// 動作確認用に、右側に3体出現させる（シーンに出現処理を作ったら削除）
	SpawnEnemy(ENEMY_TYPE_SOLDIER, 1400.0f, 200.0f);
	SpawnEnemy(ENEMY_TYPE_SOLDIER, 1400.0f, 450.0f);
	SpawnEnemy(ENEMY_TYPE_SOLDIER, 1400.0f, 700.0f);
}

void UpdateEnemy()
{
	float dt = GetDeltaTime();

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		EnemyData* e = &g_EnemyData[i];
		if (!e->active) continue;

		if (e->hitFlashTimer > 0.0f)
		{
			e->hitFlashTimer -= dt;
		}

		// 発射間隔とリロードを進める（撃つ処理は次のステップで追加）
		WeaponUpdate(&e->gun, dt);

		// 移動 = 速度(px/秒) * 経過時間(秒)
		e->pos = VAdd(e->pos, VScale(e->velocity, dt));

		// 画面外へ出たら消す
		if (e->pos.x + e->width < -ENEMY_OUT_MARGIN ||
			e->pos.x > SCREEN_WIDTH + ENEMY_OUT_MARGIN ||
			e->pos.y + e->height < -ENEMY_OUT_MARGIN ||
			e->pos.y > SCREEN_HEIGHT + ENEMY_OUT_MARGIN)
		{
			e->active = false;
		}
	}
}

void DrawEnemy()
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		EnemyData* e = &g_EnemyData[i];
		if (!e->active) continue;

		int x1 = (int)e->pos.x;
		int y1 = (int)e->pos.y;
		int x2 = (int)(e->pos.x + e->width);
		int y2 = (int)(e->pos.y + e->height);

		// TODO: 画像ができたらここを置き換える（今は仮の四角）
		unsigned int color = (e->hitFlashTimer > 0.0f) ? GetColor(255, 255, 255) : GetColor(200, 60, 60);
		DrawBox(x1, y1, x2, y2, color, TRUE);

		// HPバー
		float ratio = (float)e->hp / (float)e->maxHp;
		if (ratio < 0.0f) ratio = 0.0f;
		DrawBox(x1, y1 - 10, x2, y1 - 5, GetColor(60, 60, 60), TRUE);
		DrawBox(x1, y1 - 10, x1 + (int)(e->width * ratio), y1 - 5, GetColor(80, 255, 80), TRUE);
	}
}

void FinEnemy()
{
	// TODO: 画像を読み込んだら、ここで解放する
}

EnemyData* SpawnEnemy(EnemyType type, float x, float y)
{
	if (type < 0 || type >= ENEMY_TYPE_NUM) return nullptr;
	const EnemyDef* def = &g_EnemyDefs[type];

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		EnemyData* e = &g_EnemyData[i];
		if (e->active) continue;

		e->active = true;
		e->type = type;
		e->pos = VGet(x, y, 0.0f);
		e->velocity = VGet(-def->speed, 0.0f, 0.0f);
		e->width = def->width;
		e->height = def->height;
		e->hp = def->hp;
		e->maxHp = def->hp;
		e->contactDamage = def->contactDamage;
		// 敵は予備弾が無限（弾倉が空になったら、時間をかけてリロードするだけ）
		WeaponStateInit(&e->gun, def->weapon, GetWeaponDef(def->weapon)->magazineSize, WEAPON_AMMO_INFINITE);
		e->hitFlashTimer = 0.0f;
		return e;
	}

	return nullptr;
}

EnemyData* GetEnemy(int index)
{
	if (index < 0 || index >= ENEMY_MAX) return nullptr;
	return &g_EnemyData[index];
}

void EnemyHitPlayerBullet()
{
	for (int i = 0; i < BULLET_MAX; i++)
	{
		BulletData* b = GetBullet(i);
		if (b == nullptr || !b->active) continue;
		if (b->owner != BULLET_OWNER_PLAYER) continue;

		for (int j = 0; j < ENEMY_MAX; j++)
		{
			EnemyData* e = &g_EnemyData[j];
			if (!e->active) continue;

			// 弾が前フレームから進んだ線分と、敵の矩形で判定
			if (!CheckSegmentRect(b->prevPos, b->pos, b->radius,
				e->pos.x, e->pos.y, e->width, e->height))
			{
				continue;
			}

			// 命中：弾は消えて、敵のHPが減る
			b->active = false;
			e->hp -= b->damage;
			e->hitFlashTimer = ENEMY_HIT_FLASH_TIME;

			if (e->hp <= 0)
			{
				e->hp = 0;
				KillEnemy(e);
			}

			// 1発の弾が当たるのは1体まで
			break;
		}
	}
}