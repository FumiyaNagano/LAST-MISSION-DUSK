#include "Block.h"

BlockData g_Blocks[BLOCK_MAX] = { 0 };
int g_BlockHandle[BLOCK_TYPE_MAX] = { 0 };

void InitBlock()
{
	BlockData* block = g_Blocks;
	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		block->active = false;
		block->pos = VGet(0.0f, 0.0f, 0.0f);
		block->type = MAP_CHIP_NONE;
	}
}

void LoadBlock()
{
	g_BlockHandle[NORMAL_BLOCK] = LoadGraph("Data/Map/Block.png");
}

void StartBlock()
{}

void StepBlock()
{}

void DrawBlock()
{
	BlockData* block = g_Blocks;
	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		if (block->active)
		{
			DrawGraph((int)(block->pos.x), (int)(block->pos.y), block->handle, TRUE);
		}
	}
}

void FinBlock()
{}

BlockData* CreateBlock(MapChipType type, VECTOR pos)
{
	// 未使用のブロックを探す
	BlockData* block = g_Blocks;
	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		if (!block->active)
		{
			// ブロック生成
			block->active = true;
			block->handle = g_BlockHandle[type];
			block->pos = pos;
			block->type = type;
			return block;
		}
	}

	return nullptr;
}

BlockData* GetBlocks()
{
	return g_Blocks;
}

bool CheckBlockCollision(float x, float y, float width, float height)
{
	BlockData* block = g_Blocks;

	for (int i = 0; i < BLOCK_MAX; i++, block++)
	{
		if (!block->active) continue;

		float blockLeft = block->pos.x;
		float blockTop = block->pos.y;
		float blockRight = blockLeft + MAP_CHIP_WIDTH;
		float blockBottom = blockTop + MAP_CHIP_HEIGHT;

		float playerLeft = x;
		float playerTop = y;
		float playerRight = x + width;
		float playerBottom = y + height;

		if (playerRight > blockLeft &&
			playerLeft < blockRight &&
			playerBottom > blockTop &&
			playerTop < blockBottom)
		{
			return true;
		}
	}

	return false;
}