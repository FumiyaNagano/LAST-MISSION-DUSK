#include "DxLib.h"
#include "EnemyNav.h"
#include "../GameSetting/GameSetting.h"
#include "../Map/Block.h"
#include <math.h>
#include <queue>
#include <vector>
#include <functional>

// マスの大きさ（ピクセル）。小さいほど細かく避けられるが、計算は増える
#define NAV_CELL_SIZE		(30)
#define NAV_COLS			((SCREEN_WIDTH + NAV_CELL_SIZE - 1) / NAV_CELL_SIZE)
#define NAV_ROWS			((SCREEN_HEIGHT + NAV_CELL_SIZE - 1) / NAV_CELL_SIZE)

// 通れるかを調べるときの、サンプルの間隔（ピクセル）。ブロック（75px）より十分小さくする
#define NAV_CLEAR_STEP		(15.0f)

// 経路の何マス先まで見て、まっすぐ進める一番遠いマスを目指すか（角で引っかかったり、ジグザグしたりしないため）
#define NAV_LOOKAHEAD		(8)

// 敵のいるマスが通れない場所のとき、近くの通れるマスを探す範囲（マス）
#define NAV_SEARCH_RADIUS	(3)
// プレイヤーのいるマスが、敵には通れない場所のときに探す範囲（マス）
#define NAV_TARGET_RADIUS	(6)

static const float NAV_INF = 1.0e9f;

// 敵の体の大きさ
static float g_AgentW = 46.0f;
static float g_AgentH = 100.0f;

static bool  g_Walkable[NAV_ROWS][NAV_COLS];	// 敵の中心がそのマスにあるとき、体がブロックに当たらないか
static float g_Dist[NAV_ROWS][NAV_COLS];		// プレイヤーのいるマスまでの距離

static bool g_Built = false;		// 通れるマスを調べ済みか
static bool g_Dirty = true;			// 距離を計算し直す必要があるか
static int  g_TargetCol = -1;
static int  g_TargetRow = -1;

