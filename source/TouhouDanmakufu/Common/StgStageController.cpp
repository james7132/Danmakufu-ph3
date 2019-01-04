#include"StgStageController.hpp"
#include"StgSystem.hpp"

/**********************************************************
//StgStageController
**********************************************************/
StgStageController::StgStageController(StgSystemController* systemController)
{
	systemController_ = systemController;
	infoSystem_ = systemController_->GetSystemInformation();
}
StgStageController::~StgStageController()
{
	intersectionManager_ = NULL;
	itemManager_ = NULL;
	shotManager_ = NULL;
	enemyManager_ = NULL;
	objectManagerMain_ = NULL;
	scriptManager_ = NULL;
}
void StgStageController::Initialize(ref_count_ptr<StgStageStartData> startData)
{
	//FPU初期化
	Math::InitializeFPU();

	//キー初期化
	EDirectInput* input = EDirectInput::GetInstance();
	input->ClearKeyState();

	//3Dカメラ
	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<DxCamera> camera3D = graphics->GetCamera();
	camera3D->Reset();
	camera3D->SetProjectionMatrix(384, 448, 10, 2000);

	//2Dカメラ
	gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();
	camera2D->Reset();

	ref_count_ptr<StgStageInformation> infoStage = startData->GetStageInformation();
	ref_count_ptr<ReplayInformation::StageData> replayStageData = startData->GetStageReplayData();
	ref_count_ptr<StgStageInformation> prevStageData = startData->GetPrevStageInformation();
	ref_count_ptr<StgPlayerInformation> prevPlayerInfo = startData->GetPrevPlayerInformation();

	infoStage_ = infoStage;
	infoStage_->SetReplay(replayStageData != NULL);

	//リプレイキー設定
	int replayState = infoStage_->IsReplay() ? KeyReplayManager::STATE_REPLAY : KeyReplayManager::STATE_RECORD;
	keyReplayManager_ = new KeyReplayManager(EDirectInput::GetInstance());
	keyReplayManager_->SetManageState(replayState);
	keyReplayManager_->AddTarget(EDirectInput::KEY_LEFT);
	keyReplayManager_->AddTarget(EDirectInput::KEY_RIGHT);
	keyReplayManager_->AddTarget(EDirectInput::KEY_UP);
	keyReplayManager_->AddTarget(EDirectInput::KEY_DOWN);
	keyReplayManager_->AddTarget(EDirectInput::KEY_SHOT);
	keyReplayManager_->AddTarget(EDirectInput::KEY_BOMB);
	keyReplayManager_->AddTarget(EDirectInput::KEY_SLOWMOVE);
	keyReplayManager_->AddTarget(EDirectInput::KEY_USER1);
	keyReplayManager_->AddTarget(EDirectInput::KEY_USER2);
	keyReplayManager_->AddTarget(EDirectInput::KEY_OK);
	keyReplayManager_->AddTarget(EDirectInput::KEY_CANCEL);
	std::set<int> listReplayTargetKey = infoSystem_->GetReplayTargetKeyList();
	std::set<int>::iterator itrKey = listReplayTargetKey.begin();
	for(; itrKey != listReplayTargetKey.end() ; itrKey++)
	{
		int id = *itrKey;
		keyReplayManager_->AddTarget(id);
	}

	if(replayStageData == NULL)
		replayStageData = new ReplayInformation::StageData();
	infoStage_->SetReplayData(replayStageData);

	//ステージ要素
	infoSlow_ = new PseudoSlowInformation();
	ref_count_weak_ptr<PseudoSlowInformation> wPtr = infoSlow_;
	EFpsController::GetInstance()->AddFpsControlObject(wPtr);

	//前ステージ情報反映
	if(prevStageData != NULL)
	{
		infoStage_->SetScore(prevStageData->GetScore());
		infoStage_->SetGraze(prevStageData->GetGraze());
		infoStage_->SetPoint(prevStageData->GetPoint());
	}


	//リプレイ関連(スクリプト初期化前)
	if(!infoStage_->IsReplay())
	{
		//乱数
		int randSeed = infoStage_->GetMersenneTwister()->GetSeed();
		replayStageData->SetRandSeed(randSeed);

		//ステージ情報
		ref_count_ptr<ScriptInformation> infoParent = systemController_->GetSystemInformation()->GetMainScriptInformation();
		ref_count_ptr<ScriptInformation> infoMain = infoStage_->GetMainScriptInformation();
		std::wstring pathParentScript = infoParent->GetScriptPath();
		std::wstring pathMainScript = infoMain->GetScriptPath();
		std::wstring filenameMainScript = PathProperty::GetFileName(pathMainScript);
		std::wstring pathMainScriptRelative = PathProperty::GetRelativeDirectory(pathParentScript, pathMainScript);

		replayStageData->SetMainScriptID(infoMain->GetID());
		replayStageData->SetMainScriptName(filenameMainScript);
		replayStageData->SetMainScriptRelativePath(pathMainScriptRelative);
		replayStageData->SetStartScore(infoStage_->GetScore());
		replayStageData->SetGraze(infoStage_->GetGraze());
		replayStageData->SetPoint(infoStage_->GetPoint());
	}
	else
	{
		//乱数
		int randSeed = replayStageData->GetRandSeed();
		infoStage_->GetMersenneTwister()->Initialize(randSeed);

		//リプレイキー
		keyReplayManager_->ReadRecord(*replayStageData->GetReplayKeyRecord());

		//ステージ情報
		infoStage_->SetScore(replayStageData->GetStartScore());
		infoStage_->SetGraze(replayStageData->GetGraze());
		infoStage_->SetPoint(replayStageData->GetPoint());

		//自機設定
		prevPlayerInfo = new StgPlayerInformation();
		prevPlayerInfo->SetLife(replayStageData->GetPlayerLife());
		prevPlayerInfo->SetSpell(replayStageData->GetPlayerBombCount());
		prevPlayerInfo->SetPower(replayStageData->GetPlayerPower());
		prevPlayerInfo->SetRebirthFrame(replayStageData->GetPlayerRebirthFrame());
	}

	objectManagerMain_ = new StgStageScriptObjectManager(this);
	scriptManager_ = new StgStageScriptManager(this);
	enemyManager_ = new StgEnemyManager(this);
	shotManager_ = new StgShotManager(this);
	itemManager_ = new StgItemManager(this);
	intersectionManager_ = new StgIntersectionManager();
	pauseManager_ = new StgPauseScene(systemController_);

	//パッケージスクリプトの場合は、ステージスクリプトと関連付ける
	StgPackageController* packageController = systemController_->GetPackageController();
	if(packageController != NULL)
	{
		ref_count_ptr<ScriptManager> packageScriptManager = packageController->GetScriptManager();
		ref_count_ptr<ScriptManager> stageScriptManager = scriptManager_;
		ScriptManager::AddRelativeScriptManagerMutual(packageScriptManager, stageScriptManager);
	}

	gstd::ref_count_ptr<StgStageScriptObjectManager> objectManager = scriptManager_->GetObjectManager();

	//メインスクリプト情報
	ref_count_ptr<ScriptInformation> infoMain = infoStage_->GetMainScriptInformation();
	std::wstring dirInfo = PathProperty::GetFileDirectory(infoMain->GetScriptPath());

	ELogger::WriteTop(StringUtility::Format(L"メインスクリプト[%s]", infoMain->GetScriptPath().c_str()));

	//システムスクリプト
	std::wstring pathSystemScript = infoMain->GetSystemPath();
	if(pathSystemScript == ScriptInformation::DEFAULT)
		pathSystemScript = EPathProperty::GetStgDefaultScriptDirectory() + L"Default_System.txt";
	if(pathSystemScript.size() > 0)
	{
		pathSystemScript = EPathProperty::ExtendRelativeToFull(dirInfo, pathSystemScript);
		ELogger::WriteTop(StringUtility::Format(L"システムスクリプト[%s]", pathSystemScript.c_str()));
		_int64 idScript = scriptManager_->LoadScript(pathSystemScript, StgStageScript::TYPE_SYSTEM);
		scriptManager_->StartScript(idScript);
	}

	//自機スクリプト
	ref_count_ptr<StgPlayerObject>::unsync objPlayer = NULL;
	ref_count_ptr<ScriptInformation> infoPlayer = infoStage_->GetPlayerScriptInformation();
	std::wstring pathPlayerScript = infoPlayer->GetScriptPath();

	if(pathPlayerScript.size() > 0)
	{
		ELogger::WriteTop(StringUtility::Format(L"自機スクリプト[%s]", pathPlayerScript.c_str()));
		int idPlayer = objectManager->CreatePlayerObject();
		objPlayer = ref_count_ptr<StgPlayerObject>::unsync::DownCast(GetMainRenderObject(idPlayer));

		_int64 idScript = scriptManager_->LoadScript(pathPlayerScript, StgStageScript::TYPE_PLAYER);
		_SetupReplayTargetCommonDataArea(idScript);

		ref_count_ptr<StgStagePlayerScript> script = 
			ref_count_ptr<StgStagePlayerScript>::DownCast(scriptManager_->GetScript(idScript));
		objPlayer->SetScript(script.GetPointer());
		scriptManager_->SetPlayerScriptID(idScript);
		scriptManager_->StartScript(idScript);

		//前ステージ情報反映
		if(prevPlayerInfo != NULL)
			objPlayer->SetPlayerInforamtion(prevPlayerInfo);
	}
	if(objPlayer != NULL)
		infoStage_->SetPlayerObjectInformation(objPlayer->GetPlayerInformation());

	//メインスクリプト
	if(infoMain->GetType() == ScriptInformation::TYPE_SINGLE)
	{
		std::wstring pathMainScript = EPathProperty::GetSystemResourceDirectory() + L"script/System_SingleStage.txt";
		_int64 idScript = scriptManager_->LoadScript(pathMainScript, StgStageScript::TYPE_STAGE);
		scriptManager_->StartScript(idScript);
	}
	else if(infoMain->GetType() == ScriptInformation::TYPE_PLURAL)
	{
		std::wstring pathMainScript = EPathProperty::GetSystemResourceDirectory() + L"script/System_PluralStage.txt";
		_int64 idScript = scriptManager_->LoadScript(pathMainScript, StgStageScript::TYPE_STAGE);
		scriptManager_->StartScript(idScript);
	}
	else
	{
		std::wstring pathMainScript = infoMain->GetScriptPath();
		if(pathMainScript.size() > 0)
		{
			_int64 idScript = scriptManager_->LoadScript(pathMainScript, StgStageScript::TYPE_STAGE);
			_SetupReplayTargetCommonDataArea(idScript);
			scriptManager_->StartScript(idScript);
		}
	}

	//背景スクリプト
	std::wstring pathBack = infoMain->GetBackgroundPath();
	if(pathBack == ScriptInformation::DEFAULT)
		pathBack = L"";
	if(pathBack.size() > 0)
	{
		pathBack = EPathProperty::ExtendRelativeToFull(dirInfo, pathBack);
		ELogger::WriteTop(StringUtility::Format(L"背景スクリプト[%s]", pathBack.c_str()));
		_int64 idScript = scriptManager_->LoadScript(pathBack, StgStageScript::TYPE_STAGE);
		scriptManager_->StartScript(idScript);
	}

	//音声再生
	std::wstring pathBGM = infoMain->GetBgmPath();
	if(pathBGM == ScriptInformation::DEFAULT)
		pathBGM = L"";
	if(pathBGM.size() > 0)
	{
		pathBGM = EPathProperty::ExtendRelativeToFull(dirInfo, pathBGM);
		ELogger::WriteTop(StringUtility::Format(L"BGM[%s]", pathBGM.c_str()));
		ref_count_ptr<SoundPlayer> player = DirectSoundManager::GetBase()->GetPlayer(pathBGM);
		if(player != NULL)
		{
			player->SetSoundDivision(SoundDivision::DIVISION_BGM);
			SoundPlayer::PlayStyle style;
			style.SetLoopEnable(true);
			player->Play(style);
		}
	}

	//リプレイ関連(スクリプト初期化後)
	if(!infoStage_->IsReplay())
	{
		//自機情報
		ref_count_ptr<StgPlayerObject>::unsync objPlayer = GetPlayerObject();
		if(objPlayer != NULL)
		{
			replayStageData->SetPlayerLife(objPlayer->GetLife());
			replayStageData->SetPlayerBombCount(objPlayer->GetSpell());
			replayStageData->SetPlayerPower(objPlayer->GetPower());
			replayStageData->SetPlayerRebirthFrame(objPlayer->GetRebirthFrame());
		}
		std::wstring pathPlayerScript = infoPlayer->GetScriptPath();
		std::wstring filenamePlayerScript = PathProperty::GetFileName(pathPlayerScript);
		replayStageData->SetPlayerScriptFileName(filenamePlayerScript);
		replayStageData->SetPlayerScriptID(infoPlayer->GetID());
		replayStageData->SetPlayerScriptReplayName(infoPlayer->GetReplayName());
	}

	infoStage_->SetStageStartTime(timeGetTime());


}
void StgStageController::CloseScene()
{
	ref_count_weak_ptr<PseudoSlowInformation> wPtr = infoSlow_;
	EFpsController::GetInstance()->RemoveFpsControlObject(wPtr);

	//リプレイ
	if(!infoStage_->IsReplay())
	{
		//キー
		ref_count_ptr<RecordBuffer> recKey = new RecordBuffer();
		keyReplayManager_->WriteRecord(*recKey.GetPointer());

		ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage_->GetReplayData();
		replayStageData->SetReplayKeyRecord(recKey);

		//最終フレーム
		int stageFrame = infoStage_->GetCurrentFrame();
		replayStageData->SetEndFrame(stageFrame);

		replayStageData->SetLastScore(infoStage_->GetScore());
	}
}
void StgStageController::_SetupReplayTargetCommonDataArea(_int64 idScript)
{
	ref_count_ptr<StgStageScript> script = 
		ref_count_ptr<StgStageScript>::DownCast(scriptManager_->GetScript(idScript));
	if(script == NULL)return;

	gstd::value res = script->RequestEvent(StgStageScript::EV_REQUEST_REPLAY_TARGET_COMMON_AREA);
	if(!res.has_data())return;
	type_data::type_kind kindRes = res.get_type()->get_kind();
	if(kindRes != type_data::tk_array)return;

	ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage_->GetReplayData();
	std::set<std::string> listArea;
	int arrayLength = res.length_as_array();
	for(int iArray = 0 ; iArray < arrayLength ; iArray++)
	{
		value& arrayValue = res.index_as_array(iArray);
		std::string area = to_mbcs(arrayValue.as_string());
		listArea.insert(area);
	}

	gstd::ref_count_ptr<ScriptCommonDataManager> scriptCommonDataManager = systemController_->GetCommonDataManager();
	if(!infoStage_->IsReplay())
	{
		std::set<std::string>::iterator itrArea = listArea.begin();
		for(; itrArea != listArea.end();itrArea++)
		{
			std::string area = (*itrArea);
			ref_count_ptr<ScriptCommonData> commonData = scriptCommonDataManager->GetData(area);
			replayStageData->SetCommonData(area, commonData);
		}
	}
	else
	{
		std::set<std::string>::iterator itrArea = listArea.begin();
		for(; itrArea != listArea.end();itrArea++)
		{
			std::string area = (*itrArea);
			ref_count_ptr<ScriptCommonData> commonData = replayStageData->GetCommonData(area);
			scriptCommonDataManager->SetData(area, commonData);
		}
	}
}

