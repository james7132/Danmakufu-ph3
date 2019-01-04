#include"GcLibImpl.hpp"
#include"MainWindow.hpp"

/**********************************************************
//EApplication
**********************************************************/
EApplication::EApplication()
{

}
EApplication::~EApplication()
{

}
bool EApplication::_Initialize()
{
	ELogger* logger = ELogger::GetInstance();
	Logger::WriteTop(L"�A�v���P�[�V����������");

	EFileManager* fileManager = EFileManager::CreateInstance();
	fileManager->Initialize();

	HWND hWndMain = MainWindow::GetInstance()->GetWindowHandle();
	WindowLogger::InsertOpenCommandInSystemMenu(hWndMain);
//	::SetWindowText(hWndMain, "DnhViewer");
//	::SetClassLong(hWndMain, GCL_HICON, (LONG)LoadIcon(GetApplicationHandle(), MAKEINTRESOURCE(IDI_ICON)));

	EDirectInput* input = EDirectInput::CreateInstance();
	input->Initialize(hWndMain);

	MainWindow* wndMain = MainWindow::GetInstance();
	wndMain->StartUp();
	::SetForegroundWindow(wndMain->GetWindowHandle());

	Logger::WriteTop(L"�A�v���P�[�V��������������");

	return true;
}
bool EApplication::_Loop()
{
	ELogger* logger = ELogger::GetInstance();

	MainWindow* mainWindow = MainWindow::GetInstance();
	HWND hWndFocused = ::GetForegroundWindow();
	HWND hWndMain = mainWindow->GetWindowHandle();
	HWND hWndLogger = ELogger::GetInstance()->GetWindowHandle();
	if(hWndFocused != hWndMain && hWndFocused != hWndLogger)
	{
		//��A�N�e�B�u���͓��삵�Ȃ�
		::Sleep(10);
		return true;
	}

	EDirectInput* input = EDirectInput::GetInstance();
	input->Update();
	mainWindow->UpdateKeyAssign();

	::Sleep(10);

	return true;
}
bool EApplication::_Finalize()
{
	Logger::WriteTop(L"�A�v���P�[�V�����I�������J�n");
	EFileManager::GetInstance()->EndLoadThread();
	EDirectInput::DeleteInstance();
	EFileManager::DeleteInstance();
	Logger::WriteTop(L"�A�v���P�[�V�����I����������");
	return true;
}


