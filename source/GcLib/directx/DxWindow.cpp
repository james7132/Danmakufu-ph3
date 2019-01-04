#include"DxWindow.hpp"
#include"DirectGraphics.hpp"
#include"DirectInput.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//DxWindowManager
**********************************************************/
DxWindowManager::DxWindowManager()
{
}
DxWindowManager::~DxWindowManager()
{
	Clear();
}
void DxWindowManager::Clear()
{
	listWindow_.clear();
	wndCapture_ = NULL;
}
void DxWindowManager::_ArrangeWindow()
{
	std::list<gstd::ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin() ; itr != listWindow_.end() ; )
	{
		if(*itr == NULL)
			itr = listWindow_.erase(itr);
		else if((*itr)->IsWindowDelete())
		{
			(*itr)->Dispose();
			itr = listWindow_.erase(itr);
		}
		else itr++;
	}
}

void DxWindowManager::AddWindow(gstd::ref_count_ptr<DxWindow> window)
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr = listWindow_.begin();
	for(; itr!= listWindow_.end(); itr++)
	{
		if( (*itr) == NULL)continue;
		if( (*itr) == window)return;//���d�o�^�͂����Ȃ�		
	}
	window->manager_ = this;
	listWindow_.push_back(window);
	window->AddedManager();
}
void DxWindowManager::DeleteWindow(DxWindow* window)
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr = listWindow_.begin();
	for(; itr!= listWindow_.end(); itr++)
	{
		if( (*itr) == NULL)continue;
		if( (*itr) != window)continue;
		(*itr)->DeleteWindow();
		return;
	}
}
void DxWindowManager::DeleteWindowFromID(int id)
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if((*itr)->idWindow_ != id)continue;
		(*itr)->DeleteWindow();
		return;
	}
}
void DxWindowManager::Work()
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
	{
		if( (*itr) == NULL)continue;
		if(!(*itr)->IsWindowEnable())continue;
		if((*itr)->IsWindowDelete())continue;
		(*itr)->Work();
	}
	_DispatchMouseEvent();
	_ArrangeWindow();
}
void DxWindowManager::Render()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetLightingEnable(false);
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
	std::list<ref_count_ptr<DxWindow> >::reverse_iterator itr;
	for(itr = listWindow_.rbegin(); itr!= listWindow_.rend(); itr++)
	{
		if( (*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(!(*itr)->IsWindowVisible())continue;
		(*itr)->Render();
	}
}
gstd::ref_count_ptr<DxWindow> DxWindowManager::GetIntersectedWindow()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	if(graphics == NULL)return NULL;

	DirectInput* input = DirectInput::GetBase();
	if(input == NULL)return NULL;

	gstd::ref_count_ptr<DxWindow> res = NULL;
	POINT posMouse = graphics->GetMousePosition();
	std::list<ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
	{
		if( (*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(!(*itr)->IsWindowEnable())continue;
		if(!(*itr)->IsWindowVisible())continue;
		res = GetIntersectedWindow(posMouse, (*itr));
		if(res != NULL)break;
	}
	return res;
}
gstd::ref_count_ptr<DxWindow> DxWindowManager::GetIntersectedWindow(POINT& pos, gstd::ref_count_ptr<DxWindow> parent)
{
	gstd::ref_count_ptr<DxWindow> res = NULL;
	if(parent == NULL)
	{
		parent = *listWindow_.begin();
	}
	if(parent == NULL)return NULL;

	if(!parent->IsWindowEnable() || !parent->IsWindowVisible() || parent->IsWindowDelete())
		return NULL;

	std::list<gstd::ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = parent->listWindowChild_.begin() ; itr != parent->listWindowChild_.end() ; itr++)
	{
		if(*itr == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(!(*itr)->IsWindowEnable())continue;
		if(!(*itr)->IsWindowVisible())continue;

		res = GetIntersectedWindow(pos, (*itr));
		if(res != NULL)break;

		bool bIntersect = (*itr)->IsIntersected(pos);
		if(!bIntersect)continue;
		res = *itr;
		break;
	}

	if(res == NULL)
	{
		res = parent->IsIntersected(pos) ? parent : NULL;
	}

	return res;
}
void DxWindowManager::_DispatchMouseEvent()
{
	DirectInput* input = DirectInput::GetBase();
	if(input == NULL)return;

	gstd::ref_count_ptr<DxWindowEvent> event = new DxWindowEvent();
	gstd::ref_count_ptr<DxWindow> wndIntersect = GetIntersectedWindow();

	//���N���b�N
	int mLeftState = input->GetMouseState(DI_MOUSE_LEFT);
	int mRightState = input->GetMouseState(DI_MOUSE_RIGHT);
	if(wndCapture_ == NULL)
	{
		if(wndIntersect != NULL)
		{
			if(mLeftState == KEY_PUSH)
			{
				wndCapture_ = wndIntersect;
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_PUSH);
				event->SetSourceWindow(wndIntersect);
			}
			else if(mLeftState == KEY_HOLD)
			{
				wndCapture_ = wndIntersect;
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_PUSH);
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_HOLD);
				event->SetSourceWindow(wndIntersect);
			}
		}
	}
	else
	{
		if(wndIntersect != NULL && wndIntersect == wndCapture_)
		{
			if(mLeftState == KEY_PULL)
			{
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_RELEASE);
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_CLICK);
				event->SetSourceWindow(wndIntersect);
				wndCapture_ = NULL;
			}
		}
		else
		{
			if(mLeftState == KEY_PULL || mLeftState == KEY_FREE)
			{
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_RELEASE);
				event->SetSourceWindow(wndCapture_);
				wndCapture_ = NULL;
			}
			else if(mLeftState == KEY_PUSH || mLeftState == KEY_HOLD)
			{
				event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_HOLD);
				event->SetSourceWindow(wndCapture_);
			}
		}
	}

	if(event->IsEmpty())
	{
		if(mLeftState == KEY_PUSH)
			event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_PUSH);
		else if(mLeftState == KEY_PULL)
			event->AddEventType(DxWindowEvent::TYPE_MOUSE_LEFT_RELEASE);

		if(mRightState == KEY_PUSH)
			event->AddEventType(DxWindowEvent::TYPE_MOUSE_RIGHT_PUSH);
		else if(mRightState == KEY_PULL)
			event->AddEventType(DxWindowEvent::TYPE_MOUSE_RIGHT_RELEASE);
	}

	if(!event->IsEmpty())
	{
		std::list<ref_count_ptr<DxWindow> >::iterator itr;
		for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
		{
			if((*itr) == NULL)continue;
			if((*itr)->IsWindowDelete())continue;
			if(!(*itr)->IsWindowEnable())continue;
			if(!(*itr)->IsWindowVisible())continue;
			(*itr)->DispatchedEvent(event);
		}		
	}

}
void DxWindowManager::SetAllWindowEnable(bool bEnable)
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		(*itr)->SetWindowEnable(bEnable);
	}
	listLockID_.clear();
}
void DxWindowManager::SetWindowEnableWithoutArgumentWindow(bool bEnable, DxWindow* window)
{
	DxWindow* parent = window;
	while(parent->windowParent_ != NULL)
	{
		parent = parent->windowParent_;
	}
	int id = parent->GetID();

	if(bEnable)
	{
		bool bError = false;
		int idLock = -1;
		if(listLockID_.size() > 0)
		{
			idLock = *listLockID_.begin();
			if(id != idLock)
				bError = true;

			listLockID_.pop_front();
			if(listLockID_.size() > 0)
				id = *listLockID_.begin();
		}
		else
		{
			return;
		}

		if(bError)
		{
			throw gstd::wexception(StringUtility::Format(L"DxWindow���b�N���s���ł�:id[%d] idLock[%d]", 
							id, idLock).c_str());
		}
	}
	else
	{
		std::list<int>::iterator itr;
		for(itr = listLockID_.begin() ; itr != listLockID_.end() ; itr++)
		{
			if((*itr) != id)continue;
			return;
		}
		listLockID_.push_front(id);
	}

	std::list<ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindow_.begin(); itr!= listWindow_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(id != -1 && (*itr)->GetID() == id)continue;
		(*itr)->SetWindowEnable(bEnable);
	}
}