void StgStageController::Work()
{
	EDirectInput* input = EDirectInput::GetInstance();
	ref_count_ptr<StgSystemInformation> infoSystem = systemController_->GetSystemInformation();
	bool bPackageMode = infoSystem->IsPackageMode();

	bool bPermitRetryKey = !input->IsTargetKeyCode(DIK_BACK);
	if(!bPackageMode && bPermitRetryKey && input->GetKeyState(DIK_BACK) == KEY_PUSH)
	{
		//リトライ
		if(!infoStage_->IsReplay())
		{
			ref_count_ptr<StgSystemInformation> infoSystem = systemController_->GetSystemInformation();
			infoSystem->SetRetry();
			return;
		}
	}

	bool bCurrentPause = infoStage_->IsPause();
	if(bPackageMode && bCurrentPause)
	{
		//パッケージモードで停止中の場合は、パッケージスクリプトで処理する
		return;
	}

	bool bPauseKey = (input->GetVirtualKeyState(EDirectInput::KEY_PAUSE) == KEY_PUSH);
	if(bPauseKey && !bPackageMode)
	{
		//停止キー押下
		if(!bCurrentPause)
			pauseManager_->Start();
		else
			pauseManager_->Finish();
	}
	else
	{
		if(!bCurrentPause)
		{
			//リプレイキー更新
			keyReplayManager_->Update();

			//スクリプト処理で、自機、敵、弾の動作が行われる。
			scriptManager_->Work(StgStageScript::TYPE_SYSTEM);
			scriptManager_->Work(StgStageScript::TYPE_STAGE);
			scriptManager_->Work(StgStageScript::TYPE_SHOT);
			scriptManager_->Work(StgStageScript::TYPE_ITEM);

			ref_count_ptr<StgPlayerObject>::unsync objPlayer = GetPlayerObject();
			if(objPlayer != NULL)objPlayer->Move(); //自機だけ先に移動
			scriptManager_->Work(StgStageScript::TYPE_PLAYER);

			//オブジェクト動作処理
			if(infoStage_->IsEnd())return;
			objectManagerMain_->WorkObject();

			enemyManager_->Work();
			shotManager_->Work();
			itemManager_->Work();

			//当たり判定処理
			enemyManager_->RegistIntersectionTarget();
			shotManager_->RegistIntersectionTarget();
			intersectionManager_->Work();

			if(!infoStage_->IsReplay())
			{
				//リプレイ用情報更新
				int stageFrame = infoStage_->GetCurrentFrame();
				if(stageFrame % 60 == 0)
				{
					ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage_->GetReplayData();
					float framePerSecond = EFpsController::GetInstance()->GetCurrentFps();
					replayStageData->AddFramePerSecond(framePerSecond);
				}
			}

			infoStage_->AdvanceFrame();
			
		}
		else
		{
			//停止中
			pauseManager_->Work();
		}
	}

	ELogger* logger = ELogger::GetInstance();
	if(logger->IsWindowVisible())
	{
		//ログ関連
		logger->SetInfo(6, L"stg shot_count", StringUtility::Format(L"%d", shotManager_->GetShotCountAll()));
		logger->SetInfo(7, L"stg enemy_count", StringUtility::Format(L"%d", enemyManager_->GetEnemyCount()));
		logger->SetInfo(8, L"stg item_count", StringUtility::Format(L"%d", itemManager_->GetItemCount()));
	}
}
void StgStageController::Render()
{
	bool bPause = infoStage_->IsPause();
	if(!bPause)
	{
		objectManagerMain_->RenderObject();

		if(infoStage_->IsReplay())
		{
			//リプレイ中
		}
	}
	else
	{
		//停止
		pauseManager_->Render();
	}
}
void StgStageController::RenderToTransitionTexture()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	TextureManager* textureManager = ETextureManager::GetInstance();
	ref_count_ptr<Texture> texture = textureManager->GetTexture(TextureManager::TARGET_TRANSITION);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = GetMainObjectManager();
	graphics->SetRenderTarget(texture);
	graphics->BeginScene(true);

	//objectManager->RenderObject();
	systemController_->RenderScriptObject();

	graphics->EndScene();
	graphics->SetRenderTarget(NULL);
}

