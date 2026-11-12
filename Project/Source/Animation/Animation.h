#pragma once
#include "DxLib.h"

// 1アニメーションの最大コマ数
#define ANIM_FRAME_MAX (8)

// コマの定義（読み込み用）:幅と、コマ内での体の中心X座標
struct AnimFrameDef
{
	int width;
	int anchorX;
};

// 切り出したコマ1枚分
struct AnimFrame
{
	int handle;
	int width;
	int height;
	int anchorX;
};

// アニメーション1種類分（画像データ。キャラクター間で共有できる）
struct AnimData
{
	int srcHandle;		// 元画像（DerivationGraphの参照元）
	AnimFrame frames[ANIM_FRAME_MAX];
	int frameNum;		// 0なら未読み込み
	float frameTime;	// 1コマの表示時間（秒）
	bool loop;
};

// 再生状態（キャラクターごとに1つ持つ）
struct AnimPlayer
{
	int frame;
	float timer;
};

// 画像を読み込み、コマごとに切り出す。画像の幅の合計とコマの幅の合計が合わないとDebugビルドで警告
bool LoadAnim(AnimData* anim, const char* path, const AnimFrameDef* defs, int frameNum, float frameTime, bool loop);

// 解放する（派生ハンドル -> 元画像の順）
void FinAnim(AnimData* anim);

bool IsAnimLoaded(const AnimData* anim);

// 再生位置を先頭に戻す
void ResetAnimPlayer(AnimPlayer* player);

// 再生を進める（秒単位）
void UpdateAnimPlayer(AnimPlayer* player, const AnimData* anim, float dt);

// 描画する。centerX = 体の中心X、footY = 足元のY。flip = trueで左右反転（画像は右向き前提）
void DrawAnim(const AnimData* anim, const AnimPlayer* player, float centerX, float footY, bool flip);
