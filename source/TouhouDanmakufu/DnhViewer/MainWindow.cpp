#include"MainWindow.hpp"

/**********************************************************
//MainWindow
**********************************************************/
MainWindow::MainWindow()
{

}
MainWindow::~MainWindow()
{
	wndGraphics_ = NULL;
}
bool MainWindow::Initialize()
{
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	std::wstring nameClass = L"DnhViewerMainWindow";
	WNDCLASSEX wcex;
	ZeroMemory(&wcex,sizeof(wcex));
	wcex.cbSize=sizeof(WNDCLASSEX); 
	wcex.style=CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc=(WNDPROC)WindowBase::_StaticWindowProcedure;
	wcex.hInstance=hInst;
	wcex.hIcon=NULL;
	wcex.hCursor=LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground=(HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName=MAKEINTRESOURCE(IDR_MENU_MAIN);
	wcex.lpszClassName=nameClass.c_str();
	wcex.hIconSm=NULL;
	::RegisterClassEx(&wcex);

	std::wstring appName = L"DnhViewer ph3 ";
	appName += DNH_VERSION;
   	hWnd_=::CreateWindow(wcex.lpszClassName,
		appName.c_str(),
		WS_OVERLAPPEDWINDOW,//-WS_SIZEBOX
		0,0,800,600,NULL,NULL,hInst,NULL);
	::ShowWindow(hWnd_, SW_HIDE);
	::UpdateWindow(hWnd_);
	this->Attach(hWnd_);

	//タブ
	wndTab_ = new WTabControll();
	wndTab_->Create(hWnd_);
	HWND hTab = wndTab_->GetWindowHandle();

	//ステータスバー
	wndStatus_ = new WStatusBar();
	wndStatus_->Create(hWnd_);
	std::vector<int> sizeStatus;
	sizeStatus.push_back(180);
	sizeStatus.push_back(sizeStatus[0] + 560);
	wndStatus_->SetPartsSize(sizeStatus);

	//GraphicsWindow
	wndGraphics_ = new GraphicsWindow();
	wndGraphics_->Initialize();

	//EventScene
	panelScene_ = new ScenePanel();
	panelScene_->Initialize(hTab);
	wndTab_->AddTab(L"Scene", panelScene_);

	//初期化完了
	MoveWindowCenter();

	wndTab_->ShowPage();

	::ShowWindow(hWnd_, SW_SHOW);

	std::wstring pathLastData = PathProperty::GetModuleDirectory() + PathProperty::GetModuleName() + L".dat";
	MainWindow::GetInstance()->Load(pathLastData);

	//デバッグウィンドウ
	wndDebug_ = new DebugWindow();
	wndDebug_->Initialize();
	wndDebug_->ShowModeless();

	return true;
}
LRESULT MainWindow::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			std::wstring pathLastData = PathProperty::GetModuleDirectory() + PathProperty::GetModuleName() + L".dat";
			MainWindow::GetInstance()->Save(pathLastData);
			::DestroyWindow(hWnd);
			break;
		}
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			break;
		}
		case WM_SIZE:
		{
			RECT rect;
			::GetClientRect(hWnd_, &rect);
			int wx = rect.left;
			int wy = rect.top;
			int wWidth = rect.right-rect.left;
			int wHeight = rect.bottom-rect.top;

			wndStatus_->SetBounds(wParam, lParam);
			wndTab_->SetBounds(wx+8, wy+4, wWidth-16, wHeight-32);
			::InvalidateRect(wndTab_->GetWindowHandle(), NULL, TRUE);
			break;
		}
		case WM_COMMAND:
		{
			switch(wParam & 0xffff)
			{
				case ID_MENUITEM_MAIN_EXIT:
					DestroyWindow(hWnd_);
					break;

				case ID_MENUITEM_MAIN_LOGWINDOW:
				{
					bool bVisible = !ELogger::GetInstance()->IsWindowVisible();
					ELogger::GetInstance()->SetWindowVisible(bVisible);
					break;
				}

				case ID_MENUITEM_MAIN_DEBUGWINDOW:
				{
					bool bVisible = !wndDebug_->IsWindowVisible();
					wndDebug_->SetWindowVisible(bVisible);
					break;
				}

				case ID_MENUITEM_MAIN_FIXEDAREA:
				{
					//表示域固定
					panelScene_->SetFixedArea(!panelScene_->IsFixedArea());
					panelScene_->LocateParts();
					break;
				}
			}
			break;
		}

		case WM_SYSCOMMAND:
		{
			int nId = wParam & 0xffff;
			if (nId == WindowLogger::MENU_ID_OPEN)
			{
				ELogger::GetInstance()->ShowLogWindow();
			}
			break;
		}

		case WM_NOTIFY:
		{
			switch (((NMHDR *)lParam)->code)
			{
				case TCN_SELCHANGE:
					wndTab_->ShowPage();
					break;
			}
			break;
		}
		case WM_INITMENUPOPUP:
		{
			//HMENU hMenu = (HMENU) wParam;         // サブメニューのハンドル
			//int pos =  LOWORD(lParam);        // サブメニュー項目の位置
			HMENU hMenuMain = GetMenu(hWnd_);

			DWORD valueLog = ELogger::GetInstance()->IsWindowVisible() ? MFS_CHECKED : MFS_UNCHECKED;
			CheckMenuItem(hMenuMain, ID_MENUITEM_MAIN_LOGWINDOW, valueLog | MF_BYCOMMAND);

			DWORD valueDebug = wndDebug_->IsWindowVisible() ? MFS_CHECKED : MFS_UNCHECKED;
			CheckMenuItem(hMenuMain, ID_MENUITEM_MAIN_DEBUGWINDOW, valueDebug | MF_BYCOMMAND);

			DWORD valueFixed = panelScene_->IsFixedArea() ? MFS_CHECKED : MFS_UNCHECKED;
			CheckMenuItem(hMenuMain, ID_MENUITEM_MAIN_FIXEDAREA, valueFixed | MF_BYCOMMAND);
			return FALSE;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
void MainWindow::SetStgController(ref_count_ptr<StgControllerForViewer> controller)
{
	if(controller_ == controller)return;
	if(controller == NULL)
	{
		panelScene_->SetStgState(false);
	}
	else
	{
		panelScene_->SetStgState(true);
	}

	ETaskManager* task = ETaskManager::GetInstance();
	if(controller_ != NULL)
	{
		EFpsController* fpsController = EFpsController::GetInstance();
		fpsController->SetFastModeKey(DIK_LCONTROL);

		task->RemoveTask(controller_.GetPointer());
		controller_ = NULL;
		Logger::WriteTop(L"STGシーン終了");
	}

	controller_ = controller;
	if(controller_ != NULL)
	{
		Logger::WriteTop(L"STGシーン開始");
		try
		{
			DirectGraphics* graphics = DirectGraphics::GetBase();
			graphics->GetCamera2D()->Reset();

//			controller_->Initialize();
			task->AddTask(controller_);
			task->AddWorkFunction(TTaskFunction<StgControllerForViewer>::Create(controller_, &StgControllerForViewer::Work), StgSystemController::TASK_PRI_WORK);
			task->AddRenderFunction(TTaskFunction<StgControllerForViewer>::Create(controller_, &StgControllerForViewer::Render), StgSystemController::TASK_PRI_RENDER);
		}
		catch(gstd::wexception& e)
		{
			Logger::WriteTop(e.what());
			ErrorDialog::ShowErrorDialog(e.what());
		}

	}
}
void MainWindow::ClearData()
{

}
bool MainWindow::Load(std::wstring path)
{
	RecordBuffer record;
	bool res = record.ReadFromFile(path);
	if(!res)
	{
		//::MessageBox(hWnd_, "読み込み失敗", "設定を開く", MB_OK);
		return false;
	}

	RecordBuffer recScene;
	record.GetRecordAsRecordBuffer("Scene", recScene);
	panelScene_->Read(recScene);

	return true;
}
bool MainWindow::Save(std::wstring path)
{
	RecordBuffer recScene;
	panelScene_->Write(recScene);

	RecordBuffer record;
	record.SetRecordAsRecordBuffer("Scene", recScene);
	record.WriteToFile(path);

	return true;
}

/**********************************************************
//GraphicsWindow
**********************************************************/
GraphicsWindow::GraphicsWindow()
{
}
GraphicsWindow::~GraphicsWindow()
{
}
bool GraphicsWindow::Initialize()
{
	HWND hParent = MainWindow::GetInstance()->GetWindowHandle();
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	std::wstring nameClass = L"DnhViewerGraphicsWindow";
	WNDCLASSEX wcex;
	ZeroMemory(&wcex,sizeof(wcex));
	wcex.cbSize=sizeof(WNDCLASSEX); 
	wcex.style=CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc=(WNDPROC)WindowBase::_StaticWindowProcedure;
	wcex.hInstance=hInst;
	wcex.hIcon=NULL;
	wcex.hCursor=LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground=(HBRUSH)(COLOR_WINDOW+2);
	wcex.lpszMenuName=NULL;
	wcex.lpszClassName=nameClass.c_str();
	wcex.hIconSm=NULL;
	::RegisterClassEx(&wcex);

   	hWnd_=::CreateWindow(wcex.lpszClassName,
		L"",
		WS_OVERLAPPEDWINDOW-WS_SIZEBOX,
		0,0,800,600,hParent,NULL,hInst,NULL);
	::ShowWindow(hWnd_, SW_SHOW);
	::UpdateWindow(hWnd_);
	this->Attach(hWnd_);

//Windowを画面の中央に移動
	MoveWindowCenter();

	::ShowWindow(hWnd_, SW_HIDE);

	return true;
}
LRESULT GraphicsWindow::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			::ShowWindow(hWnd_, SW_HIDE);
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
void GraphicsWindow::SetWindowType(int type, HWND hParent)
{
	if(type == TYPE_WINDOW_PANEL)
	{
		RemoveWindowStyle(WS_OVERLAPPEDWINDOW);
		SetWindowStyle(WS_CHILD);
		SetWindowLong(hWnd_, GWL_EXSTYLE, WS_EX_STATICEDGE);
		SetParentWindow(hParent);
		SetWindowVisible(true);
	}
	else if(TYPE_WINDOW_OVERLAPPED)
	{
		RemoveWindowStyle(WS_CHILD);
		SetWindowStyle(WS_OVERLAPPEDWINDOW);
		SetWindowLong(hWnd_, GWL_EXSTYLE, 0);
		SetParentWindow(hParent);
		SetWindowVisible(false);
	}
}

/**********************************************************
//StgControllerForViewer
**********************************************************/
void StgControllerForViewer::DoEnd()
{
	MainWindow::GetInstance()->SetStgController(NULL);
}
void StgControllerForViewer::DoRetry()
{
	ScenePanel* panel = MainWindow::GetInstance()->GetScenePanel();
//	panel->EndStg();
	panel->StartStg();
}

ref_count_ptr<StgControllerForViewer> StgControllerForViewer::Create()
{
	ref_count_ptr<StgControllerForViewer> res = new StgControllerForViewer();

	return res;
}
