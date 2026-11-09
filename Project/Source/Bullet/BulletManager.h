#pragma once
#include "DxLib.h"
#include "Bullet.h"

void InitBullet();
void LoadBullet();
void UpdateBullet();
void DrawBullet();
void FinBullet();

// 弾を発射する（dirは長さ1の方向、speedはピクセル/秒）
void FireBullet(BulletOwner owner, VECTOR pos, VECTOR dir, float speed, float life, float radius, int damage);

// 当たり判定用の取得関数（範囲外ならnullptr）
BulletData* GetBullet(int index);