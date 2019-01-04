#include"Logger.hpp"

using namespace gstd;

/**********************************************************
//Logger
**********************************************************/
Logger* Logger::top_ = NULL;
Logger::Logger()
{

}
Logger::~Logger()
{
	listLogger_.clear();
	if(top_ == this)top_ = NULL;
}
void Logger::_WriteChild(SYSTEMTIME& time, std::wstring str)
{
	_Write(time, str);
	std::list<ref_count_ptr<Logger> >::iterator itr = listLogger_.begin();
	for(;itr!=listLogger_.end();itr++)
	{
		(*itr)->_Write(time, str);
	}
}

void Logger::Write(std::wstring str)
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	this->_WriteChild(systemTime, str);
}


/**********************************************************
//FileLogger
**********************************************************/
FileLogger::FileLogger()
{
	sizeMax_ = 10*1024*1024;//10MB
}

FileLogger::~FileLogger()
{

}
void FileLogger::Clear()
{
	if(!bEnable_)return;

	File file1(path_);
	file1.Delete();
	File file2(path2_);
	file2.Delete();

	_CreateFile(file1);
}
bool FileLogger::Initialize(bool bEnable)
{
	return this->Initialize(L"", bEnable);
}
bool FileLogger::Initialize(std::wstring path, bool bEnable)
{
	bEnable_ = bEnable;
	if(path.size() == 0)
	{
		path = PathProperty::GetModuleDirectory()+
			PathProperty::GetModuleName() + 
			std::wstring(L".log");
	}
	return this->SetPath(path);
}
bool FileLogger::SetPath(std::wstring path)
{
	if(!bEnable_)return false;

	path_ = path;
	File file(path);
	if(file.IsExists() == false)
	{
		file.CreateDirectory();
		_CreateFile(file);
	}

	path2_ = path_ + L"_";
	return true;
}
void FileLogger::_CreateFile(File& file)
{
	file.Create();

	//BOM�iByte Order Mark�j
	file.WriteCharacter((unsigned char)0xFF);
	file.WriteCharacter((unsigned char)0xFE);
}
void FileLogger::_Write(SYSTEMTIME& time, std::wstring str)
{
	if(!bEnable_)return;

	{
		Lock lock(lock_);
		std::wstring strTime = 
			StringUtility::Format(L"%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d ",time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		File file(path_);
		if(!file.Open(File::WRITE))return;

		std::wstring out = strTime;
		out += str;
		out += L"\r\n";
		
		int pos = file.GetSize();
		file.Seek(pos);
		file.Write(&out[0], StringUtility::GetByteSize(out));

		bool bOverSize = file.GetSize() > sizeMax_;
		file.Close();

		if(bOverSize)
		{
			::MoveFileEx(path_.c_str(), path2_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
			File file1(path_);
			_CreateFile(file1);
		}
	}
}


/**********************************************************
//WindowLogger
**********************************************************/
WindowLogger::WindowLogger()
{

}
WindowLogger::~WindowLogger()
{
	threadInfoCollect_ = NULL;
	wndInfoPanel_ = NULL;
	wndLogPanel_ = NULL;
	wndStatus_ = NULL;
	wndTab_ = NULL;
	if(hWnd_ != NULL)
		SendMessage(hWnd_, WM_ENDLOGGER, 0, 0);
	if(threadWindow_ != NULL)
	{
		threadWindow_->Stop();
		threadWindow_->Join(2000);//�O�̂��߂Ƀ^�C���A�E�g�ݒ�
	}
}
bool WindowLogger::Initialize(bool bEnable)
{
	bEnable_ = bEnable;
	if(!bEnable)return true;

	threadWindow_ = new WindowThread(this);
	threadWindow_->Start();

	while(GetWindowHandle() == NULL)
	{
		Sleep(10);//�E�B���h�E���쐬��������܂őҋ@
	}

	//LogPanel
	wndLogPanel_ = new LogPanel();
	this->AddPanel(wndLogPanel_, L"Log");

	//InfoPanel
	wndInfoPanel_ = new InfoPanel();
	this->AddPanel(wndInfoPanel_, L"Info");

	return true;
}
void WindowLogger::SaveState()
{
	std::wstring path = PathProperty::GetModuleDirectory() + L"LogWindow.dat";
	RecordBuffer recordMain;
	bool bRecordExists = recordMain.ReadFromFile(path);

	int panelIndex = wndTab_->GetCurrentPage();
	recordMain.SetRecordAsInteger("panelIndex", panelIndex);

	RECT rcWnd;
	ZeroMemory(&rcWnd, sizeof(RECT));
	if(bRecordExists)
		recordMain.GetRecord("windowRect", rcWnd);
	if(IsWindowVisible())
		GetWindowRect(hWnd_, &rcWnd);
	recordMain.SetRecord("windowRect", rcWnd);
	
	RecordBuffer recordPanel;
	int panelCount = wndTab_->GetPageCount();
	for(int iPanel = 0 ; iPanel < panelCount ; iPanel++)
	{
		ref_count_ptr<WindowLogger::Panel> panel = 
			ref_count_ptr<WindowLogger::Panel>::DownCast(wndTab_->GetPanel(iPanel));
		if(panel == NULL)continue;

		panel->_WriteRecord(recordPanel);
	}

	recordMain.SetRecordAsRecordBuffer("panel", recordPanel);

	recordMain.WriteToFile(path);
}
void WindowLogger::LoadState()
{
	std::wstring path = PathProperty::GetModuleDirectory() + L"LogWindow.dat";
	RecordBuffer recordMain;
	if(!recordMain.ReadFromFile(path))return;

	int panelIndex = recordMain.GetRecordAsInteger("panelIndex");
	if(panelIndex >= 0 && panelIndex < wndTab_->GetPageCount())
		wndTab_->SetCurrentPage(panelIndex);

	RECT rcWnd;
	recordMain.GetRecord("windowRect", rcWnd);
	if(rcWnd.left >= 0 && rcWnd.right > rcWnd.left &&
		rcWnd.top >= 0 && rcWnd.bottom > rcWnd.top)
	{
		SetBounds(rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top);
	}

	RecordBuffer recordPanel;
	recordMain.GetRecordAsRecordBuffer("panel", recordPanel);

	int panelCount = wndTab_->GetPageCount();
	for(int iPanel = 0 ; iPanel < panelCount ; iPanel++)
	{
		ref_count_ptr<WindowLogger::Panel> panel = 
			ref_count_ptr<WindowLogger::Panel>::DownCast(wndTab_->GetPanel(iPanel));
		if(panel == NULL)continue;

		panel->_ReadRecord(recordPanel);
	}
}
void WindowLogger::_CreateWindow()
{
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	std::wstring wName = L"LogWindow";

	WNDCLASSEX wcex;
	ZeroMemory(&wcex,sizeof(wcex));
	wcex.cbSize=sizeof(WNDCLASSEX); 
	wcex.lpfnWndProc=(WNDPROC)WindowBase::_StaticWindowProcedure;
	wcex.hInstance=hInst;
	wcex.hIcon=NULL;
	wcex.hCursor=LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground=(HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName=NULL;
	wcex.lpszClassName=wName.c_str();
	wcex.hIconSm=NULL;
	RegisterClassEx(&wcex);

   	hWnd_=::CreateWindow(wcex.lpszClassName,
		wName.c_str(),
		WS_OVERLAPPEDWINDOW  ,
		0,0,640,480,NULL,(HMENU)NULL,hInst,NULL);
	::ShowWindow(hWnd_, SW_HIDE);
	this->Attach(hWnd_);

	//�^�u
	wndTab_ = new WTabControll();
	wndTab_->Create(hWnd_);
	HWND hTab = wndTab_->GetWindowHandle();

	//�X�e�[�^�X�o�[
	wndStatus_ = new WStatusBar();
	wndStatus_->Create(hWnd_);
	std::vector<int> sizeStatus;
	sizeStatus.push_back(180);
	sizeStatus.push_back(sizeStatus[0] + 560);
	wndStatus_->SetPartsSize(sizeStatus);

	//���擾�X���b�h
	threadInfoCollect_ = new InfoCollectThread(wndStatus_);
	threadInfoCollect_->Start();

	//����������
	this->SetBounds(32,32,280,480);
	wndTab_->ShowPage();

	::UpdateWindow(hWnd_);
}
void WindowLogger::_Run()
{
	_CreateWindow();
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{	//���b�Z�[�W���[�v
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
void WindowLogger::_Write(SYSTEMTIME& systemTime, std::wstring str)
{
	if(hWnd_ == NULL)return;

	wchar_t timeStr[256];
	swprintf(timeStr, L"%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d ",
		systemTime.wYear, systemTime.wMonth, systemTime.wDay, 
		systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);

	std::wstring out = timeStr;
	out += str;
	out += L"\r\n";
	{
		Lock lock(lock_);
		wndLogPanel_->AddText(out);
	}
}
LRESULT WindowLogger::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			return FALSE;
		}
		case WM_CLOSE:
		{
			SaveState();
			::ShowWindow(hWnd, SW_HIDE);
			return FALSE;
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

			return FALSE;
		}
		case WM_NOTIFY:
		{
			switch (((NMHDR *)lParam)->code)
			{
				case TCN_SELCHANGE:
					wndTab_->ShowPage();
					return FALSE;
			}
			break;
		}
		case WM_ENDLOGGER:
		{
			::DestroyWindow(hWnd);
			break;
		}
		case WM_ADDPANEL:
		{
			{
				Lock lock(lock_);
				std::list<AddPanelEvent>::iterator itr;
				for(itr=listEventAddPanel_.begin(); itr != listEventAddPanel_.end();itr++)
				{
					AddPanelEvent event = *itr;
					std::wstring name = event.name;
					ref_count_ptr<Panel> panel = event.panel;

					HWND hTab = wndTab_->GetWindowHandle();
					panel->_AddedLogger(hTab);
					wndTab_->AddTab(name, panel);

					RECT rect;
					::GetClientRect(hWnd_, &rect);
					int wx = rect.left;
					int wy = rect.top;
					int wWidth = rect.right-rect.left;
					int wHeight = rect.bottom-rect.top;
					wndTab_->SetBounds(wx+8, wy+4, wWidth-16, wHeight-32);
					wndTab_->LocateParts();
				}
				listEventAddPanel_.clear();
			}
			break;
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);

}


void WindowLogger::SetInfo(int row, std::wstring textInfo, std::wstring textData)
{
	if(hWnd_ == NULL)return;
	wndInfoPanel_->SetInfo(row, textInfo, textData);
}

bool WindowLogger::AddPanel(ref_count_ptr<Panel> panel, std::wstring name)
{
	if(hWnd_ == NULL)return false;

	AddPanelEvent event;
	event.name = name;
	event.panel = panel;
	{
		Lock lock(lock_);
		listEventAddPanel_.push_back(event);
	}

	::SendMessage(hWnd_, WM_ADDPANEL, 0, 0);

	while(panel->GetWindowHandle() == NULL)
	{
		Sleep(10);//�E�B���h�E���쐬��������܂őҋ@
	}
	return true;
}
void WindowLogger::ShowLogWindow()
{
	if(!bEnable_)return;
	ShowWindow(hWnd_, SW_SHOW);
}
void WindowLogger::InsertOpenCommandInSystemMenu(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);

	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	InsertMenuItem(hMenu, 0, 1, &mii);

	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.wID = MENU_ID_OPEN;
	mii.dwTypeData = L"���O�E�B���h�E�\��";

	InsertMenuItem(hMenu, 0, 1, &mii);
}

//WindowLogger::WindowThread
WindowLogger::WindowThread::WindowThread(WindowLogger* logger)
{
	_SetOuter(logger);
}
void WindowLogger::WindowThread::_Run()
{
	WindowLogger* logger = _GetOuter();
	logger->_Run();
}

//WindowLogger::LogPanel
WindowLogger::LogPanel::LogPanel()
{

}
WindowLogger::LogPanel::~LogPanel()
{

}
bool WindowLogger::LogPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);

	gstd::WEditBox::Style styleEdit;
	styleEdit.SetStyle(WS_CHILD | WS_VISIBLE |
		ES_MULTILINE|ES_READONLY|ES_AUTOHSCROLL|ES_AUTOVSCROLL|
		WS_HSCROLL | WS_VSCROLL);
	styleEdit.SetStyleEx(WS_EX_CLIENTEDGE);
	wndEdit_.Create(hWnd_, styleEdit);
	return true;
}
void WindowLogger::LogPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();
	wndEdit_.SetBounds(wx, wy, wWidth, wHeight);
}
void WindowLogger::LogPanel::AddText(std::wstring text)
{
	HWND hEdit = wndEdit_.GetWindowHandle();
	int pos = GetWindowTextLength(hEdit);
	if(pos + wndEdit_.GetTextLength() >= wndEdit_.GetMaxTextLength())
	{
		//�ő啶�����𒴂�����50%�폜
		std::wstring text = wndEdit_.GetText();
		text = text.erase(0, text.size() / 2);
		text += L"\r\n";
		wndEdit_.SetText(text);

		pos = GetWindowTextLength(hEdit);
	}
	::SendMessage( hEdit, EM_SETSEL, pos, pos );
	::SendMessage( hEdit, EM_REPLACESEL, 0, (LPARAM)text.c_str() );
}
//WindowLogger::InfoPanel
WindowLogger::InfoPanel::InfoPanel()
{

}
WindowLogger::InfoPanel::~InfoPanel()
{

}
bool WindowLogger::InfoPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);

	gstd::WListView::Style styleListView;
	styleListView.SetStyle(WS_CHILD | WS_VISIBLE | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL| LVS_NOSORTHEADER);
	styleListView.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListView.SetListViewStyleEx(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	wndListView_.Create(hWnd_, styleListView);

	wndListView_.AddColumn(120, ROW_INFO, L"Info");
	wndListView_.AddColumn(400, ROW_DATA, L"Data");
	return true;
}
void WindowLogger::InfoPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();
	wndListView_.SetBounds(wx, wy, wWidth, wHeight);
}
void WindowLogger::InfoPanel::SetInfo(int row, std::wstring textInfo, std::wstring textData)
{
	wndListView_.SetText(row, ROW_INFO, textInfo);
	wndListView_.SetText(row, ROW_DATA, textData);
}

