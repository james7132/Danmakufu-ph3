#include"ScenePanel.hpp"
#include"MainWindow.hpp"


/**********************************************************
//EventScenePanel
**********************************************************/
LRESULT ScenePanel::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			int id = wParam & 0xffff;//LOWORD(wParam);
			if(id == buttonStart_.GetWindowId())
			{
				MainWindow* wndMain = MainWindow::GetInstance();
				if(wndMain->GetStgController() == NULL)
				{
					StartStg();
				}
				else
				{
					DirectSoundManager* soundManager = DirectSoundManager::GetBase();
					soundManager->Clear();
					EndStg();
				}

			}
			else if(id == buttonPause_.GetWindowId())
			{
				bool bCheck = SendMessage(buttonPause_.GetWindowHandle() , BM_GETCHECK , 0 , 0) == BST_CHECKED;
				ETaskManager::GetInstance()->SetWorkFunctionEnable(!bCheck);
				EFpsController::GetInstance()->SetCriticalFrame();
			}

			break;
		}

	}
	return WPanel::_WindowProcedure(hWnd, uMsg, wParam, lParam);
}
bool ScenePanel::Initialize(HWND hParent)
{
	Create(hParent);

	bFixedArea_ = false;

	buttonStart_.Create(hWnd_);
	buttonStart_.SetText(L"開始");

	panelPathEnemy_ = new ScriptPathPanel();
	panelPathEnemy_->Initialize(ScriptPathPanel::TYPE_ENEMY, hWnd_);
	panelPathPlayer_ = new ScriptPathPanel();
	panelPathPlayer_->Initialize(ScriptPathPanel::TYPE_PLAYER, hWnd_);

	WButton::Style styleCheck;
	styleCheck.SetStyle(WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX );
	buttonPause_.Create(hWnd_, styleCheck);
	buttonPause_.SetText(L"停止");
	buttonPause_.SetWindowEnable(false);

	GraphicsWindow* wndGraphics = MainWindow::GetInstance()->GetGraphicsWindow();
	wndGraphics->SetWindowType(GraphicsWindow::TYPE_WINDOW_PANEL, hWnd_);

	return true;
}
void ScenePanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	panelPathEnemy_->SetBounds(wx + 8, wy + 0, 480, 28);
	panelPathPlayer_->SetBounds(wx + 8, wy + 24, 480, 28);

	buttonStart_.SetBounds(wx + 8, wy + 60, 48, 20);
	buttonPause_.SetBounds(wx + 64, wy + 60, 48, 20);

	int gx = wx;
	int gy = wy + 88;
	int gWidth = 640;
	int gHeight = 480;

	if(!bFixedArea_)
	{
/*
		if(gWidth > gHeight*4/3)
		{//横幅が広い
			gWidth = gHeight*4/3;
			gHeight = gWidth*3/4;
			gHeight = gHeight > wHeight ? wHeight : gHeight;
			gWidth = gHeight*4/3;
		}
		else 
		{//縦幅が広い
			gHeight = gWidth*3/4;
			gHeight = gHeight > wHeight ? wHeight : gHeight;
			gWidth = gHeight*4/3;
		}
*/
		DnhConfiguration* config = DnhConfiguration::CreateInstance();
		int screenWidth = config->GetScreenWidth();
		int screenHeight = config->GetScreenHeight();

		int width = wWidth;
		int height = wHeight - gy;
		double ratioWH = (double)screenWidth / (double)screenHeight;
		double ratioHW = (double)screenHeight / (double)screenWidth;

		if(height > wHeight) height = wHeight;
		width = (double)height / ratioHW;

		if(width > wWidth)width = wWidth;
		height = (double)width / ratioWH;

		if(height > wHeight) height = wHeight;
		width = (double)height / ratioHW;

		gWidth = width;
		gHeight = height;
	}

	GraphicsWindow* wndGraphics = MainWindow::GetInstance()->GetGraphicsWindow();
	wndGraphics->SetBounds(gx, gy, gWidth, gHeight);
}
bool ScenePanel::StartStg()
{
	MainWindow* wndMain = MainWindow::GetInstance();
	std::wstring pathEnemy = panelPathEnemy_->GetPath();
	std::wstring pathPlayer = panelPathPlayer_->GetPath();
	bool res = false;
	try
	{
		ref_count_ptr<ScriptInformation> infoEnemy = panelPathEnemy_->GetSelectedScriptInformation();
		if(infoEnemy == NULL)throw gstd::wexception(L"敵スクリプトが不正です");

		ref_count_ptr<StgControllerForViewer> controller = StgControllerForViewer::Create();
		if(infoEnemy->GetType() == ScriptInformation::TYPE_PACKAGE)
		{
			ref_count_ptr<StgSystemInformation> infoStgSystem = new StgSystemInformation();
			infoStgSystem->SetMainScriptInformation(infoEnemy);
			controller->Initialize(infoStgSystem);
			controller->Start(NULL, NULL);
		}
		else
		{
			ref_count_ptr<ScriptInformation> infoPlayer = panelPathPlayer_->GetSelectedScriptInformation();
			if(infoPlayer == NULL)throw gstd::wexception(L"自機スクリプトが不正です");

			ref_count_ptr<StgSystemInformation> infoStgSystem = new StgSystemInformation();
			infoStgSystem->SetMainScriptInformation(infoEnemy);
			controller->Initialize(infoStgSystem);

			//ステージ
			controller->Start(infoPlayer, NULL);
		}

		wndMain->SetStgController(controller);
	}
	catch(gstd::wexception& e)
	{
		ErrorDialog::ShowErrorDialog(e.what());
		Logger::WriteTop(e.what());
	}
	catch(...)
	{
		MessageBox(hWnd_, L"開始失敗", L"error", MB_OK);
		Logger::WriteTop(L"開始失敗");
	}

	return true;
}
bool ScenePanel::EndStg()
{
	MainWindow* wndMain = MainWindow::GetInstance();
	wndMain->SetStgController(NULL);
	return true;
}
void ScenePanel::SetStgState(bool bStart)
{
	if(bStart)
	{
		buttonStart_.SetText(L"終了");
		buttonPause_.SetWindowEnable(true);

		panelPathEnemy_->SetWindowEnable(false);
		panelPathPlayer_->SetWindowEnable(false);
	}
	else
	{
		buttonStart_.SetText(L"開始");
		buttonPause_.SetWindowEnable(false);
		panelPathEnemy_->SetWindowEnable(true);
		panelPathPlayer_->SetWindowEnable(true);
		::SendMessage(buttonPause_.GetWindowHandle(), BM_SETCHECK, BST_UNCHECKED, 0);
	}
}
void ScenePanel::ClearData()
{
	panelPathEnemy_->SetPath(L"");
	panelPathPlayer_->SetPath(L"");
}
void ScenePanel::Read(RecordBuffer& record)
{
	std::wstring pathEnemy = record.GetRecordAsStringW("pathEnemy");
	panelPathEnemy_->SetPath(pathEnemy);
	std::wstring pathPlayer = record.GetRecordAsStringW("pathPlayer");
	panelPathPlayer_->SetPath(pathPlayer);
}
void ScenePanel::Write(RecordBuffer& record)
{
	record.SetRecordAsStringW("pathEnemy", panelPathEnemy_->GetPath());
	record.SetRecordAsStringW("pathPlayer", panelPathPlayer_->GetPath());
}

