#include "DxLib.h"
#include "Collision.h"
#include <math.h>

bool CheckCircleCircle(float x1, float y1, float r1, float x2, float y2, float r2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float rr = r1 + r2;

	// 平方根を使わず、2乗のまま比較する
	return (dx * dx + dy * dy) <= (rr * rr);
}

bool CheckSegmentCircle(VECTOR segStart, VECTOR segEnd, VECTOR center, float radius)
{
	float sx = segEnd.x - segStart.x;
	float sy = segEnd.y - segStart.y;
	float lenSq = sx * sx + sy * sy;

	// 線分上で円の中心に一番近い点を求める（t: 0=始点, 1=終点）
	float t = 0.0f;
	if (lenSq > 0.0f)
	{
		t = ((center.x - segStart.x) * sx + (center.y - segStart.y) * sy) / lenSq;
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;
	}

	float px = segStart.x + sx * t;
	float py = segStart.y + sy * t;

	float dx = center.x - px;
	float dy = center.y - py;

	return (dx * dx + dy * dy) <= (radius * radius);
}

bool CheckCircleRect(float cx, float cy, float r, float rx, float ry, float rw, float rh)
{
	// 矩形上で円の中心に一番近い点
	float nx = cx;
	if (nx < rx)		nx = rx;
	if (nx > rx + rw)	nx = rx + rw;

	float ny = cy;
	if (ny < ry)		ny = ry;
	if (ny > ry + rh)	ny = ry + rh;

	float dx = cx - nx;
	float dy = cy - ny;

	return (dx * dx + dy * dy) <= (r * r);
}

bool CheckSegmentRect(VECTOR segStart, VECTOR segEnd, float margin, float rx, float ry, float rw, float rh)
{
	float minX = rx - margin;
	float maxX = rx + rw + margin;
	float minY = ry - margin;
	float maxY = ry + rh + margin;

	float dx = segEnd.x - segStart.x;
	float dy = segEnd.y - segStart.y;

	// 線分のうち、矩形の内側にいる区間 [tMin, tMax] を絞り込む（slab法）
	float tMin = 0.0f;
	float tMax = 1.0f;

	// X方向
	if (fabsf(dx) < 0.000001f)
	{
		// X方向に動いていない場合は、X範囲の外なら当たらない
		if (segStart.x < minX || segStart.x > maxX) return false;
	}
	else
	{
		float inv = 1.0f / dx;
		float t1 = (minX - segStart.x) * inv;
		float t2 = (maxX - segStart.x) * inv;
		if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
		if (t1 > tMin) tMin = t1;
		if (t2 < tMax) tMax = t2;
		if (tMin > tMax) return false;
	}

	// Y方向
	if (fabsf(dy) < 0.000001f)
	{
		if (segStart.y < minY || segStart.y > maxY) return false;
	}
	else
	{
		float inv = 1.0f / dy;
		float t1 = (minY - segStart.y) * inv;
		float t2 = (maxY - segStart.y) * inv;
		if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
		if (t1 > tMin) tMin = t1;
		if (t2 < tMax) tMax = t2;
		if (tMin > tMax) return false;
	}

	return true;
}