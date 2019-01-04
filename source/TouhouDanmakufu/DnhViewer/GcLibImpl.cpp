#include"GcLibImpl.hpp"
#include"MainWindow.hpp"
#include"DebugWindow.hpp"

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

	EFpsController* fpsController = EFpsController::CreateInstance();

	EDirectGraphics* graphics = EDirectGraphics::CreateInstance();
	graphics->Initialize();
	HWND hWndMain = MainWindow::GetInstance()->GetWindowHandle();
	WindowLogger::InsertOpenCommandInSystemMenu(hWndMain);
//	::SetWindowText(hWndMain, "DnhViewer");
//	::SetClassLong(hWndMain, GCL_HICON, (LONG)LoadIcon(GetApplicationHandle(), MAKEINTRESOURCE(IDI_ICON)));

	ErrorDialog::SetParentWindowHandle(hWndMain);

	ETextureManager* textureManager = ETextureManager::CreateInstance();
	textureManager->Initialize();

	EShaderManager* shaderManager = EShaderManager::CreateInstance();
	shaderManager->Initialize();

	EMeshManager* meshManager = EMeshManager::CreateInstance();
	meshManager->Initialize();

	EDxTextRenderer* textRenderer = EDxTextRenderer::CreateInstance();
	textRenderer->Initialize();

	EDirectSoundManager* soundManager = EDirectSoundManager::CreateInstance();
	soundManager->Initialize(hWndMain);

	EDirectInput* input = EDirectInput::CreateInstance();
	input->Initialize(hWndMain);

	ETaskManager* taskManager = ETaskManager::CreateInstance();
	taskManager->Initialize();

	gstd::ref_count_ptr<gstd::TaskInfoPanel> panelTask = new gstd::TaskInfoPanel();
	bool bAddTaskPanel = logger->AddPanel(panelTask, L"Task");
	if(bAddTaskPanel)taskManager->SetInfoPanel(panelTask);

	gstd::ref_count_ptr<directx::TextureInfoPanel> panelTexture = new directx::TextureInfoPanel();
	bool bTexturePanel = logger->AddPanel(panelTexture, L"Texture");
	if(bTexturePanel)textureManager->SetInfoPanel(panelTexture);

	gstd::ref_count_ptr<directx::DxMeshInfoPanel> panelMesh = new directx::DxMeshInfoPanel();
	bool bMeshPanel = logger->AddPanel(panelMesh, L"Mesh");
	if(bMeshPanel)meshManager->SetInfoPanel(panelMesh);

	gstd::ref_count_ptr<directx::SoundInfoPanel> panelSound = new directx::SoundInfoPanel();
	bool bSoundPanel = logger->AddPanel(panelSound, L"Sound");
	if(bSoundPanel)soundManager->SetInfoPanel(panelSound);

	gstd::ref_count_ptr<gstd::ScriptCommonDataInfoPanel> panelCommonData = logger->GetScriptCommonDataInfoPanel();
	logger->AddPanel(panelCommonData, L"Common Data");

	gstd::ref_count_ptr<ScriptInfoPanel> panelScript = new ScriptInfoPanel();
	logger->AddPanel(panelScript, L"Script");

	DnhConfiguration* config = DnhConfiguration::CreateInstance();
	if(config->IsLogWindow())
	{
		logger->LoadState();
		logger->SetWindowVisible(true);
	}

