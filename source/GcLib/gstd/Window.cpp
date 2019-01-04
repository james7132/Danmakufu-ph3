#include"Window.hpp"

using namespace gstd;

/**********************************************************
//WindowBase
**********************************************************/
const wchar_t* PROP_THIS = L"__THIS__";
std::list<int> WindowBase::listWndId_;
CriticalSection WindowBase::lock_;

WindowBase::WindowBase()
{
	hWnd_ = NULL;
	oldWndProc_ = NULL;

	//�󂢂Ă���WindowID�擾
	{
		Lock lock(lock_);
		listWndId_.sort();
		int idFree = 0;
		std::list<int>::iterator itr;
		for(itr=listWndId_.begin(); itr!= listWndId_.end(); itr++)
		{
			if(*itr != idFree)break;
			idFree++;
		}
		idWindow_ = idFree;
		listWndId_.push_back(idFree);
	}

}

WindowBase::~WindowBase()
{
	this->Detach();

	//WindowID���
	std::list<int>::iterator itr;
	for(itr=listWndId_.begin(); itr!= listWndId_.end(); itr++)
	{
		if(*itr != idWindow_)continue;
		listWndId_.erase(itr);
		break;
	}
}

bool WindowBase::Attach(HWND hWnd)
{
	if(!hWnd)return false;
	hWnd_ = hWnd;
	//�_�C�A���O���E�B���h�E���𔻒�
	int typeProc = ::GetWindowLong(hWnd, DWL_DLGPROC)!=0 ? DWL_DLGPROC : GWL_WNDPROC;

	//�v���p�e�B�ɃC���X�^���X��o�^
	::SetProp(hWnd_, PROP_THIS, (HANDLE)this);

	//�����̃E�B���h�E���T�u�N���X��
	if(::GetWindowLong(hWnd_, typeProc) != (LONG)_StaticWindowProcedure)
		oldWndProc_ = (WNDPROC)::SetWindowLong(hWnd_, typeProc, (LONG)_StaticWindowProcedure);
	return true;
}

bool WindowBase::Detach()
{
	if(hWnd_ == NULL)return false;

    //�T�u�N���X��������
	if(oldWndProc_)
	{
		int typeProc = ::GetWindowLong(hWnd_, DWL_DLGPROC)!=0 ? DWL_DLGPROC : GWL_WNDPROC;
		::SetWindowLong(hWnd_, typeProc, (DWORD)oldWndProc_);
		oldWndProc_ = NULL;
	}
	::RemoveProp(hWnd_, PROP_THIS);
	return true;
}


LRESULT CALLBACK WindowBase::_StaticWindowProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	WindowBase* tWnd=(WindowBase*)::GetProp(hWnd, PROP_THIS);

	//�擾�ł��Ȃ������Ƃ��̏���
	if(tWnd==NULL)
	{
		if((uMsg==WM_CREATE)||(uMsg==WM_NCCREATE))
			tWnd=(WindowBase*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		else if(uMsg==WM_INITDIALOG)
			tWnd=(WindowBase*)lParam;
		if(tWnd)tWnd->Attach(hWnd);
	}

	if(tWnd)
	{
		LRESULT lResult=tWnd->_WindowProcedure(hWnd, uMsg, wParam, lParam);
		if(uMsg==WM_DESTROY)tWnd->Detach();
		return lResult;
	}

	//�_�C�A���O�ƃE�B���h�E�ŕԂ��l�𕪂���
	return ::GetWindowLong(hWnd, DWL_DLGPROC) ? 
		FALSE : ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
LRESULT WindowBase::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
LRESULT WindowBase::_CallPreviousWindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	if(oldWndProc_)
		return CallWindowProc(oldWndProc_, hWnd, uMsg, wParam, lParam);

	//�_�C�A���O�ƃE�B���h�E�ŕԂ��l�𕪂���
	return ::GetWindowLong(hWnd, DWL_DLGPROC) ? 
		FALSE : ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}
