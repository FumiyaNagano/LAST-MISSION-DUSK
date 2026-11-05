// DxLib, Visual Studio 2022でゲーム制作を行っています。プレイヤーが描画されない原因を特定してほしいです。まずPlayer.cppを貼ります。他にスクリプトが必要であれば教えてください。

#include "DxLib.h"
#include "Player.h"
#include "../FPS/FPS.h"
#include "../Input/Input.h"
//#include "../Collision/Collision.h"
//#include "../Score/Score.h"
//#include "../Effect/AnimationEffect.h"
#include "../Bullet/BulletManager.h"
//#include "../Scene/SceneManager.h"
//#include "../Sound/SoundManager.h"
#include "../Map/Block.h"
#include <math.h>

// HP表示文字列
#define PLAYER_HP_STRING ("%d")

// プレイヤー設定関連
#define PLAYER_DEFAULT_POS_X (100.0f)
#define PLAYER_DEFAULT_POS_Y (300.0f)
// 1秒あたりの移動速度（ピクセル/秒）
#define PLAYER_MOVE_SPEED (240.0f)
// 射撃アニメーションを表示する残り時間（秒）
#define PLAYER_SHOOT_ANIM_TIME (0.2f)

// 弾関連
#define PLAYER_DEFAULT_SHOT_SPEED		(1200.0f)
#define PLAYER_SHOT_LIFE				(10.0f)
#define PLAYER_SHOT_OFFSET_X			(50.0f)
#define PLAYER_SHOT_OFFSET_Y			(25.0f)
#define PLAYER_SHOT_INTERVAL			(0.1f)
#define PLAYER_BULLET_RADIUS			(6.0f)
#define PLAYER_SHOT_OFFSET_UP_X			(2.0f)	// 体の中心からの左右のずれ（右向き）
#define PLAYER_SHOT_OFFSET_UP_Y			(20.0f)
#define PLAYER_SHOT_MIN_AIM_DISTANCE	(10.0f)

// 移動可能エリア
#define PLAYER_MOVE_AREA_LEFT	(0.0f)
#define PLAYER_MOVE_AREA_RIGHT	(1874.0f)
#define PLAYER_MOVE_AREA_TOP	(0.0f)
#define PLAYER_MOVE_AREA_BOTTOM	(980.0f)

// プレイヤーのHP
#define PLAYER_HP (100)

// プレイヤー死亡エフェクトインターバル
#define PLAYER_DEAD_EFFECT_INTERVAL (2)

struct FrameDef
{
	int width;		// コマの幅
	int anchorX;	// コマ内での体の中心X座標
};

PlayerData g_PlayerData = { 0 };

static PlayerAnimData g_PlayerAnim[PLAYER_STATE_NUM] = {};

// ------------------------------------------------------------------------------------------------
// アニメーション関連
// ------------------------------------------------------------------------------------------------
// 画像を読み込み、コマごとに切り出す
static void LoadPlayerAnim(PlayerState state, const char* path, const FrameDef* defs, int frameNum, float frameTime, bool loop, bool baseFacingLeft = false)
{
	PlayerAnimData* anim = &g_PlayerAnim[state];

	int src = LoadGraph(path);
	if (src == -1)
	{
		printfDx("画像の読み込みに失敗: %s\n", path);
		return;
	}

	int imgW = 0, imgH = 0;
	GetGraphSize(src, &imgW, &imgH);

	int x = 0;
	for (int i = 0; i < frameNum && i < PLAYER_ANIM_FRAME_MAX; i++)
	{
		anim->frames[i].handle = DerivationGraph(x, 0, defs[i].width, imgH, src);
		anim->frames[i].width = defs[i].width;
		anim->frames[i].height = imgH;
		anim->frames[i].anchorX = defs[i].anchorX;
		x += defs[i].width;
	}

	// 元画像は派生ハンドルが参照しているため、ここでは削除しない
	anim->srcHandle = src;
	anim->frameNum = frameNum;
	anim->frameTime = frameTime;
	anim->loop = loop;
	anim->baseFacingLeft = baseFacingLeft;
}

// 状態を切り替える（同じ状態なら何もしない）
static void SetPlayerState(PlayerState newState)
{
	if (g_PlayerData.state == newState) return;

	g_PlayerData.state = newState;
	g_PlayerData.animFrame = 0;
	g_PlayerData.animTimer = 0.0f;
}

