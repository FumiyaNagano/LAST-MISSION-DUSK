#pragma once
#include "DxLib.h"

// 敵の道案内（壁を避けてプレイヤーへ近づく）と、視線の判定
//
// 画面を30px四方のマスに分けて、「敵の体の大きさで立てるマス」を調べておきます。
// プレイヤーのいるマスからの距離を求めておき、敵は、距離が小さくなる方へ進みます。

// 敵の大きさを設定する。マップを調べ直すことになる（ゲーム開始時に呼ぶ）
void NavInit(float agentWidth, float agentHeight);

// マップが変わったとき（ブロックが増えた・壊れたとき）に呼ぶ。次の NavUpdate で調べ直す
void NavInvalidate();

// 毎フレーム呼ぶ。プレイヤーの中心位置を渡す（プレイヤーのいるマスが変わったときだけ計算し直す）
void NavUpdate(VECTOR targetCenter);

// fromCenter にいる敵が、次に進む方向（長さ1）を返す。道案内ができない場合（画面外にいる、
// 目標に着いた、行けない場所など）は false。その場合は、プレイヤーへまっすぐ向かう
bool NavGetDirection(VECTOR fromCenter, VECTOR* outDir);

// from から to まで、width x height の矩形が、ブロックに当たらずに進めるか
// 視線の判定にも使う（弾の大きさの矩形を渡す）
bool NavIsClear(VECTOR from, VECTOR to, float width, float height);

// 確認用：通れないマスを赤く表示する
void DrawNavDebug();