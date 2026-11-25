#pragma once
#include "DxLib.h"
#include "../UIImage/UIText.h"
#include "../Weapon/Weapon.h"

// 当たり判定サイズ（見た目の画像サイズとは独立）
#define PLAYER_WIDTH	(46.0f)
#define PLAYER_HEIGHT	(100.0f)
#define PLAYER_RADIUS	(23.0f)

// 1アニメーションの最大コマ数
#define PLAYER_ANIM_FRAME_MAX (8)

// プレイヤーの状態（見た目のアニメーションの種類）
enum PlayerState
{
	PLAYER_STATE_IDLE,			// 待機
	PLAYER_STATE_MOVE,			// 左右への移動
	PLAYER_STATE_MOVE_UP,		// 上への移動（背中側）
	PLAYER_STATE_MOVE_DOWN,		// 下への移動（正面側）
	PLAYER_STATE_SHOOT,			// 射撃（横・斜め・下向き）
	PLAYER_STATE_SHOOT_UP,		// 上向きの射撃（背中側）
	PLAYER_STATE_DEAD,			// 死亡（画像ができたら使う）
	PLAYER_STATE_NUM,
};

// 1コマ分のデータ
struct PlayerAnimFrame
{
	int handle;		// 切り出した画像ハンドル
	int width;		// このコマの幅
	int height;		// このコマの高さ
	int anchorX;	// コマ内での体の中心x座標
};

// アニメーション1種類分のデータ
struct PlayerAnimData
{
	int srcHandle;		// 元画像のハンドル（DerivationGraphの参照元）
	PlayerAnimFrame frames[PLAYER_ANIM_FRAME_MAX];
	int frameNum;		// コマ数。0なら未読み込み
	float frameTime;	// 1コマの表示時間（秒）
	bool loop;			// ループするか
	bool baseFacingLeft;	// 画像が元々左向きか（trueなら、右を向くときに左右反転して描く）
};

struct PlayerData
{
	bool active;
	int hp;
	VECTOR pos;				// 当たり判定矩形の左上
	VECTOR move;			// 移動速度（ピクセル/秒）
	UITextData* hpText;

	// 武器
	WeaponState gun;		// 装備中の武器と、弾数・リロードの状態

	// 向き・照準
	VECTOR aimDir;			// 体の中心からカーソルへの向き（長さ1）
	bool facingLeft;		// 左向きか
	float shootTimer;		// 射撃アニメーションを表示する残り時間（秒）

	// 被弾
	float invincibleTimer;	// 無敵の残り時間（秒）

	// アニメーション
	PlayerState state;
	int animFrame;			// 現在のコマ番号
	float animTimer;		// 現在のコマの経過時間（秒）
};

// 関数のプロトタイプ宣言
void InitPlayer();
void LoadPlayer();
void StartPlayer();
void StepPlayer();
void UpdatePlayer();
void DrawPlayer();
void FinPlayer();

// 照準（クロスヘアとレーザー）を描く。画面の一番上に描くため、描画の最後に呼ぶ
void DrawPlayerAim();

// 取得関数
PlayerData* GetPlayer();

// 当たり判定
void PlayerHitEnemy();			// 敵との接触
void PlayerHitEnemyBullet();	// 敵の弾

// ダメージを受ける（無敵中は無効。HPが0になると死亡）
void PlayerTakeDamage(int damage);

// HPを回復する。回復したらtrue（HPが満タンのとき、または死亡中は何もせずfalse）
bool PlayerHeal(int amount);

// HPの最大値
int PlayerGetMaxHp();

// 武器
void PlayerEquipWeapon(WeaponType type, int magazine, int reserve);		// 装備する（弾倉・予備弾を指定）
bool PlayerPickupWeapon(WeaponType type, int magazine, int reserve);	// ドロップした武器を拾う。拾えたらtrue（今の武器は、その場に捨てられる）

// 死亡処理
void PlayerDead();