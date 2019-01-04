#include"StgSystem.hpp"

#include"StgPackageController.hpp"
#include"StgStageScript.hpp"
#include"StgPlayer.hpp"

/**********************************************************
//StgSystemController
**********************************************************/
StgSystemController::StgSystemController()
{
}
StgSystemController::~StgSystemController()
{

}
void StgSystemController::Initialize(ref_count_ptr<StgSystemInformation> infoSystem)
{
	infoSystem_ = infoSystem;
	commonDataManager_ = new ScriptCommonDataManager();
	scriptEngineCache_ = new ScriptEngineCache();
	infoControlScript_ = new StgControlScriptInformation();
}
void StgSystemController::Start(ref_count_ptr<ScriptInformation> infoPlayer, ref_count_ptr<ReplayInformation> infoReplay)
{
	//DirectX
	DirectGraphics* graphics = DirectGraphics::GetBase();
	gstd::ref_count_ptr<DxCamera> camera3D = graphics->GetCamera();
	gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();

	camera3D->SetProjectionMatrix(384, 448, 10, 2000);
	camera2D->Reset();

	//キャッシュをクリア
	scriptEngineCache_->Clear();

	//キー設定
	EDirectInput* input = EDirectInput::GetInstance();
	input->ResetVirtualKeyMap();

	ref_count_ptr<ScriptInformation> infoMain = infoSystem_->GetMainScriptInformation();

	//アーカイブ
	EFileManager* fileManager = EFileManager::GetInstance();
	std::wstring archiveMain = infoMain->GetArchivePath();
	if(archiveMain.size() > 0)
	{
		fileManager->AddArchiveFile(archiveMain);
	}

	if(infoPlayer != NULL)
	{
		std::wstring archivePlayer = infoPlayer->GetArchivePath();
		if(archivePlayer.size() > 0)
		{
			fileManager->AddArchiveFile(archivePlayer);
		}
	}

	if(infoSystem_->IsPackageMode())
	{
		infoSystem_->SetScene(StgSystemInformation::SCENE_PACKAGE_CONTROL);
		packageController_ = new StgPackageController(this);
		packageController_->Initialize();
	}
	else
	{
		ref_count_ptr<ReplayInformation::StageData> replayStageData = NULL;
		if(infoReplay != NULL)
			replayStageData = infoReplay->GetStageData(0);
		ref_count_ptr<StgStageInformation> infoStage = new StgStageInformation();
		infoStage->SetMainScriptInformation(infoMain);
		infoStage->SetPlayerScriptInformation(infoPlayer);
		StartStgScene(infoStage, replayStageData);
	}
}
void StgSystemController::Work()
{
	try
	{
		_ControlScene();

		ELogger* logger = ELogger::GetInstance();
		logger->UpdateCommonDataInfoPanel(commonDataManager_);
	}
	catch(gstd::wexception e)
	{
		Logger::WriteTop(e.what());
		infoSystem_->SetError(e.what());
	}

	EDirectInput* input = EDirectInput::GetInstance();
	if(infoSystem_->IsRetry())
	{
		infoSystem_->SetError(L"Retry");

		DirectSoundManager* soundManager = DirectSoundManager::GetBase();
		soundManager->Clear();

		ShaderManager* shaderManager = ShaderManager::GetBase();
		shaderManager->Clear();

		if(infoSystem_->IsPackageMode())
		{
			ref_count_ptr<StgStageStartData> oldStageStartData;
			ref_count_ptr<StgPackageInformation> infoPackage = packageController_->GetPackageInformation();
			std::vector<ref_count_ptr<StgStageStartData> > listStageData = infoPackage->GetStageDataList();
			if(listStageData.size() > 0)
			{
				oldStageStartData = *listStageData.begin();
			}
			else
			{
				oldStageStartData = infoPackage->GetNextStageData();
			}
			ref_count_ptr<StgStageInformation> oldStageInformation = oldStageStartData->GetStageInformation();

			ref_count_ptr<StgStageStartData> newStageStartData = new StgStageStartData();
			ref_count_ptr<StgStageInformation> newStageInformaiton = new StgStageInformation();
			newStageInformaiton->SetMainScriptInformation(oldStageInformation->GetMainScriptInformation());
			newStageInformaiton->SetPlayerScriptInformation(oldStageInformation->GetPlayerScriptInformation());
			newStageInformaiton->SetStageIndex(oldStageInformation->GetStageIndex());
			newStageStartData->SetStageInformation(newStageInformaiton);
			infoPackage->SetNextStageData(newStageStartData);
			infoSystem_->ResetRetry();

			StartStgScene(newStageStartData);
		}
		else
		{
			DoRetry();
		}
		return;
	}

	if(infoSystem_->IsError() || infoSystem_->IsStgEnd())
	{
		EFileManager* fileManager = EFileManager::GetInstance();
		fileManager->ResetArchiveFile();

		//終了
		bool bRetry = false;
		if(infoSystem_->IsError())
		{
			std::wstring error = infoSystem_->GetErrorMessage();
			if(error.size() > 0)
			{
				ErrorDialog::ShowErrorDialog(error);
			}
			else
			{
				//リトライ
				bRetry = true;
			}
		}

		if(!bRetry)
		{
			DirectSoundManager* soundManager = DirectSoundManager::GetBase();
			soundManager->Clear();
		}

		ELogger* logger = ELogger::GetInstance();
		logger->UpdateCommonDataInfoPanel(NULL);

		EFpsController* fpsController = EFpsController::GetInstance();
		fpsController->SetFastModeKey(DIK_LCONTROL);

		DoEnd();
		return;
	}
}
void StgSystemController::Render()
{
	if(infoSystem_->IsError())return;

	try
	{
		int scene = infoSystem_->GetScene();
		switch(scene)
		{
			case StgSystemInformation::SCENE_STG:
			case StgSystemInformation::SCENE_PACKAGE_CONTROL:	
			{
				RenderScriptObject();
				break;
			}
			case StgSystemInformation::SCENE_END:
			{
				if(endScene_ != NULL)
					endScene_->Render();
				break;
			}
			case StgSystemInformation::SCENE_REPLAY_SAVE:
			{
				if(replaySaveScene_ != NULL)
					replaySaveScene_->Render();
				break;
			}
		}
	}
	catch(gstd::wexception e)
	{
		DirectGraphics* graphics = DirectGraphics::GetBase();
		gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();
		camera2D->SetEnable(false);
		camera2D->Reset();
		Logger::WriteTop(e.what());
		infoSystem_->SetError(e.what());
	}
}
void StgSystemController::RenderScriptObject()
{
	int scene = infoSystem_->GetScene();
	bool bPackageMode = infoSystem_->IsPackageMode();
	bool bPause = false;
	if(scene == StgSystemInformation::SCENE_STG)
	{
		ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
		bPause = infoStage->IsPause();
	}

	if(bPause && !bPackageMode)
	{
		//停止
		stageController_->GetPauseManager()->Render();
	}
	else
	{
		bool bReplay = false;
		int countRender = 0;
		if(scene == StgSystemInformation::SCENE_STG && stageController_ != NULL)
		{
			ref_count_ptr<StgStageScriptObjectManager> objectManagerStage = stageController_->GetMainObjectManager();
			countRender = max(objectManagerStage->GetRenderBucketCapacity(), countRender);

			ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
			bReplay = infoStage->IsReplay();
		}

		if(infoSystem_->IsPackageMode())
		{
			ref_count_ptr<DxScriptObjectManager> objectManagerPackage = packageController_->GetMainObjectManager();
			countRender = max(objectManagerPackage->GetRenderBucketCapacity(), countRender);
		}

		int invalidPriMin = infoSystem_->GetInvaridRenderPriorityMin();
		int invalidPriMax = infoSystem_->GetInvaridRenderPriorityMax();
		if(invalidPriMin < 0 && invalidPriMax < 0) 
		{
			RenderScriptObject(0, countRender);
		}
		else
		{
			RenderScriptObject(0, invalidPriMin - 1);
			RenderScriptObject(invalidPriMax + 1, countRender);
		}


		if(bReplay)
		{
			//リプレイ中
/*
			ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
			ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
			DirectGraphics* graphics = DirectGraphics::GetBase();
			graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
			graphics->SetZBufferEnable(false);
			graphics->SetZWriteEnalbe(false);
			graphics->SetFogEnable(false);

			RECT rest = infoStage->GetStgFrameRect();
			int frame = infoStage->GetCurrentFrame();
			int fps = replayStageData->GetFramePerSecond(frame);
			std::string strFps = StringUtility::Format("%02d", fps);
			DxText dxText;
			dxText.SetFontColorTop(D3DCOLOR_ARGB(255,128,128,255));
			dxText.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
			dxText.SetFontBorderType(directx::DxFont::BORDER_FULL);
			dxText.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
			dxText.SetFontBorderWidth(1);
			dxText.SetFontSize(12);
			dxText.SetFontBold(true);
			dxText.SetText(strFps);
			dxText.SetPosition(rest.right - 18, rest.bottom - 14);
			dxText.Render();
*/
		}
	}

}
void StgSystemController::RenderScriptObject(int priMin, int priMax)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManagerStage = NULL;
	ref_count_ptr<DxScriptObjectManager> objectManagerPackage = NULL;
	std::vector<std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > > *pRenderListStage = NULL;
	std::vector<std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > > *pRenderListPackage = NULL;


	int scene = infoSystem_->GetScene();
	bool bPause = false;
	if(scene == StgSystemInformation::SCENE_STG)
	{
		ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
		bPause = infoStage->IsPause();
	}

	//以下の場合にステージ描画有効とする
	//・パッケージモードでない(一時停止もステージ処理側で処理するため)
	//・パッケージスクリプトの場合は、一時停止をパッケージスクリプトで処理するため
	//　一時停止中はSTGシーンは描画対象外とする
	bool bValidStage = (scene == StgSystemInformation::SCENE_STG || !infoSystem_->IsPackageMode()) && 
						stageController_ != NULL && !bPause;
	if(bValidStage)
	{
		objectManagerStage = stageController_->GetMainObjectManager();
		objectManagerStage->PrepareRenderObject();
		pRenderListStage = objectManagerStage->GetRenderObjectListPointer();
	}

	if(infoSystem_->IsPackageMode())
	{
		objectManagerPackage = packageController_->GetMainObjectManager();
		objectManagerPackage->PrepareRenderObject();
		pRenderListPackage = objectManagerPackage->GetRenderObjectListPointer();
	}



	//--------------------------------

	DirectGraphics* graphics = DirectGraphics::GetBase();
	gstd::ref_count_ptr<DxCamera> camera3D = graphics->GetCamera();
	gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();
	double focusRatioX = camera2D->GetRatioX();
	double focusRatioY = camera2D->GetRatioY();
	double focusAngleZ = camera2D->GetAngleZ();
	D3DXVECTOR2 orgFocusPos = camera2D->GetFocusPosition();
	D3DXVECTOR2 focusPos = orgFocusPos;

	ref_count_ptr<StgStageInformation> stageInfo = NULL;
	if(bValidStage)
	{
		stageInfo = stageController_->GetStageInformation();
		RECT rcStgFrame = stageInfo->GetStgFrameRect();

		//pause後に、フォーカスリセット値が上書きされていることがあるので
		//STGシーン用にリセット値を更新する
		gstd::ref_count_ptr<D3DXVECTOR2> pos = new D3DXVECTOR2();
		pos->x = (rcStgFrame.right - rcStgFrame.left) / 2;
		pos->y = (rcStgFrame.bottom - rcStgFrame.top) / 2;
		camera2D->SetResetFocus(pos);

		orgFocusPos = camera2D->GetFocusPosition();
		focusPos = orgFocusPos;
	}
	else
	{
		stageInfo = new StgStageInformation();

		RECT rect;
		ZeroMemory(&rect, sizeof(RECT));
		rect.right = graphics->GetScreenWidth();
		rect.bottom = graphics->GetScreenHeight();

		stageInfo->SetStgFrameRect(rect);
		if(scene != StgSystemInformation::SCENE_STG)
		{
			//STGシーンでないならカメラ座標をリセットしておく
			orgFocusPos = camera2D->GetFocusPosition();
			focusPos = orgFocusPos;
		}
	}

	RECT rcStgFrame = stageInfo->GetStgFrameRect();
	int stgWidth = rcStgFrame.right - rcStgFrame.left;
	int stgHeight = rcStgFrame.bottom - rcStgFrame.top;
	POINT stgCenter = {rcStgFrame.left + stgWidth / 2, rcStgFrame.top + stgHeight / 2};
	int priMinStgFrame = stageInfo->GetStgFrameMinPriority();
	int priMaxStgFrame = stageInfo->GetStgFrameMaxPriority();
	int priShot = stageInfo->GetShotObjectPriority();
	int priItem = stageInfo->GetItemObjectPriority();
	int priCamera = stageInfo->GetCameraFocusPermitPriority();
	int invalidPriMin = infoSystem_->GetInvaridRenderPriorityMin();
	int invalidPriMax = infoSystem_->GetInvaridRenderPriorityMax();

	std::vector<bool> listShotValidPriority;
	std::vector<bool> listItemValidPriority;
	if(bValidStage)
	{
		listShotValidPriority = stageController_->GetShotManager()->GetValidRenderPriorityList();
		listItemValidPriority = stageController_->GetItemManager()->GetValidRenderPriorityList();
	}

	focusPos.x -= stgWidth / 2;
	focusPos.y -= stgHeight / 2;

	//フォグ設定
	bool bFogEnable = false;
	D3DCOLOR fogColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	float fogStart = 0;
	float fogEnd = 0;
	if(objectManagerStage != NULL)
	{
		bFogEnable = objectManagerStage->IsFogEneble();
		fogColor = objectManagerStage->GetFogColor();
		fogStart = objectManagerStage->GetFogStart();
		fogEnd = objectManagerStage->GetFogEnd();
	}
	else if(objectManagerPackage != NULL)
	{
		bFogEnable = objectManagerPackage->IsFogEneble();
		fogColor = objectManagerPackage->GetFogColor();
		fogStart = objectManagerPackage->GetFogStart();
		fogEnd = objectManagerPackage->GetFogEnd();
	}

	graphics->SetVertexFog(bFogEnable, fogColor, fogStart, fogEnd);

	//描画開始前リセット
	camera2D->SetEnable(false);
	camera2D->Reset();
	graphics->ResetViewPort();	

	bool bClearZBufferFor2DCoordinate = false;
	bool bRunMinStgFrame = false;
	bool bRunMaxStgFrame = false;
	for(int iPri = priMin ; iPri <= priMax ; iPri++)
	{
		if(iPri >= priMinStgFrame && !bRunMinStgFrame)
		{
			//STGフレーム開始
			if(bValidStage && iPri < invalidPriMin)
				graphics->ClearRenderTarget(rcStgFrame);

			double clipNear = camera3D->GetNearClip();
			double clipFar = camera3D->GetFarClip();

			camera2D->SetEnable(true);
			camera2D->SetRatioX(focusRatioX);
			camera2D->SetRatioY(focusRatioY);
			camera2D->SetAngleZ(focusAngleZ);
			camera2D->SetClip(rcStgFrame);
			camera2D->SetFocus(stgCenter.x + focusPos.x, stgCenter.y + focusPos.y);
			camera3D->SetProjectionMatrix(rcStgFrame.right - rcStgFrame.left, rcStgFrame.bottom - rcStgFrame.top, clipNear, clipFar);
			camera3D->UpdateDeviceProjectionMatrix();

			graphics->SetViewPort(rcStgFrame.left, rcStgFrame.top, stgWidth, stgHeight);

			bRunMinStgFrame = true;
			bClearZBufferFor2DCoordinate = false;
		}

		if(objectManagerStage != NULL && !bPause)
		{
			//シェーダ設定
			ref_count_ptr<Shader> shader = objectManagerStage->GetShader(iPri);
			if(shader != NULL)
			{
				shader->Begin();
			}

			//ステージ描画
			if(listShotValidPriority[iPri])
			{
				//弾描画
				stageController_->GetShotManager()->Render(iPri);
			}
			if(listItemValidPriority[iPri])
			{
				//アイテム描画
				stageController_->GetItemManager()->Render(iPri);
			}

			if(pRenderListStage != NULL && iPri < (*pRenderListStage).size())
			{
				std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
				for(itr = (*pRenderListStage)[iPri].begin() ; itr != (*pRenderListStage)[iPri].end() ; itr++)
				{
					if(!bClearZBufferFor2DCoordinate)
					{
						DxScriptMeshObject* objMesh = dynamic_cast<DxScriptMeshObject*>((*itr).GetPointer());
						if(objMesh != NULL)
						{
							gstd::ref_count_ptr<DxMesh>& mesh = objMesh->GetMesh();
							if(mesh != NULL && mesh->IsCoordinate2D())
							{
								graphics->GetDevice()->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0,0);
								bClearZBufferFor2DCoordinate = true;
							}
						}
					}
					(*itr)->Render();
				}
				(*pRenderListStage)[iPri].clear();

			}

			if(shader != NULL)
			{
				shader->End();
			}
		}

		//パッケージ
		if(objectManagerPackage != NULL)
		{
			//シェーダ設定
			ref_count_ptr<Shader> shader = objectManagerPackage->GetShader(iPri);
			if(shader != NULL)
			{
				shader->Begin();
			}

			if(pRenderListPackage != NULL && iPri < (*pRenderListPackage).size())
			{
				std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
				for(itr = (*pRenderListPackage)[iPri].begin() ; itr != (*pRenderListPackage)[iPri].end() ; itr++)
				{
					if(!bClearZBufferFor2DCoordinate)
					{
						DxScriptMeshObject* objMesh = dynamic_cast<DxScriptMeshObject*>((*itr).GetPointer());
						if(objMesh != NULL)
						{
							gstd::ref_count_ptr<DxMesh>& mesh = objMesh->GetMesh();
							if(mesh != NULL && mesh->IsCoordinate2D())
							{
								graphics->GetDevice()->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0,0);
								bClearZBufferFor2DCoordinate = true;
							}
						}
					}
					(*itr)->Render();
				}
				(*pRenderListPackage)[iPri].clear();
			}
			if(shader != NULL)
			{
				shader->End();
			}
		}

		if(iPri == priCamera)
		{
			camera2D->SetFocus(stgCenter.x, stgCenter.y);
			camera2D->SetRatio(1);
			camera2D->SetAngleZ(0);
		}
		if(iPri >= priMaxStgFrame && !bRunMaxStgFrame)
		{
			//STGフレーム終了
			camera2D->SetEnable(false);
			camera2D->Reset();
			graphics->ResetViewPort();

			bRunMaxStgFrame = true;
			bClearZBufferFor2DCoordinate = false;
		}
	}
	camera2D->SetFocus(orgFocusPos);
	camera2D->SetRatioX(focusRatioX);
	camera2D->SetRatioY(focusRatioY);
	camera2D->SetAngleZ(focusAngleZ);

	//--------------------------------
	if(objectManagerStage != NULL)
		objectManagerStage->ClearRenderObject();
	if(objectManagerPackage != NULL)
		objectManagerPackage->ClearRenderObject();
}
void StgSystemController::_ControlScene()
{
	if(infoSystem_->IsPackageMode())
	{
		packageController_->Work();

		ref_count_ptr<StgPackageInformation> infoPackage = packageController_->GetPackageInformation();
		if(infoPackage->IsEnd())
		{
			infoSystem_->SetStgEnd();
		}
	}

	int scene = infoSystem_->GetScene();
	switch(scene)
	{
	case StgSystemInformation::SCENE_STG:
	{
		ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
		if(!infoStage->IsEnd())
			stageController_->Work();

		if(infoStage->IsEnd())
		{
			//次ステージへ
			stageController_->CloseScene();
			if(infoSystem_->IsPackageMode())
			{
				stageController_->RenderToTransitionTexture();
				if(infoStage->GetResult() == StgStageInformation::RESULT_UNKNOWN)
				{
					int sceneResult = StgStageInformation::RESULT_CLEARED;
					ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController_->GetPlayerObject();
					if(objPlayer != NULL)
					{
						int statePlayer = objPlayer->GetState();
						if(statePlayer == StgPlayerObject::STATE_END)
							sceneResult = StgStageInformation::RESULT_PLAYER_DOWN;
					}
					infoStage->SetResult(sceneResult);
				}
				infoSystem_->SetScene(StgSystemInformation::SCENE_PACKAGE_CONTROL);

				ref_count_ptr<StgPackageInformation> infoPackage = packageController_->GetPackageInformation();
				infoPackage->FinishCurrentStage();
			}
			else 
				TransStgEndScene();
		}
		break;
	}
	case StgSystemInformation::SCENE_END:
		endScene_->Work();
		break;
	case StgSystemInformation::SCENE_REPLAY_SAVE:
		replaySaveScene_->Work();
		break;
	}

	if(infoSystem_->IsPackageMode())
	{
		//シーン変化時には即座にパッケージ管理機能を実行する
		//パッケージスクリプト内で起動するシーン遷移の描画などが追いつかなくなるため
		if(scene != infoSystem_->GetScene())
		{
			packageController_->Work();
		}
	}

	ELogger* logger = ELogger::GetInstance();
	if(logger->IsWindowVisible())
	{
		//ログ関連
		int taskCount = 0;
		int objectCount = 0;
		if(packageController_ != NULL)
		{
			ref_count_ptr<StgControlScriptManager> scriptManager = packageController_->GetScriptManager();
			if(scriptManager != NULL) 
				taskCount = scriptManager->GetAllScriptThreadCount();

			ref_count_ptr<DxScriptObjectManager> objectManager =  packageController_->GetMainObjectManager();
			if(objectManager != NULL)
				objectCount += objectManager->GetAliveObjectCount();
		}
		if(stageController_ != NULL)
		{
			ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
			if(!infoStage->IsEnd())
			{
				StgControlScriptManager* scriptManager = stageController_->GetScriptManagerP();
				if(scriptManager != NULL)
					taskCount = scriptManager->GetAllScriptThreadCount();

				ref_count_ptr<DxScriptObjectManager> objectManager =  stageController_->GetMainObjectManager();
				if(objectManager != NULL)
					objectCount += objectManager->GetAliveObjectCount();
			}
		}
		logger->SetInfo(4, L"script_task_count", StringUtility::Format(L"%d", taskCount));
		logger->SetInfo(5, L"object_count", StringUtility::Format(L"%d", objectCount));
	}
}
void StgSystemController::StartStgScene(ref_count_ptr<StgStageInformation> infoStage, ref_count_ptr<ReplayInformation::StageData> replayStageData)
{
	ref_count_ptr<StgStageStartData> startData = new StgStageStartData();
	startData->SetStageInformation(infoStage);
	startData->SetStageReplayData(replayStageData);
	StartStgScene(startData);
}
void StgSystemController::StartStgScene(ref_count_ptr<StgStageStartData> startData)
{
	EDirectInput* input = EDirectInput::GetInstance();
	input->ClearKeyState();

	infoSystem_->SetScene(StgSystemInformation::SCENE_STG);
	stageController_ = new StgStageController(this);

	stageController_->Initialize(startData);
}
void StgSystemController::TransStgEndScene()
{
	bool bReplay = false;
	if(stageController_ != NULL)
	{
		ref_count_ptr<StgStageInformation> infoStage = stageController_->GetStageInformation();
		bReplay = infoStage->IsReplay();
	}

	if(!bReplay)
	{
		ref_count_ptr<ReplayInformation> infoReplay = CreateReplayInformation();
		infoSystem_->SetActiveReplayInformation(infoReplay);
		endScene_ = new StgEndScene(this);
		endScene_->Start();
		infoSystem_->SetScene(StgSystemInformation::SCENE_END);
	}
	else
	{
		infoSystem_->SetStgEnd();
	}
}

