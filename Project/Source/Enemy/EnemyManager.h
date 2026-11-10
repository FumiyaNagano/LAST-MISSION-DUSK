#pragma once
#include "DxLib.h"
#include "Enemy.h"

void InitEnemy();
void LoadEnemy();
void StartEnemy();
void UpdateEnemy();
void DrawEnemy();
void FinEnemy();

// 敵を出現させる(x, yは矩形の左上。空きが無ければnullptr)
EnemyData* SpawnEnemy(EnemyType type, float x, float y);

// 当たり判定用の取得関数（範囲外ならnullptr）
EnemyData* GetEnemy(int index);

// プレイヤーの弾が当たった時の処理（弾が動いた後に呼ぶ）
void EnemyHitPlayerBullet();