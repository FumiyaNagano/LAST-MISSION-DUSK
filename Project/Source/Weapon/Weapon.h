#pragma once
#include "../Sound/SoundManager.h"

// 弾数が無限であることを表す値（予備弾に設定する）
#define WEAPON_AMMO_INFINITE (-1)

// 武器の種類
enum WeaponType
{
	WEAPON_PISTOL,		// プレイヤーの予備武器（予備弾が無限）
	WEAPON_KAR98K,		// ここから下は、敵の武器（強くなる順）
	WEAPON_M1_GARAND,
	WEAPON_MP5,
	WEAPON_M4A1,
	WEAPON_XM250,
	WEAPON_NUM,
};

// ゲーム開始時の武器、および弾を撃ち切ったときに戻る予備武器
#define WEAPON_DEFAULT (WEAPON_PISTOL)

// 発射方式
enum FireMode
{
	FIRE_MODE_SEMI,		// クリックごとに1発
	FIRE_MODE_AUTO,		// 長押しで連射
};

// 武器1種類分の性能
struct WeaponDef
{
	const char* name;	// 表示名
	FireMode mode;
	float interval;		// 発射間隔（秒）
	float bulletSpeed;	// 弾速（ピクセル/秒）
	float bulletLife;	// 弾の寿命（秒）
	float bulletRadius;	// 弾の当たり判定の半径
	int damage;			// 1発のダメージ
	int magazineSize;	// 装弾数（弾倉の大きさ）
	int reserveMax;		// 予備弾の最大数（WEAPON_AMMO_INFINITEなら無限）
	float reloadTime;	// リロードにかかる時間（秒）
	SEType shotSE;		// 発射SE
};

// 武器を持っている側（プレイヤー・敵）が持つ、弾数やリロードの状態
struct WeaponState
{
	WeaponType type;
	int magazine;			// 弾倉に入っている弾数
	int reserve;			// 予備弾（WEAPON_AMMO_INFINITEなら無限）
	float reloadTimer;		// リロードの残り時間（秒）。0ならリロード中ではない
	float intervalTimer;	// 次に撃てるまでの時間（秒）
};

// 武器の性能を取得する（範囲外は予備武器）
const WeaponDef* GetWeaponDef(WeaponType type);

// 状態を初期化する（弾数は上限に収める。reserveにWEAPON_AMMO_INFINITEを渡すと予備弾が無限になる）
void WeaponStateInit(WeaponState* s, WeaponType type, int magazine, int reserve);

// 弾倉も予備弾も満タンで初期化する
void WeaponStateInitFull(WeaponState* s, WeaponType type);

// 毎フレーム呼ぶ（発射間隔とリロードを進める。弾倉が空なら自動でリロードを始める）
void WeaponUpdate(WeaponState* s, float dt);

// 撃てる状態か（リロード中でなく、弾があり、発射間隔が空いている）
bool WeaponCanFire(const WeaponState* s);

// 撃てるなら弾を1発消費して true を返す（弾の生成は呼び出し側で行う）
bool WeaponTryFire(WeaponState* s);

// リロードを始める（すでにリロード中、弾倉が満タン、予備弾なしの場合は false）
bool WeaponStartReload(WeaponState* s);

bool WeaponIsReloading(const WeaponState* s);

// 弾倉も予備弾も0か
bool WeaponIsOutOfAmmo(const WeaponState* s);

// リロードの進み具合（0.0〜1.0）。表示用
float WeaponReloadProgress(const WeaponState* s);

// ドロップできる武器か（予備弾が無限の武器は、落としても意味がないのでfalse）
bool WeaponCanDrop(WeaponType type);