void StgSystemController::TransReplaySaveScene()
{
	replaySaveScene_ = new StgReplaySaveScene(this);
	replaySaveScene_->Start();
	infoSystem_->SetScene(StgSystemInformation::SCENE_REPLAY_SAVE);
}

ref_count_ptr<ReplayInformation> StgSystemController::CreateReplayInformation()
{
	ref_count_ptr<ReplayInformation> res = new ReplayInformation();

	//メインスクリプト関連
	ref_count_ptr<StgStageInformation> infoLastStage = stageController_->GetStageInformation();
	ref_count_ptr<ScriptInformation> infoMain = infoSystem_->GetMainScriptInformation();
	std::wstring pathMainScript = infoMain->GetScriptPath();
	std::wstring nameMainScript = PathProperty::GetFileName(pathMainScript);

	//自機関連
	ref_count_ptr<ScriptInformation> infoPlayer = infoLastStage->GetPlayerScriptInformation();
	std::wstring pathPlayerScript = infoPlayer->GetScriptPath();
	std::wstring filenamePlayerScript = PathProperty::GetFileName(pathPlayerScript);
	res->SetPlayerScriptFileName(filenamePlayerScript);
	res->SetPlayerScriptID(infoPlayer->GetID());
	res->SetPlayerScriptReplayName(infoPlayer->GetReplayName());

	//システム関連
	_int64 totalScore = infoLastStage->GetScore();
	double fpsAvarage = 0;

	//ステージ
	if(infoSystem_->IsPackageMode())
	{
		ref_count_ptr<StgPackageInformation> infoPackage = packageController_->GetPackageInformation();
		std::vector<ref_count_ptr<StgStageStartData> > listStageData = infoPackage->GetStageDataList();
		for(int iStage = 0 ; iStage < listStageData.size() ; iStage++)
		{
			ref_count_ptr<StgStageStartData> stageData = listStageData[iStage];
			ref_count_ptr<StgStageInformation> infoStage = stageData->GetStageInformation();
			ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
			res->SetStageData(infoStage->GetStageIndex(), replayStageData);

			fpsAvarage += replayStageData->GetFramePerSecondAvarage();
		}
		if(listStageData.size() > 0)
			fpsAvarage = fpsAvarage / listStageData.size();
	}
	else
	{
		ref_count_ptr<StgStageController> stageController = stageController_;
		ref_count_ptr<ReplayInformation::StageData> replayStageData = infoLastStage->GetReplayData();
		res->SetStageData(0, replayStageData);
		fpsAvarage = replayStageData->GetFramePerSecondAvarage();
	}

	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	res->SetDate(sysTime);
	res->SetTotalScore(totalScore);
	res->SetAvarageFps(fpsAvarage);

	return res;
}

