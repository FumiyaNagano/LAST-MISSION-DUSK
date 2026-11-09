#pragma once
#include "DxLib.h"

// 円と円の当たり判定
bool CheckCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2);

// 線分（弾の前フレーム位置->現在位置）と円の当たり判定
// 弾が速くて1フレームで敵をすり抜けることを防ぐ
// radiusには「弾の半径 + 相手の半径」を渡す
bool CheckSegmentCircle(VECTOR segStart, VECTOR segEnd, VECTOR center, float radius);

// 円と矩形（左上x, y, 幅, 高さ）の当たり判定
bool CheckCircleRect(float cx, float cy, float r, float rx, float ry, float rw, float fh);

// 線分（弾の前フレーム位置->現在位置）と矩形（左上x, y, 幅, 高さ）の当たり判定
// marginには弾の半径を渡す（矩形をmarginだけ広げて判定する。角は少し甘くなる）
bool CheckSegmentRect(VECTOR segStart, VECTOR segEnd, float margin, float rx, float ry, float rw, float rh);