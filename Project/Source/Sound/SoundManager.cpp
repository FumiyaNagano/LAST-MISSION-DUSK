#include "DxLib.h"
#include "SoundManager.h"

static int g_SEHandle[SE_NUM];

void LoadSound()
{
	g_SEHandle[SE_PLAYER_SHOT] = LoadSoundMem("Data/Sound/SE/å¯â âπçHñ[_PlayerGunFire.mp3", 5);

	// òAéÀÇ≈é®è·ÇËÇ…Ç»ÇÁÇ»Ç¢ÇÊÇ§âπó Çâ∫Ç∞ÇÈÅi0Å`255Åj
	ChangeVolumeSoundMem(255 * 60 / 100, g_SEHandle[SE_PLAYER_SHOT]);
}

void PlaySE(SEType type)
{
	PlaySoundMem(g_SEHandle[type], DX_PLAYTYPE_BACK);
}

void FinSound()
{
	for (int i = 0; i < SE_NUM; i++)
	{
		DeleteSoundMem(g_SEHandle[i]);
	}
}