// 隣のマスへの移動（前半4つが上下左右、後半4つが斜め）
static const int   g_DC[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
static const int   g_DR[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
static const float g_Cost[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.41421356f, 1.41421356f, 1.41421356f, 1.41421356f };

static bool InGrid(int c, int r)
{
	return c >= 0 && c < NAV_COLS && r >= 0 && r < NAV_ROWS;
}

static VECTOR CellCenter(int c, int r)
{
	return VGet((c + 0.5f) * NAV_CELL_SIZE, (r + 0.5f) * NAV_CELL_SIZE, 0.0f);
}

static void CellOf(VECTOR pos, int* c, int* r)
{
	int cc = (int)floorf(pos.x / NAV_CELL_SIZE);
	int rr = (int)floorf(pos.y / NAV_CELL_SIZE);
	if (cc < 0) cc = 0;
	if (cc >= NAV_COLS) cc = NAV_COLS - 1;
	if (rr < 0) rr = 0;
	if (rr >= NAV_ROWS) rr = NAV_ROWS - 1;
	*c = cc;
	*r = rr;
}

// (c, r) から k 番目の方向へ進めるか。斜めのときは、角を斜めに抜けないよう、両隣も通れる必要がある
static bool CanStep(int c, int r, int k)
{
	int nc = c + g_DC[k];
	int nr = r + g_DR[k];
	if (!InGrid(nc, nr) || !g_Walkable[nr][nc]) return false;

	if (k >= 4)
	{
		if (!g_Walkable[r][nc] || !g_Walkable[nr][c]) return false;
	}
	return true;
}

// 通れるマスを調べる（敵の体の大きさの矩形が、そのマスの中心でブロックに当たらないか）
static void BuildWalkable()
{
	for (int r = 0; r < NAV_ROWS; r++)
	{
		for (int c = 0; c < NAV_COLS; c++)
		{
			VECTOR center = CellCenter(c, r);
			float x = center.x - g_AgentW * 0.5f;
			float y = center.y - g_AgentH * 0.5f;

			// 体が画面からはみ出すマスは、通れないことにする
			bool inside = x >= 0.0f && y >= 0.0f && x + g_AgentW <= SCREEN_WIDTH && y + g_AgentH <= SCREEN_HEIGHT;

			g_Walkable[r][c] = inside && !CheckBlockCollision(x, y, g_AgentW, g_AgentH);
		}
	}

	g_Built = true;
	g_Dirty = true;
	g_TargetCol = -1;
	g_TargetRow = -1;
}

// (c, r) か、その近くの通れるマスを探す。見つかったら true
static bool FindNearestWalkable(int c, int r, int radius, int* outC, int* outR)
{
	int bestD = 0x7fffffff;
	bool found = false;

	for (int dr = -radius; dr <= radius; dr++)
	{
		for (int dc = -radius; dc <= radius; dc++)
		{
			int nc = c + dc;
			int nr = r + dr;
			if (!InGrid(nc, nr) || !g_Walkable[nr][nc]) continue;

			int d = dc * dc + dr * dr;
			if (d < bestD)
			{
				bestD = d;
				*outC = nc;
				*outR = nr;
				found = true;
			}
		}
	}
	return found;
}

// 目標のマスから、すべてのマスまでの距離を求める
static void ComputeDistance(int targetC, int targetR)
{
	for (int r = 0; r < NAV_ROWS; r++)
	{
		for (int c = 0; c < NAV_COLS; c++)
		{
			g_Dist[r][c] = NAV_INF;
		}
	}

	typedef std::pair<float, int> Node;
	std::priority_queue<Node, std::vector<Node>, std::greater<Node> > open;

	g_Dist[targetR][targetC] = 0.0f;
	open.push(Node(0.0f, targetR * NAV_COLS + targetC));

	while (!open.empty())
	{
		Node top = open.top();
		open.pop();

		int r = top.second / NAV_COLS;
		int c = top.second % NAV_COLS;
		if (top.first > g_Dist[r][c]) continue;

		for (int k = 0; k < 8; k++)
		{
			if (!CanStep(c, r, k)) continue;

			int nc = c + g_DC[k];
			int nr = r + g_DR[k];
			float nd = top.first + g_Cost[k];
			if (nd < g_Dist[nr][nc])
			{
				g_Dist[nr][nc] = nd;
				open.push(Node(nd, nr * NAV_COLS + nc));
			}
		}
	}
}

void NavInit(float agentWidth, float agentHeight)
{
	g_AgentW = agentWidth;
	g_AgentH = agentHeight;
	NavInvalidate();
}

void NavInvalidate()
{
	g_Built = false;
	g_Dirty = true;
	g_TargetCol = -1;
	g_TargetRow = -1;
}

void NavUpdate(VECTOR targetCenter)
{
	// マップは、最初の更新のときに調べる（ブロックが作られた後になるように）
	if (!g_Built)
	{
		BuildWalkable();
	}

	int c = 0, r = 0;
	CellOf(targetCenter, &c, &r);

	if (!FindNearestWalkable(c, r, NAV_TARGET_RADIUS, &c, &r))
	{
		// 近くに敵が立てるマスがない：道案内はできない
		if (g_TargetCol != -1 || g_Dirty)
		{
			for (int rr = 0; rr < NAV_ROWS; rr++)
				for (int cc = 0; cc < NAV_COLS; cc++)
					g_Dist[rr][cc] = NAV_INF;
		}
		g_TargetCol = -1;
		g_TargetRow = -1;
		g_Dirty = false;
		return;
	}

	// プレイヤーのいるマスが変わらなければ、計算し直さない
	if (!g_Dirty && c == g_TargetCol && r == g_TargetRow) return;

	ComputeDistance(c, r);
	g_TargetCol = c;
	g_TargetRow = r;
	g_Dirty = false;
}

bool NavIsClear(VECTOR from, VECTOR to, float width, float height)
{
	float dx = to.x - from.x;
	float dy = to.y - from.y;
	float len = sqrtf(dx * dx + dy * dy);
	int steps = (int)(len / NAV_CLEAR_STEP) + 1;

	for (int i = 0; i <= steps; i++)
	{
		float t = (float)i / (float)steps;
		float x = from.x + dx * t - width * 0.5f;
		float y = from.y + dy * t - height * 0.5f;

		// 画面の完全に外側は、調べない（画面外から入ってくる敵や、画面外の位置を、障害物と見なさないため）
		if (x + width < 0.0f || x > SCREEN_WIDTH || y + height < 0.0f || y > SCREEN_HEIGHT) continue;

		if (CheckBlockCollision(x, y, width, height)) return false;
	}
	return true;
}

bool NavGetDirection(VECTOR fromCenter, VECTOR* outDir)
{
	if (!g_Built || g_TargetCol < 0) return false;

	// 画面の外にいる間は、道案内しない（まっすぐ入ってくる）
	if (fromCenter.x < 0.0f || fromCenter.x >= SCREEN_WIDTH ||
		fromCenter.y < 0.0f || fromCenter.y >= SCREEN_HEIGHT)
	{
		return false;
	}

	int c = 0, r = 0;
	CellOf(fromCenter, &c, &r);
	if (!FindNearestWalkable(c, r, NAV_SEARCH_RADIUS, &c, &r)) return false;
	if (g_Dist[r][c] >= NAV_INF) return false;

	// 距離が小さくなる方へ、数マス先までたどる
	int pathC[NAV_LOOKAHEAD];
	int pathR[NAV_LOOKAHEAD];
	int n = 0;
	int cc = c;
	int cr = r;

	for (int i = 0; i < NAV_LOOKAHEAD; i++)
	{
		int bestC = cc;
		int bestR = cr;
		float best = g_Dist[cr][cc];

		for (int k = 0; k < 8; k++)
		{
			if (!CanStep(cc, cr, k)) continue;

			int nc = cc + g_DC[k];
			int nr = cr + g_DR[k];
			if (g_Dist[nr][nc] < best)
			{
				best = g_Dist[nr][nc];
				bestC = nc;
				bestR = nr;
			}
		}

		if (bestC == cc && bestR == cr) break;	// 目標のマスに着いた

		pathC[n] = bestC;
		pathR[n] = bestR;
		n++;
		cc = bestC;
		cr = bestR;
	}

	if (n == 0) return false;

	// 体がそのまま通れる、一番遠いマスを目指す（遠いほうから調べる）
	int pick = 0;
	for (int i = n - 1; i >= 1; i--)
	{
		if (NavIsClear(fromCenter, CellCenter(pathC[i], pathR[i]), g_AgentW, g_AgentH))
		{
			pick = i;
			break;
		}
	}

	VECTOR target = CellCenter(pathC[pick], pathR[pick]);
	VECTOR d = VSub(target, fromCenter);
	float len = VSize(d);
	if (len < 0.001f) return false;

	*outDir = VScale(d, 1.0f / len);
	return true;
}

void DrawNavDebug()
{
	if (!g_Built) return;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
	for (int r = 0; r < NAV_ROWS; r++)
	{
		for (int c = 0; c < NAV_COLS; c++)
		{
			if (g_Walkable[r][c]) continue;
			int x = c * NAV_CELL_SIZE;
			int y = r * NAV_CELL_SIZE;
			DrawBox(x, y, x + NAV_CELL_SIZE, y + NAV_CELL_SIZE, GetColor(255, 0, 0), TRUE);
		}
	}

	if (g_TargetCol >= 0)
	{
		int x = g_TargetCol * NAV_CELL_SIZE;
		int y = g_TargetRow * NAV_CELL_SIZE;
		DrawBox(x, y, x + NAV_CELL_SIZE, y + NAV_CELL_SIZE, GetColor(0, 255, 255), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}