//ScriptPathPanel
ScenePanel::ScriptPathPanel::~ScriptPathPanel()
{

}
bool ScenePanel::ScriptPathPanel::Initialize(int type, HWND hParent)
{
	Create(hParent);

	gstd::WEditBox::Style styleEdit;
	styleEdit.SetStyle(WS_CHILD | WS_VISIBLE | WS_HSCROLL | ES_AUTOHSCROLL);
	styleEdit.SetStyleEx(WS_EX_CLIENTEDGE);
	WButton::Style stylePath;
	stylePath.SetStyle(WS_CHILD | WS_VISIBLE | BS_PUSHLIKE);

	{
		labelPath_.Create(hWnd_);
		if(type == TYPE_ENEMY)
			labelPath_.SetText(L"敵スクリプト");
		else
			labelPath_.SetText(L"自機スクリプト");

		editPath_.Create(hWnd_, styleEdit);


		buttonPath_.Create(hWnd_, stylePath);
		buttonPath_.SetText(L"選択");
	}

	labelPath_.SetBounds(8, 12, 72, 20);
	editPath_.SetBounds(80, 8, 300, 20);
	buttonPath_.SetBounds(388, 8, 48, 20);

	DragAcceptFiles(hWnd_, TRUE);

	if(type == TYPE_ENEMY)
		dialogSelect_ = new EnemySelectDialog();
	else 
		dialogSelect_ = new PlayerSelectDialog();

	dialogSelect_->Initialize();

	return true;
}
void ScenePanel::ScriptPathPanel::SetWindowEnable(bool bEnable)
{
	editPath_.SetWindowEnable(bEnable);
	buttonPath_.SetWindowEnable(bEnable);
}
LRESULT ScenePanel::ScriptPathPanel::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			int id = wParam & 0xffff;//LOWORD(wParam);
			if(id == buttonPath_.GetWindowId())
			{
/*
				std::string path;
				path.resize(MAX_PATH*4);
				OPENFILENAME ofn;
				ZeroMemory(&ofn,sizeof(OPENFILENAME));
				ofn.lStructSize=sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd_;
				ofn.nMaxFile = path.size();
				ofn.lpstrFile = &path[0];
				ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
				ofn.nFilterIndex = 1;
				ofn.lpstrDefExt = ".txt";
				ofn.lpstrFilter = "全てのファイル (*.*)\0*.*\0";
				ofn.lpstrTitle = "スクリプトを開く";
				if(GetOpenFileName(&ofn))
				{
					editPath_.SetText(path);
				}
*/
				std::wstring path = editPath_.GetText();
				dialogSelect_->ShowModal(path);

				ref_count_ptr<ScriptInformation> info = dialogSelect_->GetSelectedScript();
				if(info != NULL)
				{
					infoSelected_ = info;
					editPath_.SetText(info->GetScriptPath());
				}
			}
			break;
		}
		case WM_DROPFILES:
		{
			wchar_t szFileName[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			UINT uFileNo = DragQueryFile((HDROP)wParam,0xFFFFFFFF,NULL,0);

			for(int iDrop = 0 ; iDrop < (int)uFileNo ; iDrop++)
			{
				DragQueryFile(hDrop, iDrop, szFileName, sizeof(szFileName));
				std::wstring path = szFileName;

				ref_count_ptr<ScriptInformation> info = _CreateScriptInformation(path);
				if(info != NULL)
				{
					infoSelected_ = info;
					editPath_.SetText(info->GetScriptPath());
				}
				break;
			}
			DragFinish(hDrop);

			break;
		}
	}
	return WPanel::_WindowProcedure(hWnd, uMsg, wParam, lParam);
}
ref_count_ptr<ScriptInformation> ScenePanel::ScriptPathPanel::_CreateScriptInformation(std::wstring path)
{
	File file(path);
	if(!file.Open())return NULL;

	std::string source = "";
	int size = file.GetSize();
	source.resize(size);
	file.Read(&source[0], size);

	ref_count_ptr<ScriptInformation> info = 
		ScriptInformation::CreateScriptInformation(path, L"", source);

	return info;
}
void ScenePanel::ScriptPathPanel::SetPath(std::wstring path)
{
	ref_count_ptr<ScriptInformation> info = _CreateScriptInformation(path);
	if(info == NULL)return;

	infoSelected_ = info;
	editPath_.SetText(path);
}