//WindowLogger::InfoCollectThread
WindowLogger::InfoCollectThread::InfoCollectThread(ref_count_ptr<WStatusBar> wndStatus)
{
	wndStatus_ = wndStatus;
}
WindowLogger::InfoCollectThread::~InfoCollectThread()
{
	this->Stop();
	this->Join();
	wndStatus_ = NULL;
}
void WindowLogger::InfoCollectThread::_Run()
{
	//TODO �����ȃX�e�[�^�X�o�[�Ƀ��b�Z�[�W��
	//     ����Ƃ��A�ł܂�\������B
	infoCpu_ = this->_GetCpuInformation();

	while(this->GetStatus() == RUN)
	{
		::Sleep(500);
	}
/*
	PROCESS_MEMORY_COUNTERS memoryCounter;
	ZeroMemory(&memoryCounter, sizeof(PROCESS_MEMORY_COUNTERS));
	DWORD dwProcessID = GetCurrentProcessId();
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE ,dwProcessID);
	while(this->GetStatus() == RUN)
	{
		GetProcessMemoryInfo(hProcess, &memoryCounter, sizeof(PROCESS_MEMORY_COUNTERS));
		double pageFileUsage = memoryCounter.PagefileUsage / 1024. / 1024.;
		std::string strMemory = StringUtility::Format("Memory [ %.2fMB ]", pageFileUsage);
		if(this->GetStatus() == RUN)wndStatus_->SetText(STATUS_MEMORY, strMemory);

		double cpuPerformance=this->_GetCpuPerformance();
		CpuInfo &ci=this->infoCpu_;
		std::string strCpu = StringUtility::Format("%s %s [ %4.2fMHz (%3d %%) type:%d family:%d model:%d stepping:%d ]",ci.venderID,ci.cpuName.c_str(),ci.clock/1000/1000,(int)cpuPerformance,ci.type,ci.family,ci.model,ci.stepping);
		if(this->GetStatus() == RUN)wndStatus_->SetText(STATUS_CPU, strCpu);

		::Sleep(500);
	}
	CloseHandle(hProcess);
*/
}
WindowLogger::InfoCollectThread::CpuInfo WindowLogger::InfoCollectThread::_GetCpuInformation()
{
	int cpuid_supported;
	char VenderID[13];
	char name[17];
	int eax1cpuid_supported;
	int	eax_flags;
	int	edx_flags;
	unsigned long CPUClock;
	unsigned long start;

	CpuInfo ci;
	ZeroMemory(&ci,sizeof(CpuInfo));

	try
	{
		/*CPUID���߂��T�|�[�g���Ă��邩���ׂ�B
		�iPentium�ȍ~��CPU�Ȃ�΃T�|�[�g���Ă���炵���c�j
		�t���O���W�X�^�̂Q�P�r�b�g�ڂ�ς��邱�Ƃ��ł���΃T�|�[�g���Ă���B*/
		_asm
		{
			pushfd                ; �t���O���W�X�^�̓��e���X�^�b�N�ɕۑ�
			pushfd                ; �t���O���W�X�^�̓��e���X�^�b�N�ɓ����
			pop eax	              ; �X�^�b�N�̓��e��eax���W�X�^�Ɏ��o��
			xor eax,00200000h     ; eax���W�X�^��21�r�b�g�ڂ��r�b�g���]�A�c��͂��̂܂�
			push eax              ; eax���W�X�^�̓��e���X�^�b�N�ɓ����
			popfd                 ; �X�^�b�N�̓��e���t���O���W�X�^�Ɏ��o��
			pushfd                ; �t���O���W�X�^�̓��e���X�^�b�N�ɓ����
			pop ebx               ; �X�^�b�N�̓��e��ebx���W�X�^�Ɏ��o��
			popfd                 ; �t���O���W�X�^�̓��e�𕜌�
			cmp eax,ebx           ; ��r
			je supported          ; �������Ȃ�΃W�����v
			mov cpuid_supported,0 ; �ϐ����
			jmp exitasm           ; �������W�����v

			supported:
				mov cpuid_supported,1	; �ϐ����
			exitasm:
		};

		if(!cpuid_supported)
		{//CPUID ���߂��T�|�[�g���ĂȂ�
			throw gstd::wexception();
		}

		/*eax=0��CPUID���Ăяo���A
		�E�x���_�[ID�̎擾�i�x���_�[ID��ebx -> edx -> ecx�̏��ɃR�s�[����܂��j
		�E�x���_�[ID��AuthenticAMD�Ȃ�CPU�����擾
		�Eeax=1�̂Ƃ�CPUID���߂��T�|�[�g����Ă��邩�̒���
		�����܂��B*/
		_asm
		{
			mov	eax,0
			cpuid
			mov DWORD PTR [VenderID+0], ebx  ; �e�l���o�b�t�@�ɃR�s�[
			mov DWORD PTR [VenderID+4], edx
			mov DWORD PTR [VenderID+8], ecx
			mov BYTE PTR [VenderID+12], 0    ; �Ō��NULL������
			mov	eax1cpuid_supported,eax
		};

		//�x���_�[ID���R�s�[
		strcpy(ci.venderID,VenderID);

		//VenderID��AuthenticAMD�Ȃ�CPU�����擾�ł��܂��B
		if(0 == strcmp(ci.venderID,"AuthenticAMD"))
		{	
			//CPU�����擾���邽�߂�eax=08000002h��CPUID���Ăяo���܂��B
			_asm
			{
				mov	eax,08000002h
				cpuid
				mov DWORD PTR [name+0 ], eax  ; �e�l���o�b�t�@�ɃR�s�[
				mov DWORD PTR [name+4 ], ebx
				mov DWORD PTR [name+8 ], ecx
				mov DWORD PTR [name+12], edx
				mov BYTE PTR [name+16], 0    ; �Ō��NULL������
			};
			strcpy(ci.name,name);

			_asm
			{
				mov	eax,08000003h
				cpuid
				mov DWORD PTR [name+0 ], eax  ; �e�l���o�b�t�@�ɃR�s�[
				mov DWORD PTR [name+4 ], ebx
				mov DWORD PTR [name+8 ], ecx
				mov DWORD PTR [name+12], edx
				mov BYTE PTR [name+16], 0    ; �Ō��NULL������
			};
			strcat(ci.name,name);

			_asm
			{
				mov	eax,08000004h
				cpuid
				mov DWORD PTR [name+0 ], eax  ; �e�l���o�b�t�@�ɃR�s�[
				mov DWORD PTR [name+4 ], ebx
				mov DWORD PTR [name+8 ], ecx
				mov DWORD PTR [name+12], edx
				mov BYTE PTR [name+16], 0    ; �Ō��NULL������
			};
			strcat(ci.name,name);
		}

		//ax=1�̂Ƃ�CPUID���߂��T�|�[�g����Ă��邩�̒���
		if(1 > eax1cpuid_supported){
			//eax���W�X�^���P�̎���CPUID���߂��T�|�[�g����Ă��Ȃ�
			throw gstd::wexception();
		}

		/*eax���W�X�^��1������CPUID���Ăяo���A
		�ECPU��type,family,Mode,stepping
		�EMMX���߂̃T�|�[�g
		�ESIMD���߂̃T�|�[�g
		�E3DNow!�̃T�|�[�g
		�ERDTSC���߂̃T�|�[�g�i�T�|�[�g���Ă���΃N���b�N���g���𑪒�j
		�𒲂ׂ܂��B*/
		_asm
		{
			mov	eax,1
			cpuid
			mov eax_flags,eax
			mov	edx_flags,edx
		};

		//CPU��type,family,Mode,stepping�𒲂ׂ�
		ci.type     = (eax_flags >>12) & 0xF;
		ci.family   = (eax_flags >> 8) & 0xF;
		ci.model    = (eax_flags >> 4) & 0xF;
		ci.stepping = eax_flags & 0xF;

		//MMX���߂��T�|�[�g���Ă��邩
		ci.bMMXEnabled = (edx_flags & 0x00800000)? true: false;

		//SIMD���߂��T�|�[�g���Ă��邩
		ci.bSIMDEnabled = (edx_flags & 0x02000000)? true: false;

		//AMD 3DNow!���T�|�[�g���Ă��邩
		ci.bAMD3DNowEnabled = (edx_flags & 0x80000000)? true: false;

		//RDTSC���߂��T�|�[�g���Ă��邩���ׂ�
		if(edx_flags & 0x00000010)
		{
			//�T�|�[�g���Ă���΃N���b�N���g���𑪒�
			//RDTSR���߂�CPU�R�A��Time Stamp Counter��eax���W�X�^�Ɋi�[���閽�߂ł��B
			//Time Stamp Counter��CPU1�N���b�N���ƂɃJ�E���g�A�b�v���Ă��܂��B
			_asm rdtsc;
			_asm mov CPUClock, eax;
			start = timeGetTime();
			while(timeGetTime()-start < 1000);//1000ms�҂�
//			Sleep(1000);
			_asm rdtsc;
			_asm sub eax,CPUClock;/*�����Z*/
			_asm mov CPUClock, eax;
			ci.clock=(double)(CPUClock);
		}
		else
		{
			//RDTSC���߂��T�|�[�g���Ă��Ȃ��B
			throw gstd::wexception();
		}

		ci.cpuName=ci.name;
		if(strcmp(ci.venderID,"AuthenticAMD")==0)//AMD
		{
			//if(ci.family==5&&ci.model==8)ci.cpuName="K6-2";
			//else if(ci.family==5&&ci.model==9)ci.cpuName="K6-�V";
			//else if(ci.family==6&&ci.model==1)ci.cpuName="Athlon";
			//else if(ci.family==6&&ci.model==2)ci.cpuName="Athlon";
			//else if(ci.family==6&&ci.model==3)ci.cpuName="Athlon";
			//else if(ci.family==6&&ci.model==4)ci.cpuName="Athlon";
			//else if(ci.family==6&&ci.model==6)ci.cpuName="AthlonXp";
		}
		else if(strcmp(ci.venderID,"GenuineIntel")==0)//Intel
		{
			//if(ci.family==5&&ci.model==1)ci.cpuName="Pentium 60 / 66";
			//else if(ci.family==5&&ci.model==2)ci.cpuName="Pentium";
			//else if(ci.family==5&&ci.model==3)ci.cpuName="Pentium OverDrive for 486";
			//else if(ci.family==5&&ci.model==4)ci.cpuName="MMX Pentium";
			//else if(ci.family==6&&ci.model==1)ci.cpuName="Pentium Pro";
			//else if(ci.family==6&&ci.model==3)ci.cpuName="Pentium�U";
			//else if(ci.family==6&&ci.model==5)ci.cpuName="Pentium�U/Pentium�UXeon/Celeron";
			//else if(ci.family==6&&ci.model==6)ci.cpuName="Celeron";
			//else if(ci.family==6&&ci.model==7)ci.cpuName="Pentium�V/Petium�VXeon";
			//else if(ci.family==6&&ci.model==8)ci.cpuName="Pentium�V/Petium�VXeon/Celeron";
			//else if(ci.family==6&&ci.model==9)ci.cpuName="Petium�VXeon";
			//else if(ci.family==6&&ci.model==10)ci.cpuName="Pentium�UOverDrive";
			//else if(ci.family==15&&ci.model==0)ci.cpuName="Pentium4";
		}

	}
	catch(...)
	{

	}
	return ci;
}

double WindowLogger::InfoCollectThread::_GetCpuPerformance()
{
	HQUERY hQuery;
	HCOUNTER hCounter;
	PDH_FMT_COUNTERVALUE FmtValue;

	PdhOpenQuery(NULL, 0, &hQuery);
	PdhAddCounter(hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &hCounter);	
	Sleep(500);
	PdhCollectQueryData(hQuery);
	Sleep(500);
	PdhCollectQueryData(hQuery);
	PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &FmtValue);
	PdhCloseQuery(hQuery);
	return FmtValue.doubleValue;
}
