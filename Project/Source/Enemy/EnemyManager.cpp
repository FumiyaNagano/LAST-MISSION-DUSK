#include "DxLib.h"
#include "EnemyManager.h"
#include "../FPS/FPS.h"
#include "../Bullet/BulletManager.h"
#include "../Collision/Collision.h"
#include "../GameSetting/GameSetting.h"
#include "../Player/Player.h"
#include "../Map/Block.h"
#include "../Weapon/WeaponDrop.h"
#include <math.h>

// 配列の要素数
#define ARRAY_COUNT(a) ((int)(sizeof(a) / sizeof((a)[0])))

// 画面外に出てから消すまでの余白（ピクセル）
#define ENEMY_OUT_MARGIN		(200.0f)

// 倒したときにドロップする予備弾の割合（1.0なら、その武器の予備弾が満タン）
#define ENEMY_DROP_RESERVE_RATIO	(1.0f)

// 被弾時の白点滅の長さ（秒）
#define ENEMY_HIT_FLASH_TIME	(0.1f)

// 射撃アニメーションを表示する時間（秒）
#define ENEMY_SHOOT_ANIM_TIME	(0.2f)

// 出現関連
#define ENEMY_SPAWN_INTERVAL	(5.0f)		// 出現の間隔（秒）
#define ENEMY_ALIVE_MAX			(8)			// 同時に存在させる敵の上限
#define ENEMY_SPAWN_MARGIN		(80.0f)		// 画面の外側にどれだけ離して出現させるか
#define ENEMY_FIRST_SPAWN_TIME	(1.0f)		// 開始から最初の出現までの時間（秒）

// 敵の画像が元々左向きかどうか（falseなら右向きの画像。向きが違うときだけ左右反転して描画する）
#define ENEMY_IMAGE_FACING_LEFT	(false)

// 銃口の位置（敵の矩形の左上・中心からの距離。右向きを基準に、左向きは左右反転）
#define ENEMY_MUZZLE_OFFSET_X	(40.0f)
#define ENEMY_MUZZLE_OFFSET_Y	(35.0f)

// 敵の種類ごとの設定
struct EnemyDef
{
	float width;
	float height;
	int hp;
	float moveSpeed;		// 接近する速さ（ピクセル/秒）
	float stopDistance;		// この距離まで近づいたら止まる（ピクセル）
	float aimSpreadDeg;		// 照準のばらつき（度）。0なら正確に狙う
	int contactDamage;
};

static const EnemyDef g_EnemyDefs[ENEMY_TYPE_NUM] =
{
	//  幅      高さ    HP  速度   停止距離 ばらつき 接触ダメ
	{ 46.0f, 100.0f, 30, 100.0f, 300.0f, 5.0f, 10 },	// ソルジャー
};

// 経過時間に応じて、出現する敵が持つ武器を段階的に強くする
struct EnemyWeaponStage
{
	float startTime;		// ゲーム開始からの経過時間（秒）がこれ以上で有効
	WeaponType weapon;
};

static const EnemyWeaponStage g_WeaponStages[] =
{
	//  開始時間  武器
	{   0.0f, WEAPON_KAR98K },
	{  60.0f, WEAPON_M1_GARAND },
	{ 120.0f, WEAPON_MP5 },
	{ 180.0f, WEAPON_M4A1 },
	{ 240.0f, WEAPON_XM250 },
};

static EnemyData g_EnemyData[ENEMY_MAX] = {};

// 画像データ（武器 x 状態）
static AnimData g_EnemyAnim[WEAPON_NUM][ENEMY_STATE_NUM] = {};

// ゲーム開始からの経過時間（秒）
static float g_EnemyElapsedTime = 0.0f;
// 次の出現までの時間（秒）
static float g_EnemySpawnTimer = 0.0f;

// ------------------------------------------------------------------------------------------------
// 補助関数
// ------------------------------------------------------------------------------------------------

// 経過時間に応じた、出現時の武器を返す
static WeaponType GetStageWeapon(float elapsed)
{
	WeaponType weapon = g_WeaponStages[0].weapon;
	for (int i = 0; i < ARRAY_COUNT(g_WeaponStages); i++)
	{
		if (elapsed >= g_WeaponStages[i].startTime)
		{
			weapon = g_WeaponStages[i].weapon;
		}
	}
	return weapon;
}

