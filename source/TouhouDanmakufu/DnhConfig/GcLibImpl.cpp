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
	Logger::WriteTop(L"アプリケーション初期化");

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

	Logger::WriteTop(L"アプリケーション初期化完了");

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
		//非アクティブ時は動作しない
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
	Logger::WriteTop(L"アプリケーション終了処理開始");
	EFileManager::GetInstance()->EndLoadThread();
	EDirectInput::DeleteInstance();
	EFileManager::DeleteInstance();
	Logger::WriteTop(L"アプリケーション終了処理完了");
	return true;
}


