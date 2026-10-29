#include "DxLib.h"
#include "UIText.h"
#include <string.h>

#define UI_TEXT_MAX (256)

// UITextデータ
UITextData g_UITextData[UI_TEXT_MAX] = { 0 };

// プレイシーンで使うフォントのハンドル
int g_PlaySceneFontHandle = 0;

void InitUIText()
{
	UITextData* uiText = g_UITextData;
	for (int i = 0; i < UI_TEXT_MAX; i++, uiText++)
	{
		uiText->active = false;
		uiText->posX = 0.0f;
		uiText->posY = 0.0f;
		memset(uiText->text, 0, sizeof(uiText->text));
	}
}

void LoadUIText()
{
	g_PlaySceneFontHandle = CreateFontToHandle("Yu Gothic UI", 28, -1);
}

void StepUIText()
{
}

void UpdateUIText()
{
}

void DrawUIText()
{
	UITextData* uiText = g_UITextData;
	for (int i = 0; i < UI_TEXT_MAX; i++, uiText++)
	{
		if (!uiText->active) continue;

		// 描画
		DrawStringToHandle((int)uiText->posX, (int)uiText->posY - 10, uiText->text, GetColor(255, 255, 255), g_PlaySceneFontHandle);
	}
}

void FinUIText()
{
}

// UITextを生成
UITextData* CreateUIText(float posX, float posY, const char* text)
{
	UITextData* uiText = g_UITextData;
	for (int i = 0; i < UI_TEXT_MAX; i++, uiText++)
	{
		// 未使用枠を探す
		if (!uiText->active)
		{
			// アクティブ
			uiText->active = true;

			// 座標
			uiText->posX = posX;
			uiText->posY = posY;

			// テキスト
			strcpy_s(uiText->text, text);

			return uiText;
		}
	}

	// 空枠がなかった
	return nullptr;
}

// 指定したUITextに文字列を設定
void SetUIText(UITextData* uiText, const char* text)
{
	strcpy_s(uiText->text, text);
}

void ResetUIText()
{
	UITextData* uiText = g_UITextData;
	for (int i = 0; i < UI_TEXT_MAX; i++, uiText++)
	{
		uiText->active = false;
	}
}