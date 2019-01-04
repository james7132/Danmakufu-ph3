#include"StgUserExtendScene.hpp"

#include"StgSystem.hpp"
#include"StgStageScript.hpp"

/**********************************************************
//StgUserExtendScene
**********************************************************/
StgUserExtendScene::StgUserExtendScene(StgSystemController* controller)
{
	systemController_ = controller;
}
StgUserExtendScene::~StgUserExtendScene()
{
}
void StgUserExtendScene::_InitializeTransitionTexture()
{
	//画面キャプチャ
	StgStageController* stageController = systemController_->GetStageController();
	if(stageController != NULL)
	{
		stageController->RenderToTransitionTexture();
	}

}
void StgUserExtendScene::_InitializeScript(std::wstring path, int type)
{
	if(scriptManager_ == NULL)return;
	_int64 idScript = scriptManager_->LoadScript(path, type);
	scriptManager_->StartScript(idScript);
}
void StgUserExtendScene::_CallScriptMainLoop()
{
	if(scriptManager_ == NULL)return;
	scriptManager_->Work();
}
void StgUserExtendScene::_CallScriptFinalize()
{
	if(scriptManager_ == NULL)return;
	scriptManager_->CallScriptFinalizeAll();
}
void StgUserExtendScene::_AddRelativeManager()
{
	if(scriptManager_ == NULL)return;
	ref_count_ptr<ScriptManager> scriptManager = scriptManager_;

	StgStageController* stageController = systemController_->GetStageController();
	if(stageController != NULL)
	{
		ref_count_ptr<ScriptManager> stageScriptManager = stageController->GetScriptManagerR();
		if(stageScriptManager != NULL)
			ScriptManager::AddRelativeScriptManagerMutual(scriptManager, stageScriptManager);
	}

	StgPackageController* packageController = systemController_->GetPackageController();
	if(packageController != NULL)
	{
		ref_count_ptr<ScriptManager> packageScriptManager = packageController->GetScriptManager();
		if(packageScriptManager != NULL)
			ScriptManager::AddRelativeScriptManagerMutual(scriptManager, packageScriptManager);
	}
}

void StgUserExtendScene::Work()
{
}
void StgUserExtendScene::Render()
{
	if(scriptManager_ == NULL)return;
	scriptManager_->Render();
}

void StgUserExtendScene::Start()
{
}
void StgUserExtendScene::Finish()
{
}

/**********************************************************
//StgUserExtendSceneScriptManager
**********************************************************/
StgUserExtendSceneScriptManager::StgUserExtendSceneScriptManager(StgSystemController* controller)
{
	systemController_ = controller;
	objectManager_ = new DxScriptObjectManager();
}
StgUserExtendSceneScriptManager::~StgUserExtendSceneScriptManager()
{
}
void StgUserExtendSceneScriptManager::Work()
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
void StgUserExtendSceneScriptManager::Render()
{
	objectManager_->RenderObject();
}
ref_count_ptr<ManagedScript> StgUserExtendSceneScriptManager::Create(int type)
{
	ref_count_ptr<ManagedScript> res;
	switch(type)
	{
		case StgUserExtendSceneScript::TYPE_PAUSE_SCENE:
			res = new StgPauseSceneScript(systemController_);
			break;
		case StgUserExtendSceneScript::TYPE_END_SCENE:
			res = new StgEndSceneScript(systemController_);
			break;
		case StgUserExtendSceneScript::TYPE_REPLAY_SCENE:
			res = new StgReplaySaveScript(systemController_);
			break;
	}

	if(res != NULL)
	{
		res->SetObjectManager(objectManager_);
		res->SetScriptManager(this);
	}

	return res;
}
void StgUserExtendSceneScriptManager::CallScriptFinalizeAll()
{
	std::list<ref_count_ptr<ManagedScript> >::iterator itr = listScriptRun_.begin();
	for(; itr != listScriptRun_.end(); itr++)
	{
		ref_count_ptr<ManagedScript> script = (*itr);
		if(script->IsEventExists("Finalize"))
			script->Run("Finalize");
	}
}
gstd::value StgUserExtendSceneScriptManager::GetResultValue()
{
	gstd::value res;
	std::list<ref_count_ptr<ManagedScript> >::iterator itr = listScriptRun_.begin();
	for(; itr != listScriptRun_.end(); itr++)
	{
		ref_count_ptr<ManagedScript> script = (*itr);
		gstd::value v = script->GetResultValue();
		if(v.has_data())
		{
			res = v;
			break;
		}
	}
	return res;
}
bool StgUserExtendSceneScriptManager::IsRealValue(gstd::value val)
{
	if(listScriptRun_.size() == 0)return false;
	ref_count_ptr<ManagedScript> script = *listScriptRun_.begin();

	bool res = script->IsRealValue(val);
	return res;
}

