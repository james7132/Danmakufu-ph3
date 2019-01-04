#include"StgControlScript.hpp"
#include"StgSystem.hpp"

/**********************************************************
//StgControlScriptManager
**********************************************************/
StgControlScriptManager::StgControlScriptManager()
{

}
StgControlScriptManager::~StgControlScriptManager()
{

}

/**********************************************************
//StgControlScriptInformation
**********************************************************/
StgControlScriptInformation::StgControlScriptInformation()
{
	replayManager_ = new ReplayInformationManager();
}
StgControlScriptInformation::~StgControlScriptInformation()
{
}
void StgControlScriptInformation::LoadFreePlayerList()
{
	std::wstring dir = EPathProperty::GetPlayerScriptRootDirectory();
	listFreePlayer_ = ScriptInformation::FindPlayerScriptInformationList(dir);

	//�\�[�g
	std::sort(listFreePlayer_.begin(), listFreePlayer_.end(), ScriptInformation::PlayerListSort());
}
void StgControlScriptInformation::LoadReplayInformation(std::wstring pathMainScript)
{
	replayManager_->UpdateInformationList(pathMainScript);
}


/**********************************************************
//StgControlScript
**********************************************************/
const function stgControlFunction[] =  
{
	//�֐��F
	//STG���䋤�ʊ֐��F���ʃf�[�^
	{"SaveCommonDataAreaA1", StgControlScript::Func_SaveCommonDataAreaA1, 1},
	{"LoadCommonDataAreaA1", StgControlScript::Func_LoadCommonDataAreaA1, 1},
	{"SaveCommonDataAreaA2", StgControlScript::Func_SaveCommonDataAreaA2, 2},
	{"LoadCommonDataAreaA2", StgControlScript::Func_LoadCommonDataAreaA2, 2},

	//STG���䋤�ʊ֐��F�L�[�n
	{"AddVirtualKey", StgControlScript::Func_AddVirtualKey, 3},
	{"AddReplayTargetVirtualKey", StgControlScript::Func_AddReplayTargetVirtualKey, 1},
	{"SetSkipModeKey", StgControlScript::Func_SetSkipModeKey, 1},

	//STG���䋤�ʊ֐��F�V�X�e���֘A
	{"GetScore", StgControlScript::Func_GetScore, 0},
	{"AddScore", StgControlScript::Func_AddScore, 1},
	{"GetGraze", StgControlScript::Func_GetGraze, 0},
	{"AddGraze", StgControlScript::Func_AddGraze, 1},
	{"GetPoint", StgControlScript::Func_GetPoint, 0},
	{"AddPoint", StgControlScript::Func_AddPoint, 1},
	{"IsReplay", StgControlScript::Func_IsReplay, 0},
	{"AddArchiveFile", StgControlScript::Func_AddArchiveFile, 1},
	{"GetCurrentFps", StgControlScript::Func_GetCurrentFps, 0},
	{"GetStageTime", StgControlScript::Func_GetStageTime, 0},
	{"GetStageTimeF", StgControlScript::Func_GetStageTimeF, 0},
	{"GetPackageTime", StgControlScript::Func_GetPackageTime, 0},

	{"GetStgFrameLeft", StgControlScript::Func_GetStgFrameLeft, 0},
	{"GetStgFrameTop", StgControlScript::Func_GetStgFrameTop, 0},
	{"GetStgFrameWidth", StgControlScript::Func_GetStgFrameWidth, 0},
	{"GetStgFrameHeight", StgControlScript::Func_GetStgFrameHeight, 0},

	{"GetMainPackageScriptPath", StgControlScript::Func_GetMainPackageScriptPath, 0},
	{"GetScriptPathList", StgControlScript::Func_GetScriptPathList, 2},
	{"GetScriptInfoA1", StgControlScript::Func_GetScriptInfoA1, 2},

	//STG���ʊ֐��F�`��֘A
	{"ClearInvalidRenderPriority", StgControlScript::Func_ClearInvalidRenderPriority, 0},
	{"SetInvalidRenderPriorityA1", StgControlScript::Func_SetInvalidRenderPriorityA1, 2},
	{"GetReservedRenderTargetName", StgControlScript::Func_GetReservedRenderTargetName, 1},
	{"RenderToTextureA1", StgControlScript::Func_RenderToTextureA1, 4},
	{"RenderToTextureB1", StgControlScript::Func_RenderToTextureB1, 3},
	{"SaveSnapShotA1", StgControlScript::Func_SaveSnapShotA1, 1},
	{"SaveSnapShotA2", StgControlScript::Func_SaveSnapShotA2, 5},

	//STG���䋤�ʊ֐��F���@�֘A
	{"GetPlayerID", StgControlScript::Func_GetPlayerID, 0},
	{"GetPlayerReplayName", StgControlScript::Func_GetPlayerReplayName, 0},

	//STG���䋤�ʊ֐��F���[�U�X�N���v�g
	{"SetPauseScriptPath", StgControlScript::Func_SetPauseScriptPath, 1},
	{"SetEndSceneScriptPath", StgControlScript::Func_SetEndSceneScriptPath, 1},
	{"SetReplaySaveSceneScriptPath", StgControlScript::Func_SetReplaySaveSceneScriptPath, 1},

	//STG���䋤�ʊ֐��F���@�X�N���v�g
	{"GetLoadFreePlayerScriptList", StgControlScript::Func_GetLoadFreePlayerScriptList, 0},
	{"GetFreePlayerScriptCount", StgControlScript::Func_GetFreePlayerScriptCount, 0},
	{"GetFreePlayerScriptInfo", StgControlScript::Func_GetFreePlayerScriptInfo, 2},

	//STG���䋤�ʊ֐��F���v���C�֘A
	{"LoadReplayList", StgControlScript::Func_LoadReplayList, 0},
	{"GetValidReplayIndices", StgControlScript::Func_GetValidReplayIndices, 0},
	{"IsValidReplayIndex", StgControlScript::Func_IsValidReplayIndex, 1},
	{"GetReplayInfo", StgControlScript::Func_GetReplayInfo, 2},
	{"SetReplayInfo", StgControlScript::Func_SetReplayInfo, 2},
	{"GetReplayUserData", StgControlScript::Func_GetReplayUserData, 2},
	{"SetReplayUserData", StgControlScript::Func_SetReplayUserData, 2},
	{"IsReplayUserDataExists", StgControlScript::Func_IsReplayUserDataExists, 2},
	{"SaveReplay", StgControlScript::Func_SaveReplay, 2},

	//�萔�F
	{"EV_USER_COUNT", constant<StgControlScript::EV_USER_COUNT>::func, 0},
	{"EV_USER", constant<StgControlScript::EV_USER>::func, 0},
	{"EV_USER_SYSTEM", constant<StgControlScript::EV_USER_SYSTEM>::func, 0},
	{"EV_USER_STAGE", constant<StgControlScript::EV_USER_STAGE>::func, 0},
	{"EV_USER_PLAYER", constant<StgControlScript::EV_USER_PLAYER>::func, 0},
	{"EV_USER_PACKAGE", constant<StgControlScript::EV_USER_PACKAGE>::func, 0},

	{"TYPE_SCRIPT_ALL", constant<StgControlScript::TYPE_SCRIPT_ALL>::func, 0},
	{"TYPE_SCRIPT_PLAYER", constant<StgControlScript::TYPE_SCRIPT_PLAYER>::func, 0},
	{"TYPE_SCRIPT_SINGLE", constant<StgControlScript::TYPE_SCRIPT_SINGLE>::func, 0},
	{"TYPE_SCRIPT_PLURAL", constant<StgControlScript::TYPE_SCRIPT_PLURAL>::func, 0},
	{"TYPE_SCRIPT_STAGE", constant<StgControlScript::TYPE_SCRIPT_STAGE>::func, 0},
	{"TYPE_SCRIPT_PACKAGE", constant<StgControlScript::TYPE_SCRIPT_PACKAGE>::func, 0},

	{"INFO_SCRIPT_TYPE", constant<StgControlScript::INFO_SCRIPT_TYPE>::func, 0},
	{"INFO_SCRIPT_PATH", constant<StgControlScript::INFO_SCRIPT_PATH>::func, 0},
	{"INFO_SCRIPT_ID", constant<StgControlScript::INFO_SCRIPT_ID>::func, 0},
	{"INFO_SCRIPT_TITLE", constant<StgControlScript::INFO_SCRIPT_TITLE>::func, 0},
	{"INFO_SCRIPT_TEXT", constant<StgControlScript::INFO_SCRIPT_TEXT>::func, 0},
	{"INFO_SCRIPT_IMAGE", constant<StgControlScript::INFO_SCRIPT_IMAGE>::func, 0},
	{"INFO_SCRIPT_REPLAY_NAME", constant<StgControlScript::INFO_SCRIPT_REPLAY_NAME>::func, 0},

	{"REPLAY_FILE_PATH", constant<StgControlScript::REPLAY_FILE_PATH>::func, 0},
	{"REPLAY_DATE_TIME", constant<StgControlScript::REPLAY_DATE_TIME>::func, 0},
	{"REPLAY_USER_NAME", constant<StgControlScript::REPLAY_USER_NAME>::func, 0},
	{"REPLAY_TOTAL_SCORE", constant<StgControlScript::REPLAY_TOTAL_SCORE>::func, 0},
	{"REPLAY_FPS_AVERAGE", constant<StgControlScript::REPLAY_FPS_AVERAGE>::func, 0},
	{"REPLAY_PLAYER_NAME", constant<StgControlScript::REPLAY_PLAYER_NAME>::func, 0},
	{"REPLAY_STAGE_INDEX_LIST", constant<StgControlScript::REPLAY_STAGE_INDEX_LIST>::func, 0},
	{"REPLAY_STAGE_START_SCORE_LIST", constant<StgControlScript::REPLAY_STAGE_START_SCORE_LIST>::func, 0},
	{"REPLAY_STAGE_LAST_SCORE_LIST", constant<StgControlScript::REPLAY_STAGE_LAST_SCORE_LIST>::func, 0},
	{"REPLAY_COMMENT", constant<StgControlScript::REPLAY_COMMENT>::func, 0},

	{"REPLAY_INDEX_ACTIVE", constant<ReplayInformation::INDEX_ACTIVE>::func, 0},
	{"REPLAY_INDEX_DIGIT_MIN", constant<ReplayInformation::INDEX_DIGIT_MIN>::func, 0},
	{"REPLAY_INDEX_DIGIT_MAX", constant<ReplayInformation::INDEX_DIGIT_MAX>::func, 0},
	{"REPLAY_INDEX_USER", constant<ReplayInformation::INDEX_USER>::func, 0},

	{"RESULT_CANCEL", constant<StgControlScript::RESULT_CANCEL>::func, 0},
	{"RESULT_END", constant<StgControlScript::RESULT_END>::func, 0},
	{"RESULT_RETRY", constant<StgControlScript::RESULT_RETRY>::func, 0},
	{"RESULT_SAVE_REPLAY", constant<StgControlScript::RESULT_SAVE_REPLAY>::func, 0},
};
StgControlScript::StgControlScript(StgSystemController* systemController)
{
	systemController_ = systemController;
	scriptManager_ = NULL;
	_AddFunction(stgControlFunction, sizeof(stgControlFunction) / sizeof(function));

	bLoad_ = false;
	bEndScript_ = false;
	bAutoDeleteObject_ = false;

	SetScriptEngineCache(systemController->GetScriptEngineCache());
	commonDataManager_ = systemController->GetCommonDataManager();
}


