#include"DebugWindow.hpp"
#include"MainWindow.hpp"

/**********************************************************
//DebugWindow
**********************************************************/
DebugWindow::DebugWindow()
{
}
void DebugWindow::Initialize()
{
	MainWindow* mainWindow = MainWindow::GetInstance();
	HWND hParent = MainWindow::GetInstance()->GetWindowHandle();
	hWnd_ = CreateDialog((HINSTANCE)GetWindowLong(hParent,GWL_HINSTANCE),
							MAKEINTRESOURCE(IDD_DIALOG_DEBUG),
							hParent,(DLGPROC)this->_StaticWindowProcedure);
	this->Attach(hWnd_);

	int wx = mainWindow->GetClientX() + mainWindow->GetClientWidth() - GetClientWidth();
	int wy = mainWindow->GetClientY();
	POINT pos = {wx, wy};
	ClientToScreen(hParent, &pos);

	checkPlayerInvincivility_.Attach(GetDlgItem(hWnd_, IDC_CHECK_PLAYER_INVINCIBILITY));

	::SetWindowPos(hWnd_, HWND_TOP, pos.x, pos.y, 0, 0, SWP_NOSIZE);
}
void DebugWindow::ShowModeless()
{
	SetWindowVisible(true);
}
LRESULT DebugWindow::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CLOSE:
		{
			SetWindowVisible(false);
			break;
		}
		case WM_COMMAND:
		{
			switch(wParam & 0xffff)
			{
				case IDC_CHECK_PLAYER_INVINCIBILITY:
					break;
			}
			break;
		}

	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}

void DebugWindow::SetPlayerInvincivility(bool bInvincivility)
{
	SendMessage(checkPlayerInvincivility_.GetWindowHandle() , BM_SETCHECK , 
		bInvincivility ? BST_CHECKED : BST_UNCHECKED , 0);
}
bool DebugWindow::IsPlayerInvincivility()
{
	bool res = 
		SendMessage(checkPlayerInvincivility_.GetWindowHandle() , BM_GETCHECK , 0 , 0) == BST_CHECKED;
	return res;
}


/**********************************************************
//DebugTask
**********************************************************/
DebugTask::DebugTask()
{
	bPlayerInvincivility_ = false;
}
DebugTask::~DebugTask()
{
}
void DebugTask::Work()
{
	MainWindow* mainWindow = MainWindow::GetInstance();
	ref_count_ptr<StgControllerForViewer> controller = mainWindow->GetStgController();
	DebugWindow* wndDebug = mainWindow->GetDebugWindow();

	EDirectInput* input = EDirectInput::GetInstance();
	if(input->GetKeyState(DIK_I) == KEY_PUSH)
	{
		bool bPlayerInvincivility = wndDebug->IsPlayerInvincivility();
		wndDebug->SetPlayerInvincivility(!bPlayerInvincivility);
	}

	if(controller == NULL)
	{
		bPlayerInvincivility_ = false;
	}
	else
	{
		int FRAME_PLAYER_INVINCIVILITY = 256 * 256 * 256;
		bool bPlayerInvincivility = wndDebug->IsPlayerInvincivility();
		StgStageController* stageController = controller->GetStageController();
		if(stageController != NULL)
		{
			ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
			if(bPlayerInvincivility && objPlayer->GetInvincibilityFrame() < FRAME_PLAYER_INVINCIVILITY)
			{
				//Ž©‹@–³“G
				objPlayer->SetInvincibilityFrame(FRAME_PLAYER_INVINCIVILITY);
			}
			else if(bPlayerInvincivility_ && !bPlayerInvincivility)
			{
				//Ž©‹@–³“G‰ðœ
				objPlayer->SetInvincibilityFrame(0);
			}
		}

		bPlayerInvincivility_ = bPlayerInvincivility;
	}



}