// アニメーションを進める（秒単位）
static void UpdatePlayerAnim()
{
	PlayerAnimData* anim = &g_PlayerAnim[g_PlayerData.state];
	if (anim->frameNum <= 0) return;

	g_PlayerData.animTimer += GetDeltaTime();

	// 処理落ちで複数コマ分進む場合に備えてwhileにする
	while (g_PlayerData.animTimer >= anim->frameTime)
	{
		g_PlayerData.animTimer -= anim->frameTime;
		g_PlayerData.animFrame++;

		if (g_PlayerData.animFrame >= anim->frameNum)
		{
			// ループなら最初へ、そうでなければ最後のコマで止める
			g_PlayerData.animFrame = anim->loop ? 0 : anim->frameNum - 1;
		}
	}
}

// ------------------------------------------------------------------------------------------------
// プレイヤー本体
// ------------------------------------------------------------------------------------------------
void InitPlayer()
{
	g_PlayerData.hp = 0;
	g_PlayerData.pos = VGet(PLAYER_DEFAULT_POS_X, PLAYER_DEFAULT_POS_Y, 0.0f);
	g_PlayerData.move = VGet(0.0f, 0.0f, 0.0f);
	g_PlayerData.hpText = nullptr;
	//g_PlayerData.bulletIntervalTimer = 0;
	g_PlayerData.state = PLAYER_STATE_IDLE;
	g_PlayerData.animFrame = 0;
	g_PlayerData.animTimer = 0.0f;
	g_PlayerData.active = false;
	g_PlayerData.aimDir = VGet(1.0f, 0.0f, 0.0f);
	g_PlayerData.shootTimer = 0.0f;
	g_PlayerData.facingLeft = false;
}

void LoadPlayer()
{
	// 各コマの {直径, 半径}。画像編集ソフトで測った値を入れる
	// 幅の合計 = 画像の横幅になっていること
	static const FrameDef idleDefs[] = { {46, 23} };
	static const FrameDef moveDefs[] = { {84, 42}, {90, 45}, {92, 46}, {92, 46}, {88, 44}, {88, 44} };
	static const FrameDef moveUpDefs[] = { {64, 32}, {76, 38}, {68, 34} };
	static const FrameDef moveDownDefs[] = { {70, 35}, {90, 45}, {74, 37} };
	static const FrameDef shootDefs[] = { {88, 44}, {112, 56} };
	static const FrameDef shootUpDefs[] = { {70, 35}, {70, 35} };

	// LoadPlayerAnim(PLAYER_STATE_****, "Data/Player/Player****.png", ****Defs, コマ数, 時間, ループ);
	LoadPlayerAnim(PLAYER_STATE_IDLE, "Data/Player/PlayerIdle.png", idleDefs, 1, 0.15f, false);
	LoadPlayerAnim(PLAYER_STATE_MOVE, "Data/Player/PlayerMoveForward.png", moveDefs, 6, 0.15f, true);
	LoadPlayerAnim(PLAYER_STATE_MOVE_UP, "Data/Player/PlayerMoveBackView.png", moveUpDefs, 3, 0.2f, true);
	LoadPlayerAnim(PLAYER_STATE_MOVE_DOWN, "Data/Player/PlayerMoveFrontView.png", moveDownDefs, 3, 0.2f, true);
	LoadPlayerAnim(PLAYER_STATE_SHOOT, "Data/Player/PlayerShooting.png", shootDefs, 2, 0.10f, true);
	LoadPlayerAnim(PLAYER_STATE_SHOOT_UP, "Data/Player/PlayerShootingBackView.png", shootUpDefs, 2, 0.10f, true, true);
	// 画像でき次第:LoadPlayerAnim(PLAYER_STATE_DEAD, "Data/Player/PlayerDead.png", deadDefs, 1, 0.15f, false);
}

void StartPlayer()
{
	// 生存フラグを立てる
	g_PlayerData.active = true;

	// HPの初期値設定
	g_PlayerData.hp = PLAYER_HP;

	// アニメーションを初期状態に戻す
	g_PlayerData.state = PLAYER_STATE_IDLE;
	g_PlayerData.animFrame = 0;
	g_PlayerData.animTimer = 0.0f;

	// HPのUI生成
	//char text[256];
	//sprintf_s(text, PLAYER_HP_STRING, g_PlayerData.hp);
	//g_PlayerData.hpText = CreateUIText(60.0f, 900.0f, text);
}