//STG���䋤�ʊ֐��F���ʃf�[�^
gstd::value StgControlScript::Func_SaveCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring area = argv[0].as_string();
	std::string sArea = to_mbcs(area);
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	ref_count_ptr<ScriptCommonData> commonData = commonDataManager->GetData(sArea);
	if(commonData == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	std::wstring pathMain = infoSystem->GetMainScriptInformation()->GetScriptPath();
	std::wstring pathSave = EPathProperty::GetCommonDataPath(pathMain, area);
	std::wstring dirSave = PathProperty::GetFileDirectory(pathSave);

	File fDir(dirSave);
	fDir.CreateDirectory();
	
	RecordBuffer record;
	commonData->WriteRecord(record);
	bool res = record.WriteToFile(pathSave);

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgControlScript::Func_LoadCommonDataAreaA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring area = argv[0].as_string();
	std::string sArea = to_mbcs(area);
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	
	std::wstring pathMain = infoSystem->GetMainScriptInformation()->GetScriptPath();
	std::wstring pathSave = EPathProperty::GetCommonDataPath(pathMain, area);
	RecordBuffer record;
	bool res = record.ReadFromFile(pathSave);
	if(!res)
		return value(machine->get_engine()->get_boolean_type(), false);

	ref_count_ptr<ScriptCommonData> commonData = new ScriptCommonData();
	commonData->ReadRecord(record);
	commonDataManager->SetData(sArea, commonData);

	return value(machine->get_engine()->get_boolean_type(), true);
}

