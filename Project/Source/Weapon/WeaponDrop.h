#pragma once
#include "DxLib.h"
#include "Weapon.h"

// 同時に存在できるドロップの最大数
#define WEAPON_DROP_MAX (32)

// 地面に落ちている武器1個分のデータ
struct WeaponDropData
{
	bool active;
	WeaponType type;
	int magazine;		// 弾倉に入っている弾数
	int reserve;		// 予備弾
	VECTOR pos;			// 中心の位置
	float life;			// 消えるまでの残り時間（秒）
	float age;			// 出現してからの時間（秒）。上下の揺れに使う
	bool pickable;		// falseの間は拾えない（捨てた直後に、すぐ拾い直さないため）
};

void InitWeaponDrop();
void StartWeaponDrop();
void UpdateWeaponDrop();
void DrawWeaponDrop();

// 武器を地面に落とす（posは中心）。pickable=falseなら、プレイヤーが一度離れるまで拾えない
// 空きがなければ、残り時間が最も短いものを消して使う
WeaponDropData* SpawnWeaponDrop(WeaponType type, int magazine, int reserve, VECTOR pos, bool pickable);

// 取得関数（範囲外ならnullptr）
WeaponDropData* GetWeaponDrop(int index);