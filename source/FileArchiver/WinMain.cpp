#include"LibImpl.hpp"
#include"MainWindow.hpp"

/**********************************************************
WinMain
**********************************************************/
int APIENTRY wWinMain(HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPWSTR lpCmdLine,
                        int nCmdShow )
{
	DebugUtility::DumpMemoryLeaksOnExit();
	try
	{
		ELogger::CreateInstance();

		MainWindow* wndMain = MainWindow::CreateInstance();
		wndMain->Initialize();
		wndMain->SetWindowVisible(true);

		EApplication* app = EApplication::CreateInstance();
		app->Initialize();
		app->Run();
	}
	catch(std::exception& e)
	{
		std::wstring error = StringUtility::ConvertMultiToWide(e.what());
		Logger::WriteTop(error);
	}
	catch(gstd::wexception& e)
	{
		Logger::WriteTop(e.what());
	}

	EApplication::DeleteInstance();
	MainWindow::DeleteInstance();
	ELogger::DeleteInstance();

	return 0;
}
