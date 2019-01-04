#ifndef __GSTD_FPSCONTROLLER__
#define __GSTD_FPSCONTROLLER__

#include"GstdConstant.hpp"
#include"SmartPointer.hpp"

namespace gstd
{
	class FpsControlObject;
	/**********************************************************
	//FpsController
	**********************************************************/
	class FpsController
	{
		public:
			enum
			{
				FPS_FAST_MODE = 1200,
			};
		protected:
			int fps_;//�ݒ肳��Ă���FPS
			bool bUseTimer_;//�^�C�}�[����
			bool bCriticalFrame_;
			bool bFastMode_;

			std::list<ref_count_weak_ptr<FpsControlObject> > listFpsControlObject_;

			inline int _GetTime();
			inline void _Sleep(int msec);
		public:
			FpsController();
			virtual ~FpsController();
			virtual void SetFps(int fps){fps_ = fps;}
			virtual int GetFps(){return fps_;}
			virtual void SetTimerEnable(bool b){bUseTimer_ = b;}
			
			virtual void Wait() = 0;
			virtual bool IsSkip(){return false;}
			virtual void SetCriticalFrame(){bCriticalFrame_ = true;}
			virtual float GetCurrentFps() = 0;
			virtual float GetCurrentWorkFps(){return GetCurrentFps();}
			virtual float GetCurrentRenderFps(){return GetCurrentFps();}
			bool IsFastMode(){return bFastMode_;}
			void SetFastMode(bool b){bFastMode_ = b;}

			void AddFpsControlObject(ref_count_weak_ptr<FpsControlObject> obj);
			void RemoveFpsControlObject(ref_count_weak_ptr<FpsControlObject> obj);
			int GetControlObjectFps();
	};

	/**********************************************************
	//StaticFpsController
	**********************************************************/
	class StaticFpsController : public FpsController
	{
		enum
		{
			FAST_MODE_SKIP_RATE = 10,
		};
		protected:
			float fpsCurrent_;//���݂�FPS
			int timePrevious_;//�O��Wait�����Ƃ��̎���
			int timeError_;//�����z������(�덷)
			int timeCurrentFpsUpdate_;//1�b�𑪒肷�邽�߂̎��ԕێ�
			int rateSkip_;//�`��X�L�b�v��
			int countSkip_;//�`��X�L�b�v�J�E���g
			std::list<int> listFps_;//1�b���ƂɌ���fps���v�Z���邽�߂�fps��ێ�

		public:
			StaticFpsController();
			~StaticFpsController();
			
			virtual void Wait();
			virtual bool IsSkip();
			virtual void SetCriticalFrame(){bCriticalFrame_=true;timeError_=0;countSkip_=0;}

			void SetSkipRate(int value);
			virtual float GetCurrentFps();
			virtual float GetCurrentWorkFps();
			virtual float GetCurrentRenderFps();
	};

	/**********************************************************
	//AutoSkipFpsController
	**********************************************************/
	class AutoSkipFpsController : public FpsController
	{
		protected:
			float fpsCurrentWork_;	//���ۂ�fps
			float fpsCurrentRender_;	//���ۂ�fps
			int timePrevious_;//�O��Wait�����Ƃ��̎���
			int timePreviousWork_;
			int timePreviousRender_;
			int timeError_;//�����z������(�덷)
			int timeCurrentFpsUpdate_;//1�b�𑪒肷�邽�߂̎��ԕێ�
			std::list<int> listFpsWork_;
			std::list<int> listFpsRender_;
			double countSkip_;//�A���`��X�L�b�v��
			int countSkipMax_;//�ő�A���`��X�L�b�v��

		public:
			AutoSkipFpsController();
			~AutoSkipFpsController();

			virtual void Wait();
			virtual bool IsSkip();
			virtual void SetCriticalFrame(){bCriticalFrame_=true;timeError_=0;countSkip_=0;}

			virtual float GetCurrentFps(){return GetCurrentWorkFps();}
			float GetCurrentWorkFps(){return fpsCurrentWork_;};
			float GetCurrentRenderFps(){return fpsCurrentRender_;};
	};

	/**********************************************************
	//FpsControlObject
	**********************************************************/
	class FpsControlObject
	{
		public:
			FpsControlObject(){}
			virtual ~FpsControlObject(){}
			virtual int GetFps() = 0;
	};
}

#endif
