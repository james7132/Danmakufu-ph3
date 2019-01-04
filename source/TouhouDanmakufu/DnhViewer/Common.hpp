#ifndef __TOUHOUDANMAKUFU_VIEW_COMMON__
#define __TOUHOUDANMAKUFU_VIEW_COMMON__

#include"../Common/DnhCommon.hpp"
#include"GcLibImpl.hpp"

/**********************************************************
//SystemResidentTask
**********************************************************/
class SystemResidentTask : public TaskBase
{
	public:
		enum
		{
			TASK_PRI_RENDER_FPS = 9,
		};

	private:
		DxText textFps_;

	public:
		SystemResidentTask();
		~SystemResidentTask();
		void RenderFps();
};


#endif

