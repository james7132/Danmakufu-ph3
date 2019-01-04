#ifndef __TOUHOUDANMAKUFU_DNHCOMMON__
#define __TOUHOUDANMAKUFU_DNHCOMMON__

#include"DnhConstant.hpp"

/**********************************************************
//ScriptInformation
**********************************************************/
class ScriptInformation
{
	public:
		enum
		{
			TYPE_UNKNOWN,
			TYPE_PLAYER,
			TYPE_SINGLE,
			TYPE_PLURAL,
			TYPE_STAGE,
			TYPE_PACKAGE,

			MAX_ID = 8,
		};
		const static std::wstring DEFAULT;
		class Sort;
		struct PlayerListSort;
	private:
		int type_;
		std::wstring pathArchive_;
		std::wstring pathScript_;

		std::wstring id_;
		std::wstring title_;
		std::wstring text_;
		std::wstring pathImage_;
		std::wstring pathSystem_;
		std::wstring pathBackground_;
		std::wstring pathBGM_;
		std::vector<std::wstring> listPlayer_;

		std::wstring replayName_;
		

	public:
		ScriptInformation(){}
		virtual ~ScriptInformation(){}
		int GetType(){return type_;}
		void SetType(int type){type_ = type;}
		std::wstring GetArchivePath(){return pathArchive_;}
		void SetArchivePath(std::wstring path){pathArchive_ = path;}
		std::wstring GetScriptPath(){return pathScript_;}
		void SetScriptPath(std::wstring path){pathScript_ = path;}

		std::wstring GetID(){return id_;}
		void SetID(std::wstring id){id_ = id;}
		std::wstring GetTitle(){return title_;}
		void SetTitle(std::wstring title){title_ = title;}
		std::wstring GetText(){return text_;}
		void SetText(std::wstring text){text_ = text;}
		std::wstring GetImagePath(){return pathImage_;}
		void SetImagePath(std::wstring path){pathImage_ = path;}
		std::wstring GetSystemPath(){return pathSystem_;}
		void SetSystemPath(std::wstring path){pathSystem_ = path;}
		std::wstring GetBackgroundPath(){return pathBackground_;}
		void SetBackgroundPath(std::wstring path){pathBackground_ = path;}
		std::wstring GetBgmPath(){return pathBGM_;}
		void SetBgmPath(std::wstring path){pathBGM_ = path;}
		std::vector<std::wstring> GetPlayerList(){return listPlayer_;}
		void SetPlayerList(std::vector<std::wstring> list){listPlayer_ = list;}

		std::wstring GetReplayName(){return replayName_;}
		void SetReplayName(std::wstring name){replayName_ = name;}

		std::vector<ref_count_ptr<ScriptInformation> > CreatePlayerScriptInformationList();

	public:
		static ref_count_ptr<ScriptInformation> CreateScriptInformation(std::wstring pathScript, bool bNeedHeader = true);
		static ref_count_ptr<ScriptInformation> CreateScriptInformation(std::wstring pathScript, std::wstring pathArchive, std::string source, bool bNeedHeader = true);
		
		static std::vector<ref_count_ptr<ScriptInformation> > CreateScriptInformationList(std::wstring path, bool bNeedHeader = true);
		static std::vector<ref_count_ptr<ScriptInformation> > FindPlayerScriptInformationList(std::wstring dir);
		static bool IsExcludeExtention(std::wstring ext);

	private:
		static std::wstring _GetString(Scanner& scanner);
		static std::vector<std::wstring> _GetStringList(Scanner& scanner);
};