void StgSystemController::TerminateScriptAll()
{
	std::wstring error = L"force terminate";
	if(packageController_ != NULL)
	{
		ref_count_ptr<ScriptManager> scriptManager = packageController_->GetScriptManager();
		if(scriptManager != NULL)
			scriptManager->TerminateScriptAll(error);
	}

	if(stageController_ != NULL)
	{
		ScriptManager* scriptManager = stageController_->GetScriptManagerP();
		if(scriptManager != NULL)
			scriptManager->TerminateScriptAll(error);

		ref_count_ptr<StgPauseScene> pauseScene = stageController_->GetPauseManager();
		if(pauseScene != NULL)
		{
			ref_count_ptr<ScriptManager> pauseScriptManager = pauseScene->GetScriptManager();
			if(pauseScriptManager != NULL)
				pauseScriptManager->TerminateScriptAll(error);
		}
	}

	if(endScene_ != NULL)
	{
		ref_count_ptr<ScriptManager> scriptManager = endScene_->GetScriptManager();
		if(scriptManager != NULL)
			scriptManager->TerminateScriptAll(error);
	}

	if(replaySaveScene_ != NULL)
	{
		ref_count_ptr<ScriptManager> scriptManager = replaySaveScene_->GetScriptManager();
		if(scriptManager != NULL)
			scriptManager->TerminateScriptAll(error);
	}

}

