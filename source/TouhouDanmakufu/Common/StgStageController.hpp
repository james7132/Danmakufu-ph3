#ifndef __TOUHOUDANMAKUFU_DNHSTG_STAGECONTROLLER__
#define __TOUHOUDANMAKUFU_DNHSTG_STAGECONTROLLER__

#include"StgCommon.hpp"
#include"StgStageScript.hpp"
#include"StgPlayer.hpp"
#include"StgEnemy.hpp"
#include"StgShot.hpp"
#include"StgItem.hpp"
#include"StgIntersection.hpp"
#include"StgUserExtendScene.hpp"

class StgStageInformation;
class StgStageStartData;
class PseudoSlowInformation;
/**********************************************************
//StgStageController
**********************************************************/
class StgStageController
{
	private:
		StgSystemController* systemController_;
		ref_count_ptr<StgSystemInformation> infoSystem_;
		ref_count_ptr<StgStageInformation> infoStage_;
		ref_count_ptr<PseudoSlowInformation> infoSlow_;

		ref_count_ptr<StgPauseScene> pauseManager_;
		ref_count_ptr<KeyReplayManager> keyReplayManager_;
		ref_count_ptr<StgStageScriptObjectManager> objectManagerMain_;
		ref_count_ptr<StgStageScriptManager> scriptManager_;
		ref_count_ptr<StgEnemyManager> enemyManager_;
		ref_count_ptr<StgShotManager> shotManager_;
		ref_count_ptr<StgItemManager> itemManager_;
		ref_count_ptr<StgIntersectionManager> intersectionManager_;

		void _SetupReplayTargetCommonDataArea(_int64 idScript);

	public:
		StgStageController(StgSystemController* systemController);
		virtual ~StgStageController();
		void Initialize(ref_count_ptr<StgStageStartData> startData);
		void CloseScene();
		void Work();
		void Render();
		void RenderToTransitionTexture();

		ref_count_ptr<StgStageScriptObjectManager> GetMainObjectManager(){return objectManagerMain_;}
		StgStageScriptManager* GetScriptManagerP(){return scriptManager_.GetPointer();}
		ref_count_ptr<StgStageScriptManager> GetScriptManagerR(){return scriptManager_;}
		StgEnemyManager* GetEnemyManager(){return enemyManager_.GetPointer();}
		StgShotManager* GetShotManager(){return shotManager_.GetPointer();}
		StgItemManager* GetItemManager(){return itemManager_.GetPointer();}
		StgIntersectionManager* GetIntersectionManager(){return intersectionManager_.GetPointer();}
		ref_count_ptr<StgPauseScene> GetPauseManager(){return pauseManager_;}

		ref_count_ptr<DxScriptObjectBase>::unsync GetMainRenderObject(int idObject);
		ref_count_ptr<StgPlayerObject>::unsync GetPlayerObject();

		StgSystemController* GetSystemController(){return systemController_;}
		ref_count_ptr<StgSystemInformation> GetSystemInformation(){return infoSystem_;}
		ref_count_ptr<StgStageInformation> GetStageInformation(){return infoStage_;}
		ref_count_ptr<KeyReplayManager> GetKeyReplayManager(){return keyReplayManager_;}

		ref_count_ptr<PseudoSlowInformation> GetSlowInformation(){return infoSlow_;}
};


/**********************************************************
//StgStageInformation
**********************************************************/
class StgStageInformation
{
	public:
		enum
		{
			RESULT_UNKNOWN,
			RESULT_BREAK_OFF,
			RESULT_PLAYER_DOWN,
			RESULT_CLEARED,
		};

	private:
		bool bEndStg_;
		bool bPause_;
		bool bReplay_;//ÉäÉvÉåÉC
		int frame_;
		int stageIndex_;

		ref_count_ptr<ScriptInformation> infoMainScript_;
		ref_count_ptr<ScriptInformation> infoPlayerScript_;
		ref_count_ptr<StgPlayerInformation> infoPlayerObject_;
		ref_count_ptr<ReplayInformation::StageData> replayStageData_;

		//STGê›íË
		RECT rcStgFrame_;
		int priMinStgFrame_;
		int priMaxStgFrame_;
		int priShotObject_;
		int priItemObject_;
		int priCameraFocusPermit_;
		RECT rcShotAutoDeleteClip_;

		//STGèÓïÒ
		ref_count_ptr<MersenneTwister> rand_;
		_int64 score_;
		_int64 graze_;
		_int64 point_;
		int result_;
		int timeStart_;

	public:
		StgStageInformation();
		virtual ~StgStageInformation();
		bool IsEnd(){return bEndStg_;}
		void SetEnd(){bEndStg_ = true;}
		bool IsPause(){return bPause_;}
		void SetPause(bool bPause){bPause_ = bPause;}
		bool IsReplay(){return bReplay_;}
		void SetReplay(bool bReplay){bReplay_ = bReplay;}
		int GetCurrentFrame(){return frame_;}
		void AdvanceFrame(){frame_++;}
		int GetStageIndex(){return stageIndex_;}
		void SetStageIndex(int index){stageIndex_ = index;}