/**********************************************************
//DxWindow
**********************************************************/
std::list<int> DxWindow::listWndId_;
DxWindow::DxWindow()
{
	windowParent_ = NULL;
	bWindowDelete_ = false;
	bWindowEnable_ = true;
	bWindowVisible_ = true;
	SetRect(&rectWindow_, 0, 0, 0, 0);
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);

	//�󂢂Ă���WindowID�擾
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

	typeRenderFrame_ = DirectGraphics::MODE_BLEND_ALPHA;
}
DxWindow::~DxWindow()
{
	//WindowID���
	std::list<int>::iterator itr;
	for(itr=listWndId_.begin(); itr!= listWndId_.end(); itr++)
	{
		if(*itr != idWindow_)continue;
		listWndId_.erase(itr);
		break;
	}
}
void DxWindow::DeleteWindow()
{
	bWindowDelete_ = true;
	if(windowParent_ == NULL && manager_ != NULL)
	{
		manager_->DeleteWindowFromID(idWindow_);
	}

	std::list<gstd::ref_count_ptr<DxWindow> >::iterator itr;
	for(itr = listWindowChild_.begin(); itr != listWindowChild_.end() ; itr++)
	{
		if((*itr)->IsWindowDelete())continue;
		(*itr)->DeleteWindow();
	}
}
void DxWindow::Dispose()
{
	windowParent_ = NULL;
	listWindowChild_.clear();
}
void DxWindow::AddChild(gstd::ref_count_ptr<DxWindow> window)
{
	std::list<ref_count_ptr<DxWindow> >::iterator itr;;
	for(itr = listWindowChild_.begin(); itr!= listWindowChild_.end(); itr++)
	{
		if( (*itr) == NULL)continue;
		if( (*itr) == window)return;//���d�o�^�͂����Ȃ�		
	}
	window->manager_ = manager_;
	window->windowParent_ = this;
	listWindowChild_.push_back(window);
	window->AddedManager();
}
void DxWindow::_WorkChild()
{
	if(bWindowDelete_)return;
	std::list<ref_count_ptr<DxWindow> >::iterator itr;;
	for(itr = listWindowChild_.begin(); itr!= listWindowChild_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		(*itr)->Work();
	}
}
void DxWindow::_RenderChild()
{
	if(!bWindowVisible_ || bWindowDelete_)return;
	std::list<ref_count_ptr<DxWindow> >::iterator itr;;
	for(itr = listWindowChild_.begin(); itr!= listWindowChild_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(!(*itr)->IsWindowVisible())continue;
		(*itr)->Render();
	}
}
void DxWindow::_DispatchEventToChild(gstd::ref_count_ptr<DxWindowEvent> event)
{
	if(!bWindowEnable_ || bWindowDelete_)return;
	std::list<ref_count_ptr<DxWindow> >::iterator itr;;
	for(itr = listWindowChild_.begin(); itr!= listWindowChild_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		if(!(*itr)->IsWindowEnable())continue;
		(*itr)->DispatchedEvent(event);
	}
}
void DxWindow::_RenderFrame()
{
	if(spriteFrame_ == NULL)return;

	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(typeRenderFrame_);
	int alphaWindow = GetAbsoluteAlpha();
	int alphaSprite = ColorAccess::GetColorA(spriteFrame_->GetVertex(0)->diffuse_color);
	int alpha = min(255, alphaWindow * alphaSprite / 255);
	RECT rcDest = GetAbsoluteWindowRect();
	RECT_D drcDest = GetRectD(rcDest);
	spriteFrame_->SetDestinationRect(drcDest);
	spriteFrame_->SetAlpha(alpha);
	spriteFrame_->Render();
	spriteFrame_->SetAlpha(alphaSprite);

	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
}
bool DxWindow::IsIntersected(POINT pos)
{
	RECT rect=GetAbsoluteWindowRect();
	return pos.x >= rect.left && pos.x <= rect.right && pos.y >= rect.top && pos.y <= rect.bottom;
}
RECT DxWindow::GetAbsoluteWindowRect()
{
	RECT res = rectWindow_;
	DxWindow* parent = windowParent_;
	while(parent != NULL)
	{
		RECT& rect = parent->rectWindow_;
		res.left += rect.left;
		res.right += rect.left;
		res.top += rect.top;
		res.bottom += rect.top;
		parent = parent->windowParent_;
	}
	return res;
}
bool DxWindow::IsWindowExists(int id)
{
	if(bWindowDelete_)return false;
	bool res = false;
	if(GetID() == id)return true;
	std::list<ref_count_ptr<DxWindow> >::iterator itr;;
	for(itr = listWindowChild_.begin(); itr!= listWindowChild_.end(); itr++)
	{
		if((*itr) == NULL)continue;
		if((*itr)->IsWindowDelete())continue;
		res |= (*itr)->IsWindowExists(id);
		if(res)break;
	}
	return res;
}
int DxWindow::GetAbsoluteAlpha()
{
	int res = GetAlpha();
	DxWindow* parent = windowParent_;
	while(parent != NULL)
	{
		res = res * parent->GetAlpha() / 255;
		parent = parent->windowParent_;
	}
	return res;
}

