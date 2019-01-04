#include"DnhGcLibImpl.hpp"
#include"DnhCommon.hpp"

/**********************************************************
//EPathProperty
**********************************************************/
std::wstring EPathProperty::GetSystemResourceDirectory()
{
	std::wstring path = GetModuleDirectory() + L"resource/";
	return path;
}
std::wstring EPathProperty::GetSystemImageDirectory()
{
	std::wstring path = GetSystemResourceDirectory() + L"img/";
	return path;
}
std::wstring EPathProperty::GetSystemBgmDirectory()
{
	std::wstring path = GetSystemResourceDirectory() + L"bgm/";
	return path;
}
std::wstring EPathProperty::GetSystemSeDirectory()
{
	std::wstring path = GetSystemResourceDirectory() + L"se/";
	return path;
}

std::wstring EPathProperty::GetStgScriptRootDirectory()
{
	std::wstring path = GetModuleDirectory() + L"script/";
	return path;
}
std::wstring EPathProperty::GetStgDefaultScriptDirectory()
{
	std::wstring path = GetStgScriptRootDirectory() + L"default_system/";
	return path;
}
std::wstring EPathProperty::GetPlayerScriptRootDirectory()
{
	std::wstring path = GetModuleDirectory() + L"script/player/";
	return path;
}
std::wstring EPathProperty::GetReplaySaveDirectory(std::wstring scriptPath)
{
	std::wstring scriptName = PathProperty::GetFileNameWithoutExtension(scriptPath);
	std::wstring dir = PathProperty::GetFileDirectory(scriptPath) + L"replay/";
	return dir;
}
std::wstring EPathProperty::GetCommonDataPath(std::wstring scriptPath, std::wstring area)
{
	std::wstring dirSave = PathProperty::GetFileDirectory(scriptPath) + L"data/";
	std::wstring nameMain = PathProperty::GetFileNameWithoutExtension(scriptPath);
	std::wstring path = dirSave + nameMain + StringUtility::Format(L"_common_%s.dat", area.c_str());
	return path;
}
std::wstring EPathProperty::ExtendRelativeToFull(std::wstring dir, std::wstring path)
{
	path = StringUtility::ReplaceAll(path, L"\\", L"/");
	if(path.size() >= 2)
	{
		if(path[0] == L'.' && path[1] == L'/')
		{
			path = path.substr(2);
			path = dir + path;
		}
	}

	std::wstring drive = PathProperty::GetDriveName(path);
	if(drive.size() == 0)
	{
		path = GetModuleDirectory() + path;
	}

	return path;
}


/**********************************************************
//ELogger
**********************************************************/
ELogger::ELogger()
{

}
void ELogger::Initialize(bool bFile, bool bWindow)
{
	gstd::ref_count_ptr<gstd::FileLogger> fileLogger = new gstd::FileLogger();
	fileLogger->Initialize(bFile);
	fileLogger->Clear();

	this->AddLogger(fileLogger);

	Logger::SetTop(this);
	WindowLogger::Initialize(bWindow);

	panelCommonData_ = new gstd::ScriptCommonDataInfoPanel();
}
void ELogger::UpdateCommonDataInfoPanel(gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager)
{
	panelCommonData_->Update(commonDataManager);
}

/**********************************************************
//EFpsController
**********************************************************/
EFpsController::EFpsController()
{
	DnhConfiguration* config = DnhConfiguration::GetInstance();
	int fpsType = config->GetFpsType();
	if(fpsType == DnhConfiguration::FPS_NORMAL ||
		fpsType == DnhConfiguration::FPS_1_2 ||
		fpsType == DnhConfiguration::FPS_1_3)
	{
		StaticFpsController* controller = new StaticFpsController();
		if(fpsType == DnhConfiguration::FPS_1_2)
			controller->SetSkipRate(1);
		else if(fpsType == DnhConfiguration::FPS_1_3)
			controller->SetSkipRate(2);
		controller_ = controller;
	}
	else
	{
		AutoSkipFpsController* controller = new AutoSkipFpsController();
		controller_ = controller;
	}

	SetFps(STANDARD_FPS);
	fastModeKey_ = DIK_LCONTROL;
}

/**********************************************************
//EFileManager
**********************************************************/
void EFileManager::ResetArchiveFile()
{
	mapArchiveFile_.clear();
}

/**********************************************************
//ETaskManager
**********************************************************/
bool ETaskManager::Initialize()
{
	InitializeFunctionDivision(TASK_WORK_PRI_MAX, TASK_RENDER_PRI_MAX);
	return true;
}


/**********************************************************
//ETextureManager
**********************************************************/
bool ETextureManager::Initialize()
{
	bool res = TextureManager::Initialize();

	for(int iRender = 0 ; iRender < MAX_RESERVED_RENDERTARGET ; iRender++)
	{
		std::wstring name = GetReservedRenderTargetName(iRender);
		ref_count_ptr<Texture> texture = new Texture();
		res &= texture->CreateRenderTarget(name);
		Add(name, texture);
	}

	if(!res)
	{
		throw gstd::wexception(L"ETextureManager���������s");
	}
	return res;
}
std::wstring ETextureManager::GetReservedRenderTargetName(int index)
{
	std::wstring res = L"__RESERVED_RENDER_TARGET__";
	res += StringUtility::Format(L"%d", index);
	return res;
}

/**********************************************************
//EDirectInput
**********************************************************/
bool EDirectInput::Initialize(HWND hWnd)
{
	padIndex_ = 0;

	VirtualKeyManager::Initialize(hWnd);

	ResetVirtualKeyMap();

	return true;
}
void EDirectInput::ResetVirtualKeyMap()
{
	ClearKeyMap();

	//�L�[�o�^
	DnhConfiguration* config = DnhConfiguration::GetInstance();

	AddKeyMap(KEY_LEFT, config->GetVirtualKey(KEY_LEFT));
	AddKeyMap(KEY_RIGHT, config->GetVirtualKey(KEY_RIGHT));
	AddKeyMap(KEY_UP, config->GetVirtualKey(KEY_UP));
	AddKeyMap(KEY_DOWN, config->GetVirtualKey(KEY_DOWN));

	AddKeyMap(KEY_OK, config->GetVirtualKey(KEY_OK));
	AddKeyMap(KEY_CANCEL, config->GetVirtualKey(KEY_CANCEL));	

	AddKeyMap(KEY_SHOT, config->GetVirtualKey(KEY_SHOT));
	AddKeyMap(KEY_BOMB, config->GetVirtualKey(KEY_BOMB));
	AddKeyMap(KEY_SLOWMOVE, config->GetVirtualKey(KEY_SLOWMOVE));
	AddKeyMap(KEY_USER1, config->GetVirtualKey(KEY_USER1));
	AddKeyMap(KEY_USER2, config->GetVirtualKey(KEY_USER2));

	AddKeyMap(KEY_PAUSE, config->GetVirtualKey(KEY_PAUSE));
}