gstd::value StgControlScript::Func_SaveCommonDataAreaA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::string area = to_mbcs(argv[0].as_string());
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	ref_count_ptr<ScriptCommonData> commonData = commonDataManager->GetData(area);
	if(commonData == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	std::wstring pathSave = argv[1].as_string();
	std::wstring dirSave = PathProperty::GetFileDirectory(pathSave);

	File fDir(dirSave);
	fDir.CreateDirectory();
	
	RecordBuffer record;
	commonData->WriteRecord(record);
	bool res = record.WriteToFile(pathSave);

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgControlScript::Func_LoadCommonDataAreaA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::string area = to_mbcs(argv[0].as_string());
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	
	std::wstring pathSave = argv[1].as_string();
	RecordBuffer record;
	bool res = record.ReadFromFile(pathSave);
	if(!res)
		return value(machine->get_engine()->get_boolean_type(), false);

	ref_count_ptr<ScriptCommonData> commonData = new ScriptCommonData();
	commonData->ReadRecord(record);
	commonDataManager->SetData(area, commonData);

	return value(machine->get_engine()->get_boolean_type(), true);
}

//STG���䋤�ʊ֐��F�L�[�n
gstd::value StgControlScript::Func_AddVirtualKey(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	EDirectInput* input = EDirectInput::GetInstance();
	int padIndex = input->GetPadIndex();
	
	int id = (int)argv[0].as_real();
	int key = (int)argv[1].as_real();
	int padButton = (int)argv[2].as_real();

	ref_count_ptr<VirtualKey> vkey = new VirtualKey(key, padIndex, padButton);
	input->AddKeyMap(id, vkey);

	return value();
}
gstd::value StgControlScript::Func_AddReplayTargetVirtualKey(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	StgStageController* stageController = script->systemController_->GetStageController();

	int id = (int)argv[0].as_real();
	infoSystem->AddReplayTargetKey(id);
	if(stageController != NULL)
	{
		ref_count_ptr<KeyReplayManager> keyReplayManager = stageController->GetKeyReplayManager();
		keyReplayManager->AddTarget(id);
	}

	return value();
}
gstd::value StgControlScript::Func_SetSkipModeKey(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	int id = (int)argv[0].as_real();
	EFpsController* fpsController = EFpsController::GetInstance();
	fpsController->SetFastModeKey(id);

	return value();
}