/**********************************************************
//DxLabel
**********************************************************/
DxLabel::DxLabel()
{
}
void DxLabel::Work()
{
}
void DxLabel::Render()
{
	_RenderFrame();
	if(text_ != NULL)text_->Render();
	_RenderChild();
}
void DxLabel::SetText(std::wstring str)
{
	if(text_ == NULL)
	{
		RECT rect = GetAbsoluteWindowRect();
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		text_ = new DxText();
		text_->SetHorizontalAlignment(DxText::ALIGNMENT_CENTER);
		text_->SetFontSize(min(width, height));
		text_->SetPosition(rect.top, rect.bottom);
	}
	text_->SetText(str);
}
void DxLabel::SetText(ref_count_ptr<DxText> text, bool bArrange)
{
	text_ = text;

	if(bArrange)
	{
		int sizeFont = text->GetFontSize();
		RECT rect = GetAbsoluteWindowRect();
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		text_->SetMaxWidth(width);
		text_->SetHorizontalAlignment(DxText::ALIGNMENT_CENTER);
		text_->SetPosition(rect.left, rect.top + (height - sizeFont) / 2);	
	}
}


/**********************************************************
//DxButton
**********************************************************/
DxButton::DxButton()
{
	bIntersected_ = false;
	bSelected_ = false;
}
void DxButton::Work()
{
	if(manager_ == NULL)return;
	if(IsWindowDelete())return;
	ref_count_ptr<DxWindow> wnd = manager_->GetIntersectedWindow();

	bool bOldIntersected = bIntersected_;
	bIntersected_ = wnd != NULL && wnd->GetID() == GetID();
	if(!bOldIntersected && bIntersected_)
	{
		IntersectMouseCursor();
	}
	_WorkChild();
}
void DxButton::Render()
{
	_RenderFrame();
	if(text_ != NULL)text_->Render();
	_RenderChild();

	if(bIntersected_)
		RenderIntersectedFrame();
	if(bSelected_)
		RenderSelectedFrame();
}
void DxButton::RenderIntersectedFrame()
{
	if(!bIntersected_)return;
	if(!IsWindowEnable())return;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ADD_RGB);
	Sprite2D sprite;
	int alpha = 64;
	RECT_D rcSrc = {1, 1, 2, 2};
	RECT_D rcDest = GetRectD(GetAbsoluteWindowRect());
	sprite.SetVertex(rcSrc, rcDest, D3DCOLOR_ARGB(alpha, alpha, alpha, alpha));
	sprite.Render();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
}
void DxButton::RenderSelectedFrame()
{
	if(!bSelected_)return;
	if(!IsWindowEnable())return;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ADD_RGB);
	Sprite2D sprite;
	int alpha = 64;
	RECT_D rcSrc = {1, 1, 2, 2};
	RECT_D rcDest = GetRectD(GetAbsoluteWindowRect());
	sprite.SetVertex(rcSrc, rcDest, D3DCOLOR_ARGB(alpha, alpha, alpha, alpha));
	sprite.Render();
	graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
}