void WindowBase::MoveWindowCenter()
{
	RECT drect,mrect;
	HWND hDesk=::GetDesktopWindow();
	::GetWindowRect(hDesk, &drect);
	::GetWindowRect(hWnd_, &mrect);
	int tWidth=mrect.right-mrect.left;
	int tHeight=mrect.bottom-mrect.top;
	int left=drect.right/2-tWidth/2;
	int top=drect.bottom/2-tHeight/2;
	::MoveWindow(hWnd_,left,top,tWidth,tHeight,TRUE);
}
HWND WindowBase::GetTopParentWindow(HWND hWnd)
{
	HWND res = hWnd;
	while(true)
	{
		HWND hParent = GetParent(hWnd);
		if(hParent == NULL)break;
		res = hParent;
	}
	return res;
}

/**********************************************************
//ModalDialog
**********************************************************/
void ModalDialog::Create(HWND hParent, LPCTSTR resource)
{
	hParent_ = hParent;
	hWnd_ = CreateDialog((HINSTANCE)GetWindowLong(hParent,GWL_HINSTANCE),
							resource,
							hParent,(DLGPROC)this->_StaticWindowProcedure);
	this->Attach(hWnd_);
}
void ModalDialog::_RunMessageLoop()
{
	bEndDialog_ = false;
	::EnableWindow(hParent_, FALSE);

	::ShowWindow(hWnd_, SW_SHOW);

	MSG msg;
	while(!bEndDialog_)
	{	//���b�Z�[�W���[�v
		::GetMessage(&msg, NULL, 0, 0);
		if (IsDialog() && hWnd_ && IsDialogMessage(hWnd_ , &msg)) continue;

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);	
	}

	::EnableWindow(hParent_, TRUE);
	::SetWindowPos(hParent_, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	::BringWindowToTop(hParent_);
	::SetActiveWindow(hParent_);

}
void ModalDialog::_FinishMessageLoop()
{
	::SetWindowPos(hParent_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	bEndDialog_ = true;
}
/**********************************************************
//WPanel
**********************************************************/
void WPanel::Create(HWND hWndParent)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

    hWnd_ = ::CreateWindowEx(
        0, L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);
	this->Attach(hWnd_);
	this->SetWindowStyle(SS_NOTIFY);
}
LRESULT WPanel::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			LocateParts();
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}

