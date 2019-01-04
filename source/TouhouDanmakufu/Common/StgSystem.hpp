#ifndef __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__
#define __TOUHOUDANMAKUFU_DNHSTG_SYSTEM__

#include"StgCommon.hpp"
#include"StgStageScript.hpp"
#include"StgStageController.hpp"
#include"StgPlayer.hpp"
#include"StgEnemy.hpp"
#include"StgShot.hpp"
#include"StgItem.hpp"
#include"StgIntersection.hpp"
#include"StgUserExtendScene.hpp"
#include"StgPackageController.hpp"


class StgSystemInformation;
/**********************************************************
//StgSystemController
**********************************************************/
class StgSystemController : public TaskBase
{
	public:
		enum
		{
			TASK_PRI_WORK = 4,
			TASK_PRI_RENDER = 4,
		};

	protected:
		ref_count_ptr<StgSystemInformation> infoSystem_;
		ref_count_ptr<ScriptEngineCache> scriptEngineCache_;
		gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;

		ref_count_ptr<StgEndScene> endScene_;
		ref_count_ptr<StgReplaySaveScene> replaySaveScene_;

		ref_count_ptr<StgStageController> stageController_;

		ref_count_ptr<StgPackageController> packageController_;
		ref_count_ptr<StgControlScriptInformation> infoControlScript_;

		virtual void DoEnd() = 0;
		virtual void DoRetry() = 0;
		void _ControlScene();

	public:
		StgSystemController();
		~StgSystemController();
		void Initialize(ref_count_ptr<StgSystemInformation> infoSystem);
		void Start(ref_count_ptr<ScriptInformation> infoPlayer, ref_count_ptr<ReplayInformation> infoReplay);
		void Work();
		void Render();
		void RenderScriptObject();
		void RenderScriptObject(int priMin, int priMax);

		ref_count_ptr<StgSystemInformation>& GetSystemInformation(){return infoSystem_;}
		StgStageController* GetStageController(){return stageController_.GetPointer();}
		StgPackageController* GetPackageController(){return packageController_.GetPointer();}
		ref_count_ptr<StgControlScriptInformation>& GetControlScriptInformation(){return infoControlScript_;}


		gstd::ref_count_ptr<ScriptEngineCache> GetScriptEngineCache(){return scriptEngineCache_;}
		gstd::ref_count_ptr<ScriptCommonDataManager> GetCommonDataManager(){return commonDataManager_;}

		void StartStgScene(ref_count_ptr<StgStageInformation> infoStage, ref_count_ptr<ReplayInformation::StageData> replayStageData);
		void StartStgScene(ref_count_ptr<StgStageStartData> startData);

		void TransStgEndScene();
		void TransReplaySaveScene();

		ref_count_ptr<ReplayInformation> CreateReplayInformation();
		void TerminateScriptAll();
};

/**********************************************************
//StgSystemInformation
**********************************************************/
class StgSystemInformation
{
	public:
		enum
		{
			SCENE_NULL,
			SCENE_PACKAGE_CONTROL,
			SCENE_STG,
			SCENE_REPLAY_SAVE,
			SCENE_END,
		};

	private:
		int scene_;
		bool bEndStg_;
		bool bRetry_;

		std::wstring pathPauseScript_;
		std::wstring pathEndSceneScript_;
		std::wstring pathReplaySaveSceneScript_;

		std::list<std::wstring> listError_;
		ref_count_ptr<ScriptInformation> infoMain_;
		ref_count_ptr<ReplayInformation> infoReplayActive_; //�A�N�e�B�u���v���C���

		int invalidPriMin_;
		int invalidPriMax_;
		std::set<int> listReplayTargetKey_;

	public:
		StgSystemInformation();
		virtual ~StgSystemInformation();

		bool IsPackageMode();
		void ResetRetry();
		int GetScene(){return scene_;}
		void SetScene(int scene){scene_ = scene;}
		bool IsStgEnd(){return bEndStg_;}
		void SetStgEnd(){bEndStg_ = true;}
		bool IsRetry(){return bRetry_;}
		void SetRetry(){bRetry_ = true;}
		bool IsError(){return listError_.size() > 0;}
		void SetError(std::wstring error){listError_.push_back(error);}
		std::wstring GetErrorMessage();

		std::wstring GetPauseScriptPath(){return pathPauseScript_;}
		void SetPauseScriptPath(std::wstring path){pathPauseScript_ = path;}
		std::wstring GetEndSceneScriptPath(){return pathEndSceneScript_;}
		void SetEndSceneScriptPath(std::wstring path){pathEndSceneScript_ = path;}
		std::wstring GetReplaySaveSceneScriptPath(){return pathReplaySaveSceneScript_;}
		void SetReplaySaveSceneScriptPath(std::wstring path){pathReplaySaveSceneScript_ = path;}

		ref_count_ptr<ScriptInformation> GetMainScriptInformation(){return infoMain_;}
		void SetMainScriptInformation(ref_count_ptr<ScriptInformation> info){infoMain_ = info;}

		ref_count_ptr<ReplayInformation> GetActiveReplayInformation(){return infoReplayActive_;}
		void SetActiveReplayInformation(ref_count_ptr<ReplayInformation> info){infoReplayActive_ = info;}

		void SetInvaridRenderPriority(int priMin, int priMax);
		int GetInvaridRenderPriorityMin(){return invalidPriMin_;}
		int GetInvaridRenderPriorityMax(){return invalidPriMax_;}

		void AddReplayTargetKey(int id){listReplayTargetKey_.insert(id);}
		std::set<int> GetReplayTargetKeyList(){return listReplayTargetKey_;}

};



#endif