//STG���䋤�ʊ֐��F�V�X�e���֘A
gstd::value StgControlScript::Func_GetScore(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0.0);

	long double res = stageController->GetStageInformation()->GetScore();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_AddScore(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)return value();

	long double inc = (_int64)argv[0].as_real();
	stageController->GetStageInformation()->AddScore(inc);
	return value();
}
gstd::value StgControlScript::Func_GetGraze(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0.0);

	long double res = stageController->GetStageInformation()->GetGraze();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_AddGraze(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)return value();

	long double inc = (_int64)argv[0].as_real();
	stageController->GetStageInformation()->AddGraze(inc);
	return value();
}
gstd::value StgControlScript::Func_GetPoint(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0.0);

	long double res = stageController->GetStageInformation()->GetPoint();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_AddPoint(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)return value();

	long double inc = (_int64)argv[0].as_real();
	stageController->GetStageInformation()->AddPoint(inc);
	return value();
}
gstd::value StgControlScript::Func_IsReplay(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	bool res = stageController->GetStageInformation()->IsReplay();
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgControlScript::Func_AddArchiveFile(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	FileManager* fileManager = FileManager::GetBase();

	std::wstring path = argv[0].as_string();
	bool res = fileManager->AddArchiveFile(path);
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgControlScript::Func_GetCurrentFps(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	EFpsController* fpsController = EFpsController::GetInstance();
	long double res = fpsController->GetCurrentWorkFps();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetStageTime(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0);

	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	int time = timeGetTime();

	int timeStart = infoStage->GetStageStartTime();
	long double res = timeStart > 0 ? time - timeStart : 0;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetStageTimeF(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0);

	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	long double res = infoStage->GetCurrentFrame();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetPackageTime(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgPackageController* packageController = script->systemController_->GetPackageController();
	if(packageController == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0);

	ref_count_ptr<StgPackageInformation> infoPackage = packageController->GetPackageInformation();
	int time = timeGetTime();

	int timeStart = infoPackage->GetPackageStartTime();
	long double res = timeStart > 0 ? time - timeStart : 0;
	return value(machine->get_engine()->get_real_type(), res);
}

gstd::value StgControlScript::Func_GetStgFrameLeft(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();

	long double res = 0;
	if(stageController != NULL)
	{
		RECT rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = rect.left;
	}
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetStgFrameTop(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();

	long double res = 0;
	if(stageController != NULL)
	{
		RECT rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = rect.top;
	}
	return value(machine->get_engine()->get_real_type(), res);
}	
gstd::value StgControlScript::Func_GetStgFrameWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();

	long double res = 0;
	if(stageController != NULL)
	{
		RECT rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = res = rect.right - rect.left;
	}
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetStgFrameHeight(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();

	long double res = 0;
	if(stageController != NULL)
	{
		RECT rect = stageController->GetStageInformation()->GetStgFrameRect();
		res = res = rect.bottom - rect.top;
	}
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgControlScript::Func_GetMainPackageScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgPackageController* packageController = script->systemController_->GetPackageController();

	std::wstring path = L"";
	if(packageController != NULL)
	{
		ref_count_ptr<ScriptInformation> infoScript = 
			packageController->GetPackageInformation()->GetMainScriptInformation();
		path = infoScript->GetScriptPath();
	}

	std::wstring res = path;
	return value(machine->get_engine()->get_string_type(), res);
}
gstd::value StgControlScript::Func_GetScriptPathList(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;

	std::vector<std::wstring> listRes;
	std::wstring dir = argv[0].as_string();
	dir = PathProperty::GetFileDirectory(dir);

	int typeScript = (int)argv[1].as_real();
	std::vector<std::wstring> listFile = File::GetFilePathList(dir);
	for(int iFile = 0 ; iFile < listFile.size() ; iFile++)
	{
		std::wstring path = listFile[iFile];

		//���炩�Ɋ֌W�Ȃ������Ȋg���q�͏��O
		std::wstring ext = PathProperty::GetFileExtension(path);
		if(ScriptInformation::IsExcludeExtention(ext))continue;

		path = PathProperty::GetUnique(path);
		ref_count_ptr<ScriptInformation> infoScript = ScriptInformation::CreateScriptInformation(path, true);
		if(infoScript == NULL)continue;
		if(typeScript != TYPE_SCRIPT_ALL && typeScript != infoScript->GetType())continue;

		script->mapScriptInfo_[path] = infoScript;
		listRes.push_back(path);
	}

	gstd::value res = script->CreateStringArrayValue(listRes);
	return res;
}
gstd::value StgControlScript::Func_GetScriptInfoA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;

	std::wstring path = argv[0].as_string();
	int type = (int)argv[1].as_real();

	ref_count_ptr<ScriptInformation> infoScript = NULL;
	if(script->mapScriptInfo_.find(path) != script->mapScriptInfo_.end())
		infoScript = script->mapScriptInfo_[path];
	else
	{
		infoScript = ScriptInformation::CreateScriptInformation(path, true);
		script->mapScriptInfo_[path] = infoScript;
	}
	if(infoScript == NULL)
		throw gstd::wexception(ErrorUtility::GetFileNotFoundErrorMessage(path));

	value res;
	switch(type)
	{
		case INFO_SCRIPT_TYPE:
			res = script->CreateRealValue(infoScript->GetType());
			break;
		case INFO_SCRIPT_PATH:
			res = script->CreateStringValue(infoScript->GetScriptPath());
			break;
		case INFO_SCRIPT_ID:
			res = script->CreateStringValue(infoScript->GetID());
			break;
		case INFO_SCRIPT_TITLE:
			res = script->CreateStringValue(infoScript->GetTitle());
			break;
		case INFO_SCRIPT_TEXT:
			res = script->CreateStringValue(infoScript->GetText());
			break;
		case INFO_SCRIPT_IMAGE:
			res = script->CreateStringValue(infoScript->GetImagePath());
			break;
		case INFO_SCRIPT_REPLAY_NAME:
			res = script->CreateStringValue(infoScript->GetReplayName());
			break;
	}

	return res;
}

//STG���ʊ֐��F�`��֘A
gstd::value StgControlScript::Func_ClearInvalidRenderPriority(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgSystemController* systemController = script->systemController_;
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();
	infoSystem->SetInvaridRenderPriority(-1, -1);

	return value();
}
gstd::value StgControlScript::Func_SetInvalidRenderPriorityA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgSystemController* systemController = script->systemController_;
	ref_count_ptr<StgSystemInformation> infoSystem = systemController->GetSystemInformation();

	int priMin = (int)argv[0].as_real();
	int priMax = (int)argv[1].as_real();
	infoSystem->SetInvaridRenderPriority(priMin, priMax);

	return value();
}

gstd::value StgControlScript::Func_GetReservedRenderTargetName(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;

	int index = (int)argv[0].as_real();
	ETextureManager* textureManager = ETextureManager::GetInstance();
	std::wstring name = textureManager->GetReservedRenderTargetName(index);

	return value(machine->get_engine()->get_string_type(), name);
}
gstd::value StgControlScript::Func_RenderToTextureA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring name = argv[0].as_string();
	int priMin = (int)argv[1].as_real();
	int priMax = (int)argv[2].as_real();
	bool bClear = argv[3].as_boolean();

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<Texture> texture = script->_GetTexture(name);
	if(texture == NULL)
		textureManager->GetTexture(name);
	if(texture == NULL && textureManager->IsDataExists(name))
	{
		texture = new Texture();
		texture->CreateRenderTarget(name);
	}

	graphics->SetRenderTarget(texture);
	graphics->BeginScene(bClear);

	systemController->RenderScriptObject(priMin, priMax);

	graphics->EndScene();
	graphics->SetRenderTarget(NULL);
/*
	{
		static int count = 0;
		std::string path = StringUtility::Format("R:/TEMP/temp_%04d.bmp", count);
		RECT rect = {0, 0, 640, 480};
		IDirect3DSurface9* pBackSurface = texture->GetD3DSurface();
		D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP, 
								pBackSurface, NULL, &rect );
		count++;
	}
*/
	return value();
}
gstd::value StgControlScript::Func_RenderToTextureB1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();

	std::wstring name = argv[0].as_string();
	int id = (int)argv[1].as_real();
	bool bClear = argv[2].as_boolean();

	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<Texture> texture = script->_GetTexture(name);
	if(texture == NULL)
		textureManager->GetTexture(name);

	if(texture == NULL && textureManager->IsDataExists(name))
	{
		texture = new Texture();
		texture->CreateRenderTarget(name);
	}

	graphics->SetRenderTarget(texture);
	graphics->BeginScene(bClear);

	obj->Render();

	graphics->EndScene();
	graphics->SetRenderTarget(NULL);

	return value();
}

