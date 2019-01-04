#include"Common.hpp"

/**********************************************************
//SystemResidentTask
**********************************************************/
SystemResidentTask::SystemResidentTask()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int screenWidth = graphics->GetScreenWidth();
	int screenHeight = graphics->GetScreenHeight();

	textFps_.SetFontColorTop(D3DCOLOR_ARGB(255,160,160,255));
	textFps_.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
	textFps_.SetFontBorderType(directx::DxFont::BORDER_FULL);
	textFps_.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
	textFps_.SetFontBorderWidth(2);
	textFps_.SetFontSize(14);
	textFps_.SetFontBold(true);
	textFps_.SetMaxWidth(screenWidth - 8);
	textFps_.SetHorizontalAlignment(DxText::ALIGNMENT_RIGHT);
	textFps_.SetPosition(0, screenHeight - 20);
}
SystemResidentTask::~SystemResidentTask()
{
}
void SystemResidentTask::RenderFps()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetFogEnable(false);

	EFpsController* fpsController = EFpsController::GetInstance();
	float fps = fpsController->GetCurrentWorkFps();
	textFps_.SetText(StringUtility::Format(L"%.2ffps", fps));
	//textFps_.Render();
}
