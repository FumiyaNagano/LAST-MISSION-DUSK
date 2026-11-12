#include "DxLib.h"
#include "Animation.h"
#include <math.h>
#include <cmath>

bool LoadAnim(AnimData* anim, const char* path, const AnimFrameDef* defs, int frameNum, float frameTime, bool loop)
{
	int src = LoadGraph(path);
	if (src == -1)
	{
		printfDx("画像の読み込みに失敗: %s\n", path);
		return false;
	}

	int imgW = 0, imgH = 0;
	GetGraphSize(src, &imgW, &imgH);

	if (frameNum > ANIM_FRAME_MAX)
	{
		frameNum = ANIM_FRAME_MAX;
	}

	int x = 0;
	for (int i = 0; i < frameNum; i++)
	{
		anim->frames[i].handle = DerivationGraph(x, 0, defs[i].width, imgH, src);
		anim->frames[i].width = defs[i].width;
		anim->frames[i].height = imgH;
		anim->frames[i].anchorX = defs[i].anchorX;
		x += defs[i].width;
	}

#ifdef _DEBUG
	// 幅の合計が画像の横幅と一致しているか確認
	if (x != imgW)
	{
		printfDx("%s: 幅の合計(%d)が画像の横幅(%d)と一致しません\n", path, x, imgW);
	}
#endif

	// 元画像は派生ハンドルが参照しているため、ここでは削除しない
	anim->srcHandle = src;
	anim->frameNum = frameNum;
	anim->frameTime = frameTime;
	anim->loop = loop;
	return true;
}

void FinAnim(AnimData* anim)
{
	if (anim->frameNum <= 0) return;

	for (int i = 0; i < anim->frameNum; i++)
	{
		DeleteGraph(anim->frames[i].handle);
	}
	DeleteGraph(anim->srcHandle);
	anim->frameNum = 0;
}

bool IsAnimLoaded(const AnimData* anim)
{
	return anim->frameNum > 0;
}

void ResetAnimPlayer(AnimPlayer* player)
{
	player->frame = 0;
	player->timer = 0.0f;
}

void UpdateAnimPlayer(AnimPlayer* player, const AnimData* anim, float dt)
{
	if (anim->frameNum <= 0 || anim->frameTime <= 0.0f) return;
	if (!(dt > 0.0f)) return;	// 0以下・NaNは無視する

	player->timer += dt;

	// 値が壊れている場合は、ここで復帰させる（無限大・NaN）
	if (!std::isfinite(player->timer))
	{
		player->timer = 0.0f;
		return;
	}

	if (player->timer < anim->frameTime) return;

	// 何コマ進むかを、ループではなく割り算で求める（処理落ちや異常な値でも、必ず終わる）
	float steps = floorf(player->timer / anim->frameTime);
	player->timer -= steps * anim->frameTime;
	if (player->timer < 0.0f || player->timer >= anim->frameTime)
	{
		player->timer = 0.0f;	// 桁落ちなどで範囲外になった場合の保険
	}

	if (anim->loop)
	{
		// ループ：コマ数で割った余りぶんだけ進める
		int add = (int)fmodf(steps, (float)anim->frameNum);
		player->frame = (player->frame + add) % anim->frameNum;
	}
	else
	{
		// 非ループ：最後のコマで止める
		int add = (steps > (float)anim->frameNum) ? anim->frameNum : (int)steps;
		int next = player->frame + add;
		player->frame = (next >= anim->frameNum) ? anim->frameNum - 1 : next;
	}
}

void DrawAnim(const AnimData* anim, const AnimPlayer* player, float centerX, float footY, bool flip)
{
	if (anim->frameNum <= 0) return;

	int index = player->frame;
	if (index >= anim->frameNum) index = anim->frameNum - 1;

	const AnimFrame* f = &anim->frames[index];
	float drawY = footY - (float)f->height;

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