gstd::value StgControlScript::Func_SaveSnapShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring path = argv[0].as_string();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<Texture> texture = textureManager->GetTexture(TextureManager::TARGET_TRANSITION);

	graphics->SetRenderTarget(texture);
	graphics->BeginScene(true);
	systemController->RenderScriptObject(0, 100);
	graphics->EndScene();
	graphics->SetRenderTarget(NULL);

	//�t�H���_����
	std::wstring dir = PathProperty::GetFileDirectory(path);
	File fDir(dir);
	fDir.CreateDirectory();

	//�ۑ�
	IDirect3DSurface9* pSurface = texture->GetD3DSurface();
	RECT rect = {0, 0, graphics->GetScreenWidth(), graphics->GetScreenHeight()};
	HRESULT hr = D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP, 
							pSurface, NULL, &rect );

	return value();
}
gstd::value StgControlScript::Func_SaveSnapShotA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ETextureManager* textureManager = ETextureManager::GetInstance();
	StgSystemController* systemController = script->systemController_;

	std::wstring path = argv[0].as_string();
	int rcLeft = (int)argv[1].as_real();
	int rcTop = (int)argv[2].as_real();
	int rcRight = (int)argv[3].as_real();
	int rcBottom = (int)argv[4].as_real();

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<Texture> texture = textureManager->GetTexture(TextureManager::TARGET_TRANSITION);

	graphics->SetRenderTarget(texture);
	graphics->BeginScene(true);
	systemController->RenderScriptObject(0, 100);
	graphics->EndScene();
	graphics->SetRenderTarget(NULL);

	//�t�H���_����
	std::wstring dir = PathProperty::GetFileDirectory(path);
	File fDir(dir);
	fDir.CreateDirectory();

	//�ۑ�
	IDirect3DSurface9* pSurface = texture->GetD3DSurface();
	RECT rect = {rcLeft, rcTop, rcRight, rcBottom};
	HRESULT hr = D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP, 
							pSurface, NULL, &rect );
	return value();
}