ref_count_ptr<DxScriptObjectBase>::unsync StgStageController::GetMainRenderObject(int idObject)
{
	return objectManagerMain_->GetObject(idObject);
}
ref_count_ptr<StgPlayerObject>::unsync StgStageController::GetPlayerObject()
{
	int idPlayer = objectManagerMain_->GetPlayerObjectID();
	if(idPlayer == DxScript::ID_INVALID)return NULL;
	return ref_count_ptr<StgPlayerObject>::unsync::DownCast(GetMainRenderObject(idPlayer));
}

/**********************************************************
//StgStageInformation
**********************************************************/
StgStageInformation::StgStageInformation()
{
	bEndStg_ = false;
	bPause_ = false;
	bReplay_ = false;
	frame_ = 0;
	stageIndex_ = 0;

	SetRect(&rcStgFrame_, 32, 16, 32 + 384, 16 + 448);
	SetStgFrameRect(rcStgFrame_);
	priMinStgFrame_ = 20;
	priMaxStgFrame_ = 80;
	priShotObject_ = 50;
	priItemObject_ = 60;
	priCameraFocusPermit_ = 69;
	SetRect(&rcShotAutoDeleteClip_, -64, -64, 64, 64);

	rand_ = new MersenneTwister();
	rand_->Initialize(timeGetTime());
	score_ = 0;
	graze_ = 0;
	point_ = 0;
	result_ = RESULT_UNKNOWN;

	timeStart_ = 0;
}
StgStageInformation::~StgStageInformation()
{
}
void StgStageInformation::SetStgFrameRect(RECT rect, bool bUpdateFocusResetValue)
{
	rcStgFrame_ = rect;
	gstd::ref_count_ptr<D3DXVECTOR2> pos = new D3DXVECTOR2();
	pos->x = (rect.right - rect.left) / 2;
	pos->y = (rect.bottom - rect.top) / 2;

	if(bUpdateFocusResetValue)
	{
		DirectGraphics* graphics = DirectGraphics::GetBase();
		gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();
		camera2D->SetResetFocus(pos);
		camera2D->Reset();
	}
}

