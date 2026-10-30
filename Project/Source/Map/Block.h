#pragma once
#include "MapParameter.h"

void InitBlock();
void LoadBlock();
void StartBlock();
void StepBlock();
void DrawBlock();
void FinBlock();

BlockData* CreateBlock(MapChipType type, VECTOR pos);
BlockData* GetBlocks();

bool CheckBlockCollision(
    float x,
    float y,
    float width,
    float height
);