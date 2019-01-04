#include"StgPackageScript.hpp"
#include"StgSystem.hpp"
#include"StgPackageController.hpp"

/**********************************************************
//StgPackageScriptManager
**********************************************************/
StgPackageScriptManager::StgPackageScriptManager(StgSystemController* controller)
{
	systemController_ = controller;
	objectManager_ = new DxScriptObjectManager();
}
StgPackageScriptManager::~StgPackageScriptManager()
{
}
void StgPackageScriptManager::Work()
{
	if(!IsError())
	{
		StgControlScriptManager::Work();
		objectManager_->WorkObject();
	}
	else
	{
		systemController_->GetSystemInformation()->SetError(error_);
	}

}
void StgPackageScriptManager::Render()
{
	objectManager_->RenderObject();
}
ref_count_ptr<ManagedScript> StgPackageScriptManager::Create(int type)
{
	ref_count_ptr<ManagedScript> res;
	switch(type)
	{
		case StgPackageScript::TYPE_PACKAGE_MAIN:
			res = new StgPackageScript(systemController_->GetPackageController());
			break;
	}

	if(res != NULL)
	{
		res->SetObjectManager(objectManager_);
		res->SetScriptManager(this);
	}

	return res;
}


/**********************************************************
//StgPackageScript
**********************************************************/
const function stgPackageFunction[] =  
{
	//パッケージ共通関数：パッケージ操作
	{"ClosePackage", StgPackageScript::Func_ClosePackage, 0},

	//パッケージ共通関数：ステージ操作
	{"InitializeStageScene", StgPackageScript::Func_InitializeStageScene, 0},
	{"FinalizeStageScene", StgPackageScript::Func_FinalizeStageScene, 0},
	{"StartStageScene", StgPackageScript::Func_StartStageScene, 0},
	{"SetStageIndex", StgPackageScript::Func_SetStageIndex, 1},
	{"SetStageMainScript", StgPackageScript::Func_SetStageMainScript, 1},
	{"SetStagePlayerScript", StgPackageScript::Func_SetStagePlayerScript, 1},
	{"SetStageReplayFile", StgPackageScript::Func_SetStageReplayFile, 1},
	{"GetStageSceneState", StgPackageScript::Func_GetStageSceneState, 0},
	{"GetStageSceneResult", StgPackageScript::Func_GetStageSceneResult, 0},
	{"PauseStageScene", StgPackageScript::Func_PauseStageScene, 1},
	{"TerminateStageScene", StgPackageScript::Func_TerminateStageScene, 0},

	//定数：
	{"STAGE_STATE_FINISHED", constant<StgPackageScript::STAGE_STATE_FINISHED>::func, 0},

	{"STAGE_RESULT_BREAK_OFF", constant<StgStageInformation::RESULT_BREAK_OFF>::func, 0},
	{"STAGE_RESULT_PLAYER_DOWN", constant<StgStageInformation::RESULT_PLAYER_DOWN>::func, 0},
	{"STAGE_RESULT_CLEARED", constant<StgStageInformation::RESULT_CLEARED>::func, 0},

};
StgPackageScript::StgPackageScript(StgPackageController* packageController) : StgControlScript(packageController->GetSystemController())
{
	_AddFunction(stgPackageFunction, sizeof(stgPackageFunction) / sizeof(function));

	typeScript_ = TYPE_PACKAGE_MAIN;
	packageController_ = packageController;
}
void StgPackageScript::_CheckNextStageExists()
{
	ref_count_ptr<StgPackageInformation> infoPackage = packageController_->GetPackageInformation();
	ref_count_ptr<StgStageStartData> nextStageData = infoPackage->GetNextStageData();
	if(nextStageData == NULL)RaiseError(L"not initialized stage data");
}

//パッケージ共通関数：パッケージ操作
gstd::value StgPackageScript::Func_ClosePackage(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	infoPackage->SetEnd();
	
	return value();
}

