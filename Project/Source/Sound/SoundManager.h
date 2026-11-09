#pragma once

enum SEType
{
	SE_PLAYER_SHOT,
	SE_PLAYER_DEAD,
	SE_NUM,
};

void LoadSound();
void PlaySE(SEType type);
void FinSound();