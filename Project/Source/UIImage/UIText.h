#pragma once

#define UI_TEXT_LENGTH_MAX (256)

struct UITextData
{
	bool active;
	char text[UI_TEXT_LENGTH_MAX];
	float posX;
	float posY;
};

void InitUIText();
void LoadUIText();
void StepUIText();
void UpdateUIText();
void DrawUIText();
void FinUIText();

UITextData* CreateUIText(float posX, float posY, const char* text);

void SetUIText(UITextData* uiText, const char* text);

void ResetUIText();