		ref_count_ptr<ScriptInformation> GetMainScriptInformation(){return infoMainScript_;}
		void SetMainScriptInformation(ref_count_ptr<ScriptInformation> info){infoMainScript_ = info;}
		ref_count_ptr<ScriptInformation> GetPlayerScriptInformation(){return infoPlayerScript_;}
		void SetPlayerScriptInformation(ref_count_ptr<ScriptInformation> info ){infoPlayerScript_ = info;}
		ref_count_ptr<StgPlayerInformation> GetPlayerObjectInformation(){return infoPlayerObject_;}
		void SetPlayerObjectInformation(ref_count_ptr<StgPlayerInformation> info ){infoPlayerObject_ = info;}
		ref_count_ptr<ReplayInformation::StageData> GetReplayData(){return replayStageData_;}
		void SetReplayData(ref_count_ptr<ReplayInformation::StageData> data){replayStageData_ = data;}

		RECT GetStgFrameRect(){return rcStgFrame_;}
		void SetStgFrameRect(RECT rect, bool bUpdateFocusResetValue = true);
		int GetStgFrameMinPriority(){return priMinStgFrame_;}
		void SetStgFrameMinPriority(int pri){priMinStgFrame_ = pri;}
		int GetStgFrameMaxPriority(){return priMaxStgFrame_;}
		void SetStgFrameMaxPriority(int pri){priMaxStgFrame_ = pri;}
		int GetShotObjectPriority(){return priShotObject_;}
		void SetShotObjectPriority(int pri){priShotObject_ = pri;}
		int GetItemObjectPriority(){return priItemObject_;}
		void SetItemObjectPriority(int pri){priItemObject_ = pri;}
		int GetCameraFocusPermitPriority(){return priCameraFocusPermit_;}
		void SetCameraFocusPermitPriority(int pri){priCameraFocusPermit_ = pri;}
		RECT GetShotAutoDeleteClip(){return rcShotAutoDeleteClip_;}
		void SetShotAutoDeleteClip(RECT rect){rcShotAutoDeleteClip_ = rect;}

		ref_count_ptr<MersenneTwister> GetMersenneTwister(){return rand_;}
		void SetMersenneTwisterSeed(int seed){rand_->Initialize(seed);}
		_int64 GetScore(){return score_;}
		void SetScore(_int64 score){score_ = score;}
		void AddScore(_int64 inc){score_ += inc;}
		_int64 GetGraze(){return graze_;}
		void SetGraze(_int64 graze){graze_ = graze;}
		void AddGraze(_int64 inc){graze_ += inc;}
		_int64 GetPoint(){return point_;}
		void SetPoint(_int64 point){point_ = point;}
		void AddPoint(_int64 inc){point_ += inc;}

		int GetResult(){return result_;}
		void SetResult(int result){result_ = result;}

		int GetStageStartTime(){return timeStart_;}
		void SetStageStartTime(int time){timeStart_ = time;}
};

/**********************************************************
//StgStageStartData
**********************************************************/
class StgStageStartData
{
	private:
		ref_count_ptr<StgStageInformation> infoStage_;
		ref_count_ptr<ReplayInformation::StageData> replayStageData_;
		ref_count_ptr<StgStageInformation> prevStageInfo_;
		ref_count_ptr<StgPlayerInformation> prevPlayerInfo_;

	public:
		StgStageStartData(){}
		virtual ~StgStageStartData(){}

		ref_count_ptr<StgStageInformation> GetStageInformation(){return infoStage_;}
		void SetStageInformation(ref_count_ptr<StgStageInformation> info){infoStage_ = info;}
		ref_count_ptr<ReplayInformation::StageData> GetStageReplayData(){return replayStageData_;}
		void SetStageReplayData(ref_count_ptr<ReplayInformation::StageData> data){replayStageData_ = data;}
		ref_count_ptr<StgStageInformation> GetPrevStageInformation(){return prevStageInfo_;}
		void SetPrevStageInformation(ref_count_ptr<StgStageInformation> info){prevStageInfo_ = info;}
		ref_count_ptr<StgPlayerInformation> GetPrevPlayerInformation(){return prevPlayerInfo_;}
		void SetPrevPlayerInformation(ref_count_ptr<StgPlayerInformation> info){prevPlayerInfo_ = info;}

};

/**********************************************************
//PseudoSlowInformation
**********************************************************/
class PseudoSlowInformation : public gstd::FpsControlObject
{
	public:
		class SlowData;
		enum
		{
			OWNER_PLAYER = 0,
			OWNER_ENEMY,
			TARGET_ALL,
		};

	private:
		int current_;
		std::map<int, gstd::ref_count_ptr<SlowData> > mapDataPlayer_;
		std::map<int, gstd::ref_count_ptr<SlowData> > mapDataEnemy_;
		std::map<int, bool> mapValid_;
		
	public:
		PseudoSlowInformation(){current_ = 0;}
		virtual ~PseudoSlowInformation(){}
		virtual int GetFps();

		bool IsValidFrame(int target);
		void Next();

		void AddSlow(int fps, int owner, int target);
		void RemoveSlow(int owner, int target);
};

class PseudoSlowInformation::SlowData
{
	private:
		int fps_;
	public:
		SlowData(){fps_ = STANDARD_FPS;}
		virtual ~SlowData(){}
		int GetFps(){return fps_;}
		void SetFps(int fps){fps_ = fps;}
};

#endif