/**********************************************************
//StgUserExtendSceneScript
**********************************************************/
StgUserExtendSceneScript::StgUserExtendSceneScript(StgSystemController* systemController) : StgControlScript(systemController)
{
	StgStageController* stageController = systemController_->GetStageController();

	StgStageScriptManager* scriptManager = stageController->GetScriptManagerP();

	mainThreadID_ = scriptManager->GetMainThreadID();
}
StgUserExtendSceneScript::~StgUserExtendSceneScript()
{
}


/**********************************************************
//StgPauseScene
**********************************************************/
StgPauseScene::StgPauseScene(StgSystemController* controller) : StgUserExtendScene(controller)
{
}
StgPauseScene::~StgPauseScene()
{
}
void StgPauseScene::Work()
{
	if(scriptManager_ == NULL)return;
	_CallScriptMainLoop();

	ref_count_ptr<StgSystemInformation> infoSystem = systemController_->GetSystemInformation();
	ref_count_ptr<StgStageInformation> infoStage = systemController_->GetStageController()->GetStageInformation();
	gstd::value resValue = scriptManager_->GetResultValue();
	if(scriptManager_->IsRealValue(resValue))
	{
		int result = (int)resValue.as_real();
		if(result == StgControlScript::RESULT_CANCEL)
		{
		}
		else if(result == StgControlScript::RESULT_END)
		{
			if(infoSystem->IsPackageMode())
			{
				infoStage->SetResult(StgStageInformation::RESULT_BREAK_OFF);
				infoStage->SetEnd();
				//infoSystem->SetScene(StgSystemInformation::SCENE_PACKAGE_CONTROL);
			}
			else
			{
				infoSystem->SetStgEnd();
			}
		}
		else if(result == StgControlScript::RESULT_RETRY)
		{
			if(!infoStage->IsReplay())
				infoSystem->SetRetry();
		}
		EDirectInput::GetInstance()->ResetInputState();
		Finish();
	}
}

void StgPauseScene::Start()
{
	//停止イベント呼び出し
	StgStageController* stageController = systemController_->GetStageController();
	StgStageScriptManager* stageScriptManager = stageController->GetScriptManagerP();
	stageScriptManager->RequestEventAll(StgStageScript::EV_PAUSE_ENTER);

	//停止処理初期化
	scriptManager_ = NULL;
	scriptManager_ = new StgUserExtendSceneScriptManager(systemController_);
	_AddRelativeManager();
	ref_count_ptr<StgSystemInformation> sysInfo = systemController_->GetSystemInformation();

	_InitializeTransitionTexture();

	//スクリプト初期化
	std::wstring path = sysInfo->GetPauseScriptPath();
	_InitializeScript(path, StgUserExtendSceneScript::TYPE_PAUSE_SCENE);

	if(stageController != NULL)
		stageController->GetStageInformation()->SetPause(true);
}
void StgPauseScene::Finish()
{
	StgStageController* stageController = systemController_->GetStageController();
	if(stageController != NULL)
		stageController->GetStageInformation()->SetPause(false);

	if(scriptManager_ == NULL)return;
	_CallScriptFinalize();
	scriptManager_ = NULL;

	//解除イベント呼び出し
	StgStageScriptManager* stageScriptManager = stageController->GetScriptManagerP();
	stageScriptManager->RequestEventAll(StgStageScript::EV_PAUSE_LEAVE);
}

/**********************************************************
//StgPauseSceneScript
**********************************************************/
const function stgPauseFunction[] =  
{
	//関数：

	//定数：
	{"__stgPauseFunction__", constant<0>::func, 0},

};
StgPauseSceneScript::StgPauseSceneScript(StgSystemController* controller) : StgUserExtendSceneScript(controller)
{
	typeScript_ = TYPE_PAUSE_SCENE;
	_AddFunction(stgPauseFunction, sizeof(stgPauseFunction) / sizeof(function));
}
StgPauseSceneScript::~StgPauseSceneScript()
{
}

//一時停止専用関数：一時停止操作


