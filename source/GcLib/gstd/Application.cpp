#include"Application.hpp"
#include"Logger.hpp"

using namespace gstd;

/**********************************************************
//Application
**********************************************************/
Application* Application::thisBase_ = NULL;
Application::Application()
{
	::InitCommonControls();
}
Application::~Application()
{
	thisBase_=NULL;
}
bool Application::Initialize()
{
	if(thisBase_!=NULL)return false;
	thisBase_ = this;
	hAppInstance_ = ::GetModuleHandle(NULL);
	bAppRun_= true;
	bAppActive_=true;
	return true;
}
bool Application::Run()
{
	if(bAppRun_==false)
	{
		return false;
	}

	try
	{
		bool res = _Initialize();
		if(res == false)
			throw gstd::wexception(L"���������ɗ�O���������܂����B");
	}
	catch(std::exception& e)
	{
		std::wstring log = StringUtility::ConvertMultiToWide(e.what());
		Logger::WriteTop(log);
		Logger::WriteTop(L"���������ɗ�O���������܂����B�����I�����܂��B");
		bAppRun_ = false;
	}
	catch(gstd::wexception& e)
	{
		std::wstring log = e.what();
		Logger::WriteTop(log);
		Logger::WriteTop(L"���������ɗ�O���������܂����B�����I�����܂��B");
		bAppRun_ = false;
	}
	catch(...)
	{
		Logger::WriteTop(L"���������ɗ�O���������܂����B�����I�����܂��B");
		bAppRun_ = false;
	}

	MSG msg;
	while(true)
	{	
		if(bAppRun_ == false)break;
		if(::PeekMessage(&msg,0,0,0,PM_NOREMOVE))
		{
			if(!::GetMessage(&msg,NULL,0,0))break;
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{	
			if(bAppActive_ == false)
			{
				Sleep(10);
				continue;
			}
			try
			{
				if(_Loop() == false)break;
			}
			catch(std::exception& e)
			{
				std::wstring log = StringUtility::ConvertMultiToWide(e.what());
				Logger::WriteTop(log);
				Logger::WriteTop(L"���s���ɗ�O���������܂����B�I�����܂��B");
				break;
			}
			catch(gstd::wexception& e)
			{
				std::wstring log = e.what();
				Logger::WriteTop(log);
				Logger::WriteTop(L"���s���ɗ�O���������܂����B�I�����܂��B");
				break;
			}
//			catch(...)
//			{
//				Logger::WriteTop(L"���s���ɗ�O���������܂����B�I�����܂��B");
//				break;
//			}
		}
	}

	bAppRun_ = false;

	try
	{
		bool res = _Finalize();
		if(res == false)
			throw gstd::wexception(L"�I�����ɗ�O���������܂����B");
	}
	catch(std::exception& e)
	{
		std::wstring log = StringUtility::ConvertMultiToWide(e.what());
		Logger::WriteTop(log);
		Logger::WriteTop(L"����ɏI���ł��܂���ł����B");
	}
	catch(gstd::wexception& e)
	{
		std::wstring log = e.what();
		Logger::WriteTop(log);
		Logger::WriteTop(L"����ɏI���ł��܂���ł����B");
		bAppRun_ = false;
	}
	catch(...)
	{
		Logger::WriteTop(L"����ɏI���ł��܂���ł����B");
	}
	return true;
}