/**********************************************************
//PseudoSlowInformation
**********************************************************/
int PseudoSlowInformation::GetFps()
{
	int fps = STANDARD_FPS;
	int target = TARGET_ALL;
	if(mapDataPlayer_.find(target) != mapDataPlayer_.end())
	{
		ref_count_ptr<SlowData> data = mapDataPlayer_[target];
		fps = min(fps, data->GetFps());
	}
	if(mapDataEnemy_.find(target) != mapDataEnemy_.end())
	{
		ref_count_ptr<SlowData> data = mapDataEnemy_[target];
		fps = min(fps, data->GetFps());
	}
	return fps;
}
bool PseudoSlowInformation::IsValidFrame(int target)
{
	bool res = mapValid_.find(target) == mapValid_.end() ||
				mapValid_[target];
	return res;
}
void PseudoSlowInformation::Next()
{
	int fps = STANDARD_FPS;
	int target = TARGET_ALL;
	if(mapDataPlayer_.find(target) != mapDataPlayer_.end())
	{
		ref_count_ptr<SlowData> data = mapDataPlayer_[target];
		fps = min(fps, data->GetFps());
	}
	if(mapDataEnemy_.find(target) != mapDataEnemy_.end())
	{
		ref_count_ptr<SlowData> data = mapDataEnemy_[target];
		fps = min(fps, data->GetFps());
	}

	bool bValid = false;
	if(fps == STANDARD_FPS)
	{
		bValid = true;
	}
	else
	{
		current_ += fps;
		if(current_ >= STANDARD_FPS)
		{
			current_ %= STANDARD_FPS;
			bValid = true;
		}
	}

	mapValid_[target] = bValid;
}
void PseudoSlowInformation::AddSlow(int fps, int owner, int target)
{
	fps = max(1, fps);
	fps = min(STANDARD_FPS, fps);
	ref_count_ptr<SlowData> data = new SlowData();
	data->SetFps(fps);
	switch(owner)
	{
		case OWNER_PLAYER:
			mapDataPlayer_[target] = data;
			break;
		case OWNER_ENEMY:
			mapDataEnemy_[target] = data;
			break;
	}
}
void PseudoSlowInformation::RemoveSlow(int owner, int target)
{
	switch(owner)
	{
		case OWNER_PLAYER:
			mapDataPlayer_.erase(target);
			break;
		case OWNER_ENEMY:
			mapDataEnemy_.erase(target);
			break;
	}
}