/**********************************************************
//StgEndScene
**********************************************************/
StgEndScene::StgEndScene(StgSystemController* controller) : StgUserExtendScene(controller)
{

}
StgEndScene::~StgEndScene()
{
}
void StgEndScene::Work()
{
	if(scriptManager_ == NULL)return;
	_CallScriptMainLoop();

	ref_count_ptr<StgSystemInformation> infoSystem = systemController_->GetSystemInformation();
	gstd::value resValue = scriptManager_->GetResultValue();
	if(scriptManager_->IsRealValue(resValue))
	{
		int result = (int)resValue.as_real();
		if(result == StgControlScript::RESULT_SAVE_REPLAY)
		{
			//info->SetStgEnd();
			systemController_->TransReplaySaveScene();
		}
		else if(result == StgControlScript::RESULT_END)
		{
			infoSystem->SetStgEnd();
		}
		else if(result == StgControlScript::RESULT_RETRY)
		{
			infoSystem->SetRetry();
		}
		EDirectInput::GetInstance()->ResetInputState();
		Finish();
	}

}

void StgEndScene::Start()
{
	scriptManager_ = NULL;
	scriptManager_ = new StgUserExtendSceneScriptManager(systemController_);
	_AddRelativeManager();

	ref_count_ptr<StgSystemInformation> info = systemController_->GetSystemInformation();

	_InitializeTransitionTexture();

	//スクリプト初期化
	std::wstring path = info->GetEndSceneScriptPath();
	_InitializeScript(path, StgUserExtendSceneScript::TYPE_END_SCENE);
}
void StgEndScene::Finish()
{
	ref_count_ptr<StgSystemInformation> info = systemController_->GetSystemInformation();

	if(scriptManager_ == NULL)return;
	_CallScriptFinalize();
	scriptManager_ = NULL;
}

/**********************************************************
//StgEndSceneScript
**********************************************************/
const function stgEndFunction[] =  
{
	//関数：

	//定数：
	{"__stgEndFunction__", constant<0>::func, 0},

};
StgEndSceneScript::StgEndSceneScript(StgSystemController* controller) : StgUserExtendSceneScript(controller)
{
	typeScript_ = TYPE_END_SCENE;
	_AddFunction(stgEndFunction, sizeof(stgEndFunction) / sizeof(function));
}
StgEndSceneScript::~StgEndSceneScript()
{
}

/**********************************************************
//StgReplaySaveScene
**********************************************************/
StgReplaySaveScene::StgReplaySaveScene(StgSystemController* controller) : StgUserExtendScene(controller)
{

}
StgReplaySaveScene::~StgReplaySaveScene()
{
}
void StgReplaySaveScene::Work()
{
	if(scriptManager_ == NULL)return;
	_CallScriptMainLoop();

	ref_count_ptr<StgSystemInformation> infoSystem = systemController_->GetSystemInformation();
	gstd::value resValue = scriptManager_->GetResultValue();
	if(scriptManager_->IsRealValue(resValue))
	{
		int result = (int)resValue.as_real();
		if(result == StgControlScript::RESULT_END)
		{
			infoSystem->SetStgEnd();
		}
		else if(result == StgControlScript::RESULT_CANCEL)
		{
			systemController_->TransStgEndScene();
		}

		EDirectInput::GetInstance()->ResetInputState();
		Finish();
	}
}

void StgReplaySaveScene::Start()
{
	scriptManager_ = NULL;
	scriptManager_ = new StgUserExtendSceneScriptManager(systemController_);
	_AddRelativeManager();

	ref_count_ptr<StgSystemInformation> info = systemController_->GetSystemInformation();

	//_InitializeTransitionTexture();

	//スクリプト初期化
	std::wstring path = info->GetReplaySaveSceneScriptPath();
	_InitializeScript(path, StgUserExtendSceneScript::TYPE_REPLAY_SCENE);
}
void StgReplaySaveScene::Finish()
{
	ref_count_ptr<StgSystemInformation> info = systemController_->GetSystemInformation();

	if(scriptManager_ == NULL)return;
	_CallScriptFinalize();
	scriptManager_ = NULL;
}

/**********************************************************
//StgReplaySaveScript
**********************************************************/
const function stgReplaySaveFunction[] =  
{
	//関数：

	//定数：
	{"__stgReplaySaveFunction__", constant<0>::func, 0},

};
StgReplaySaveScript::StgReplaySaveScript(StgSystemController* controller) : StgUserExtendSceneScript(controller)
{
	typeScript_ = TYPE_REPLAY_SCENE;
	_AddFunction(stgReplaySaveFunction, sizeof(stgReplaySaveFunction) / sizeof(function));
}
StgReplaySaveScript::~StgReplaySaveScript()
{
}