//パッケージ共通関数：ステージ操作
gstd::value StgPackageScript::Func_InitializeStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;

	StgSystemController* systemController = packageController->GetSystemController();
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();
	infoSystem->SetActiveReplayInformation(NULL);

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	infoPackage->InitializeStageData();
	
	return value();
}
gstd::value StgPackageScript::Func_FinalizeStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	StgSystemController* systemController = packageController->GetSystemController();
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	if(infoPackage->GetNextStageData() != NULL && infoPackage->GetNextStageData()->GetPrevStageInformation() == NULL)
		script->RaiseError(L"not finished stage");

	std::vector<ref_count_ptr<StgStageStartData> > listStage = infoPackage->GetStageDataList();
	if(listStage.size() > 0)
	{
		ref_count_ptr<StgStageStartData> stageData = listStage[listStage.size() - 1];
		ref_count_ptr<StgStageInformation> infoStage = stageData->GetStageInformation();
		bool bReplay = infoStage->IsReplay();
		if(bReplay)return value();
	}

	ref_count_ptr<ReplayInformation> infoReplay = systemController->CreateReplayInformation();
	infoSystem->SetActiveReplayInformation(infoReplay);	

	return value();
}
gstd::value StgPackageScript::Func_StartStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	StgSystemController* systemController = packageController->GetSystemController();
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();
	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	ref_count_ptr<StgStageStartData> nextStageData = infoPackage->GetNextStageData();
	ref_count_ptr<StgStageInformation> infoStage = nextStageData->GetStageInformation();

	int stageIndex = infoStage->GetStageIndex();
	ref_count_ptr<ReplayInformation> infoReplay = infoPackage->GetReplayInformation();
	std::wstring replayPlayerID;
	std::wstring replayPlayerScriptFileName;
	if(infoReplay != NULL)
	{
		ref_count_ptr<ReplayInformation::StageData> replayStageData = infoReplay->GetStageData(stageIndex);
		if(replayStageData == NULL)
			script->RaiseError(L"invalid replay stage index");
		nextStageData->SetStageReplayData(replayStageData);

		//自機スクリプト
		replayPlayerID = replayStageData->GetPlayerScriptID();
		replayPlayerScriptFileName = replayStageData->GetPlayerScriptFileName();
	}
	else
	{
		ref_count_ptr<ScriptInformation> infoPlayer = infoStage->GetPlayerScriptInformation();
		if(infoPlayer != NULL)
		{
			replayPlayerID = infoPlayer->GetID();
			replayPlayerScriptFileName = PathProperty::GetFileName(infoPlayer->GetScriptPath());
		}
	}

	//自機を検索
	infoStage->SetPlayerScriptInformation(NULL);
	ref_count_ptr<ScriptInformation> infoMain = infoSystem->GetMainScriptInformation();
	std::vector<ref_count_ptr<ScriptInformation> > listPlayer;
	std::vector<std::wstring> listPlayerPath = infoMain->GetPlayerList();
	if(listPlayerPath.size() == 0)
	{
		std::wstring dir = EPathProperty::GetPlayerScriptRootDirectory();
		listPlayer = ScriptInformation::FindPlayerScriptInformationList(dir);
	}
	else
	{
		listPlayer = infoMain->CreatePlayerScriptInformationList();
	}

	for(int iPlayer = 0 ; iPlayer < listPlayer.size() ; iPlayer++)
	{
		ref_count_ptr<ScriptInformation> tInfo = listPlayer[iPlayer];
		if(tInfo->GetID() != replayPlayerID)continue;
		std::wstring tPlayerScriptFileName = PathProperty::GetFileName(tInfo->GetScriptPath());
		if(tPlayerScriptFileName != replayPlayerScriptFileName)continue;

		infoStage->SetPlayerScriptInformation(tInfo);
		break;
	}

	if(infoStage->GetPlayerScriptInformation() == NULL)
		script->RaiseError(L"player not found");

	//packageController->RenderToTransitionTexture();

	systemController->StartStgScene(nextStageData);

	return value();
}
gstd::value StgPackageScript::Func_SetStageIndex(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	ref_count_ptr<StgStageStartData> nextStageData = infoPackage->GetNextStageData();
	ref_count_ptr<StgStageInformation> infoStage = nextStageData->GetStageInformation();

	int stageIndex = (int)argv[0].as_real();	
	std::vector<ref_count_ptr<StgStageStartData> > listStage = infoPackage->GetStageDataList();
	for(int iStage = 0 ; iStage < listStage.size() ;iStage++)
	{
		ref_count_ptr<StgStageStartData> stageData = listStage[iStage];
		if(stageIndex == stageData->GetStageInformation()->GetStageIndex())
			script->RaiseError(L"stage index already used");
	}


	infoStage->SetStageIndex(stageIndex);
	
	return value();
}
gstd::value StgPackageScript::Func_SetStageMainScript(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	ref_count_ptr<StgStageStartData> nextStageData = infoPackage->GetNextStageData();
	ref_count_ptr<StgStageInformation> infoStage = nextStageData->GetStageInformation();

	std::wstring path = argv[0].as_string();
	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if(reader == NULL || !reader->Open())
	{
		std::wstring error = ErrorUtility::GetFileNotFoundErrorMessage(path);
		script->RaiseError(error);
	}

	std::string source = "";
	int size = reader->GetFileSize();
	source.resize(size);
	reader->Read(&source[0], size);

	ref_count_ptr<ScriptInformation> infoScript = 
		ScriptInformation::CreateScriptInformation(path, L"", source, false);
	if(infoScript == NULL)
		script->RaiseError(ErrorUtility::GetFileNotFoundErrorMessage(path));

	infoStage->SetMainScriptInformation(infoScript);
	
	return value();
}
gstd::value StgPackageScript::Func_SetStagePlayerScript(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	ref_count_ptr<StgStageStartData> nextStageData = infoPackage->GetNextStageData();
	ref_count_ptr<StgStageInformation> infoStage = nextStageData->GetStageInformation();

	std::wstring path = argv[0].as_string();
	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if(reader == NULL || !reader->Open())
	{
		std::wstring error = ErrorUtility::GetFileNotFoundErrorMessage(path);
		script->RaiseError(error);
	}

	std::string source = "";
	int size = reader->GetFileSize();
	source.resize(size);
	reader->Read(&source[0], size);

	ref_count_ptr<ScriptInformation> infoScript = 
		ScriptInformation::CreateScriptInformation(path, L"", source);
	infoStage->SetPlayerScriptInformation(infoScript);
	
	return value(machine->get_engine()->get_boolean_type(), true);
}
gstd::value StgPackageScript::Func_SetStageReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();

	std::wstring pathReplay = argv[0].as_string();
	ref_count_ptr<ReplayInformation> infoRepray = 
		ReplayInformation::CreateFromFile(pathReplay);
	if(infoRepray == NULL)
	{
		std::wstring path = ErrorUtility::GetFileNotFoundErrorMessage(pathReplay);
		script->RaiseError(path);
	}
	infoPackage->SetReplayInformation(infoRepray);
	return value();
}
gstd::value StgPackageScript::Func_GetStageSceneState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	StgSystemController* systemController = packageController->GetSystemController();
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();

	int res = -1;
	int scene = infoSystem->GetScene();
	if(scene == StgSystemInformation::SCENE_PACKAGE_CONTROL)
		res = STAGE_STATE_FINISHED;

	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgPackageScript::Func_GetStageSceneResult(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	script->_CheckNextStageExists();

	int res = StgStageInformation::RESULT_UNKNOWN;
	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	std::vector<ref_count_ptr<StgStageStartData> > listStage = infoPackage->GetStageDataList();
	if(listStage.size() > 0)
	{
		ref_count_ptr<StgStageStartData> stageData = listStage[listStage.size() - 1];
		ref_count_ptr<StgStageInformation> infoStage = stageData->GetStageInformation();
		res = infoStage->GetResult();
	}

	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgPackageScript::Func_PauseStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	StgSystemController* systemController = packageController->GetSystemController();
	StgStageController* stageController = systemController->GetStageController();
	if(stageController == NULL)return gstd::value();

	bool bPause = argv[0].as_boolean();

	StgStageScriptManager* stageScriptManager = stageController->GetScriptManagerP();
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	if(bPause && !infoStage->IsPause())
		stageScriptManager->RequestEventAll(StgStageScript::EV_PAUSE_ENTER);
	else if(!bPause && infoStage->IsPause())
		stageScriptManager->RequestEventAll(StgStageScript::EV_PAUSE_LEAVE);

	infoStage->SetPause(bPause);

	return gstd::value();
}
gstd::value StgPackageScript::Func_TerminateStageScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgPackageScript* script = (StgPackageScript*)machine->data;
	StgPackageController* packageController = script->packageController_;
	StgSystemController* systemController = packageController->GetSystemController();
	StgStageController* stageController = systemController->GetStageController();
	if(stageController == NULL)return gstd::value();

	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	infoStage->SetResult(StgStageInformation::RESULT_BREAK_OFF);
	infoStage->SetEnd();
	return gstd::value();
}