//STG���䋤�ʊ֐��F���@�֘A
gstd::value StgControlScript::Func_GetPlayerID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_string_type(), std::wstring());

	std::wstring id = stageController->GetStageInformation()->GetPlayerScriptInformation()->GetID();
	return value(machine->get_engine()->get_string_type(), id);
}
gstd::value StgControlScript::Func_GetPlayerReplayName(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	StgStageController* stageController = script->systemController_->GetStageController();
	if(stageController == NULL)
		return value(machine->get_engine()->get_string_type(), std::wstring());

	std::wstring replayName = stageController->GetStageInformation()->GetPlayerScriptInformation()->GetReplayName();
	return value(machine->get_engine()->get_string_type(), replayName);
}


//STG���䋤�ʊ֐��F���[�U�X�N���v�g
gstd::value StgControlScript::Func_SetPauseScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetPauseScriptPath(path);

	return value();
}
gstd::value StgControlScript::Func_SetEndSceneScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetEndSceneScriptPath(path);

	return value();
}
gstd::value StgControlScript::Func_SetReplaySaveSceneScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> info = script->systemController_->GetSystemInformation();

	std::wstring path = argv[0].as_string();
	info->SetReplaySaveSceneScriptPath(path);

	return value();
}

//STG���䋤�ʊ֐��F���@�X�N���v�g
gstd::value StgControlScript::Func_GetLoadFreePlayerScriptList(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();

	infoControlScript->LoadFreePlayerList();
	std::vector<ref_count_ptr<ScriptInformation> > listFreePlayer = infoControlScript->GetFreePlayerList();
	int res = listFreePlayer.size();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgControlScript::Func_GetFreePlayerScriptCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	std::vector<ref_count_ptr<ScriptInformation> > listFreePlayer = infoControlScript->GetFreePlayerList();

	int res = listFreePlayer.size();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgControlScript::Func_GetFreePlayerScriptInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	std::vector<ref_count_ptr<ScriptInformation> > listFreePlayer = infoControlScript->GetFreePlayerList();

	int index = (int)argv[0].as_real();
	int type = (int)argv[1].as_real();
	if(index < 0 || index >= listFreePlayer.size())
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	ref_count_ptr<ScriptInformation> infoPlayer = listFreePlayer[index];
	value res;
	switch(type)
	{
		case INFO_SCRIPT_PATH:
			res = script->CreateStringValue(infoPlayer->GetScriptPath());
			break;
		case INFO_SCRIPT_ID:
			res = script->CreateStringValue(infoPlayer->GetID());
			break;
		case INFO_SCRIPT_TITLE:
			res = script->CreateStringValue(infoPlayer->GetTitle());
			break;
		case INFO_SCRIPT_TEXT:
			res = script->CreateStringValue(infoPlayer->GetText());
			break;
		case INFO_SCRIPT_IMAGE:
			res = script->CreateStringValue(infoPlayer->GetImagePath());
			break;
		case INFO_SCRIPT_REPLAY_NAME:
			res = script->CreateStringValue(infoPlayer->GetReplayName());
			break;
	}

	return res;
}

