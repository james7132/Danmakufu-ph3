#ifndef __TOUHOUDANMAKUFU_VIEW_DEBUGWINDOW__
#define __TOUHOUDANMAKUFU_VIEW_DEBUGWINDOW__

#include"Constant.hpp"
#include"Common.hpp"

/**********************************************************
//DebugWindow
**********************************************************/
class DebugWindow : public WindowBase
{
	protected:
		WButton checkPlayerInvincivility_;

		virtual LRESULT _WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	public:
		DebugWindow();
		void Initialize();
		void ShowModeless();

		bool IsPlayerInvincivility();
		void SetPlayerInvincivility(bool bInvincivility);
};

/**********************************************************
//DebugTask
**********************************************************/
class DebugTask : public TaskBase
{
	public:
		enum
		{
			TASK_PRI_WORK = 0,
		};

	private:
		bool bPlayerInvincivility_;

	public:
		DebugTask();
		~DebugTask();
		void Work();
};

#endif