// 武器と状態に対応する画像を返す。未登録なら、Kar98kの同じ状態→待機の順で代用する
static const AnimData* GetEnemyAnim(WeaponType weapon, EnemyState state)
{
	if (weapon < 0 || weapon >= WEAPON_NUM) weapon = WEAPON_KAR98K;

	if (IsAnimLoaded(&g_EnemyAnim[weapon][state])) return &g_EnemyAnim[weapon][state];
	if (IsAnimLoaded(&g_EnemyAnim[WEAPON_KAR98K][state])) return &g_EnemyAnim[WEAPON_KAR98K][state];
	if (IsAnimLoaded(&g_EnemyAnim[WEAPON_KAR98K][ENEMY_STATE_IDLE])) return &g_EnemyAnim[WEAPON_KAR98K][ENEMY_STATE_IDLE];
	return nullptr;
}

static void SetEnemyState(EnemyData* e, EnemyState state)
{
	if (e->state == state) return;

	e->state = state;
	ResetAnimPlayer(&e->anim);
}

static int CountAliveEnemies()
{
	int count = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (g_EnemyData[i].active) count++;
	}
	return count;
}

// 敵を倒したときの処理
static void KillEnemy(EnemyData* e)
{
	e->active = false;

	// 持っていた武器をその場にドロップする（予備弾が無限の武器は、落としても意味がないので落とさない）
	WeaponType type = e->gun.type;
	if (WeaponCanDrop(type))
	{
		const WeaponDef* def = GetWeaponDef(type);
		int reserve = (int)((float)def->reserveMax * ENEMY_DROP_RESERVE_RATIO);
		VECTOR center = VGet(e->pos.x + e->width * 0.5f, e->pos.y + e->height * 0.5f, 0.0f);
		SpawnWeaponDrop(type, def->magazineSize, reserve, center, true);
	}
}

// 画面の外周のどこかから、経過時間に応じた武器を持つ敵を出現させる
static void SpawnEnemyAtEdge()
{
	const EnemyDef* def = &g_EnemyDefs[ENEMY_TYPE_SOLDIER];

	int maxX = (int)(SCREEN_WIDTH - def->width);
	int maxY = (int)(SCREEN_HEIGHT - def->height);
	float x = 0.0f;
	float y = 0.0f;

	switch (GetRand(3))
	{
	case 0:	// 左
		x = -def->width - ENEMY_SPAWN_MARGIN;
		y = (float)GetRand(maxY);
		break;
	case 1:	// 右
		x = SCREEN_WIDTH + ENEMY_SPAWN_MARGIN;
		y = (float)GetRand(maxY);
		break;
	case 2:	// 上
		x = (float)GetRand(maxX);
		y = -def->height - ENEMY_SPAWN_MARGIN;
		break;
	default:	// 下
		x = (float)GetRand(maxX);
		y = SCREEN_HEIGHT + ENEMY_SPAWN_MARGIN;
		break;
	}

	SpawnEnemy(ENEMY_TYPE_SOLDIER, GetStageWeapon(g_EnemyElapsedTime), x, y);
}

// 敵が弾を1発撃つ（弾数の消費は呼び出し前に済んでいる）
static void FireEnemyBullet(const EnemyData* e, VECTOR target)
{
	const WeaponDef* def = GetWeaponDef(e->gun.type);

	float side = e->facingLeft ? -1.0f : 1.0f;
	VECTOR muzzle = VGet(e->pos.x + e->width * 0.5f + side * ENEMY_MUZZLE_OFFSET_X,
		e->pos.y + ENEMY_MUZZLE_OFFSET_Y, 0.0f);

	VECTOR dir = VSub(target, muzzle);
	if (VSquareSize(dir) < 1.0f)
	{
		dir = VGet(side, 0.0f, 0.0f);
	}
	else
	{
		dir = VNorm(dir);
	}

	// 照準にばらつきを持たせる（-aimSpread 〜 +aimSpread の範囲で回転）
	float angle = e->aimSpread * ((float)GetRand(2000) / 1000.0f - 1.0f);
	float c = cosf(angle);
	float s = sinf(angle);
	dir = VGet(dir.x * c - dir.y * s, dir.x * s + dir.y * c, 0.0f);

	FireBullet(BULLET_OWNER_ENEMY, muzzle, dir,
		def->bulletSpeed, def->bulletLife, def->bulletRadius, def->damage);
}

// 敵の矩形がブロックに当たるか。画面の完全に外側にいるときは判定しない
// （画面外から入ってくる敵が、出現位置のせいで動けなくならないようにするため）
static bool EnemyHitsBlock(float x, float y, float w, float h)
{
	if (x + w < 0.0f || x > SCREEN_WIDTH || y + h < 0.0f || y > SCREEN_HEIGHT)
	{
		return false;
	}
	return CheckBlockCollision(x, y, w, h);
}

