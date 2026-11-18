#pragma once
#include "DxLib.h"

// 照準の表示（クロスヘアと、狙っている方向を示すレーザー）

// origin から dir 方向（長さ1）へ進んで、ブロックか画面の端に着く手前の位置を返す。
// probeSize は、通れるかを調べる矩形の大きさ（弾の直径に合わせる）
VECTOR AimFindEnd(VECTOR origin, VECTOR dir, float probeSize);

// 狙っている方向を示すレーザーを描く。dimmed = true のときは、薄いグレー（リロード中など、撃てないとき）
void DrawAimLaser(VECTOR muzzle, VECTOR dir, float probeSize, bool dimmed);

// マウスの位置にクロスヘアを描く。
// 敵の上にあるときは赤、リロード中は黄色（reloadProgress は 0.0?1.0。リロード中でないときは負の値を渡す）
void DrawCrosshair(VECTOR mouse, float reloadProgress);