//STG���䋤�ʊ֐��F���v���C�֘A
gstd::value StgControlScript::Func_LoadReplayList(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	std::wstring pathMainScript = infoSystem->GetMainScriptInformation()->GetScriptPath();
	infoControlScript->LoadReplayInformation(pathMainScript);
	return gstd::value();
}
gstd::value StgControlScript::Func_GetValidReplayIndices(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	std::vector<int> listValidIndices = replayInfoManager->GetIndexList();
	std::vector<long double> list;
	for(int iList = 0 ; iList < listValidIndices.size(); iList++)
		list.push_back(listValidIndices[iList]);
		
	gstd::value res = script->CreateRealArrayValue(list);
	return res;
}
gstd::value StgControlScript::Func_IsValidReplayIndex(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();

	int index = (int)argv[0].as_real();
	bool res = replayInfoManager->GetInformation(index) != NULL;
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgControlScript::Func_GetReplayInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	int index = (int)argv[0].as_real();
	int type = (int)argv[1].as_real();

	ref_count_ptr<ReplayInformation> replayInfo;
	if(index == ReplayInformation::INDEX_ACTIVE)replayInfo = infoSystem->GetActiveReplayInformation();
	else replayInfo = replayInfoManager->GetInformation(index);

	if(replayInfo == NULL)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	value res;
	switch(type)
	{
		case REPLAY_FILE_PATH:
			res = script->CreateStringValue(replayInfo->GetPath());
			break;
		case REPLAY_DATE_TIME:
			res = script->CreateStringValue(replayInfo->GetDateAsString());
			break;
		case REPLAY_USER_NAME:
			res = script->CreateStringValue(replayInfo->GetUserName());
			break;
		case REPLAY_TOTAL_SCORE:
			res = script->CreateRealValue(replayInfo->GetTotalScore());
			break;
		case REPLAY_FPS_AVERAGE:
			res = script->CreateRealValue(replayInfo->GetAvarageFps());
			break;
		case REPLAY_PLAYER_NAME:
			res = script->CreateStringValue(replayInfo->GetPlayerScriptReplayName());
			break;
		case REPLAY_STAGE_INDEX_LIST:
		{
			std::vector<int> listStageI = replayInfo->GetStageIndexList();
			std::vector<long double> listStageD;
			for(int iStage = 0 ; iStage < listStageI.size() ; iStage++)
			{
				long double stage = listStageI[iStage];
				listStageD.push_back(stage);
			}
			res = script->CreateRealArrayValue(listStageD);
			break;
		}
		case REPLAY_STAGE_START_SCORE_LIST:
		{
			std::vector<int> listStage = replayInfo->GetStageIndexList();
			std::vector<long double> listScoreD;
			for(int iStage = 0 ; iStage < listStage.size() ; iStage++)
			{
				int stage = listStage[iStage];
				ref_count_ptr<ReplayInformation::StageData> data = replayInfo->GetStageData(stage);

				long double score = data->GetStartScore();
				listScoreD.push_back(score);
			}
			res = script->CreateRealArrayValue(listScoreD);
			break;
		}
		case REPLAY_STAGE_LAST_SCORE_LIST:
		{
			std::vector<int> listStage = replayInfo->GetStageIndexList();
			std::vector<long double> listScoreD;
			for(int iStage = 0 ; iStage < listStage.size() ; iStage++)
			{
				int stage = listStage[iStage];
				ref_count_ptr<ReplayInformation::StageData> data = replayInfo->GetStageData(stage);

				long double score = data->GetLastScore();
				listScoreD.push_back(score);
			}
			res = script->CreateRealArrayValue(listScoreD);
			break;
		}
		case REPLAY_COMMENT:
			res = script->CreateStringValue(replayInfo->GetComment());
			break;
	}

	return res;
}
gstd::value StgControlScript::Func_SetReplayInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ReplayInformation> replayInfo = infoSystem->GetActiveReplayInformation();
	if(replayInfo == NULL)
		script->RaiseError(L"save target replay not found");

	int type = (int)argv[0].as_real();

	switch(type)
	{
		case REPLAY_COMMENT:
			replayInfo->SetComment(argv[1].as_string());
			break;
	}
	return value();
}
gstd::value StgControlScript::Func_GetReplayUserData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	int index = (int)argv[0].as_real();
	std::string key = to_mbcs(argv[1].as_string());

	ref_count_ptr<ReplayInformation> replayInfo;
	if(index == ReplayInformation::INDEX_ACTIVE)replayInfo = infoSystem->GetActiveReplayInformation();
	else replayInfo = replayInfoManager->GetInformation(index);

	if(replayInfo == NULL)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	gstd::value res = replayInfo->GetUserData(key);
	return res;
}
gstd::value StgControlScript::Func_SetReplayUserData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ReplayInformation> replayInfo = infoSystem->GetActiveReplayInformation();
	if(replayInfo == NULL)
		script->RaiseError(L"save target replay not found");

	std::string key = to_mbcs(argv[0].as_string());
	gstd::value val = argv[1];
	replayInfo->SetUserData(key, val);
	return value();
}
gstd::value StgControlScript::Func_IsReplayUserDataExists(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgControlScriptInformation> infoControlScript = script->systemController_->GetControlScriptInformation();
	ref_count_ptr<ReplayInformationManager> replayInfoManager = infoControlScript->GetReplayInformationManager();
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();

	int index = (int)argv[0].as_real();
	std::string key = to_mbcs(argv[1].as_string());

	ref_count_ptr<ReplayInformation> replayInfo;
	if(index == ReplayInformation::INDEX_ACTIVE)replayInfo = infoSystem->GetActiveReplayInformation();
	else replayInfo = replayInfoManager->GetInformation(index);

	if(replayInfo == NULL)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	bool res = replayInfo->IsUserDataExists(key);
	return value(machine->get_engine()->get_boolean_type(), res);
}

