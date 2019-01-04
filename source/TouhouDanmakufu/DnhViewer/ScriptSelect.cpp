#include"ScriptSelect.hpp"
#include"MainWindow.hpp"

/**********************************************************
//ScriptSelectDialog
**********************************************************/
const std::wstring ScriptSelectDialog::KEY_ALL = L"All";
const std::wstring ScriptSelectDialog::KEY_SINGLE = L"Single";
const std::wstring ScriptSelectDialog::KEY_PLURAL = L"Plural";
const std::wstring ScriptSelectDialog::KEY_STAGE = L"Stage";
const std::wstring ScriptSelectDialog::KEY_PACKAGE = L"Package";
const std::wstring ScriptSelectDialog::KEY_PLAYER = L"Player";
void ScriptSelectDialog::Initialize()
{
	hParent_ = MainWindow::GetInstance()->GetWindowHandle();
	Create(hParent_, MAKEINTRESOURCE(IDD_DIALOG_SELECT_SCRIPT));
	SetWindowVisible(false);

	//ListView
	HWND hList=GetDlgItem(hWnd_, IDC_LIST_SELECT_SCRIPT);
	listView_.Attach(hList);
	DWORD dwStyle=ListView_GetExtendedListViewStyle(hList);
	dwStyle |= LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES;
	ListView_SetExtendedListViewStyle(hList, dwStyle);

	listView_.AddColumn(48, LIST_TYPE, L"Type");
	listView_.AddColumn(200, LIST_NAME, L"Name");
	listView_.AddColumn(64, LIST_ARCHIVE, L"Archive");
	listView_.AddColumn(64, LIST_PATH, L"Path");
	listView_.AddColumn(300, LIST_TEXT, L"Text");

	//ボタン
	buttonOk_.Attach(GetDlgItem(hWnd_, IDOK));
	buttonCancel_.Attach(GetDlgItem(hWnd_, IDCANCEL));
}
void ScriptSelectDialog::_SearchScript(std::wstring dir)
{
	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::wstring findDir = dir + L"*.*";
	hFind = FindFirstFile(findDir.c_str(), &data);
	do 
	{
		std::wstring name = data.cFileName;
		if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			(name != L".." && name != L"."))
		{
			//ディレクトリ
			std::wstring tDir = dir + name;
			tDir += L"/";

			_SearchScript(tDir);
			continue;
		}
		if(wcscmp(data.cFileName, L"..")==0 || wcscmp(data.cFileName, L".")==0)
			continue;

		//ファイル
		std::wstring path = dir + name;

		std::vector<ref_count_ptr<ScriptInformation> >listInfo = 
			ScriptInformation::CreateScriptInformationList(path, true);
		for(int iInfo = 0 ; iInfo < listInfo.size() ; iInfo++)
		{
			ref_count_ptr<ScriptInformation> info = listInfo[iInfo];
			if(!_IsValidScript(info))continue;

			listInfo_.push_back(info);
		}

	}while(FindNextFile(hFind,&data));
	FindClose(hFind);
}