//	SystemController* systemController = SystemController::CreateInstance();
//	systemController->Reset();

	//常駐タスク登録
	ref_count_ptr<SystemResidentTask> taskResident = new SystemResidentTask();
	taskManager->AddTask(taskResident);
	taskManager->AddRenderFunction(TTaskFunction<SystemResidentTask>::Create(
		taskResident, &SystemResidentTask::RenderFps), SystemResidentTask::TASK_PRI_RENDER_FPS);

	//デバッグタスク登録
	ref_count_ptr<DebugTask> taskDebug = new DebugTask();
	taskManager->AddTask(taskDebug);
	taskManager->AddWorkFunction(TTaskFunction<DebugTask>::Create(
		taskDebug, &DebugTask::Work), DebugTask::TASK_PRI_WORK);

	MainWindow* wndMain = MainWindow::GetInstance();
	::SetForegroundWindow(wndMain->GetWindowHandle());

	Logger::WriteTop(L"アプリケーション初期化完了");

	return true;
}
bool EApplication::_Loop()
{
	ELogger* logger = ELogger::GetInstance();
	ETaskManager* taskManager = ETaskManager::GetInstance();
	EFpsController* fpsController = EFpsController::GetInstance();
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();

	MainWindow* mainWindow = MainWindow::GetInstance();
	HWND hWndFocused = ::GetForegroundWindow();
	HWND hWndGraphics = mainWindow->GetWindowHandle();
	HWND hWndLogger = ELogger::GetInstance()->GetWindowHandle();
	HWND hWndDebug = mainWindow->GetDebugWindow()->GetWindowHandle();
	if(hWndFocused != hWndGraphics && hWndFocused != hWndLogger && hWndFocused != hWndDebug)
	{
		//非アクティブ時は動作しない
		::Sleep(10);
		return true;
	}

	EDirectInput* input = EDirectInput::GetInstance();
	input->Update();
	if(input->GetKeyState(DIK_R) == KEY_PUSH)
	{
		//リセット
//		SystemController* systemController = SystemController::CreateInstance();
//		systemController->Reset();
	}
	
	taskManager->CallWorkFunction();

	if(!fpsController->IsSkip())
	{
		graphics->BeginScene();
		taskManager->CallRenderFunction();
		graphics->EndScene();
	}

	fpsController->Wait();

	//ログ関連
	SYSTEMTIME time;
	GetLocalTime(&time);
	std::wstring fps = StringUtility::Format(L"Work：%.2ffps、Draw：%.2ffps",
		fpsController->GetCurrentWorkFps(), 
		fpsController->GetCurrentRenderFps());
	logger->SetInfo(0, L"fps", fps);

	int widthConfig = graphics->GetConfigData().GetScreenWidth();
	int heightConfig = graphics->GetConfigData().GetScreenHeight();
	int widthScreen = widthConfig * graphics->GetScreenWidthRatio();
	int heightScreen = heightConfig * graphics->GetScreenHeightRatio();

	std::wstring screen = StringUtility::Format(L"width：%d/%d、height：%d/%d",
		widthScreen, widthConfig, 
		heightScreen, heightConfig);
	logger->SetInfo(1, L"screen", screen);

	logger->SetInfo(2, L"font cache", 
		StringUtility::Format(L"%d", EDxTextRenderer::GetInstance()->GetCacheCount() ));
	
	//高速動作
	int fastModeKey = fpsController->GetFastModeKey();
	if(input->GetKeyState(fastModeKey) == KEY_HOLD)
	{
		if(!fpsController->IsFastMode())
			fpsController->SetFastMode(true);
	}
	else if(input->GetKeyState(fastModeKey) == KEY_PULL)
	{
		if(fpsController->IsFastMode())
			fpsController->SetFastMode(false);
	}
	return true;
}
bool EApplication::_Finalize()
{
	Logger::WriteTop(L"アプリケーション終了処理開始");
	MainWindow::GetInstance()->SetStgController(NULL);
	ETaskManager::DeleteInstance();
	EFileManager::GetInstance()->EndLoadThread();
	EDirectInput::DeleteInstance();
	EDirectSoundManager::DeleteInstance();
	EDxTextRenderer::DeleteInstance();
	EMeshManager::DeleteInstance();
	EShaderManager::DeleteInstance();
	ETextureManager::DeleteInstance();
	EDirectGraphics::DeleteInstance();
	EFpsController::DeleteInstance();
	EFileManager::DeleteInstance();

	ELogger* logger = ELogger::GetInstance();
	logger->SaveState();
	logger->SetWindowVisible(true);

	Logger::WriteTop(L"アプリケーション終了処理完了");
	return true;
}




/**********************************************************
//EDirectGraphics
**********************************************************/
EDirectGraphics::EDirectGraphics()
{

}
EDirectGraphics::~EDirectGraphics()
{
}
bool EDirectGraphics::Initialize()
{
	DnhConfiguration* dnhConfig = DnhConfiguration::GetInstance();
	int screenWidth = dnhConfig->GetScreenWidth();
	int screenHeight = dnhConfig->GetScreenHeight();

	HWND hWndGraph = MainWindow::GetInstance()->GetGraphicsWindow()->GetWindowHandle();
	DirectGraphicsConfig config;
	config.SetScreenWidth(screenWidth);
	config.SetScreenHeight(screenHeight);
	return DirectGraphics::Initialize(hWndGraph, config);
}
void EDirectGraphics::SetRenderStateFor2D(int blend)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(blend);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
}