// プレイヤーの弾を1発撃つ（shootUp: 上向き射撃のアニメーションか）
static void FirePlayerBullet(bool shootUp)
{
	// 銃口の位置（右向きの値を基準に、左向きは左右を反転）
	VECTOR muzzle;
	if (shootUp)
	{
		float dx = g_PlayerData.facingLeft ? -PLAYER_SHOT_OFFSET_UP_X : PLAYER_SHOT_OFFSET_UP_X;
		muzzle = VGet(g_PlayerData.pos.x + PLAYER_WIDTH * 0.5f + dx, g_PlayerData.pos.y + PLAYER_SHOT_OFFSET_UP_Y, 0.0f);
	}
	else if (g_PlayerData.facingLeft)
	{
		muzzle = VGet(g_PlayerData.pos.x + PLAYER_WIDTH - PLAYER_SHOT_OFFSET_X, g_PlayerData.pos.y + PLAYER_SHOT_OFFSET_Y, 0.0f);
	}
	else
	{
		muzzle = VGet(g_PlayerData.pos.x + PLAYER_SHOT_OFFSET_X, g_PlayerData.pos.y + PLAYER_SHOT_OFFSET_Y, 0.0f);
	}

	// 銃口からカーソルへの向きを計算し直す
	VECTOR dir = VSub(GetMousePosition(), muzzle);
	if (VSize(dir) < PLAYER_SHOT_MIN_AIM_DISTANCE)
	{
		dir = g_PlayerData.aimDir;
	}
	else
	{
		dir = VNorm(dir);
	}

	FireBullet(BULLET_OWNER_PLAYER, muzzle, dir, PLAYER_DEFAULT_SHOT_SPEED, PLAYER_SHOT_LIFE, PLAYER_BULLET_RADIUS);
}

void StepPlayer()
{
	/*if (g_PlayerData.bulletIntervalTimer > 0.0f)
	{
		g_PlayerData.bulletIntervalTimer -= GetDeltaTime();
	}*/

	if (!g_PlayerData.active)
	{
		g_PlayerData.move = VGet(0.0f, 0.0f, 0.0f);
		return;
	}

	// 移動入力
	VECTOR dir = VGet(0.0f, 0.0f, 0.0f);

	if (IsInputKey(KEY_W)) dir.y -= 1.0f;
	if (IsInputKey(KEY_S)) dir.y += 1.0f;
	if (IsInputKey(KEY_A)) dir.x -= 1.0f;
	if (IsInputKey(KEY_D)) dir.x += 1.0f;

	PlayerState nextState = PLAYER_STATE_IDLE;

	if (VSquareSize(dir) > 0.0f)
	{
		// 左右の入力があるときだけ向きを更新（AD同時押しはdir.x = 0なので維持）
		if (dir.x < 0.0f) g_PlayerData.facingLeft = true;
		if (dir.x > 0.0f) g_PlayerData.facingLeft = false;

		// 左右の入力を優先して状態を決める（斜め移動も左右アニメーション）
		if (dir.x != 0.0f)		nextState = PLAYER_STATE_MOVE;
		else if (dir.y < 0.0f)	nextState = PLAYER_STATE_MOVE_UP;
		else					nextState = PLAYER_STATE_MOVE_DOWN;

		dir = VNorm(dir);
	}

	g_PlayerData.move = VScale(dir, PLAYER_MOVE_SPEED);

	// 狙う方向
	VECTOR center = VGet(g_PlayerData.pos.x + PLAYER_WIDTH * 0.5f, g_PlayerData.pos.y + PLAYER_HEIGHT * 0.5f, 0.0f);

	VECTOR aim = VSub(GetMousePosition(), center);

	// カーソルが中心と重なった時は、前回の向きを維持する
	if (VSquareSize(aim) > 0.0f)
	{
		g_PlayerData.aimDir = VNorm(aim);
	}

	// 射撃
	bool fired = IsTriggerKey(MOUSE_LEFT);
	if (fired)
	{
		g_PlayerData.shootTimer = PLAYER_SHOOT_ANIM_TIME;
	}

	if (g_PlayerData.shootTimer > 0.0f)
	{
		g_PlayerData.shootTimer -= GetDeltaTime();

		// 射撃中は、体の向きで狙う方向を決める
		if (g_PlayerData.aimDir.x < 0.0f) g_PlayerData.facingLeft = true;
		if (g_PlayerData.aimDir.x > 0.0f) g_PlayerData.facingLeft = false;

		// 縦成分が大きい上向きだけ専用アニメーション。横、斜め、下は横向きを使う
		bool shootUp = (g_PlayerData.aimDir.y < 0.0f && fabsf(g_PlayerData.aimDir.y) > fabs(g_PlayerData.aimDir.x));
		nextState = shootUp ? PLAYER_STATE_SHOOT_UP : PLAYER_STATE_SHOOT;

		// 体の向きが確定した後で撃つ（銃口の位置が向きに依存するため）
		if (fired)
		{
			FirePlayerBullet(shootUp);
		}
	}

	SetPlayerState(nextState);
}