LRESULT ScriptSelectDialog::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		{
			_FinishMessageLoop();
			break;
		}
		case WM_CLOSE:
			SetWindowVisible(false);
			_FinishMessageLoop();
			break;
		case WM_COMMAND:
		{
			switch(wParam & 0xffff)
			{
				case IDCANCEL://閉じるボタン
					SetWindowVisible(false);
					_FinishMessageLoop();
					break;

				case IDOK:
				{
					int index = listView_.GetSelectedRow();
					if(index != -1)
						infoSelected_ = listInfo_[index];
					else infoSelected_ = NULL;
					SetWindowVisible(false);
					_FinishMessageLoop();
					break;
				}

				case IDC_BUTTON_RELOAD_SCRIPT:
					_Filter();
					break;
			}
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

			int topList = wy+110;
			int heightList = wHeight - topList - 40;
			listView_.SetBounds(wx+10, topList, wWidth-20, heightList);

			combo_.SetBounds(wx + 64, wy + 60, 200, 100 );

			int topButton = topList + heightList + 10;
			int widthButton = buttonOk_.GetClientWidth();
			int heightButton = buttonOk_.GetClientHeight();
			buttonCancel_.SetBounds(wWidth - widthButton * 1 - 8, topButton, widthButton, heightButton);
			buttonOk_.SetBounds(wWidth - widthButton * 2 - 16, topButton, widthButton, heightButton);

			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if(lpnmhdr->hwndFrom == listView_.GetWindowHandle())
			{
				switch( lpnmhdr->code )
				{
					case NM_DBLCLK:
					//case NM_CLICK:
					{
						int index = listView_.GetSelectedRow();
						if(index != -1)
							infoSelected_ = listInfo_[index];
						else infoSelected_ = NULL;
						SetWindowVisible(false);
						_FinishMessageLoop();
						break;
					}
				}
			}
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
void ScriptSelectDialog::ShowModal(std::wstring path)
{
	for(int iInfo = 0 ; iInfo < listInfo_.size() ; iInfo++)
	{
		ref_count_ptr<ScriptInformation> info = listInfo_[iInfo];
		if(!File::IsEqualsPath(path, info->GetScriptPath()))continue;
		listView_.SetSelectedRow(iInfo);
		break;
	}

	MoveWindowCenter();
	SetWindowVisible(true);
	_RunMessageLoop();
}
/**********************************************************
//EnemySelectDialog
**********************************************************/
EnemySelectDialog::EnemySelectDialog()
{

}
void EnemySelectDialog::Initialize()
{
	ScriptSelectDialog::Initialize();

	SetWindowText(GetWindowHandle(), L"敵スクリプト選択");

	//コンボ
	combo_.Attach(GetDlgItem(hWnd_, IDC_COMBO_TYPE_SCRIPT));
	combo_.InsertString(INDEX_ALL, KEY_ALL);
	combo_.InsertString(INDEX_SINGLE, KEY_SINGLE);
	combo_.InsertString(INDEX_PLURAL, KEY_PLURAL);
	combo_.InsertString(INDEX_STAGE, KEY_STAGE);
	combo_.InsertString(INDEX_PACKAGE, KEY_PACKAGE);
	combo_.SetSelectedIndex(INDEX_ALL);

	_Filter();

	SetBounds(0, 0, 640, 480);
}
void EnemySelectDialog::_Filter()
{
	listView_.Clear();

	int index = combo_.GetSelectedIndex();
	listInfo_.clear();

	std::wstring dir = PathProperty::GetModuleDirectory() + L"script/";
	_SearchScript(dir);

	std::sort(listInfo_.begin(), listInfo_.end(), ScriptInformation::Sort());

	std::vector<ref_count_ptr<ScriptInformation> >::iterator itr = listInfo_.begin();
	for(int iRow = 0; itr != listInfo_.end() ; itr++, iRow++)
	{
		ref_count_ptr<ScriptInformation> info = (*itr);
		std::wstring strType = L"";
		switch(info->GetType())
		{
		case ScriptInformation::TYPE_SINGLE:strType = KEY_SINGLE;break;
		case ScriptInformation::TYPE_PLURAL:strType = KEY_PLURAL;break;
		case ScriptInformation::TYPE_STAGE:strType = KEY_STAGE;break;
		case ScriptInformation::TYPE_PACKAGE:strType = KEY_PACKAGE;break;
		}

		std::wstring path = info->GetScriptPath();
		path = PathProperty::ReplaceYenToSlash(path);

		std::wstring root = PathProperty::GetModuleDirectory();
		root = PathProperty::ReplaceYenToSlash(root);
		path = StringUtility::ReplaceAll(path, root, L"");

		std::wstring archive = info->GetArchivePath();
		if(archive.size() > 0)
		{
			archive = PathProperty::ReplaceYenToSlash(archive);
			archive = StringUtility::ReplaceAll(archive, root, L"");
		}
		
		listView_.SetText(iRow, LIST_TYPE, strType);
		listView_.SetText(iRow, LIST_NAME, PathProperty::GetFileName(path));
		listView_.SetText(iRow, LIST_ARCHIVE, archive);
		listView_.SetText(iRow, LIST_PATH, path);
		listView_.SetText(iRow, LIST_TEXT, info->GetText());
	}

}
bool EnemySelectDialog::_IsValidScript(ref_count_ptr<ScriptInformation> info)
{
	int type = combo_.GetSelectedIndex();
	int typeScript = info->GetType();
	bool bTarget = false;
	switch(type)
	{
		case INDEX_SINGLE:
			bTarget = (typeScript == ScriptInformation::TYPE_SINGLE);
			break;
		case INDEX_PLURAL:
			bTarget = (typeScript == ScriptInformation::TYPE_PLURAL);
			break;
		case INDEX_STAGE:
			bTarget = (typeScript == ScriptInformation::TYPE_STAGE);
			break;
		case INDEX_PACKAGE:
			bTarget = (typeScript == ScriptInformation::TYPE_PACKAGE);
			break;
		case INDEX_ALL:
			bTarget = (typeScript != ScriptInformation::TYPE_PLAYER);
			break;
	}

	return bTarget;
}

/**********************************************************
//PlayerSelectDialog
**********************************************************/
PlayerSelectDialog::PlayerSelectDialog()
{

}
void PlayerSelectDialog::Initialize()
{
	ScriptSelectDialog::Initialize();

	SetWindowText(GetWindowHandle(), L"自機スクリプト選択");

	//コンボ
	combo_.Attach(GetDlgItem(hWnd_, IDC_COMBO_TYPE_SCRIPT));
	combo_.InsertString(INDEX_PLAYER, KEY_PLAYER);
	combo_.SetSelectedIndex(INDEX_PLAYER);
	combo_.SetWindowEnable(false);

	_Filter();

	SetBounds(0, 0, 640, 480);
}
void PlayerSelectDialog::_Filter()
{
	listView_.Clear();

	int index = combo_.GetSelectedIndex();
	listInfo_.clear();

	std::wstring dir = PathProperty::GetModuleDirectory() + L"script/";
	_SearchScript(dir);

	std::sort(listInfo_.begin(), listInfo_.end(), ScriptInformation::Sort());

	std::vector<ref_count_ptr<ScriptInformation> >::iterator itr = listInfo_.begin();
	for(int iRow = 0; itr != listInfo_.end() ; itr++, iRow++)
	{
		ref_count_ptr<ScriptInformation> info = (*itr);
		std::wstring strType = L"";
		switch(info->GetType())
		{
		case ScriptInformation::TYPE_PLAYER:strType = KEY_PLAYER;break;
		}

		std::wstring path = info->GetScriptPath();
		path = PathProperty::ReplaceYenToSlash(path);

		std::wstring root = PathProperty::GetModuleDirectory();
		root = PathProperty::ReplaceYenToSlash(root);
		path = StringUtility::ReplaceAll(path, root, L"");

		std::wstring archive = info->GetArchivePath();
		if(archive.size() > 0)
		{
			archive = PathProperty::ReplaceYenToSlash(archive);
			archive = StringUtility::ReplaceAll(archive, root, L"");
		}
		
		listView_.SetText(iRow, LIST_TYPE, strType);
		listView_.SetText(iRow, LIST_NAME, PathProperty::GetFileName(path));
		listView_.SetText(iRow, LIST_ARCHIVE, archive);
		listView_.SetText(iRow, LIST_PATH, path);
		listView_.SetText(iRow, LIST_TEXT, info->GetText());
	}

}
bool PlayerSelectDialog::_IsValidScript(ref_count_ptr<ScriptInformation> info)
{
	int type = combo_.GetSelectedIndex();
	int typeScript = info->GetType();
	bool bTarget = false;
	switch(type)
	{
		case INDEX_PLAYER:
			bTarget = (typeScript == ScriptInformation::TYPE_PLAYER);
			break;
	}

	return bTarget;
}


