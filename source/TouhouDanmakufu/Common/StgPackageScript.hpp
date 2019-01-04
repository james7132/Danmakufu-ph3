#ifndef __TOUHOUDANMAKUFU_DNHSTG_PACKAGESCRIPT__
#define __TOUHOUDANMAKUFU_DNHSTG_PACKAGESCRIPT__

#include"StgCommon.hpp"
#include"StgControlScript.hpp"
#include"StgStageController.hpp"

/**********************************************************
//StgPackageScriptManager
**********************************************************/
class StgPackageScript;
class StgPackageScriptManager : public StgControlScriptManager
{
	protected:
		StgSystemController* systemController_;
		ref_count_ptr<DxScriptObjectManager> objectManager_;

	public:
		StgPackageScriptManager(StgSystemController* controller);
		virtual ~StgPackageScriptManager();
		virtual void Work();
		virtual void Render();
		virtual ref_count_ptr<ManagedScript> Create(int type);

		ref_count_ptr<DxScriptObjectManager> GetObjectManager(){return objectManager_;}
};

/**********************************************************
//StgPackageScript
**********************************************************/
class StgPackageScript : public StgControlScript
{
	public:
		enum
		{
			TYPE_PACKAGE_MAIN,

			STAGE_STATE_FINISHED,
		};

	private:
		StgPackageController* packageController_;
		void _CheckNextStageExists();

	public:
		StgPackageScript(StgPackageController* packageController);


		//�p�b�P�[�W���ʊ֐��F�p�b�P�[�W����
		static gstd::value Func_ClosePackage(gstd::script_machine* machine, int argc, gstd::value const * argv);

		//�p�b�P�[�W���ʊ֐��F�X�e�[�W����
		static gstd::value Func_InitializeStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_FinalizeStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_StartStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetStageIndex(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetStageMainScript(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetStagePlayerScript(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_SetStageReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetStageSceneState(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_GetStageSceneResult(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_PauseStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv);
		static gstd::value Func_TerminateStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv);


};

#endif