/**********************************************************
//WLabel
**********************************************************/
WLabel::WLabel()
{
	colorText_ = RGB(0, 0, 0);
	colorBack_ = GetSysColor(COLOR_WINDOW);
}
void WLabel::Create(HWND hWndParent)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

    hWnd_ = ::CreateWindowEx(
        0, L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE ,
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
LRESULT WLabel::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CTLCOLORSTATIC:case WM_CTLCOLOR:
		{
			::SetTextColor((HDC)wParam, colorText_);
			::SetBkColor((HDC)wParam, colorBack_);
//			::SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)::GetStockObject( NULL_BRUSH );
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
void WLabel::SetText(std::wstring text)
{
	if(text == GetText())return;
	::SetWindowText(hWnd_, text.c_str());
}
void WLabel::SetTextColor(int color)
{
	colorText_ = color;
	::InvalidateRect(hWnd_, NULL, TRUE);
}
void WLabel::SetBackColor(int color)
{
	colorBack_ = color;
	::InvalidateRect(hWnd_, NULL, TRUE);
}
std::wstring WLabel::GetText()
{
	if(GetTextLength() == 0)return L"";

	std::wstring res;
	int count = GetTextLength() + 1;
	res.resize(count);
	::GetWindowText(hWnd_, &res[0], count);
	return res;
}
int WLabel::GetTextLength()
{
	return ::GetWindowTextLength(hWnd_);
}
/**********************************************************
//WButton
**********************************************************/
void WButton::Create(HWND hWndParent)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
	hWnd_ = ::CreateWindow(
                L"BUTTON", L"", 
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,0,0,0, hWndParent, (HMENU)GetWindowId(),
				hInst, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WButton::Create(HWND hWndParent, WButton::Style& style)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

    hWnd_ = ::CreateWindowEx(
        style.GetStyleEx(), L"BUTTON", NULL, 
        style.GetStyle(),
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);

	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WButton::SetText(std::wstring text)
{
	::SetWindowText(hWnd_, text.c_str());
}
bool WButton::IsChecked()
{
	bool bCheck = SendMessage(hWnd_ , BM_GETCHECK , 0 , 0) == BST_CHECKED;
	return bCheck;
}

/**********************************************************
//WGroupBox
**********************************************************/
void WGroupBox::Create(HWND hWndParent)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
	hWnd_ = ::CreateWindow(
                L"BUTTON", L"", 
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                0,0,0,0, hWndParent, (HMENU)GetWindowId(),
				hInst, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}

void WGroupBox::SetText(std::wstring text)
{
	::SetWindowText(hWnd_, text.c_str());
}

/**********************************************************
//WEditBox
**********************************************************/
void WEditBox::Create(HWND hWndParent, WEditBox::Style& style)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

//	DWORD style = 0;//ES_MULTILINE|ES_READONLY|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL | WS_VSCROLL;
    hWnd_ = ::CreateWindowEx(
        style.GetStyleEx(), L"EDIT", NULL, 
        style.GetStyle(),
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);

	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WEditBox::SetText(std::wstring text)
{
	if(text == GetText())return;
	::SetWindowText(hWnd_, text.c_str());
}
std::wstring WEditBox::GetText()
{
	if(GetTextLength() == 0)return L"";

	std::wstring res;
	int count = GetTextLength() + 1;
	res.resize(count);
	::GetWindowText(hWnd_, &res[0], count);
	return res;
}
int WEditBox::GetTextLength()
{
	return ::GetWindowTextLength(hWnd_);
}

/**********************************************************
//WListBox
**********************************************************/
//http://gurigumi.s349.xrea.com/programming/visualcpp/sdk_dialog_listbox.html
//http://web.kyoto-inet.or.jp/people/ysskondo/from16/chap17.html
void WListBox::Create(HWND hWndParent, WListBox::Style& style)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

//	DWORD style = 0;//ES_MULTILINE|ES_READONLY|ES_AUTOHSCROLL|ES_AUTOVSCROLL|WS_HSCROLL | WS_VSCROLL;
    hWnd_ = ::CreateWindowEx(
        style.GetStyleEx(), L"LISTBOX", NULL, 
        style.GetStyle(),
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);

	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WListBox::Clear()
{
	SendMessage(hWnd_, LB_RESETCONTENT, 0, 0);
}
int WListBox::GetCount()
{
	int res = (int)SendMessage(hWnd_, LB_GETCOUNT, 0, 0);
	return res;
}
void WListBox::SetSelectedIndex(int index)
{
	SendMessage(hWnd_, LB_SETCURSEL, index, 0); 
}
int WListBox::GetSelectedIndex()
{
	int res = (int)SendMessage(hWnd_, LB_GETCURSEL, 0, 0);
	return res;
}
void WListBox::AddString(std::wstring str)
{
	SendMessage(hWnd_, LB_ADDSTRING, 0, (LPARAM)str.c_str()); 
}
void WListBox::InsertString(int index, std::wstring str)
{
	SendMessage(hWnd_, LB_INSERTSTRING, index, (LPARAM)str.c_str());
}
void WListBox::DeleteString(int index)
{
	SendMessage(hWnd_, LB_DELETESTRING, index, 0); 
}
std::wstring WListBox::GetText(int index)
{
	std::wstring res;
	int length = (int)SendMessage(hWnd_, LB_GETTEXTLEN, index, 0);
	res.resize(length);
	SendMessage(hWnd_, LB_GETTEXT, index, (LPARAM)&res[0]);
	return res;
}

/**********************************************************
//WComboBox
**********************************************************/
void WComboBox::Create(HWND hWndParent, WComboBox::Style& style)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);

    hWnd_ = ::CreateWindowEx(
        style.GetStyleEx(), L"COMBOBOX", NULL, 
        style.GetStyle(),
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);

	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}

void WComboBox::SetItemHeight(int height)
{
	::SendMessage(hWnd_, CB_SETITEMHEIGHT, height, (LPARAM)0);
}
void WComboBox::SetDropDownListWidth(int width)
{
	::SendMessage(hWnd_, CB_SETDROPPEDWIDTH, width, (LPARAM)0);
}
void WComboBox::Clear()
{
	while(GetCount() != 0)
	{
		::SendMessage(hWnd_, CB_DELETESTRING, 0, 0);
	}
}
int WComboBox::GetCount()
{
	return ::SendMessage(hWnd_, CB_GETCOUNT, 0, 0);
}
void WComboBox::SetSelectedIndex(int index)
{
	::SendMessage(hWnd_, CB_SETCURSEL, index, 0);
}
int WComboBox::GetSelectedIndex()
{
	return ::SendMessage(hWnd_, CB_GETCURSEL, 0, 0);
}
std::wstring WComboBox::GetSelectedText()
{
	int index = GetSelectedIndex();
	wchar_t buf[256];
	::SendMessage(hWnd_, CB_GETLBTEXT, index, (LPARAM)buf);
	std::wstring res = buf;
	return res;
}
void WComboBox::AddString(std::wstring str)
{
	::SendMessage(hWnd_, CB_ADDSTRING, 0, (LPARAM)str.c_str());
}
void WComboBox::InsertString(int index, std::wstring str)
{
	SendMessage(hWnd_, CB_INSERTSTRING, index, (LPARAM)str.c_str());
}
/**********************************************************
//WListView
**********************************************************/
void WListView::Create(HWND hWndParent, Style& style)
{
	//WS_EX_CLIENTEDGE
	//WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS 
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
    hWnd_ = ::CreateWindowEx(
        style.GetStyleEx(), WC_LISTVIEW, NULL, 
        style.GetStyle(),
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);

	DWORD styleListViewEx = ListView_GetExtendedListViewStyle(hWnd_);
	styleListViewEx |= style.GetListViewStyleEx();
	ListView_SetExtendedListViewStyle(hWnd_, styleListViewEx);

	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WListView::Clear()
{
	ListView_DeleteAllItems(hWnd_);
}
void WListView::AddColumn(int cx,int sub, DWORD fmt, std::wstring text)
{
	LV_COLUMN lvcol;
	lvcol.mask=LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt=fmt;
	lvcol.cx=cx;
	lvcol.pszText=(wchar_t*)text.c_str();
	lvcol.iSubItem=sub;
	ListView_InsertColumn(hWnd_,sub,&lvcol);
}
void WListView::SetColumnText(int cx, std::wstring text)
{
	LV_COLUMN lvcol;
	ListView_GetColumn(hWnd_, cx, &lvcol);
	lvcol.pszText=(wchar_t*)text.c_str();
	ListView_SetColumn(hWnd_, cx, &lvcol);
}
void WListView::AddRow(std::wstring text)
{
	LVITEM item;
	item.mask = LVIF_TEXT | LVIF_PARAM;
	item.iItem = 0;
	item.iSubItem = 0;
	item.pszText = (wchar_t*)text.c_str();
	item.lParam = ListView_GetItemCount(hWnd_);
	ListView_InsertItem(hWnd_, &item);
}
void WListView::SetText(int row,int column, std::wstring text)
{
	std::wstring pre = GetText(row, column);
	if(pre == text)return;

	int count = ListView_GetItemCount(hWnd_);
	for(int iRow=count; iRow<=row ; iRow++)
	{
		LVITEM item;
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = iRow;
		item.iSubItem = 0;
		item.pszText = L"";
		item.lParam = iRow;
		ListView_InsertItem(hWnd_, &item);
	}

	LV_ITEM item;
	item.iItem=row;
	item.pszText=(wchar_t*)text.c_str();
	item.iSubItem=column;
	item.mask=LVIF_TEXT;
	ListView_SetItem(hWnd_, &item);
}
void WListView::DeleteRow(int row)
{
	ListView_DeleteItem(hWnd_, row);
}
int WListView::GetRowCount()
{
	return ListView_GetItemCount(hWnd_);
}
std::wstring WListView::GetText(int row,int column)
{
	std::wstring res;
	wchar_t buf[256];
	ListView_GetItemText(hWnd_, row, column, buf, sizeof(buf));
	res = buf;
	return res;
}
bool WListView::IsExistsInColumn(std::wstring value, int column)
{
	int count = ListView_GetItemCount(hWnd_);
	for(int iRow=0; iRow<=count ; iRow++)
	{
		std::wstring str = GetText(iRow, column);
		if(str == value)return true;
	}
	return false;
}
int WListView::GetIndexInColumn(std::wstring value, int column)
{
	int res = -1;
	if(column == 0)
	{
		LVFINDINFO info;
		ZeroMemory(&info, sizeof(LVFINDINFO));
		info.flags = LVFI_STRING;
		info.psz = value.c_str();
		res = ListView_FindItem(hWnd_, -1, &info);
	}
	else
	{
		int count = ListView_GetItemCount(hWnd_);
		for(int iRow=0; iRow<=count ; iRow++)
		{
			std::wstring str = GetText(iRow, column);
			if(str == value)
			{
				res = iRow;
			}
		}
	}

	return res;
}
void WListView::SetSelectedRow(int index)
{
	ListView_SetItemState(hWnd_, index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	ListView_EnsureVisible(hWnd_, index, TRUE);
}
int WListView::GetSelectedRow()
{
	return ListView_GetNextItem(hWnd_, -1, LVNI_SELECTED);
}
void WListView::ClearSelection()
{
	int index = GetSelectedRow();
	if(index >= 0)
		ListView_SetItemState(hWnd_, index,  0, LVIS_FOCUSED | LVIS_SELECTED);
}

/**********************************************************
//WTreeView
**********************************************************/
WTreeView::WTreeView()
{
}
WTreeView::~WTreeView()
{
	this->Clear();
}
void WTreeView::Create(HWND hWndParent, Style& style)
{
	//TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
	hWnd_ = CreateWindowEx(style.GetStyleEx(), WC_TREEVIEW, L"",
					style.GetStyle(),
					0, 0, 0, 0,
					hWndParent, (HMENU)GetWindowId(), hInst, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WTreeView::CreateRootItem(ItemStyle& style)
{
	this->Clear();

	itemRoot_ = new Item();
	itemRoot_->hTree_ = hWnd_;

	TVINSERTSTRUCT& tvis = style.GetInsertStruct();
	tvis.hParent = TVI_ROOT;
	itemRoot_->hItem_ = TreeView_InsertItem(hWnd_, &tvis);
}
ref_count_ptr<WTreeView::Item> WTreeView::GetSelectedItem()
{
	HTREEITEM hTreeItem = TreeView_GetSelection(hWnd_);
	if(hTreeItem == NULL)return NULL;

	ref_count_ptr<Item> res = new Item();
	res->hTree_ = hWnd_;
	res->hItem_ = hTreeItem;
	return res;
}
//Item
WTreeView::Item::Item()
{
	hItem_ = NULL;
}
WTreeView::Item::~Item()
{

}
ref_count_ptr<WTreeView::Item> WTreeView::Item::CreateChild(WTreeView::ItemStyle& style)
{
	ref_count_ptr<Item> item = new Item();
	item->hTree_ = hTree_;

	TVINSERTSTRUCT& tvis = style.GetInsertStruct();
	tvis.hParent = hItem_;
	item->hItem_ = TreeView_InsertItem(hTree_, &tvis);
	return item;
}
void WTreeView::Item::Delete()
{
	TreeView_DeleteItem(hTree_, hItem_);
}
void WTreeView::Item::SetText(std::wstring text)
{
	if(GetText() == text)return;
	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.hItem= hItem_;
	tvi.pszText = (wchar_t*)text.c_str();
	TreeView_SetItem(hTree_, &tvi);
}
std::wstring WTreeView::Item::GetText()
{
	wchar_t text[256];
	ZeroMemory(text, sizeof(text));
	TVITEM tvi;
	tvi.mask = TVIF_TEXT;
	tvi.hItem= hItem_;
	tvi.cchTextMax = sizeof(text)-1;
	tvi.pszText = text;
	TreeView_GetItem(hTree_, &tvi);
	return std::wstring(text);
}
void WTreeView::Item::SetParam(LPARAM param)
{
	TVITEM tvi;
	tvi.mask = TVIF_PARAM;
	tvi.hItem= hItem_;
	tvi.lParam = param;
	TreeView_SetItem(hTree_, &tvi);
}
LPARAM WTreeView::Item::GetParam()
{
	TVITEM tvi;
	tvi.mask = TVIF_PARAM;
	tvi.hItem= hItem_;
	TreeView_GetItem(hTree_, &tvi);
	return tvi.lParam;
}
std::list<ref_count_ptr<WTreeView::Item> > WTreeView::Item::GetChildList()
{
	std::list<ref_count_ptr<Item> > res;
	HTREEITEM hChild = TreeView_GetChild(hTree_, hItem_);
	while(true)
	{
		if(hChild == NULL)break;

		ref_count_ptr<Item> item = new Item();
		item->hTree_ = hTree_;
		item->hItem_ = hChild;
		res.push_back(item);

		hChild = TreeView_GetNextSibling(hTree_, hChild);
	}

	return res;
}
/**********************************************************
//WTabControll
**********************************************************/
WTabControll::~WTabControll()
{
	vectPanel_.clear();
}
void WTabControll::Create(HWND hWndParent)
{
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
    hWnd_ = ::CreateWindowEx(
        0, WC_TABCONTROL, NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS| WS_CLIPCHILDREN,
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	::SetClassLong(hWnd_, GCL_STYLE, 0);
}
LRESULT WTabControll::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_SIZE:
		{
			LocateParts();
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}
void WTabControll::AddTab(std::wstring text)
{
	ref_count_ptr<WPanel> panel = new WPanel();
	this->AddTab(text, panel);
}
void WTabControll::AddTab(std::wstring text, ref_count_ptr<WPanel> panel)
{
	TC_ITEM item;
	item.mask = TCIF_TEXT;
    item.pszText = const_cast<wchar_t*>(text.c_str());
	SendMessage(hWnd_, TCM_INSERTITEM, (WPARAM)vectPanel_.size(), (LPARAM)&item);

	vectPanel_.push_back(panel);
}
void WTabControll::ShowPage()
{
	int page = TabCtrl_GetCurSel(hWnd_);
	if(page==-1)return;
	for(int iPanel=0 ; iPanel<vectPanel_.size() ; iPanel++)
	{
		vectPanel_[iPanel]->SetWindowVisible(false);
	}
	vectPanel_[page]->SetWindowVisible(true);
}

void WTabControll::LocateParts()
{
	RECT rect;
	::GetClientRect(hWnd_,&rect);
	TabCtrl_AdjustRect(hWnd_ ,FALSE, &rect);

	int wx = rect.left;
	int wy = rect.top;
	int wWidth = rect.right-rect.left;
	int wHeight = rect.bottom-rect.top;

	for(int iPanel=0;iPanel < vectPanel_.size();iPanel++)
	{
		vectPanel_[iPanel]->SetBounds(wx+1, wy+3, wWidth-2, wHeight-5);
	}
}

/**********************************************************
//WStatusBar
**********************************************************/
void WStatusBar::Create(HWND hWndParent)
{
	hWnd_=CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, NULL, hWndParent, NULL);
	this->Attach(hWnd_);
	::SendMessage(hWnd_, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
}
void WStatusBar::SetPartsSize(std::vector<int> parts)
{
	::SendMessage(hWnd_, SB_SETPARTS, (WPARAM)parts.size(), (LPARAM)(LPINT)&parts[0]);
}
void WStatusBar::SetText(int pos, std::wstring text)
{
	::SendMessage(hWnd_, SB_SETTEXT, 0 | pos, (WPARAM)(wchar_t*)text.c_str());
}
void WStatusBar::SetBounds(WPARAM wParam,LPARAM lParam)
{
	::SendMessage(hWnd_, WM_SIZE, wParam, lParam);
}

/**********************************************************
//WSplitter
**********************************************************/
WSplitter::WSplitter()
{
	bCapture_ = false;
	ratioX_ = 0.5f;
	ratioY_ = 0.5f;
}
WSplitter::~WSplitter()
{
}
void WSplitter::Create(HWND hWndParent, SplitType type)
{
	//WS_EX_DLGMODALFRAME
	type_ = type;
	HINSTANCE hInst = (HINSTANCE)::GetWindowLong(hWndParent, GWL_HINSTANCE);
    hWnd_ = ::CreateWindowEx(
        0, L"STATIC", NULL, 
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS|WS_CLIPCHILDREN,
        0,0,0,0, hWndParent, (HMENU)GetWindowId(),
        hInst, NULL);
	this->Attach(hWnd_);
	this->SetWindowStyle(SS_NOTIFY);
}
LRESULT WSplitter::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_MOUSEMOVE:
		{
			if(bCapture_)
			{
				HWND hWndParent = GetParent(hWnd);
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hWndParent, &pt);

				RECT rect;
				::GetClientRect(hWndParent, &rect);

				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				if(type_ == TYPE_VERTICAL)
				{
					ratioX_ = (float)(pt.x)/(float)(rect.right-rect.left);
					if(ratioX_ <= 0.05f)ratioX_=0.05f;
					if(ratioX_ >= 0.95f)ratioX_=0.95f;
				}
				else if(type_ == TYPE_HORIZONTAL)
				{
					ratioY_ = (float)(pt.y)/(float)(rect.bottom-rect.top);
					if(ratioY_ <= 0.05f)ratioY_=0.05f;
					if(ratioY_ >= 0.95f)ratioY_=0.95f;
				}

				SendMessage(hWndParent, WM_SIZE , 0,
					MAKELPARAM(rect.right, rect.bottom));
			}
			break;
		}
		case WM_SETCURSOR:
		{
			RECT rect;
			POINT pt;
			GetCursorPos(&pt);
			::GetWindowRect(hWnd, &rect);
			if(pt.x>=rect.left&&pt.x<=rect.right &&
				pt.y>rect.top&&pt.y<rect.bottom)
			{
				wchar_t* cursor = type_ == TYPE_VERTICAL ? IDC_SIZEWE : IDC_SIZENS;
				SetCursor(LoadCursor(NULL, cursor));
				SetClassLong(hWnd, GCL_HCURSOR, 0);
			}
			else 
			{
				SetCursor(LoadCursor(NULL,IDC_ARROW));
				SetClassLong(hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL,IDC_ARROW));
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			RECT rect;
			POINT pt;
			GetCursorPos(&pt);
			::GetWindowRect(hWnd, &rect);
			if(type_ == TYPE_VERTICAL &&
				pt.x>=rect.left && pt.x<=rect.right)
			{
				bCapture_ = true;
				SetCapture(hWnd);
			}
			else if(type_ == TYPE_HORIZONTAL &&
				pt.y>=rect.top && pt.y<=rect.bottom)
			{
				bCapture_ = true;
				SetCapture(hWnd);
			}
			else bCapture_ = false;
			break;
		}
		case WM_LBUTTONUP:
		{
			SetClassLong(hWnd, GCL_HCURSOR, (LONG)LoadCursor(NULL,IDC_ARROW));
			bCapture_ = false;
			ReleaseCapture();
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}


/**********************************************************
//WindowUtility
**********************************************************/
void WindowUtility::SetMouseVisible(bool bVisible)
{
	if(bVisible) 
	{
		while(ShowCursor( TRUE ) < 0){}
	}
	else
	{
		while(ShowCursor( FALSE ) >= 0){}
	}
}
