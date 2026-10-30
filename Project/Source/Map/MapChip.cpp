#include "MapChip.h"
#include "DxLib.h"
#include "../GameSetting/GameSetting.h"
#include "MapParameter.h"
#include "Block.h"

MapChipData g_MapChip[MAP_CHIP_Y_NUM][MAP_CHIP_X_NUM] = { 0 };

void LoadMapChipData()
{
	FILE* fp;

	if (fopen_s(&fp, "Data/Map/Map.bin", "rb") != 0) return;

	for (int i = 0; i < MAP_CHIP_Y_NUM; i++)
	{
		for (int j = 0; j < MAP_CHIP_X_NUM; j++)
		{
			int map = fgetc(fp);
			g_MapChip[i][j].mapChip = map;
		}
	}
}

void CreateMap()
{
	for (int i = 0; i < MAP_CHIP_Y_NUM; i++)
	{
		for (int j = 0; j < MAP_CHIP_X_NUM; j++)
		{
			MapChipType type = (MapChipType)g_MapChip[i][j].mapChip;

			if (type == MAP_CHIP_NONE) continue;

			VECTOR pos = VGet(j * MAP_CHIP_WIDTH, i * MAP_CHIP_HEIGHT, 0.0f);
			g_MapChip[i][j].data = CreateBlock(type, pos);
		}
	}
}

MapChipData GetMapChipData(int x, int y)
{
	return g_MapChip[y][x];
}