// ブロックを避けて移動する（X方向とY方向を別々に判定するので、壁に沿って滑る）
static void MoveEnemyWithBlocks(EnemyData* e, VECTOR delta)
{
	float nextX = e->pos.x + delta.x;
	if (!EnemyHitsBlock(nextX, e->pos.y, e->width, e->height))
	{
		e->pos.x = nextX;
	}

	float nextY = e->pos.y + delta.y;
	if (!EnemyHitsBlock(e->pos.x, nextY, e->width, e->height))
	{
		e->pos.y = nextY;
	}
}

// 敵の行動：撃ちながらプレイヤーへ接近し、リロード中はその場で止まる
static void UpdateEnemyAI(EnemyData* e, float dt)
{
	const PlayerData* p = GetPlayer();

	// プレイヤーが死んでいる間は、何もしない
	if (!p->active)
	{
		e->velocity = VGet(0.0f, 0.0f, 0.0f);
		SetEnemyState(e, ENEMY_STATE_IDLE);
		return;
	}

	VECTOR enemyCenter = VGet(e->pos.x + e->width * 0.5f, e->pos.y + e->height * 0.5f, 0.0f);
	VECTOR playerCenter = VGet(p->pos.x + PLAYER_WIDTH * 0.5f, p->pos.y + PLAYER_HEIGHT * 0.5f, 0.0f);
	VECTOR toPlayer = VSub(playerCenter, enemyCenter);
	float dist = VSize(toPlayer);

	// プレイヤーのいる側を向く（ほぼ真上・真下のときは向きを変えない）
	if (fabsf(toPlayer.x) > 1.0f)
	{
		e->facingLeft = (toPlayer.x < 0.0f);
	}

	bool reloading = WeaponIsReloading(&e->gun);

	// 移動：リロード中、または十分近いときは止まる
	if (reloading || dist <= e->stopDistance)
	{
		e->velocity = VGet(0.0f, 0.0f, 0.0f);
	}
	else
	{
		e->velocity = VScale(VNorm(toPlayer), e->moveSpeed);
	}

	// 射撃：画面の中にいるときだけ撃つ
	bool onScreen = enemyCenter.x >= 0.0f && enemyCenter.x <= SCREEN_WIDTH &&
		enemyCenter.y >= 0.0f && enemyCenter.y <= SCREEN_HEIGHT;

	if (onScreen && WeaponTryFire(&e->gun))
	{
		FireEnemyBullet(e, playerCenter);
		e->shootTimer = ENEMY_SHOOT_ANIM_TIME;
	}

	// 見た目の状態：射撃 > 移動 > 待機
	if (e->shootTimer > 0.0f)
	{
		e->shootTimer -= dt;
		SetEnemyState(e, ENEMY_STATE_SHOOT);
	}
	else if (VSquareSize(e->velocity) > 0.0f)
	{
		SetEnemyState(e, ENEMY_STATE_MOVE);
	}
	else
	{
		SetEnemyState(e, ENEMY_STATE_IDLE);
	}
}

// ------------------------------------------------------------------------------------------------
// 公開関数
// ------------------------------------------------------------------------------------------------

void InitEnemy()
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		g_EnemyData[i].active = false;
	}
}

void LoadEnemy()
{
	// 各コマの {幅, 体の中心x}。画像編集ソフトで測った値を入れる。
	// 幅の合計 = 画像の横幅 になっていること（Debugビルドで合っていないと警告が出る）
	// 以下の数値とファイル名は仮です。実際の画像に合わせて書き換えてください。
	static const AnimFrameDef kar98kIdle[] = { { 58, 24 } };
	static const AnimFrameDef kar98kMove[] = { { 72, 30 }, {76, 33}, { 84, 30 }, { 80, 32 }, { 82, 28 } };
	static const AnimFrameDef kar98kShoot[] = { { 142, 29 }, { 120, 42 } };

	//                   画像データ                                      ファイル                                 定義           コマ数                    時間   ループ
	LoadAnim(&g_EnemyAnim[WEAPON_KAR98K][ENEMY_STATE_IDLE], "Data/Enemy/Kar98kEnemyIdle.png", kar98kIdle, ARRAY_COUNT(kar98kIdle), 0.20f, true);
	LoadAnim(&g_EnemyAnim[WEAPON_KAR98K][ENEMY_STATE_MOVE], "Data/Enemy/Kar98kEnemyMoving.png", kar98kMove, ARRAY_COUNT(kar98kMove), 0.12f, true);
	LoadAnim(&g_EnemyAnim[WEAPON_KAR98K][ENEMY_STATE_SHOOT], "Data/Enemy/Kar98kEnemyShooting.png", kar98kShoot, ARRAY_COUNT(kar98kShoot), 0.10f, true);

	// 他の武器（M1 Garand など）の画像ができたら、同じ形で g_EnemyAnim[武器][状態] に読み込む。
	// 読み込んでいない武器・状態は、Kar98kの画像で代用される。
}

