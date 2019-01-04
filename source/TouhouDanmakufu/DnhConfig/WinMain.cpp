#include"GcLibImpl.hpp"
#include"MainWindow.hpp"


/**********************************************************
WinMain
**********************************************************/
int WINAPI wWinMain(HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPWSTR lpCmdLine,
                        int nCmdShow )
{
	gstd::DebugUtility::DumpMemoryLeaksOnExit();

	try
	{
		DnhConfiguration::CreateInstance();
		ELogger* logger = ELogger::CreateInstance();
		logger->Initialize(false, false);
		EPathProperty::CreateInstance();
		MainWindow* wndMain = MainWindow::CreateInstance();
		wndMain->Initialize();

		EApplication* app = EApplication::CreateInstance();
		app->Initialize();
		app->Run();
	}
	catch(std::exception& e)
	{
		std::wstring log = StringUtility::ConvertMultiToWide(e.what());
		Logger::WriteTop(log);
	}
	catch(gstd::wexception& e)
	{
		Logger::WriteTop(e.what());
	}
//	catch(...)
//	{
//		Logger::WriteTop("ïsñæÇ»ÉGÉâÅ[");
//	}

	EApplication::DeleteInstance();
	MainWindow::DeleteInstance();
	EPathProperty::DeleteInstance();
	ELogger::DeleteInstance();
	DnhConfiguration::DeleteInstance();

	return 0;
}