gstd::value StgControlScript::Func_SaveReplay(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgControlScript* script = (StgControlScript*)machine->data;
	ref_count_ptr<StgSystemInformation> infoSystem = script->systemController_->GetSystemInformation();
	ref_count_ptr<ScriptInformation> infoMain = script->systemController_->GetSystemInformation()->GetMainScriptInformation();

	ref_count_ptr<ReplayInformation> replayInfoActive = infoSystem->GetActiveReplayInformation();
	if(replayInfoActive == NULL)
		script->RaiseError(L"replay information not found");

	int index = (int)argv[0].as_real();
	if(index <= 0) 
		script->RaiseError(L"replay index error");

	std::wstring userName = argv[1].as_string();
	replayInfoActive->SetUserName(userName);

	replayInfoActive->SaveToFile(infoMain->GetScriptPath(), index);
	bool res = true;
	return value(machine->get_engine()->get_boolean_type(), res);
}


/**********************************************************
//ScriptInfoPanel
**********************************************************/
ScriptInfoPanel::ScriptInfoPanel()
{
}
bool ScriptInfoPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);
	buttonTerminateScript_.Create(hWnd_);
	buttonTerminateScript_.SetText(L"Terminate(�����I��)");

	LocateParts();
	return true;
}
void ScriptInfoPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	buttonTerminateScript_.SetBounds(wx + 16, wy + 16, 160, 32);
}
LRESULT ScriptInfoPanel::_WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			int id = wParam & 0xffff;
			if(id == buttonTerminateScript_.GetWindowId())
			{
				_TerminateScriptAll();
				return FALSE;
			}
		}
	}
	return _CallPreviousWindowProcedure(hWnd, uMsg, wParam, lParam);
}

void ScriptInfoPanel::_TerminateScriptAll()
{
	ETaskManager* taskManager = ETaskManager::GetInstance();
	std::list<ref_count_ptr<TaskBase> > listTask = taskManager->GetTaskList();
	std::list<ref_count_ptr<TaskBase> >::iterator itr = listTask.begin();
	for(; itr != listTask.end() ; itr++)
	{
		ref_count_ptr<StgSystemController> systemController = 
			ref_count_ptr<StgSystemController>::DownCast(*itr);
		if(systemController != NULL)
		{
			systemController->TerminateScriptAll();
		}
	}
}