void UpdatePlayer()
{
	if (!g_PlayerData.active) return;

	UpdatePlayerAnim();

	// このフレームの移動量 = 速度(px / 秒) * 経過時間(秒)
	VECTOR delta = VScale(g_PlayerData.move, GetDeltaTime());

	// X方向の移動
	VECTOR nextPos = VAdd(g_PlayerData.pos, delta);

	if (!CheckBlockCollision(nextPos.x, g_PlayerData.pos.y, PLAYER_WIDTH, PLAYER_HEIGHT))
	{
		g_PlayerData.pos.x = nextPos.x;
	}

	if (!CheckBlockCollision(g_PlayerData.pos.x, nextPos.y, PLAYER_WIDTH, PLAYER_HEIGHT))
	{
		g_PlayerData.pos.y = nextPos.y;
	}

	// 画面外への移動制限
	if (g_PlayerData.pos.x < PLAYER_MOVE_AREA_LEFT)
		g_PlayerData.pos.x = PLAYER_MOVE_AREA_LEFT;

	if (g_PlayerData.pos.x > PLAYER_MOVE_AREA_RIGHT)
		g_PlayerData.pos.x = PLAYER_MOVE_AREA_RIGHT;

	if (g_PlayerData.pos.y < PLAYER_MOVE_AREA_TOP)
		g_PlayerData.pos.y = PLAYER_MOVE_AREA_TOP;

	if (g_PlayerData.pos.y > PLAYER_MOVE_AREA_BOTTOM)
		g_PlayerData.pos.y = PLAYER_MOVE_AREA_BOTTOM;
}

void DrawPlayer()
{
	if (!g_PlayerData.active) return;

	PlayerAnimData* anim = &g_PlayerAnim[g_PlayerData.state];
	if (anim->frameNum <= 0) return;

	// 未読み込み状態は描画しない
	PlayerAnimFrame* f = &anim->frames[g_PlayerData.animFrame];

	float centerX = g_PlayerData.pos.x + PLAYER_WIDTH * 0.5f;
	float drawY = g_PlayerData.pos.y + PLAYER_HEIGHT - (float)f->height;

	bool flip = (g_PlayerData.facingLeft != anim->baseFacingLeft);

	if (flip)
	{
		// 反転後の体の中心は、左端から（幅 - anchorX）の位置になる
		float drawX = centerX - (float)(f->width - f->anchorX);
		DrawTurnGraphF(drawX, drawY, f->handle, TRUE);
	}
	else
	{
		float drawX = centerX - (float)f->anchorX;
		DrawGraphF(drawX, drawY, f->handle, TRUE);
	}
}

void FinPlayer()
{
	for (int s = 0; s < PLAYER_STATE_NUM; s++)
	{
		PlayerAnimData* anim = &g_PlayerAnim[s];
		if (anim->frameNum <= 0) continue;

		// 先に派生ハンドル、最後に元画像
		for (int i = 0; i < anim->frameNum; i++)
		{
			DeleteGraph(anim->frames[i].handle);
		}
		DeleteGraph(anim->srcHandle);
		anim->frameNum = 0;
	}
}

PlayerData* GetPlayer()
{
	return &g_PlayerData;
}

void PlayerDead()
{
	// 死亡
	g_PlayerData.active = false;

	// 死亡エフェクト
	//StartAnimationEffect(PLAYER_DEAD, g_PlayerData.pos.x, g_PlayerData.pos.y, PLAYER_DEAD_EFFECT_INTERVAL);

	// 死亡SE
	//PlaySE(SE_PLAYER_DEAD);
}