/**********************************************************
//DxMessageBox
**********************************************************/
DxMessageBox::DxMessageBox()
{
	index_ = INDEX_NULL;
}
void DxMessageBox::DispatchedEvent(gstd::ref_count_ptr<DxWindowEvent> event)
{
	_DispatchEventToChild(event);

}
void DxMessageBox::SetText(ref_count_ptr<DxText> text)
{
	text_ = text;
}
void DxMessageBox::SetButton(std::vector<gstd::ref_count_ptr<DxButton> > listButton)
{
	listButton_ = listButton;
}
void DxMessageBox::UpdateWindowRect()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int scrnWidth = graphics->GetScreenWidth();
	int scrnHeight = graphics->GetScreenHeight();

	int margin = 16;
	RECT rcWnd = GetWindowRect();
	int wndWidth = rcWnd.right - rcWnd.left;
	text_->SetMaxWidth(wndWidth - margin * 2);
	text_->SetPosition(rcWnd.left + margin, rcWnd.top + margin);
	gstd::ref_count_ptr<DxTextInfo> textInfo = text_->GetTextInfo();
	int textHeight = textInfo->GetTotalHeight();

	int iButton = 0 ;
	int totalButtonWidth = 0;
	int buttonHeight = 0;
	for(iButton = 0 ; iButton < listButton_.size() ; iButton++)
	{
		RECT rect = listButton_[iButton]->GetWindowRect();
		totalButtonWidth += rect.right - rect.left + margin;
		buttonHeight = max(buttonHeight, rect.bottom - rect.top);
	}

	int leftButton = wndWidth / 2 - totalButtonWidth / 2;
	int topButton = textHeight + margin * 2;
	for(iButton = 0 ; iButton < listButton_.size() ; iButton++)
	{
		RECT rcButton = listButton_[iButton]->GetWindowRect();
		int width = rcButton.right - rcButton.left;
		int height = rcButton.bottom - rcButton.top;
		RECT rect = {leftButton, topButton, leftButton + width, topButton + height};
		leftButton += width + margin;
		listButton_[iButton]->SetWindowRect(rect);
	}

	RECT rect = {rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.top + textHeight + buttonHeight + margin * 3};
	SetWindowRect(rect);
}