void StartEnemy()
{
	InitEnemy();

	g_EnemyElapsedTime = 0.0f;
	g_EnemySpawnTimer = ENEMY_FIRST_SPAWN_TIME;
}

void UpdateEnemy()
{
	float dt = GetDeltaTime();

	g_EnemyElapsedTime += dt;

	// 出現（画面の外周から）
	g_EnemySpawnTimer -= dt;
	if (g_EnemySpawnTimer <= 0.0f)
	{
		if (CountAliveEnemies() < ENEMY_ALIVE_MAX)
		{
			SpawnEnemyAtEdge();
		}
		g_EnemySpawnTimer += ENEMY_SPAWN_INTERVAL;
	}

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		EnemyData* e = &g_EnemyData[i];
		if (!e->active) continue;

		if (e->hitFlashTimer > 0.0f)
		{
			e->hitFlashTimer -= dt;
		}

		// 発射間隔とリロードを進める
		WeaponUpdate(&e->gun, dt);

		// 行動を決める
		UpdateEnemyAI(e, dt);

		// 移動 = 速度(px/秒) * 経過時間(秒)
		MoveEnemyWithBlocks(e, VScale(e->velocity, dt));

		// アニメーションを進める
		const AnimData* anim = GetEnemyAnim(e->gun.type, e->state);
		if (anim != nullptr)
		{
			UpdateAnimPlayer(&e->anim, anim, dt);
		}

		// 画面外へ大きく出たら消す
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

		float centerX = e->pos.x + e->width * 0.5f;
		float footY = e->pos.y + e->height;

		// 画像の向きと、今向いている向きが違うときだけ反転する
		bool flip = (e->facingLeft != ENEMY_IMAGE_FACING_LEFT);

		const AnimData* anim = GetEnemyAnim(e->gun.type, e->state);
		if (anim != nullptr)
		{
			DrawAnim(anim, &e->anim, centerX, footY, flip);

			// 被弾時は、同じ画像を加算合成で重ねて白く光らせる
			if (e->hitFlashTimer > 0.0f)
			{
				SetDrawBlendMode(DX_BLENDMODE_ADD, 160);
				DrawAnim(anim, &e->anim, centerX, footY, flip);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}
		else
		{
			// 画像が読み込めていないときの代用（仮の四角）
			unsigned int color = (e->hitFlashTimer > 0.0f) ? GetColor(255, 255, 255) : GetColor(200, 60, 60);
			DrawBox(x1, y1, x2, y2, color, TRUE);
		}

		// HPバー
		float ratio = (float)e->hp / (float)e->maxHp;
		if (ratio < 0.0f) ratio = 0.0f;
		DrawBox(x1, y1 - 10, x2, y1 - 5, GetColor(60, 60, 60), TRUE);
		DrawBox(x1, y1 - 10, x1 + (int)(e->width * ratio), y1 - 5, GetColor(80, 255, 80), TRUE);

		// リロード中は、HPバーの下に進み具合を黄色で表示
		if (WeaponIsReloading(&e->gun))
		{
			float progress = WeaponReloadProgress(&e->gun);
			DrawBox(x1, y1 - 4, x1 + (int)(e->width * progress), y1 - 1, GetColor(255, 220, 0), TRUE);
		}
	}
}

void FinEnemy()
{
	for (int w = 0; w < WEAPON_NUM; w++)
	{
		for (int s = 0; s < ENEMY_STATE_NUM; s++)
		{
			FinAnim(&g_EnemyAnim[w][s]);
		}
	}
}

EnemyData* SpawnEnemy(EnemyType type, WeaponType weapon, float x, float y)
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
		e->velocity = VGet(0.0f, 0.0f, 0.0f);
		e->width = def->width;
		e->height = def->height;
		e->hp = def->hp;
		e->maxHp = def->hp;
		e->contactDamage = def->contactDamage;
		e->moveSpeed = def->moveSpeed;
		e->stopDistance = def->stopDistance;
		e->aimSpread = def->aimSpreadDeg * 3.14159265f / 180.0f;

		// 敵は予備弾が無限（弾倉が空になったら、時間をかけてリロードするだけ）
		WeaponStateInit(&e->gun, weapon, GetWeaponDef(weapon)->magazineSize, WEAPON_AMMO_INFINITE);

		e->facingLeft = false;
		e->state = ENEMY_STATE_IDLE;
		ResetAnimPlayer(&e->anim);
		e->shootTimer = 0.0f;
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