class ScriptInformation::Sort
{
	public:
	BOOL operator()(const ref_count_ptr<ScriptInformation>& lf, const ref_count_ptr<ScriptInformation>& rf)
	{
		ref_count_ptr<ScriptInformation> lsp = lf;
		ref_count_ptr<ScriptInformation> rsp = rf;
		ScriptInformation* lp = (ScriptInformation*)lsp.GetPointer();
		ScriptInformation* rp = (ScriptInformation*)rsp.GetPointer();
		std::wstring lPath = lp->GetScriptPath();
		std::wstring rPath = rp->GetScriptPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};

struct ScriptInformation::PlayerListSort
{
	BOOL operator()(const ref_count_ptr<ScriptInformation>& lf, const ref_count_ptr<ScriptInformation>& rf)
	{
		ref_count_ptr<ScriptInformation> lsp = lf;
		ref_count_ptr<ScriptInformation> rsp = rf;
		std::wstring lPath = lsp->GetScriptPath();
		std::wstring rPath = rsp->GetScriptPath();
		BOOL res = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
			lPath.c_str(), -1, rPath.c_str(), -1);
		return res == CSTR_LESS_THAN ? TRUE : FALSE;
	}
};


/**********************************************************
//ErrorDialog
**********************************************************/
class ErrorDialog : public ModalDialog
{
	protected:
		static HWND hWndParentStatic_;

		WEditBox edit_;
		WButton button_;

		virtual LRESULT _WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	public:
		ErrorDialog(HWND hParent);
		bool ShowModal(std::wstring msg);

		static void SetParentWindowHandle(HWND hWndParent){hWndParentStatic_ = hWndParent;}
		static void ShowErrorDialog(std::wstring msg){ErrorDialog dialog(hWndParentStatic_); dialog.ShowModal(msg);}
};



/**********************************************************
//DnhConfiguration
**********************************************************/
class DnhConfiguration : public Singleton<DnhConfiguration>
{
		static const int VERSION;
	public:
		enum
		{
			WINDOW_SIZE_640x480 = 0,
			WINDOW_SIZE_800x600,
			WINDOW_SIZE_960x720,
			WINDOW_SIZE_1280x960,
			WINDOW_SIZE_1600x1200,
			WINDOW_SIZE_1920x1200,

			FPS_NORMAL = 0,
			FPS_1_2,// 1/2
			FPS_1_3,// 1/3
			FPS_AUTO,
		};

	private:
		int modeScreen_;//DirectGraphics::SCREENMODE_FULLSCREEN,SCREENMODE_WINDOW
		int sizeWindow_;
		int fpsType_;

		int padIndex_;
		std::map<int, ref_count_ptr<VirtualKey> > mapKey_;

		bool bLogWindow_;
		bool bLogFile_;
		bool bMouseVisible_;

		std::wstring pathPackageScript_;
		std::wstring windowTitle_;
		int screenWidth_;
		int screenHeight_;

		bool _LoadDefintionFile();
	public:
		DnhConfiguration();
		virtual ~DnhConfiguration();
		bool LoadConfigFile();
		bool SaveConfigFile();

		int GetScreenMode(){return modeScreen_;}
		void SetScreenMode(int mode){modeScreen_ = mode;}
		int GetWindowSize(){return sizeWindow_;}
		void SetWindowSize(int size){sizeWindow_ = size;}
		int GetFpsType(){return fpsType_;}
		void SetFpsType(int type){fpsType_ = type;}

		int GetPadIndex(){return padIndex_;}
		void SetPadIndex(int index){padIndex_ = index;}
		ref_count_ptr<VirtualKey> GetVirtualKey(int id);

		bool IsLogWindow(){return bLogWindow_;}
		void SetLogWindow(bool b){bLogWindow_ = b;}
		bool IsLogFile(){return bLogFile_;}
		void SetLogFile(bool b){bLogFile_ = b;}
		bool IsMouseVisible(){return bMouseVisible_;}
		void SetMouseVisible(bool b){bMouseVisible_ = b;}

		std::wstring GetPackageScriptPath(){return pathPackageScript_;}
		std::wstring GetWindowTitle(){return windowTitle_;}
		int GetScreenWidth(){return screenWidth_;}
		int GetScreenHeight(){return screenHeight_;}
};

#endif