/**********************************************************
//StgSystemInformation
**********************************************************/
StgSystemInformation::StgSystemInformation()
{
	scene_ = SCENE_NULL;
	bEndStg_ = false;
	bRetry_ = false;

	invalidPriMin_ = -1;
	invalidPriMax_ = -1;

	pathPauseScript_ = EPathProperty::GetStgDefaultScriptDirectory() + L"Default_Pause.txt";
	pathEndSceneScript_ = EPathProperty::GetStgDefaultScriptDirectory() + L"Default_EndScene.txt";
	pathReplaySaveSceneScript_ = EPathProperty::GetStgDefaultScriptDirectory() + L"Default_ReplaySaveScene.txt";
}
StgSystemInformation::~StgSystemInformation()
{
}
std::wstring StgSystemInformation::GetErrorMessage()
{
	std::wstring res = L"";
	std::list<std::wstring>::iterator itr = listError_.begin();
	for(; itr != listError_.end() ; itr++)
	{
		std::wstring str = (*itr);
		if(str == L"Retry")continue;
		res += str + L"\r\n" + L"\r\n";
	}
	return res;
}
bool StgSystemInformation::IsPackageMode()
{
	bool res = infoMain_->GetType() == ScriptInformation::TYPE_PACKAGE;
	return res;
}
void StgSystemInformation::ResetRetry()
{
	bEndStg_ = false;
	bRetry_ = false;
	listError_.clear();
}
void StgSystemInformation::SetInvaridRenderPriority(int priMin, int priMax)
{
	invalidPriMin_ = priMin;
	invalidPriMax_ = priMax;
}
