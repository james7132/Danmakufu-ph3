#include"StgStageScript.hpp"
#include"StgSystem.hpp"
#include"StgPlayer.hpp"
#include"StgShot.hpp"
#include"StgItem.hpp"


/**********************************************************
//StgStageScriptManager
**********************************************************/
StgStageScriptManager::StgStageScriptManager(StgStageController* stageController)
{
	stageController_ = stageController;
	objManager_ = stageController_->GetMainObjectManager();
	idPlayerScript_ = ID_INVALID;
	idItemScript_ = ID_INVALID;
	idShotScript_ = ID_INVALID;
}
StgStageScriptManager::~StgStageScriptManager()
{

}

void StgStageScriptManager::SetError(std::wstring error)
{
	StgControlScriptManager::SetError(error);
	stageController_->GetSystemInformation()->SetError(error);
}
bool StgStageScriptManager::IsError()
{
	bool res = error_ != L"" || stageController_->GetSystemInformation()->IsError();
	return res;
}

ref_count_ptr<ManagedScript> StgStageScriptManager::Create(int type)
{
	ref_count_ptr<ManagedScript> res = NULL;
	switch(type)
	{
		case StgStageScript::TYPE_STAGE:
			res = new StgStageScript(stageController_);
			break;
		case StgStageScript::TYPE_SYSTEM:
			res = new StgStageSystemScript(stageController_);
			break;
		case StgStageScript::TYPE_ITEM:
			res = new StgStageItemScript(stageController_);
			break;
		case StgStageScript::TYPE_SHOT:
			res = new StgStageShotScript(stageController_);
			break;
		case StgStageScript::TYPE_PLAYER:
			res = new StgStagePlayerScript(stageController_);
			break;

	}

	if(res != NULL)
	{
		res->SetScriptManager(stageController_->GetScriptManagerP());
	}

	return res;
}
ref_count_ptr<ManagedScript> StgStageScriptManager::GetItemScript()
{
	ref_count_ptr<ManagedScript> res = NULL;
	if(idItemScript_ != StgControlScriptManager::ID_INVALID)
	{
		res = GetScript(idItemScript_);
	}
	return res;
}
ref_count_ptr<ManagedScript> StgStageScriptManager::GetShotScript()
{
	ref_count_ptr<ManagedScript> res = NULL;
	if(idShotScript_ != StgControlScriptManager::ID_INVALID)
	{
		res = GetScript(idShotScript_);
	}
	return res;
}


/**********************************************************
//StgStageScriptObjectManager
**********************************************************/
StgStageScriptObjectManager::StgStageScriptObjectManager(StgStageController* stageController)
{
	stageController_ = stageController;
	SetMaxObject(256 * 256);

	idObjPleyer_ = DxScript::ID_INVALID;
}
StgStageScriptObjectManager::~StgStageScriptObjectManager()
{
	if(idObjPleyer_ != DxScript::ID_INVALID)
	{
		ref_count_ptr<StgPlayerObject>::unsync obj = ref_count_ptr<StgPlayerObject>::unsync::DownCast(GetObject(idObjPleyer_));
		if(obj != NULL)
			obj->Clear();
	}
}
void StgStageScriptObjectManager::RenderObject()
{
/*
	if(invalidPriMin_ < 0 && invalidPriMax_ < 0) 
	{
		RenderObject(0, objRender_.size());
	}
	else
	{
		RenderObject(0, invalidPriMin_);
		RenderObject(invalidPriMax_ + 1, objRender_.size());
	}
*/
}

void StgStageScriptObjectManager::RenderObject(int priMin, int priMax)
{
/*
	std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
	for(itr = listActiveObject_.begin() ; itr != listActiveObject_.end() ; itr++)
	{
		gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = (*itr);
		if(obj == NULL || obj->IsDeleted())continue;
		if(!obj->IsVisible())continue;
		AddRenderObject(obj);
	}

	DirectGraphics* graphics = DirectGraphics::GetBase();
	gstd::ref_count_ptr<DxCamera> camera3D = graphics->GetCamera();
	gstd::ref_count_ptr<DxCamera2D> camera2D = graphics->GetCamera2D();

	ref_count_ptr<StgStageInformation> stageInfo = stageController_->GetStageInformation();
	RECT rcStgFrame = stageInfo->GetStgFrameRect();
	int stgWidth = rcStgFrame.right - rcStgFrame.left;
	int stgHeight = rcStgFrame.bottom - rcStgFrame.top;
	POINT stgCenter = {rcStgFrame.left + stgWidth / 2, rcStgFrame.top + stgHeight / 2};
	int priMinStgFrame = stageInfo->GetStgFrameMinPriority();
	int priMaxStgFrame = stageInfo->GetStgFrameMaxPriority();
	int priShot = stageInfo->GetShotObjectPriority();
	int priItem = stageInfo->GetItemObjectPriority();
	int priCamera = stageInfo->GetCameraFocusPermitPriority();

	double focusRatio = camera2D->GetRatio();
	D3DXVECTOR2 orgFocusPos = camera2D->GetFocusPosition();
	D3DXVECTOR2 focusPos = orgFocusPos;
	focusPos.x -= stgWidth / 2;
	focusPos.y -= stgHeight / 2;

	//フォグ設定
	graphics->SetVertexFog(bFogEnable_, fogColor_, fogStart_, fogEnd_);

	//描画開始前リセット
	camera2D->SetEnable(false);
	camera2D->Reset();
	graphics->ResetViewPort();	

	bool bClearZBufferFor2DCoordinate = false;
	bool bRunMinStgFrame = false;
	bool bRunMaxStgFrame = false;
	for(int iPri = priMin ; iPri <= priMax ; iPri++)
	{
		if(iPri >= objRender_.size())break;

		if(iPri >= priMinStgFrame && !bRunMinStgFrame)
		{
			//STGフレーム開始
			graphics->ClearRenderTarget(rcStgFrame);
			camera2D->SetEnable(true);
			camera2D->SetRatio(focusRatio);
			camera2D->SetClip(rcStgFrame);
			camera2D->SetFocus(stgCenter.x + focusPos.x, stgCenter.y + focusPos.y);
			camera3D->SetProjectionMatrix(rcStgFrame.right - rcStgFrame.left, rcStgFrame.bottom - rcStgFrame.top, 10, 2000);
			camera3D->UpdateDeviceProjectionMatrix();

			graphics->SetViewPort(rcStgFrame.left, rcStgFrame.top, stgWidth, stgHeight);

			bRunMinStgFrame = true;
			bClearZBufferFor2DCoordinate = false;
		}
		if(iPri == priShot)
		{
			//弾描画
			stageController_->GetShotManager()->Render();
		}
		if(iPri == priItem)
		{
			//アイテム描画
			stageController_->GetItemManager()->Render();
		}

		std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
		for(itr = objRender_[iPri].begin() ; itr != objRender_[iPri].end() ; itr++)
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
		objRender_[iPri].clear();

		if(iPri == priCamera)
		{
			camera2D->SetFocus(stgCenter.x, stgCenter.y);
			camera2D->SetRatio(1);
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
	camera2D->SetRatio(focusRatio);
*/
}
int StgStageScriptObjectManager::CreatePlayerObject()
{
	//自機オブジェクト生成
	ref_count_ptr<StgPlayerObject>::unsync objPlayer = new StgPlayerObject(stageController_);
	idObjPleyer_ = AddObject(objPlayer);
	return idObjPleyer_;
}


/**********************************************************
//StgStageScript
**********************************************************/
function const stgFunction[] =  
{
	//STG共通関数：共通データ
	{"SaveCommonDataAreaToReplayFile", StgStageScript::Func_SaveCommonDataAreaToReplayFile, 1},
	{"LoadCommonDataAreaFromReplayFile", StgStageScript::Func_LoadCommonDataAreaFromReplayFile, 1},

	//STG共通関数：システム関連
	{"GetMainStgScriptPath", StgStageScript::Func_GetMainStgScriptPath, 0},
	{"GetMainStgScriptDirectory", StgStageScript::Func_GetMainStgScriptDirectory, 0},
	{"SetStgFrame", StgStageScript::Func_SetStgFrame, 6},
	{"SetItemRenderPriorityI", StgStageScript::Func_SetItemRenderPriorityI, 1},
	{"SetShotRenderPriorityI", StgStageScript::Func_SetShotRenderPriorityI, 1},
	{"GetStgFrameRenderPriorityMinI", StgStageScript::Func_GetStgFrameRenderPriorityMinI, 0},
	{"GetStgFrameRenderPriorityMaxI", StgStageScript::Func_GetStgFrameRenderPriorityMaxI, 0},
	{"GetItemRenderPriorityI", StgStageScript::Func_GetItemRenderPriorityI, 0},
	{"GetShotRenderPriorityI", StgStageScript::Func_GetShotRenderPriorityI, 0},
	{"GetPlayerRenderPriorityI", StgStageScript::Func_GetPlayerRenderPriorityI, 0},
	{"GetCameraFocusPermitPriorityI", StgStageScript::Func_GetCameraFocusPermitPriorityI, 0},
	{"CloseStgScene", StgStageScript::Func_CloseStgScene, 0},
	{"GetReplayFps", StgStageScript::Func_GetReplayFps, 0},

	//STG共通関数：自機
	{"GetPlayerObjectID", StgStageScript::Func_GetPlayerObjectID, 0},
	{"SetPlayerSpeed", StgStageScript::Func_SetPlayerSpeed, 2},
	{"SetPlayerClip", StgStageScript::Func_SetPlayerClip, 4},
	{"SetPlayerLife", StgStageScript::Func_SetPlayerLife, 1},
	{"SetPlayerSpell", StgStageScript::Func_SetPlayerSpell, 1},
	{"SetPlayerPower", StgStageScript::Func_SetPlayerPower, 1},
	{"SetPlayerInvincibilityFrame", StgStageScript::Func_SetPlayerInvincibilityFrame, 1},
	{"SetPlayerDownStateFrame", StgStageScript::Func_SetPlayerDownStateFrame, 1},
	{"SetPlayerRebirthFrame", StgStageScript::Func_SetPlayerRebirthFrame, 1},
	{"SetPlayerRebirthLossFrame", StgStageScript::Func_SetPlayerRebirthLossFrame, 1},
	{"SetPlayerAutoItemCollectLine", StgStageScript::Func_SetPlayerAutoItemCollectLine, 1},
	{"SetForbidPlayerShot", StgStageScript::Func_SetForbidPlayerShot, 1},
	{"SetForbidPlayerSpell", StgStageScript::Func_SetForbidPlayerSpell, 1},
	{"GetPlayerX", StgStageScript::Func_GetPlayerX, 0},
	{"GetPlayerY", StgStageScript::Func_GetPlayerY, 0},
	{"GetPlayerState", StgStageScript::Func_GetPlayerState, 0},
	{"GetPlayerSpeed", StgStageScript::Func_GetPlayerSpeed, 0},
	{"GetPlayerClip", StgStageScript::Func_GetPlayerClip, 0},
	{"GetPlayerLife", StgStageScript::Func_GetPlayerLife, 0},
	{"GetPlayerSpell", StgStageScript::Func_GetPlayerSpell, 0},
	{"GetPlayerPower", StgStageScript::Func_GetPlayerPower, 0},
	{"GetPlayerInvincibilityFrame", StgStageScript::Func_GetPlayerInvincibilityFrame, 0},
	{"GetPlayerDownStateFrame", StgStageScript::Func_GetPlayerDownStateFrame, 0},
	{"GetPlayerRebirthFrame", StgStageScript::Func_GetPlayerRebirthFrame, 0},
	{"GetAngleToPlayer", StgStageScript::Func_GetAngleToPlayer, 1},
	{"IsPermitPlayerShot", StgStageScript::Func_IsPermitPlayerShot, 0},
	{"IsPermitPlayerSpell", StgStageScript::Func_IsPermitPlayerSpell, 0},
	{"IsPlayerLastSpellWait", StgStageScript::Func_IsPlayerLastSpellWait, 0},
	{"IsPlayerSpellActive", StgStageScript::Func_IsPlayerSpellActive, 0},
	{"GetPlayerScriptID", StgStageScript::Func_GetPlayerScriptID, 0},

	//STG共通関数：敵
	{"GetEnemyBossSceneObjectID", StgStageScript::Func_GetEnemyBossSceneObjectID, 0},
	{"GetEnemyBossObjectID", StgStageScript::Func_GetEnemyBossObjectID, 0},
	{"GetAllEnemyID", StgStageScript::Func_GetAllEnemyID, 0},
	{"GetIntersectionRegistedEnemyID", StgStageScript::Func_GetIntersectionRegistedEnemyID, 0},
	{"GetAllEnemyIntersectionPosition", StgStageScript::Func_GetAllEnemyIntersectionPosition, 0},
	{"GetEnemyIntersectionPosition", StgStageScript::Func_GetEnemyIntersectionPosition, 3},
	{"GetEnemyIntersectionPositionByIdA1", StgStageScript::Func_GetEnemyIntersectionPositionByIdA1, 1},
	{"GetEnemyIntersectionPositionByIdA2", StgStageScript::Func_GetEnemyIntersectionPositionByIdA2, 3},
	{"LoadEnemyShotData", StgStageScript::Func_LoadEnemyShotData, 1},
	{"ReloadEnemyShotData", StgStageScript::Func_ReloadEnemyShotData, 1},

	//STG共通関数：弾
	{"DeleteShotAll", StgStageScript::Func_DeleteShotAll, 2},
	{"DeleteShotInCircle", StgStageScript::Func_DeleteShotInCircle, 5},
	{"CreateShotA1", StgStageScript::Func_CreateShotA1, 6},
	{"CreateShotA2", StgStageScript::Func_CreateShotA2, 8},
	{"CreateShotOA1", StgStageScript::Func_CreateShotOA1, 5},
	{"CreateShotB1", StgStageScript::Func_CreateShotB1, 6},
	{"CreateShotB2", StgStageScript::Func_CreateShotB2, 10},
	{"CreateShotOB1", StgStageScript::Func_CreateShotOB1, 5},
	{"CreateLooseLaserA1", StgStageScript::Func_CreateLooseLaserA1, 8},
	{"CreateStraightLaserA1", StgStageScript::Func_CreateStraightLaserA1, 8},
	{"CreateCurveLaserA1", StgStageScript::Func_CreateCurveLaserA1, 8},
	//{"StgStraightLaserA1", StgStageScript::Func_CreateStraightLaserA1, 8},
	{"SetShotIntersectionCircle", StgStageScript::Func_SetShotIntersectionCircle, 3},
	{"SetShotIntersectionLine", StgStageScript::Func_SetShotIntersectionLine, 5},
	{"GetShotIdInCircleA1", StgStageScript::Func_GetShotIdInCircleA1, 3},
	{"GetShotIdInCircleA2", StgStageScript::Func_GetShotIdInCircleA2, 4},
	{"GetShotCount", StgStageScript::Func_GetShotCount, 1},
	{"SetShotAutoDeleteClip", StgStageScript::Func_SetShotAutoDeleteClip, 4},
	{"GetShotDataInfoA1", StgStageScript::Func_GetShotDataInfoA1, 3},
	{"StartShotScript", StgStageScript::Func_StartShotScript, 1},

	//STG共通関数：アイテム
	{"CreateItemA1", StgStageScript::Func_CreateItemA1, 4},
	{"CreateItemA2", StgStageScript::Func_CreateItemA2, 6},
	{"CreateItemU1", StgStageScript::Func_CreateItemU1, 4},
	{"CreateItemU2", StgStageScript::Func_CreateItemU2, 6},
	{"CreateItemScore", StgStageScript::Func_CreateItemScore, 3},
	{"CollectAllItems", StgStageScript::Func_CollectAllItems, 0},
	{"CollectItemsByType", StgStageScript::Func_CollectItemsByType, 1},
	{"CollectItemsInCircle", StgStageScript::Func_CollectItemsInCircle, 3},
	{"CancelCollectItems", StgStageScript::Func_CancelCollectItems, 0},
	{"StartItemScript", StgStageScript::Func_StartItemScript, 1},
	{"SetDefaultBonusItemEnable", StgStageScript::Func_SetDefaultBonusItemEnable, 1},
	{"LoadItemData", StgStageScript::Func_LoadItemData, 1},
	{"ReloadItemData", StgStageScript::Func_ReloadItemData, 1},

	//STG共通関数：その他
	{"StartSlow", StgStageScript::Func_StartSlow, 2},
	{"StopSlow", StgStageScript::Func_StopSlow, 1},
	{"IsIntersected_Line_Circle", StgStageScript::Func_IsIntersected_Line_Circle, 8},
	{"IsIntersected_Obj_Obj", StgStageScript::Func_IsIntersected_Obj_Obj, 2},

	//STG共通関数：移動オブジェクト操作
	{"ObjMove_SetX", StgStageScript::Func_ObjMove_SetX, 2},
	{"ObjMove_SetY", StgStageScript::Func_ObjMove_SetY, 2},
	{"ObjMove_SetPosition", StgStageScript::Func_ObjMove_SetPosition, 3},
	{"ObjMove_SetSpeed", StgStageScript::Func_ObjMove_SetSpeed, 2},
	{"ObjMove_SetAngle", StgStageScript::Func_ObjMove_SetAngle, 2},
	{"ObjMove_SetAcceleration", StgStageScript::Func_ObjMove_SetAcceleration, 2},
	{"ObjMove_SetMaxSpeed", StgStageScript::Func_ObjMove_SetMaxSpeed, 2},
	{"ObjMove_SetAngularVelocity", StgStageScript::Func_ObjMove_SetAngularVelocity, 2},
	{"ObjMove_SetDestAtSpeed", StgStageScript::Func_ObjMove_SetDestAtSpeed, 4},
	{"ObjMove_SetDestAtFrame", StgStageScript::Func_ObjMove_SetDestAtFrame, 4},
	{"ObjMove_SetDestAtWeight", StgStageScript::Func_ObjMove_SetDestAtWeight, 5},
	{"ObjMove_AddPatternA1", StgStageScript::Func_ObjMove_AddPatternA1, 4},
	{"ObjMove_AddPatternA2", StgStageScript::Func_ObjMove_AddPatternA2, 7},
	{"ObjMove_AddPatternA3", StgStageScript::Func_ObjMove_AddPatternA3, 8},
	{"ObjMove_AddPatternA4", StgStageScript::Func_ObjMove_AddPatternA4, 9},
	{"ObjMove_AddPatternB1", StgStageScript::Func_ObjMove_AddPatternB1, 4},
	{"ObjMove_AddPatternB2", StgStageScript::Func_ObjMove_AddPatternB2, 8},
	{"ObjMove_AddPatternB3", StgStageScript::Func_ObjMove_AddPatternB3, 9},
	{"ObjMove_GetX", StgStageScript::Func_ObjMove_GetX, 1},
	{"ObjMove_GetY", StgStageScript::Func_ObjMove_GetY, 1},
	{"ObjMove_GetSpeed", StgStageScript::Func_ObjMove_GetSpeed, 1},
	{"ObjMove_GetAngle", StgStageScript::Func_ObjMove_GetAngle, 1},

	//STG共通関数：敵オブジェクト操作
	{"ObjEnemy_Create", StgStageScript::Func_ObjEnemy_Create, 1},
	{"ObjEnemy_Regist", StgStageScript::Func_ObjEnemy_Regist, 1},
	{"ObjEnemy_GetInfo", StgStageScript::Func_ObjEnemy_GetInfo, 2},
	{"ObjEnemy_SetLife", StgStageScript::Func_ObjEnemy_SetLife, 2},
	{"ObjEnemy_AddLife", StgStageScript::Func_ObjEnemy_AddLife, 2},
	{"ObjEnemy_SetDamageRate", StgStageScript::Func_ObjEnemy_SetDamageRate, 3},
	{"ObjEnemy_AddIntersectionCircleA", StgStageScript::Func_ObjEnemy_AddIntersectionCircleA, 4},
	{"ObjEnemy_SetIntersectionCircleToShot", StgStageScript::Func_ObjEnemy_SetIntersectionCircleToShot, 4},
	{"ObjEnemy_SetIntersectionCircleToPlayer", StgStageScript::Func_ObjEnemy_SetIntersectionCircleToPlayer, 4},

	//STG共通関数：敵ボスシーンオブジェクト操作
	{"ObjEnemyBossScene_Create", StgStageScript::Func_ObjEnemyBossScene_Create, 0},
	{"ObjEnemyBossScene_Regist", StgStageScript::Func_ObjEnemyBossScene_Regist, 1},
	{"ObjEnemyBossScene_Add", StgStageScript::Func_ObjEnemyBossScene_Add, 3},
	{"ObjEnemyBossScene_LoadInThread", StgStageScript::Func_ObjEnemyBossScene_LoadInThread, 1},
	{"ObjEnemyBossScene_GetInfo", StgStageScript::Func_ObjEnemyBossScene_GetInfo, 2},
	{"ObjEnemyBossScene_SetSpellTimer", StgStageScript::Func_ObjEnemyBossScene_SetSpellTimer, 2},
	{"ObjEnemyBossScene_StartSpell", StgStageScript::Func_ObjEnemyBossScene_StartSpell, 1},

	//STG共通関数：弾オブジェクト操作
	{"ObjShot_Create", StgStageScript::Func_ObjShot_Create, 1},
	{"ObjShot_Regist", StgStageScript::Func_ObjShot_Regist, 1},
	{"ObjShot_SetAutoDelete", StgStageScript::Func_ObjShot_SetAutoDelete, 2},
	{"ObjShot_FadeDelete", StgStageScript::Func_ObjShot_FadeDelete, 1},
	{"ObjShot_SetDeleteFrame", StgStageScript::Func_ObjShot_SetDeleteFrame, 2},
	{"ObjShot_SetDelay", StgStageScript::Func_ObjShot_SetDelay, 2},
	{"ObjShot_SetSpellResist", StgStageScript::Func_ObjShot_SetSpellResist, 2},
	{"ObjShot_SetGraphic", StgStageScript::Func_ObjShot_SetGraphic, 2},
	{"ObjShot_SetSourceBlendType", StgStageScript::Func_ObjShot_SetSourceBlendType, 2},
	{"ObjShot_SetDamage", StgStageScript::Func_ObjShot_SetDamage, 2},
	{"ObjShot_SetPenetration", StgStageScript::Func_ObjShot_SetPenetration, 2},
	{"ObjShot_SetEraseShot", StgStageScript::Func_ObjShot_SetEraseShot, 2},
	{"ObjShot_SetSpellFactor", StgStageScript::Func_ObjShot_SetSpellFactor, 2},
	{"ObjShot_ToItem", StgStageScript::Func_ObjShot_ToItem, 1},
	{"ObjShot_AddShotA1", StgStageScript::Func_ObjShot_AddShotA1, 3},
	{"ObjShot_AddShotA2", StgStageScript::Func_ObjShot_AddShotA2, 5},
	{"ObjShot_SetIntersectionCircleA1", StgStageScript::Func_ObjShot_SetIntersectionCircleA1, 2},
	{"ObjShot_SetIntersectionCircleA2", StgStageScript::Func_ObjShot_SetIntersectionCircleA2, 4},
	{"ObjShot_SetIntersectionLine", StgStageScript::Func_ObjShot_SetIntersectionLine, 6},
	{"ObjShot_SetIntersectionEnable", StgStageScript::Func_ObjShot_SetIntersectionEnable, 2},
	{"ObjShot_SetItemChange", StgStageScript::Func_ObjShot_SetItemChange, 2},
	{"ObjShot_GetDelay", StgStageScript::Func_ObjShot_GetDelay, 1},
	{"ObjShot_GetDamage", StgStageScript::Func_ObjShot_GetDamage, 1},
	{"ObjShot_GetPenetration", StgStageScript::Func_ObjShot_GetPenetration, 1},
	{"ObjShot_IsSpellResist", StgStageScript::Func_ObjShot_IsSpellResist, 1},
	{"ObjShot_GetImageID", StgStageScript::Func_ObjShot_GetImageID, 1},

	{"ObjLaser_SetLength", StgStageScript::Func_ObjLaser_SetLength, 2},
	{"ObjLaser_SetRenderWidth", StgStageScript::Func_ObjLaser_SetRenderWidth, 2},
	{"ObjLaser_SetIntersectionWidth", StgStageScript::Func_ObjLaser_SetIntersectionWidth, 2},
	{"ObjLaser_SetInvalidLength", StgStageScript::Func_ObjLaser_SetInvalidLength, 3},
	{"ObjLaser_SetGrazeInvalidFrame", StgStageScript::Func_ObjLaser_SetGrazeInvalidFrame, 2},
	{"ObjLaser_SetItemDistance", StgStageScript::Func_ObjLaser_SetItemDistance, 2},
	{"ObjLaser_GetLength", StgStageScript::Func_ObjLaser_GetLength, 1},
	{"ObjLaser_GetRenderWidth", StgStageScript::Func_ObjLaser_GetRenderWidth, 1},
	{"ObjLaser_GetIntersectionWidth", StgStageScript::Func_ObjLaser_GetIntersectionWidth, 1},
	{"ObjStLaser_SetAngle", StgStageScript::Func_ObjStLaser_SetAngle, 2},
	{"ObjStLaser_GetAngle", StgStageScript::Func_ObjStLaser_GetAngle, 1},
	{"ObjStLaser_SetSource", StgStageScript::Func_ObjStLaser_SetSource, 2},
	{"ObjCrLaser_SetTipDecrement", StgStageScript::Func_ObjCrLaser_SetTipDecrement, 2},

	//STG共通関数：アイテムオブジェクト操作
	{"ObjItem_Create", StgStageScript::Func_ObjItem_Create, 1},
	{"ObjItem_Regist", StgStageScript::Func_ObjItem_Regist, 1},
	{"ObjItem_SetItemID", StgStageScript::Func_ObjItem_SetItemID, 2},
	{"ObjItem_SetRenderScoreEnable", StgStageScript::Func_ObjItem_SetRenderScoreEnable, 2},
	{"ObjItem_SetAutoCollectEnable", StgStageScript::Func_ObjItem_SetAutoCollectEnable, 2},
	{"ObjItem_SetDefinedMovePatternA1", StgStageScript::Func_ObjItem_SetDefinedMovePatternA1, 2},
	{"ObjItem_GetInfo", StgStageScript::Func_ObjItem_GetInfo, 2},

	//STG共通関数：自機オブジェクト操作
	{"ObjPlayer_AddIntersectionCircleA1", StgStageScript::Func_ObjPlayer_AddIntersectionCircleA1, 5},
	{"ObjPlayer_AddIntersectionCircleA2", StgStageScript::Func_ObjPlayer_AddIntersectionCircleA2, 4},
	{"ObjPlayer_ClearIntersection", StgStageScript::Func_ObjPlayer_ClearIntersection, 1},

	//STG共通関数：当たり判定オブジェクト操作
	{"ObjCol_IsIntersected", StgStageScript::Func_ObjCol_IsIntersected, 1},
	{"ObjCol_GetListOfIntersectedEnemyID", StgStageScript::Func_ObjCol_GetListOfIntersectedEnemyID, 1},
	{"ObjCol_GetIntersectedCount", StgStageScript::Func_ObjCol_GetIntersectedCount, 1},

	//定数
	{"SCREEN_WIDTH", constant<640>::func, 0},
	{"SCREEN_HEIGHT", constant<480>::func, 0},
	{"TYPE_ALL", constant<StgStageScript::TYPE_ALL>::func, 0},
	{"TYPE_SHOT", constant<StgStageScript::TYPE_SHOT>::func, 0},
	{"TYPE_CHILD", constant<StgStageScript::TYPE_CHILD>::func, 0},
	{"TYPE_IMMEDIATE", constant<StgStageScript::TYPE_IMMEDIATE>::func, 0},
	{"TYPE_FADE", constant<StgStageScript::TYPE_FADE>::func, 0},
	{"TYPE_ITEM", constant<StgStageScript::TYPE_ITEM>::func, 0},

	{"STATE_NORMAL", constant<StgPlayerObject::STATE_NORMAL>::func, 0},
	{"STATE_HIT", constant<StgPlayerObject::STATE_HIT>::func, 0},
	{"STATE_DOWN", constant<StgPlayerObject::STATE_DOWN>::func, 0},
	{"STATE_END", constant<StgPlayerObject::STATE_END>::func, 0},

	{"ITEM_1UP", constant<StgItemObject::ITEM_1UP>::func, 0},
	{"ITEM_1UP_S", constant<StgItemObject::ITEM_1UP_S>::func, 0},
	{"ITEM_SPELL", constant<StgItemObject::ITEM_SPELL>::func, 0},
	{"ITEM_SPELL_S", constant<StgItemObject::ITEM_SPELL_S>::func, 0},
	{"ITEM_POWER", constant<StgItemObject::ITEM_POWER>::func, 0},
	{"ITEM_POWER_S", constant<StgItemObject::ITEM_POWER_S>::func, 0},
	{"ITEM_POINT", constant<StgItemObject::ITEM_POINT>::func, 0},
	{"ITEM_POINT_S", constant<StgItemObject::ITEM_POINT_S>::func, 0},
	{"ITEM_USER", constant<StgItemObject::ITEM_USER>::func, 0},

	{"ITEM_MOVE_DOWN", constant<StgMovePattern_Item::MOVE_DOWN>::func, 0},
	{"ITEM_MOVE_TOPLAYER", constant<StgMovePattern_Item::MOVE_TOPLAYER>::func, 0},

	{"OBJ_PLAYER", constant<StgStageScript::OBJ_PLAYER>::func, 0},
	{"OBJ_SPELL_MANAGE", constant<StgStageScript::OBJ_SPELL_MANAGE>::func, 0},
	{"OBJ_SPELL", constant<StgStageScript::OBJ_SPELL>::func, 0},
	{"OBJ_ENEMY", constant<StgStageScript::OBJ_ENEMY>::func, 0},
	{"OBJ_ENEMY_BOSS", constant<StgStageScript::OBJ_ENEMY_BOSS>::func, 0},
	{"OBJ_ENEMY_BOSS_SCENE", constant<StgStageScript::OBJ_ENEMY_BOSS_SCENE>::func, 0},
	{"OBJ_SHOT", constant<StgStageScript::OBJ_SHOT>::func, 0},
	{"OBJ_LOOSE_LASER", constant<StgStageScript::OBJ_LOOSE_LASER>::func, 0},
	{"OBJ_STRAIGHT_LASER", constant<StgStageScript::OBJ_STRAIGHT_LASER>::func, 0},
	{"OBJ_CURVE_LASER", constant<StgStageScript::OBJ_CURVE_LASER>::func, 0},
	{"OBJ_ITEM", constant<StgStageScript::OBJ_ITEM>::func, 0},

	{"INFO_LIFE", constant<StgStageScript::INFO_LIFE>::func, 0},
	{"INFO_DAMAGE_RATE_SHOT", constant<StgStageScript::INFO_DAMAGE_RATE_SHOT>::func, 0},
	{"INFO_DAMAGE_RATE_SPELL", constant<StgStageScript::INFO_DAMAGE_RATE_SPELL>::func, 0},
	{"INFO_SHOT_HIT_COUNT", constant<StgStageScript::INFO_SHOT_HIT_COUNT>::func, 0},
	{"INFO_TIMER", constant<StgStageScript::INFO_TIMER>::func, 0},
	{"INFO_TIMERF", constant<StgStageScript::INFO_TIMERF>::func, 0},
	{"INFO_ORGTIMERF", constant<StgStageScript::INFO_ORGTIMERF>::func, 0},
	{"INFO_IS_SPELL", constant<StgStageScript::INFO_IS_SPELL>::func, 0},
	{"INFO_IS_LAST_SPELL", constant<StgStageScript::INFO_IS_LAST_SPELL>::func, 0},
	{"INFO_IS_DURABLE_SPELL", constant<StgStageScript::INFO_IS_DURABLE_SPELL>::func, 0},
	{"INFO_SPELL_SCORE", constant<StgStageScript::INFO_SPELL_SCORE>::func, 0},
	{"INFO_REMAIN_STEP_COUNT", constant<StgStageScript::INFO_REMAIN_STEP_COUNT>::func, 0},
	{"INFO_ACTIVE_STEP_LIFE_COUNT", constant<StgStageScript::INFO_ACTIVE_STEP_LIFE_COUNT>::func, 0},
	{"INFO_ACTIVE_STEP_TOTAL_MAX_LIFE", constant<StgStageScript::INFO_ACTIVE_STEP_TOTAL_MAX_LIFE>::func, 0},
	{"INFO_ACTIVE_STEP_TOTAL_LIFE", constant<StgStageScript::INFO_ACTIVE_STEP_TOTAL_LIFE>::func, 0},
	{"INFO_ACTIVE_STEP_LIFE_RATE_LIST", constant<StgStageScript::INFO_ACTIVE_STEP_LIFE_RATE_LIST>::func, 0},
	{"INFO_IS_LAST_STEP", constant<StgStageScript::INFO_IS_LAST_STEP>::func, 0},
	{"INFO_PLAYER_SHOOTDOWN_COUNT", constant<StgStageScript::INFO_PLAYER_SHOOTDOWN_COUNT>::func, 0},
	{"INFO_PLAYER_SPELL_COUNT", constant<StgStageScript::INFO_PLAYER_SPELL_COUNT>::func, 0},
	{"INFO_CURRENT_LIFE", constant<StgStageScript::INFO_CURRENT_LIFE>::func, 0},
	{"INFO_CURRENT_LIFE_MAX", constant<StgStageScript::INFO_CURRENT_LIFE_MAX>::func, 0},

	{"INFO_ITEM_SCORE", constant<StgStageScript::INFO_ITEM_SCORE>::func, 0},

	{"INFO_RECT", constant<StgStageScript::INFO_RECT>::func, 0},
	{"INFO_DELAY_COLOR", constant<StgStageScript::INFO_DELAY_COLOR>::func, 0},
	{"INFO_BLEND", constant<StgStageScript::INFO_BLEND>::func, 0},
	{"INFO_COLLISION", constant<StgStageScript::INFO_COLLISION>::func, 0},
	{"INFO_COLLISION_LIST", constant<StgStageScript::INFO_COLLISION_LIST>::func, 0},

	{"EV_REQUEST_LIFE", constant<StgStageScript::EV_REQUEST_LIFE>::func, 0},
	{"EV_REQUEST_TIMER", constant<StgStageScript::EV_REQUEST_TIMER>::func, 0},
	{"EV_REQUEST_IS_SPELL", constant<StgStageScript::EV_REQUEST_IS_SPELL>::func, 0},
	{"EV_REQUEST_IS_LAST_SPELL", constant<StgStageScript::EV_REQUEST_IS_LAST_SPELL>::func, 0},
	{"EV_REQUEST_IS_DURABLE_SPELL", constant<StgStageScript::EV_REQUEST_IS_DURABLE_SPELL>::func, 0},
	{"EV_REQUEST_SPELL_SCORE", constant<StgStageScript::EV_REQUEST_SPELL_SCORE>::func, 0},
	{"EV_REQUEST_REPLAY_TARGET_COMMON_AREA", constant<StgStageScript::EV_REQUEST_REPLAY_TARGET_COMMON_AREA>::func, 0},

	{"EV_TIMEOUT", constant<StgStageScript::EV_TIMEOUT>::func, 0},
	{"EV_START_BOSS_SPELL", constant<StgStageScript::EV_START_BOSS_SPELL>::func, 0},
	{"EV_GAIN_SPELL", constant<StgStageScript::EV_GAIN_SPELL>::func, 0},
	{"EV_START_BOSS_STEP", constant<StgStageScript::EV_START_BOSS_STEP>::func, 0},
	{"EV_END_BOSS_STEP", constant<StgStageScript::EV_END_BOSS_STEP>::func, 0},

	{"EV_PLAYER_SHOOTDOWN", constant<StgStageScript::EV_PLAYER_SHOOTDOWN>::func, 0},
	{"EV_PLAYER_SPELL", constant<StgStageScript::EV_PLAYER_SPELL>::func, 0},
	{"EV_PLAYER_REBIRTH", constant<StgStageScript::EV_PLAYER_REBIRTH>::func, 0},

	{"EV_PAUSE_ENTER", constant<StgStageScript::EV_PAUSE_ENTER>::func, 0},
	{"EV_PAUSE_LEAVE", constant<StgStageScript::EV_PAUSE_LEAVE>::func, 0},

	{"TARGET_ALL", constant<StgStageScript::TARGET_ALL>::func, 0},
	{"TARGET_ENEMY", constant<StgStageScript::TARGET_ENEMY>::func, 0},
	{"TARGET_PLAYER", constant<StgStageScript::TARGET_PLAYER>::func, 0},

	{"NO_CHANGE", constant<StgMovePattern::NO_CHANGE>::func, 0},
};
StgStageScript::StgStageScript(StgStageController* stageController) : StgControlScript(stageController->GetSystemController())
{
	stageController_ = stageController;

	typeScript_ = TYPE_STAGE;
	_AddFunction(stgFunction, sizeof(stgFunction) / sizeof(function));

	ref_count_ptr<StgStageInformation> info = stageController_->GetStageInformation();
	mt_ = info->GetMersenneTwister();

	scriptManager_ = stageController_->GetScriptManagerP();
	StgStageScriptManager* scriptManager = (StgStageScriptManager*)scriptManager_;
	SetObjectManager(scriptManager->GetObjectManager());
}
StgStageScript::~StgStageScript()
{
}
ref_count_ptr<StgStageScriptObjectManager> StgStageScript::GetStgObjectManager()
{
	StgStageScriptManager* scriptManager = (StgStageScriptManager*)scriptManager_;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = scriptManager->GetObjectManager();
	return objectManager;
}


//STG制御共通関数：共通データ
gstd::value StgStageScript::Func_SaveCommonDataAreaToReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	if(infoStage->IsReplay())
		script->RaiseError(L"call only in normal play (not replay)");

	std::string area = to_mbcs(argv[0].as_string());
	ref_count_ptr<ScriptCommonData> commonDataO = commonDataManager->GetData(area);
	if(commonDataO == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	ref_count_ptr<ScriptCommonData> commonDataS = new ScriptCommonData();
	commonDataS->Copy(commonDataO);
	replayStageData->SetCommonData(area, commonDataS);

	return value(machine->get_engine()->get_boolean_type(), true);
}
gstd::value StgStageScript::Func_LoadCommonDataAreaFromReplayFile(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();
	ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	if(!infoStage->IsReplay())
		script->RaiseError(L"call only in replay");

	std::string area = to_mbcs(argv[0].as_string());
	ref_count_ptr<ScriptCommonData> commonDataS = replayStageData->GetCommonData(area);
	if(commonDataS == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	ref_count_ptr<ScriptCommonData> commonDataO = new ScriptCommonData();
	commonDataO->Copy(commonDataS);
	commonDataManager->SetData(area, commonDataO);

	return value(machine->get_engine()->get_boolean_type(), true);
}

//STG共通関数：システム関連
gstd::value StgStageScript::Func_GetMainStgScriptPath(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<ScriptInformation> infoMain = stageController->GetStageInformation()->GetMainScriptInformation();

	std::wstring path = infoMain->GetScriptPath();
	path = PathProperty::GetUnique(path);

	return value(machine->get_engine()->get_string_type(), path);
}	
gstd::value StgStageScript::Func_GetMainStgScriptDirectory(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<ScriptInformation> infoMain = stageController->GetStageInformation()->GetMainScriptInformation();

	std::wstring path = infoMain->GetScriptPath();
	path = PathProperty::GetUnique(path);

	std::wstring dir = PathProperty::GetFileDirectory(path);

	return value(machine->get_engine()->get_string_type(), dir);
}
gstd::value StgStageScript::Func_SetStgFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	RECT rect;
	rect.left = (int)argv[0].as_real();
	rect.top = (int)argv[1].as_real();
	rect.right = (int)argv[2].as_real();
	rect.bottom = (int)argv[3].as_real();

	int min = (int)argv[4].as_real();
	int max = (int)argv[5].as_real();

	ref_count_ptr<StgStageInformation> stageInfo = stageController->GetStageInformation();
	stageInfo->SetStgFrameRect(rect);
	stageInfo->SetStgFrameMinPriority(min);
	stageInfo->SetStgFrameMaxPriority(max);

	return value();
}

gstd::value StgStageScript::Func_SetItemRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	int pri = (int)argv[0].as_real();
	//pri = min(pri, info->GetStgFrameMaxPriority());
	//pri = max(pri, info->GetStgFrameMinPriority());
	info->SetItemObjectPriority(pri);
	return value();
}
gstd::value StgStageScript::Func_SetShotRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	int pri = (int)argv[0].as_real();
	//pri = min(pri, info->GetStgFrameMaxPriority());
	//pri = max(pri, info->GetStgFrameMinPriority());
	info->SetShotObjectPriority(pri);
	return value();
}
gstd::value StgStageScript::Func_GetStgFrameRenderPriorityMinI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	long double res = stageController->GetStageInformation()->GetStgFrameMinPriority();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetStgFrameRenderPriorityMaxI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	long double res = stageController->GetStageInformation()->GetStgFrameMaxPriority();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetItemRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	long double res = stageController->GetStageInformation()->GetItemObjectPriority();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetShotRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	long double res = stageController->GetStageInformation()->GetShotObjectPriority();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetPlayerRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = script->GetStgObjectManager();
	int idObjPlayer = objectManager->GetPlayerObjectID();

	long double res = 30;
	StgPlayerObject* obj = dynamic_cast<StgPlayerObject*>(script->GetObjectPointer(idObjPlayer));
	if(obj != NULL)
	{
		double pri = obj->GetRenderPriority();
		int vacket = objectManager->GetRenderBucketCapacity();
		res = pri * (vacket - 1);		
	}
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetCameraFocusPermitPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	long double res = stageController->GetStageInformation()->GetCameraFocusPermitPriority();
	return value(machine->get_engine()->get_real_type(), res);
}

gstd::value StgStageScript::Func_CloseStgScene(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgSystemController* systemController = script->stageController_->GetSystemController();

	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> info = stageController->GetStageInformation();
	info->SetEnd();

	return value();
}
gstd::value StgStageScript::Func_GetReplayFps(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	int fps = 0;
	if(infoStage->IsReplay())
	{
		ref_count_ptr<ReplayInformation::StageData> replayStageData = infoStage->GetReplayData();
		int frame = infoStage->GetCurrentFrame();
		fps = replayStageData->GetFramePerSecond(frame);
	}

	return value(machine->get_engine()->get_real_type(), (long double)fps);
}

//STG共通関数：自機
gstd::value StgStageScript::Func_GetPlayerObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = script->GetStgObjectManager();
	long double res = objectManager->GetPlayerObjectID();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value StgStageScript::Func_GetPlayerScriptID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgStageScriptManager* scriptManager = stageController->GetScriptManagerP();

	_int64 res = scriptManager->GetPlayerScriptID();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_SetPlayerSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	double speedFast = argv[0].as_real();
	double speedSlow = argv[1].as_real();
	obj->SetFastSpeed(speedFast);
	obj->SetSlowSpeed(speedSlow);
	return value();
}
gstd::value StgStageScript::Func_SetPlayerClip(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = script->GetStgObjectManager();
	int idObjPlayer = objectManager->GetPlayerObjectID();

	StgPlayerObject* obj = dynamic_cast<StgPlayerObject*>(script->GetObjectPointer(idObjPlayer));
	if(obj == NULL)return value();

	RECT rect;
	rect.left = (int)argv[0].as_real();
	rect.top = (int)argv[1].as_real();
	rect.right = (int)argv[2].as_real();
	rect.bottom = (int)argv[3].as_real();
	obj->SetClip(rect);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerLife(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	double life = argv[0].as_real();
	obj->SetLife(life);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	double spell = argv[0].as_real();
	obj->SetSpell(spell);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerPower(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	double power = argv[0].as_real();
	obj->SetPower(power);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerInvincibilityFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	int invi = (int)argv[0].as_real();
	obj->SetInvincibilityFrame(invi);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerDownStateFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	int frame = (int)argv[0].as_real();
	obj->SetDownStateFrame(frame);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerRebirthFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	int frame = (int)argv[0].as_real();
	obj->SetRebirthFrame(frame);
	obj->SetRebirthFrameMax(frame);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerRebirthLossFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	int frame = (int)argv[0].as_real();
	obj->SetRebirthLossFrame(frame);

	return value();
}
gstd::value StgStageScript::Func_SetPlayerAutoItemCollectLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	int posY = (int)argv[0].as_real();
	obj->SetAutoItemCollectY(posY);

	return value();
}
gstd::value StgStageScript::Func_SetForbidPlayerShot(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	bool bForbid = argv[0].as_boolean();
	obj->SetForbidShot(bForbid);

	return value();
}
gstd::value StgStageScript::Func_SetForbidPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	if(obj == NULL)return value();

	bool bForbid = argv[0].as_boolean();
	obj->SetForbidSpell(bForbid);

	return value();
}
gstd::value StgStageScript::Func_GetPlayerX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetX() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetY() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetState() : StgPlayerObject::STATE_END;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();

	std::vector<long double> listValue;
	listValue.push_back(obj->GetFastSpeed());
	listValue.push_back(obj->GetSlowSpeed());

	gstd::value res = script->CreateRealArrayValue(listValue);
	return res;
}
gstd::value StgStageScript::Func_GetPlayerClip(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();

	RECT clip = obj->GetClip();
	std::vector<long double> listValue;
	listValue.push_back(clip.left);
	listValue.push_back(clip.top);
	listValue.push_back(clip.right);
	listValue.push_back(clip.bottom);

	gstd::value res = script->CreateRealArrayValue(listValue);
	return res;
}
gstd::value StgStageScript::Func_GetPlayerLife(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetLife() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetSpell() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerPower(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetPower() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerInvincibilityFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetInvincibilityFrame() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerDownStateFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetDownStateFrame() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetPlayerRebirthFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	double res = obj != NULL ? obj->GetRebirthFrame() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetAngleToPlayer(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
	if(objPlayer == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	double px = objPlayer->GetPositionX();
	double py = objPlayer->GetPositionY();

	int id = (int)argv[0].as_real();
	ref_count_ptr<DxScriptRenderObject>::unsync objMove = 
		ref_count_ptr<DxScriptRenderObject>::unsync::DownCast(script->GetObject(id));
	if(objMove == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	double tx = objMove->GetPosition().x;
	double ty = objMove->GetPosition().y;

	long double angle = atan2(py-ty, px-tx) * 180 / PAI;
	return value(machine->get_engine()->get_real_type(), (long double)angle);
}

gstd::value StgStageScript::Func_IsPermitPlayerShot(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	bool res = obj != NULL ? obj->IsPermitShot() : false;
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_IsPermitPlayerSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	bool res = obj != NULL ? obj->IsPermitSpell() : false;
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_IsPlayerLastSpellWait(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	bool res = obj != NULL ? obj->IsWaitLastSpell() : false;
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_IsPlayerSpellActive(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	bool res = false;
	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
	if(objPlayer != NULL)
	{
		ref_count_ptr<StgPlayerSpellManageObject>::unsync objSpell = objPlayer->GetSpellManageObject();
		res = (objSpell != NULL && !objSpell->IsDeleted());
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}


//STG共通関数：敵
gstd::value StgStageScript::Func_GetEnemyBossSceneObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();
	
	int res = ID_INVALID;
	ref_count_ptr<StgEnemyBossSceneObject>::unsync obj = enemyManager->GetBossSceneObject();
	if(obj != NULL && !obj->IsDeleted())
		res = obj->GetObjectID();

	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value StgStageScript::Func_GetEnemyBossObjectID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();
	ref_count_ptr<StgEnemyBossSceneObject>::unsync scene = enemyManager->GetBossSceneObject();

	std::vector<long double> listLD;
	if(scene != NULL)
	{
		ref_count_ptr<StgEnemyBossSceneData>::unsync data = scene->GetActiveData();
		if(data != NULL)
		{
			std::vector<ref_count_ptr<StgEnemyBossObject>::unsync > listEnemy = data->GetEnemyObjectList();
			for(int iEnemy = 0 ; iEnemy < listEnemy.size() ; iEnemy++)
			{
				ref_count_ptr<StgEnemyBossObject>::unsync obj = listEnemy[iEnemy];
				if(obj->IsDeleted())continue;
				int id = obj->GetObjectID();
				listLD.push_back(id);
			}
		}
	}

	return script->CreateRealArrayValue(listLD);
}
gstd::value StgStageScript::Func_GetAllEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	std::list<ref_count_ptr<StgEnemyObject>::unsync >& listEnemy = enemyManager->GetEnemyList();

	std::vector<long double> listLD;
	std::list<ref_count_ptr<StgEnemyObject>::unsync >::iterator itr = listEnemy.begin();
	for(; itr != listEnemy.end() ; itr++)
	{
		ref_count_ptr<StgEnemyObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;
		int id = obj->GetObjectID();
		listLD.push_back(id);
	}

	return script->CreateRealArrayValue(listLD);
}
gstd::value StgStageScript::Func_GetIntersectionRegistedEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();

	std::vector<long double> listLD;
	std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
	for(int iPoint = 0 ; iPoint < listPoint->size() ; iPoint++)
	{
		StgIntersectionTargetPoint& target = listPoint->at(iPoint);
		int id = target.GetObjectID();
		listLD.push_back(id);
	}

	return script->CreateRealArrayValue(listLD);
}
gstd::value StgStageScript::Func_GetAllEnemyIntersectionPosition(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();

	std::vector<gstd::value> listV;
	std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
	for(int iPoint = 0 ; iPoint < listPoint->size() ; iPoint++)
	{
		StgIntersectionTargetPoint& target = listPoint->at(iPoint);
		POINT pos = target.GetPoint();
		std::vector<long double> listLD;
		listLD.push_back(pos.x);
		listLD.push_back(pos.y);
		gstd::value v = script->CreateRealArrayValue(listLD);
		listV.push_back(v);
	}
	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPosition(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	struct SortDistance
	{
		static std::vector<POINT> Sort(int posX, int posY, std::vector<_int64>& listDist, std::vector<POINT>& listRes)
		{
			std::sort(listDist.begin(), listDist.end());
			std::vector<POINT> listResCopy = listRes;
			std::vector<_int64> listDistCopy = listDist;

			for(int iRes = 0 ; iRes < listResCopy.size(); iRes++)
			{
				POINT& pos = listResCopy[iRes];
				_int64 dist = (pos.x - posX) * (pos.x - posX) + (pos.y - posY) * (pos.y - posY);
				for(int iDist = 0 ;iDist < listDistCopy.size() ; iDist++)
				{
					if(dist == listDistCopy[iDist])
					{
						listRes[iDist] = pos;
						listDistCopy[iDist] = -1;
						break;
					}
				}
			}

			return listRes;
		};
	};

	int posX = (int)argv[0].as_real();
	int posY = (int)argv[1].as_real();
	int countRes = (int)argv[2].as_real();

	std::vector<StgIntersectionTargetPoint>* listPoint = intersectionManager->GetAllEnemyTargetPoint();
	std::vector<POINT> listRes;
	std::vector<_int64> listDist;
	int iPoint = 0;
	for(iPoint = 0; iPoint < listPoint->size() && countRes > 0; iPoint++)
	{
		StgIntersectionTargetPoint& target = listPoint->at(iPoint);
		POINT pos = target.GetPoint();
		if(listRes.size() < countRes)
		{
			listRes.push_back(pos);
			listDist.push_back((pos.x - posX) * (pos.x - posX) + (pos.y - posY) * (pos.y - posY));

			if(listRes.size() == countRes)
			{
				listRes = SortDistance::Sort(posX, posY, listDist, listRes);
			}
		}
		else
		{
			_int64 dist = (pos.x - posX) * (pos.x - posX) + (pos.y - posY) * (pos.y - posY);
			_int64 target = listDist[listDist.size() - 1];
			if(dist >= target)continue;
			
			for(int iDist = 0 ;iDist < listDist.size() ; iDist++)
			{
				if(dist < listDist[iDist])
				{
					listRes.insert(listRes.begin() + iDist, pos);
					listRes.pop_back();
					listDist.insert(listDist.begin() + iDist, dist);
					listDist.pop_back();
				}
			}
		}
	}

	std::vector<gstd::value> listV;
	listRes = SortDistance::Sort(posX, posY, listDist, listRes);
	for(iPoint = 0 ; iPoint < listRes.size() ; iPoint++)
	{
		POINT& pos = listRes[iPoint];
		std::vector<long double> listLD;
		listLD.push_back(pos.x);
		listLD.push_back(pos.y);
		gstd::value v = script->CreateRealArrayValue(listLD);
		listV.push_back(v);
	}
	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPositionByIdA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	//引数1（敵オブジェクトID）自機からもアクセス可能
	//指定した敵オブジェクトIDが持つ自機ショットへの当たり判定位置を全て取得
	//二次元配列が返る。([<インデックス>][<0:x座標, 1:y座標>])　配列の0番目が最も敵本体の座標に近い
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));

	std::vector<gstd::value> listV;
	if(obj != NULL)
	{
		std::map<_int64, POINT> mapPos;
		int enemyX = obj->GetPositionX();
		int enemyY = obj->GetPositionY();
		StgStageController* stageController = script->stageController_;
		StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();
		std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
		for(int iPoint = 0 ; iPoint < listPoint->size() ; iPoint++)
		{
			StgIntersectionTargetPoint& target = listPoint->at(iPoint);
			if(target.GetObjectID() != id)continue;

			POINT pos = target.GetPoint();
			_int64 dist = (pos.x - enemyX) * (pos.x - enemyX) + (pos.y - enemyY) * (pos.y - enemyY);
			mapPos[dist] = pos;
		}

		std::map<_int64, POINT>::iterator itr = mapPos.begin();
		for(; itr != mapPos.end(); itr++)
		{
			POINT pos = (itr->second);
			std::vector<long double> listLD;
			listLD.push_back(pos.x);
			listLD.push_back(pos.y);
			gstd::value v = script->CreateRealArrayValue(listLD);
			listV.push_back(v);
		}
	}

	return script->CreateValueArrayValue(listV);
}
gstd::value StgStageScript::Func_GetEnemyIntersectionPositionByIdA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	//引数3（敵オブジェクトID・x座標・y座標）自機からもアクセス可能
	//指定した敵オブジェクトIDが持つ、自機ショットへの当たり判定のうち、指定座標に最も近い1つを取得
	//配列が返る。([<0:x座標, 1:y座標>])

	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));

	std::vector<gstd::value> listV;
	if(obj != NULL)
	{
		std::map<_int64, POINT> mapPos;
		int tX = (int)argv[1].as_real();
		int tY = (int)argv[2].as_real();
		StgStageController* stageController = script->stageController_;
		StgIntersectionManager* interSectionManager = stageController->GetIntersectionManager();
		std::vector<StgIntersectionTargetPoint>* listPoint = interSectionManager->GetAllEnemyTargetPoint();
		for(int iPoint = 0 ; iPoint < listPoint->size() ; iPoint++)
		{
			StgIntersectionTargetPoint& target = listPoint->at(iPoint);
			if(target.GetObjectID() != id)continue;

			POINT pos = target.GetPoint();
			_int64 dist = (pos.x - tX) * (pos.x - tX) + (pos.y - tY) * (pos.y - tY);
			mapPos[dist] = pos;
		}

		std::map<_int64, POINT>::iterator itr = mapPos.begin();
		for(; itr != mapPos.end(); itr++)
		{
			POINT pos = (itr->second);
			std::vector<long double> listLD;
			listLD.push_back(pos.x);
			listLD.push_back(pos.y);
			gstd::value v = script->CreateRealArrayValue(listLD);
			listV.push_back(v);
		}
	}

	return script->CreateValueArrayValue(listV);
}

gstd::value StgStageScript::Func_LoadEnemyShotData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadEnemyShotData(path);

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_ReloadEnemyShotData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadEnemyShotData(path, true);

	return value(machine->get_engine()->get_boolean_type(), res);
}

//STG共通関数：弾
gstd::value StgStageScript::Func_DeleteShotAll(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeDel = (int)argv[0].as_real();
	int typeTo = (int)argv[1].as_real();

	switch(typeDel)
	{
	case TYPE_ALL:typeDel = StgShotManager::DEL_TYPE_ALL;break;
	case TYPE_SHOT:typeDel = StgShotManager::DEL_TYPE_SHOT;break;
	case TYPE_CHILD:typeDel = StgShotManager::DEL_TYPE_CHILD;break;
	}

	switch(typeTo)
	{
	case TYPE_IMMEDIATE:typeTo = StgShotManager::TO_TYPE_IMMEDIATE;break;
	case TYPE_FADE:typeTo = StgShotManager::TO_TYPE_FADE;break;
	case TYPE_ITEM:typeTo = StgShotManager::TO_TYPE_ITEM;break;
	}

	stageController->GetShotManager()->DeleteInCircle(typeDel, typeTo, StgShotObject::OWNER_ENEMY, 0, 0, 256*256);

	return value();
}
gstd::value StgStageScript::Func_DeleteShotInCircle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeDel = (int)argv[0].as_real();
	int typeTo = (int)argv[1].as_real();
	int posX = (int)argv[2].as_real();
	int posY = (int)argv[3].as_real();
	double radius = argv[4].as_real();

	switch(typeDel)
	{
	case TYPE_ALL:typeDel = StgShotManager::DEL_TYPE_ALL;break;
	case TYPE_SHOT:typeDel = StgShotManager::DEL_TYPE_SHOT;break;
	case TYPE_CHILD:typeDel = StgShotManager::DEL_TYPE_CHILD;break;
	}

	switch(typeTo)
	{
	case TYPE_IMMEDIATE:typeTo = StgShotManager::TO_TYPE_IMMEDIATE;break;
	case TYPE_FADE:typeTo = StgShotManager::TO_TYPE_FADE;break;
	case TYPE_ITEM:typeTo = StgShotManager::TO_TYPE_ITEM;break;
	}

	stageController->GetShotManager()->DeleteInCircle(typeDel, typeTo, StgShotObject::OWNER_ENEMY, posX, posY, radius);

	return value();
}
gstd::value StgStageScript::Func_CreateShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speed = argv[2].as_real();
		long double angle = argv[3].as_real();
		int idShot = (int)argv[4].as_real();
		int delay = (int)argv[5].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateShotA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speed = argv[2].as_real();
		long double angle = argv[3].as_real();
		long double accele = argv[4].as_real();
		long double maxSpeed = argv[5].as_real();
		int idShot = (int)argv[6].as_real();
		int delay = (int)argv[7].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);

		StgMoveObject* objMove = (StgMoveObject*)obj.GetPointer();
		StgMovePattern_Angle* pattern = (StgMovePattern_Angle*)objMove->GetPattern().GetPointer();
		pattern->SetAcceleration(accele);
		pattern->SetMaxSpeed(maxSpeed);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateShotOA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int tId = (int)argv[0].as_real();
	DxScriptRenderObject* tObj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(tId));
	if(tObj == NULL)
		return value(machine->get_engine()->get_real_type(),(long double)ID_INVALID);

	double posX = tObj->GetPosition().x;
	double posY = tObj->GetPosition().y;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double speed = argv[1].as_real();
		long double angle = argv[2].as_real();
		int idShot = (int)argv[3].as_real();
		int delay = (int)argv[4].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateShotB1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speedX = argv[2].as_real();
		long double speedY = argv[3].as_real();
		int idShot = (int)argv[4].as_real();
		int delay = (int)argv[5].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);

		ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj.GetPointer());
		pattern->SetSpeedX(speedX);
		pattern->SetSpeedY(speedY);
		obj->SetPattern(pattern);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateShotB2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speedX = argv[2].as_real();
		long double speedY = argv[3].as_real();
		long double accelX = argv[4].as_real();
		long double accelY = argv[5].as_real();
		long double maxSpeedX = argv[6].as_real();
		long double maxSpeedY = argv[7].as_real();
		int idShot = (int)argv[8].as_real();
		int delay = (int)argv[9].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);

		ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj.GetPointer());
		pattern->SetSpeedX(speedX);
		pattern->SetSpeedY(speedY);
		pattern->SetAccelerationX(accelX);
		pattern->SetAccelerationY(accelY);
		pattern->SetMaxSpeedX(maxSpeedX);
		pattern->SetMaxSpeedY(maxSpeedY);
		obj->SetPattern(pattern);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateShotOB1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int tId = (int)argv[0].as_real();
	DxScriptRenderObject* tObj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(tId));
	if(tObj == NULL)
		return value(machine->get_engine()->get_real_type(),(long double)ID_INVALID);

	double posX = tObj->GetPosition().x;
	double posY = tObj->GetPosition().y;

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double speedX = argv[1].as_real();
		long double speedY = argv[2].as_real();
		int idShot = (int)argv[3].as_real();
		int delay = (int)argv[4].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);

		ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj.GetPointer());
		pattern->SetSpeedX(speedX);
		pattern->SetSpeedY(speedY);
		obj->SetPattern(pattern);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}

gstd::value StgStageScript::Func_CreateLooseLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgLooseLaserObject>::unsync obj = new StgLooseLaserObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speed = argv[2].as_real();
		long double angle = argv[3].as_real();
		int length = argv[4].as_real();
		int width = argv[5].as_real();
		int idShot = (int)argv[6].as_real();
		int delay = (int)argv[7].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetLength(length);
		obj->SetRenderWidth(width);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}

gstd::value StgStageScript::Func_CreateStraightLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgStraightLaserObject>::unsync obj = new StgStraightLaserObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double angle = argv[2].as_real();
		int length = argv[3].as_real();
		int width = argv[4].as_real();
		int deleteFrame = (int)argv[5].as_real();
		int idShot = (int)argv[6].as_real();
		int delay = (int)argv[7].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetLaserAngle(angle);
		obj->SetLength(length);
		obj->SetRenderWidth(width);
		obj->SetAutoDeleteFrame(deleteFrame);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateCurveLaserA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgCurveLaserObject>::unsync obj = new StgCurveLaserObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speed = argv[2].as_real();
		long double angle = argv[3].as_real();
		int length = argv[4].as_real();
		int width = argv[5].as_real();
		int idShot = (int)argv[6].as_real();
		int delay = (int)argv[7].as_real();
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;

		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetLength(length);
		obj->SetRenderWidth(width);
		obj->SetShotDataID(idShot);
		obj->SetDelay(delay);
		obj->SetOwnerType(typeOwner);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}

gstd::value StgStageScript::Func_SetShotIntersectionCircle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeTarget = script->GetScriptType() == TYPE_PLAYER ? 
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px = (int)argv[0].as_real();
	int py = (int)argv[1].as_real();
	int radius = (int)argv[2].as_real();
	DxCircle circle(px, py, radius);

	//当たり判定
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(typeTarget);
	target->SetCircle(circle);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStageScript::Func_SetShotIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int typeTarget = script->GetScriptType() == TYPE_PLAYER ? 
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px1 = (int)argv[0].as_real();
	int py1 = (int)argv[1].as_real();
	int px2 = (int)argv[2].as_real();
	int py2 = (int)argv[3].as_real();
	int width = (int)argv[4].as_real();
	DxWidthLine line(px1, py1, px2, py2, width);

	//当たり判定
	ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
	target->SetTargetType(typeTarget);
	target->SetLine(line);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStageScript::Func_GetShotIdInCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgShotManager* shotManager = stageController->GetShotManager();
	int px = (int)argv[0].as_real();
	int py = (int)argv[1].as_real();
	int radius = (int)argv[2].as_real();
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ? StgShotObject::OWNER_ENEMY : StgShotObject::OWNER_PLAYER;

	std::vector<int> listID = shotManager->GetShotIdInCircle(typeOwner, px, py, radius);
	std::vector<long double> listRes;
	for(int iID = 0 ; iID < listID.size() ; iID++)
		listRes.push_back(listID[iID]);
	gstd::value res = script->CreateRealArrayValue(listRes);

	return res;
}
gstd::value StgStageScript::Func_GetShotIdInCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	StgShotManager* shotManager = stageController->GetShotManager();
	int px = (int)argv[0].as_real();
	int py = (int)argv[1].as_real();
	int radius = (int)argv[2].as_real();
	int target = (int)argv[3].as_real();

	int typeOwner = StgShotObject::OWNER_NULL;
	switch(target)
	{
	case TARGET_ALL:typeOwner = StgShotObject::OWNER_NULL;break;
	case TARGET_PLAYER:typeOwner = StgShotObject::OWNER_PLAYER;break;
	case TARGET_ENEMY:typeOwner = StgShotObject::OWNER_ENEMY;break;
	}

	std::vector<int> listID = shotManager->GetShotIdInCircle(typeOwner, px, py, radius);
	std::vector<long double> listRes;
	for(int iID = 0 ; iID < listID.size() ; iID++)
		listRes.push_back(listID[iID]);
	gstd::value res = script->CreateRealArrayValue(listRes);

	return res;
}
gstd::value StgStageScript::Func_GetShotCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	int target = (int)argv[0].as_real();
	int typeOwner = StgShotObject::OWNER_NULL;
	switch(target)
	{
	case TARGET_ALL:typeOwner = StgShotObject::OWNER_NULL;break;
	case TARGET_PLAYER:typeOwner = StgShotObject::OWNER_PLAYER;break;
	case TARGET_ENEMY:typeOwner = StgShotObject::OWNER_ENEMY;break;
	}
	
	int res = shotManager->GetShotCount(typeOwner);
	return value(machine->get_engine()->get_real_type(),(long double)res);
}
gstd::value StgStageScript::Func_SetShotAutoDeleteClip(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	RECT rect;
	rect.left = -(int)argv[0].as_real();
	rect.top = -(int)argv[1].as_real();
	rect.right = (int)argv[2].as_real();
	rect.bottom = (int)argv[3].as_real();
	infoStage->SetShotAutoDeleteClip(rect);

	return value();
}
gstd::value StgStageScript::Func_GetShotDataInfoA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	ref_count_ptr<StgStageInformation> infoStage = stageController->GetStageInformation();

	int idShot = (int)argv[0].as_real();
	int target = (int)argv[1].as_real();
	int type = (int)argv[2].as_real();
	
	StgShotManager* shotManager = stageController->GetShotManager();
	StgShotDataList* dataList = (target == TARGET_PLAYER) ?
		shotManager->GetPlayerShotDataList() : shotManager->GetEnemyShotDataList();

	ref_count_ptr<StgShotData>::unsync shotData = NULL;
	if(dataList != NULL)
		shotData = dataList->GetData(idShot);

	if(shotData == NULL)
		script->RaiseError(ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_OUTOFRANGE_INDEX));

	gstd::value res;
	switch(type)
	{
		case INFO_RECT:
		{
			RECT rect = shotData->GetRect(0);
			std::vector<long double> list;
			list.push_back(rect.left);
			list.push_back(rect.top);
			list.push_back(rect.right);
			list.push_back(rect.bottom);
			res = script->CreateRealArrayValue(list);
			break;
		}

		case INFO_DELAY_COLOR:
		{
			D3DCOLOR color = shotData->GetDelayColor();
			int colorR = ColorAccess::GetColorR(color);
			int colorG = ColorAccess::GetColorG(color);
			int colorB = ColorAccess::GetColorB(color);

			std::vector<long double> list;
			list.push_back(colorR);
			list.push_back(colorG);
			list.push_back(colorB);
			res = script->CreateRealArrayValue(list);
			break;
		}

		case INFO_BLEND:
		{
			int blendType = shotData->GetRenderType();
			res = value(machine->get_engine()->get_real_type(),(long double)blendType);
			break;
		}

		case INFO_COLLISION:
		{
			double radius = 0;
			std::vector<DxCircle>* listCircle = shotData->GetIntersectionCircleList();
			if(listCircle->size() > 0)
			{
				DxCircle circle = listCircle->at(0);
				radius = circle.GetR();
			}
			res = value(machine->get_engine()->get_real_type(),(long double)radius);
			break;
		}

		case INFO_COLLISION_LIST:
		{
			std::vector<DxCircle>* listCircle = shotData->GetIntersectionCircleList();
			std::vector<gstd::value> listValue;
			for(int iCircle = 0 ; iCircle < listCircle->size() ; iCircle++)
			{
				DxCircle circle = listCircle->at(iCircle);
				std::vector<long double> list;
				list.push_back(circle.GetR());
				list.push_back(circle.GetX());
				list.push_back(circle.GetY());
				gstd::value val = script->CreateRealArrayValue(list);
				listValue.push_back(val);
			}
			res = script->CreateValueArrayValue(listValue);
			break;
		}
	}

	return res;
}
gstd::value StgStageScript::Func_StartShotScript(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgStageScriptManager* scriptManager = stageController->GetScriptManagerP();

	if(scriptManager->GetShotScriptID() != StgControlScriptManager::ID_INVALID)
		script->RaiseError(L"already started shot script");

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	int type = script->GetScriptType();
	_int64 idScript = scriptManager->LoadScript(path, StgStageScript::TYPE_SHOT);
	scriptManager->StartScript(idScript);
	scriptManager->SetShotScriptID(idScript);
	return value();
}

//STG共通関数：アイテム
gstd::value StgStageScript::Func_CreateItemA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int type = (int)argv[0].as_real();
	ref_count_ptr<StgItemObject>::unsync obj = itemManager->CreateItem(type);
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		itemManager->AddItem(obj);
		int posX = (int)argv[1].as_real();
		int posY = (int)argv[2].as_real();
		int score = (int)argv[3].as_real();
		POINT to = {posX, posY - 128};
		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateItemA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int type = (int)argv[0].as_real();
	ref_count_ptr<StgItemObject>::unsync obj = itemManager->CreateItem(type);
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		itemManager->AddItem(obj);
		int posX = (int)argv[1].as_real();
		int posY = (int)argv[2].as_real();
		int score = (int)argv[5].as_real();
		POINT to = {(int)argv[3].as_real(), (int)argv[4].as_real()};
		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateItemU1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int type = StgItemObject::ITEM_USER;
	ref_count_ptr<StgItemObject_User>::unsync obj = 
		ref_count_ptr<StgItemObject_User>::unsync::DownCast(itemManager->CreateItem(type));
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		itemManager->AddItem(obj);
		int itemID = (int)argv[0].as_real();
		int posX = (int)argv[1].as_real();
		int posY = (int)argv[2].as_real();
		int score = (int)argv[3].as_real();
		POINT to = {posX, posY - 128};

		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
		obj->SetImageID(itemID);
		obj->SetMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateItemU2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int type = StgItemObject::ITEM_USER;
	ref_count_ptr<StgItemObject_User>::unsync obj = 
		ref_count_ptr<StgItemObject_User>::unsync::DownCast(itemManager->CreateItem(type));
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		itemManager->AddItem(obj);
		int itemID = (int)argv[0].as_real();
		int posX = (int)argv[1].as_real();
		int posY = (int)argv[2].as_real();
		int score = (int)argv[5].as_real();
		POINT to = {(int)argv[3].as_real(), (int)argv[4].as_real()};
		obj->SetPositionX(posX);
		obj->SetPositionY(posY);
		obj->SetScore(score);
		obj->SetToPosition(to);
		obj->SetImageID(itemID);
		obj->SetMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CreateItemScore(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	long double score = argv[0].as_real();
	int posX = (int)argv[1].as_real();
	int posY = (int)argv[2].as_real();

	ref_count_ptr<StgItemObject_Score>::unsync obj = new StgItemObject_Score(stageController);
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		itemManager->AddItem(obj);
		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetScore(score);
	}

	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStageScript::Func_CollectAllItems(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();
	itemManager->CollectItemsAll();

	return value();
}
gstd::value StgStageScript::Func_CollectItemsByType(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int type = (int)argv[0].as_real();
	itemManager->CollectItemsByType(type);
	return value();
}
gstd::value StgStageScript::Func_CollectItemsInCircle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	int cx = (int)argv[0].as_real();
	int cy = (int)argv[1].as_real();
	int cr = (int)argv[2].as_real();
	DxCircle circle(cx, cy ,cr);
	itemManager->CollectItemsInCircle(circle);
	return value();
}
gstd::value StgStageScript::Func_CancelCollectItems(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	itemManager->CancelCollectItems();
	return value();
}
gstd::value StgStageScript::Func_StartItemScript(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgStageScriptManager* scriptManager = stageController->GetScriptManagerP();

	if(scriptManager->GetItemScriptID() != StgControlScriptManager::ID_INVALID)
		script->RaiseError(L"already started item script");

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	int type = script->GetScriptType();
	_int64 idScript = scriptManager->LoadScript(path, StgStageScript::TYPE_ITEM);
	scriptManager->StartScript(idScript);
	scriptManager->SetItemScriptID(idScript);
	return value();
}
gstd::value StgStageScript::Func_SetDefaultBonusItemEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	bool bEnable = argv[0].as_boolean();
	itemManager->SetDefaultBonusItemEnable(bEnable);
	return value();
}
gstd::value StgStageScript::Func_LoadItemData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = itemManager->LoadItemData(path);
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_ReloadItemData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgItemManager* itemManager = stageController->GetItemManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = itemManager->LoadItemData(path, true);
	return value(machine->get_engine()->get_boolean_type(), res);
}

//STG共通関数：その他
gstd::value StgStageScript::Func_StartSlow(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int target = (int)argv[0].as_real();
	int fps = (int)argv[1].as_real();

	int slowTarget = PseudoSlowInformation::TARGET_ALL;
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
		PseudoSlowInformation::OWNER_PLAYER : PseudoSlowInformation::OWNER_ENEMY;

	ref_count_ptr<PseudoSlowInformation> infoSlow = stageController->GetSlowInformation();
	infoSlow->AddSlow(fps, typeOwner, slowTarget);

	return value();
}
gstd::value StgStageScript::Func_StopSlow(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int target = (int)argv[0].as_real();

	int slowTarget = PseudoSlowInformation::TARGET_ALL;
	int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
		PseudoSlowInformation::OWNER_PLAYER : PseudoSlowInformation::OWNER_ENEMY;

	ref_count_ptr<PseudoSlowInformation> infoSlow = stageController->GetSlowInformation();
	infoSlow->RemoveSlow(typeOwner, slowTarget);

	return value();
}
gstd::value StgStageScript::Func_IsIntersected_Line_Circle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxWidthLine line(
		argv[0].as_real(),
		argv[1].as_real(),
		argv[2].as_real(),
		argv[3].as_real(),
		argv[4].as_real()
	);

	DxCircle circle(
		argv[5].as_real(),
		argv[6].as_real(),
		argv[7].as_real()
	);
	
	bool res = DxMath::IsIntersected(circle, line);
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_IsIntersected_Obj_Obj(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id1 = (int)argv[0].as_real();
	int id2 = (int)argv[1].as_real();

	StgShotObject* obj1 = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id1));
	if(obj1 == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	StgShotObject* obj2 = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id2));
	if(obj2 == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listTaget1 =
		obj1->GetIntersectionTargetList();
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listTaget2 =
		obj2->GetIntersectionTargetList();

	bool res = false;
	for(int iTarget1 = 0 ; iTarget1 < listTaget1.size() && !res; iTarget1++)
	{
		for(int iTarget2 = 0 ; iTarget2 < listTaget2.size() && !res ; iTarget2++)
		{
			ref_count_ptr<StgIntersectionTarget>::unsync target1 = listTaget1[iTarget1];
			ref_count_ptr<StgIntersectionTarget>::unsync target2 = listTaget2[iTarget2];
			res = StgIntersectionManager::IsIntersected(target1, target2);
		}
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}


//STD共通関数：移動オブジェクト操作
gstd::value StgStageScript::Func_ObjMove_SetX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double pos = argv[1].as_real();
	obj->SetPositionX(pos);

	DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(objR == NULL)return value();
	objR->SetX(pos);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double pos = argv[1].as_real();
	obj->SetPositionY(pos);

	DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(objR == NULL)return value();
	objR->SetY(pos);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetPosition(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double posX = argv[1].as_real();
	double posY = argv[2].as_real();
	obj->SetPositionX(posX);
	obj->SetPositionY(posY);

	DxScriptRenderObject* objR = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(objR == NULL)return value();
	objR->SetX(posX);
	objR->SetY(posY);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double speed = argv[1].as_real();
	obj->SetSpeed(speed);
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double angle = argv[1].as_real();
	obj->SetDirectionAngle(angle);
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAcceleration(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = ref_count_ptr<StgMovePattern_Angle>::unsync::DownCast(obj->GetPattern());
	if(pattern == NULL)
	{
		pattern = new StgMovePattern_Angle(obj);
		obj->SetPattern(pattern);
	}

	double param = argv[1].as_real();
	pattern->SetAcceleration(param);
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetAngularVelocity(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = 
		ref_count_ptr<StgMovePattern_Angle>::unsync::DownCast(obj->GetPattern());
	if(pattern == NULL)
	{
		pattern = new StgMovePattern_Angle(obj);
		obj->SetPattern(pattern);
	}

	double param = argv[1].as_real();
	pattern->SetAngularVelocity(param);
	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetMaxSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<StgMovePattern_Angle>::unsync pattern =
		ref_count_ptr<StgMovePattern_Angle>::unsync::DownCast(obj->GetPattern());
	if(pattern == NULL)
	{
		pattern = new StgMovePattern_Angle(obj);
		obj->SetPattern(pattern);
	}

	double param = argv[1].as_real();
	pattern->SetMaxSpeed(param);
	return value();
}

gstd::value StgStageScript::Func_ObjMove_SetDestAtSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double tx = argv[1].as_real();
	double ty = argv[2].as_real();
	double speed = argv[3].as_real();
	
	ref_count_ptr<StgMovePattern_Line>::unsync pattern = new StgMovePattern_Line(obj);
	pattern->SetAtSpeed(tx, ty, speed);
	obj->SetPattern(pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetDestAtFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double tx = argv[1].as_real();
	double ty = argv[2].as_real();
	int frame = (int)argv[3].as_real();
	
	ref_count_ptr<StgMovePattern_Line>::unsync pattern = new StgMovePattern_Line(obj);
	pattern->SetAtFrame(tx, ty, frame);
	obj->SetPattern(pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_SetDestAtWeight(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double tx = argv[1].as_real();
	double ty = argv[2].as_real();
	double weight = argv[3].as_real();
	double maxSpeed = argv[4].as_real();
	
	ref_count_ptr<StgMovePattern_Line>::unsync pattern = new StgMovePattern_Line(obj);
	pattern->SetAtWait(tx, ty, weight, maxSpeed);
	obj->SetPattern(pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speed = argv[2].as_real();
	double angle = argv[3].as_real();
	
	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = new StgMovePattern_Angle(obj);
	pattern->SetSpeed(speed);
	pattern->SetDirectionAngle(angle);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speed = argv[2].as_real();
	double angle = argv[3].as_real();
	double accele = argv[4].as_real();
	double angV = argv[5].as_real();
	double maxSpeed = argv[6].as_real();

	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = new StgMovePattern_Angle(obj);
	pattern->SetSpeed(speed);
	pattern->SetDirectionAngle(angle);
	pattern->SetAcceleration(accele);
	pattern->SetAngularVelocity(angV);
	pattern->SetMaxSpeed(maxSpeed);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA3(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speed = argv[2].as_real();
	double angle = argv[3].as_real();
	double accele = argv[4].as_real();
	double angV = argv[5].as_real();
	double maxSpeed = argv[6].as_real();
	int idShot = (int)argv[7].as_real();

	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = new StgMovePattern_Angle(obj);
	pattern->SetSpeed(speed);
	pattern->SetDirectionAngle(angle);
	pattern->SetAcceleration(accele);
	pattern->SetAngularVelocity(angV);
	pattern->SetMaxSpeed(maxSpeed);
	pattern->SetShotDataID(idShot);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternA4(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speed = argv[2].as_real();
	double angle = argv[3].as_real();
	double accele = argv[4].as_real();
	double angV = argv[5].as_real();
	double maxSpeed = argv[6].as_real();
	int idRelative = (int)argv[7].as_real();
	int idShot = (int)argv[8].as_real();

	ref_count_ptr<StgMovePattern_Angle>::unsync pattern = new StgMovePattern_Angle(obj);
	pattern->SetSpeed(speed);
	pattern->SetDirectionAngle(angle);
	pattern->SetAcceleration(accele);
	pattern->SetAngularVelocity(angV);
	pattern->SetMaxSpeed(maxSpeed);
	pattern->SetShotDataID(idShot);
	pattern->SetRelativeObjectID(idRelative);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speedX = argv[2].as_real();
	double speedY = argv[3].as_real();

	ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj);
	pattern->SetSpeedX(speedX);
	pattern->SetSpeedY(speedY);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speedX = argv[2].as_real();
	double speedY = argv[3].as_real();
	double accelX = argv[4].as_real();
	double accelY = argv[5].as_real();
	double maxSpeedX = argv[6].as_real();
	double maxSpeedY = argv[7].as_real();

	ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj);
	pattern->SetSpeedX(speedX);
	pattern->SetSpeedY(speedY);
	pattern->SetAccelerationX(accelX);
	pattern->SetAccelerationY(accelY);
	pattern->SetMaxSpeedX(maxSpeedX);
	pattern->SetMaxSpeedY(maxSpeedY);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_AddPatternB3(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	double speedX = argv[2].as_real();
	double speedY = argv[3].as_real();
	double accelX = argv[4].as_real();
	double accelY = argv[5].as_real();
	double maxSpeedX = argv[6].as_real();
	double maxSpeedY = argv[7].as_real();
	int idShot = (int)argv[8].as_real();

	ref_count_ptr<StgMovePattern_XY>::unsync pattern = new StgMovePattern_XY(obj);
	pattern->SetSpeedX(speedX);
	pattern->SetSpeedY(speedY);
	pattern->SetAccelerationX(accelX);
	pattern->SetAccelerationY(accelY);
	pattern->SetMaxSpeedX(maxSpeedX);
	pattern->SetMaxSpeedY(maxSpeedY);
	pattern->SetShotDataID(idShot);
	obj->AddPattern(frame, pattern);

	return value();
}
gstd::value StgStageScript::Func_ObjMove_GetX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	double pos = obj->GetPositionX();
	return value(machine->get_engine()->get_real_type(),(long double)pos);
}
gstd::value StgStageScript::Func_ObjMove_GetY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	double pos = obj->GetPositionY();
	return value(machine->get_engine()->get_real_type(),(long double)pos);
}
gstd::value StgStageScript::Func_ObjMove_GetSpeed(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(),(long double)0);

	double speed = obj->GetSpeed();
	return value(machine->get_engine()->get_real_type(),(long double)speed);
}
gstd::value StgStageScript::Func_ObjMove_GetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgMoveObject* obj = dynamic_cast<StgMoveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(),(long double)0);

	double angle = obj->GetDirectionAngle();
	return value(machine->get_engine()->get_real_type(),(long double)angle);
}

//STG共通関数：敵オブジェクト操作
gstd::value StgStageScript::Func_ObjEnemy_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	int type = (int)argv[0].as_real();
	ref_count_ptr<DxScriptObjectBase>::unsync obj;
	if(type == OBJ_ENEMY)
	{
		obj = new StgEnemyObject(stageController);
	}
	else if(type == OBJ_ENEMY_BOSS)
	{
		ref_count_ptr<StgEnemyBossSceneObject>::unsync objScene = enemyManager->GetBossSceneObject();
		if(objScene == NULL)
		{
			throw gstd::wexception(L"EnemyBossSceneが作成されていません");
		}
		
		ref_count_ptr<StgEnemyBossSceneData>::unsync data = objScene->GetActiveData();
		int id = data->GetEnemyBossIdInCreate();
		return value(machine->get_engine()->get_real_type(), (long double)id);
	}

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->SetObjectManager(script->objManager_.GetPointer());
		id = script->AddObject(obj, false);
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}
gstd::value StgStageScript::Func_ObjEnemy_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	ref_count_ptr<StgEnemyObject>::unsync objEnemy = 
		ref_count_ptr<StgEnemyObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objEnemy != NULL)
	{
		StgEnemyManager* enemyManager = stageController->GetEnemyManager();
		enemyManager->AddEnemy(objEnemy);
		objEnemy->Activate();

		script->ActivateObject(objEnemy->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = (int)argv[1].as_real();

	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if(obj == NULL)
	{
		switch(type)
		{
			case INFO_LIFE:
			case INFO_DAMAGE_RATE_SHOT:
			case INFO_DAMAGE_RATE_SPELL:
			case INFO_SHOT_HIT_COUNT:
				return value(machine->get_engine()->get_real_type(), (long double)0);
		}
		return value();
	}

	switch(type)
	{
		case INFO_LIFE:
			return value(machine->get_engine()->get_real_type(), (long double)obj->GetLife());
		case INFO_DAMAGE_RATE_SHOT:
			return value(machine->get_engine()->get_real_type(), (long double)obj->GetShotDamageRate());
		case INFO_DAMAGE_RATE_SPELL:
			return value(machine->get_engine()->get_real_type(), (long double)obj->GetSpellDamageRate());
		case INFO_SHOT_HIT_COUNT:
			return value(machine->get_engine()->get_real_type(), (long double)obj->GetIntersectedPlayerShotCount());
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetLife(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double param = argv[1].as_real();
	obj->SetLife(param);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_AddLife(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double inc = argv[1].as_real();
	obj->AddLife(inc);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetDamageRate(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyObject* obj = dynamic_cast<StgEnemyObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double rateShot = argv[1].as_real();
	double rateSpell = argv[2].as_real();
	obj->SetDamageRate(rateShot, rateSpell);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_AddIntersectionCircleA(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgEnemyObject>::unsync obj =
		ref_count_ptr<StgEnemyObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();

	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	double radius = argv[3].as_real();

	DxCircle circle(px, py, radius);

	//当たり判定
	ref_count_weak_ptr<StgEnemyObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
	target->SetObject(wObj);
	target->SetCircle(circle);
	obj->AddIntersectionRelativeTarget(target);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetIntersectionCircleToShot(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgEnemyObject>::unsync obj =
		ref_count_ptr<StgEnemyObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();

	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	double radius = argv[3].as_real();

	DxCircle circle(px, py, radius);

	//当たり判定
	ref_count_weak_ptr<StgEnemyObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
	target->SetObject(wObj);
	target->SetCircle(circle);
	intersectionManager->AddEnemyTargetToShot(target);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemy_SetIntersectionCircleToPlayer(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgEnemyObject>::unsync obj =
		ref_count_ptr<StgEnemyObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();

	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	double radius = argv[3].as_real();

	DxCircle circle(px, py, radius);

	//当たり判定
	ref_count_weak_ptr<StgEnemyObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY);
	target->SetObject(wObj);
	target->SetCircle(circle);
	intersectionManager->AddEnemyTargetToPlayer(target);

	return value();
}

//STG共通関数：敵ボスシーンオブジェクト操作
gstd::value StgStageScript::Func_ObjEnemyBossScene_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;
	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	ref_count_ptr<DxScriptObjectBase>::unsync obj = new StgEnemyBossSceneObject(stageController);

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->SetObjectManager(script->objManager_.GetPointer());
		id = script->AddObject(obj, false);
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	StgEnemyManager* enemyManager = stageController->GetEnemyManager();

	ref_count_ptr<StgEnemyBossSceneObject>::unsync objScene = 
		ref_count_ptr<StgEnemyBossSceneObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objScene != NULL)
	{
		enemyManager->SetBossSceneObject(objScene);
		objScene->Activate();
		script->ActivateObject(objScene->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_Add(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int step = (int)argv[1].as_real();
	std::wstring path = argv[2].as_string();
	path = PathProperty::GetUnique(path);

	ref_count_ptr<StgEnemyBossSceneData>::unsync data = new StgEnemyBossSceneData();
	data->SetPath(path);
	obj->AddData(step, data);

	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_LoadInThread(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->LoadAllScriptInThread();
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = (int)argv[1].as_real();

	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if(obj == NULL)
	{
		switch(type)
		{
			case INFO_IS_SPELL:
			case INFO_IS_LAST_SPELL:
			case INFO_IS_DURABLE_SPELL:
			case INFO_IS_LAST_STEP:
				return value(machine->get_engine()->get_boolean_type(), false);
			case INFO_TIMER:
			case INFO_TIMERF:
			case INFO_ORGTIMERF:
			case INFO_SPELL_SCORE:
			case INFO_REMAIN_STEP_COUNT:
			case INFO_ACTIVE_STEP_LIFE_COUNT:
			case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
			case INFO_ACTIVE_STEP_TOTAL_LIFE:
			case INFO_PLAYER_SHOOTDOWN_COUNT:
			case INFO_PLAYER_SPELL_COUNT:
			case INFO_CURRENT_LIFE:
			case INFO_CURRENT_LIFE_MAX:
				return value(machine->get_engine()->get_real_type(), (long double)0);
			case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
				return script->CreateRealArrayValue(std::vector<long double>());

		}
		return value();
	}

	ref_count_ptr<StgEnemyBossSceneData>::unsync sceneData = obj->GetActiveData();
	switch(type)
	{
		case INFO_IS_SPELL:
		{
			bool res = false;
			if(sceneData != NULL)res = sceneData->IsSpellCard();
				return value(machine->get_engine()->get_boolean_type(), res);
		}
		case INFO_IS_LAST_SPELL:
		{
			bool res = false;
			if(sceneData != NULL)res = sceneData->IsLastSpell();
				return value(machine->get_engine()->get_boolean_type(), res);
		}
		case INFO_IS_DURABLE_SPELL:
		{
			bool res = false;
			if(sceneData != NULL)res = sceneData->IsDurable();
			return value(machine->get_engine()->get_boolean_type(), res);
		}
		case INFO_TIMER:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				int timer = sceneData->GetSpellTimer();
				if(timer < 0)res = 99;
				else res = timer / STANDARD_FPS;
			}
			return script->CreateRealValue(res);
		}
		case INFO_TIMERF:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				res = sceneData->GetSpellTimer();
			}
			return script->CreateRealValue(res);
		}
		case INFO_ORGTIMERF:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				res = sceneData->GetOriginalSpellTimer();
			}
			return script->CreateRealValue(res);
		}
		case INFO_SPELL_SCORE:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				res = sceneData->GetCurrentSpellScore();
			}
			return script->CreateRealValue(res);
		}
		case INFO_REMAIN_STEP_COUNT:
			return script->CreateRealValue(obj->GetRemainStepCount());
		case INFO_ACTIVE_STEP_LIFE_COUNT:
			return script->CreateRealValue(obj->GetActiveStepLifeCount());
		case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
			return script->CreateRealValue(obj->GetActiveStepTotalMaxLife());
		case INFO_ACTIVE_STEP_TOTAL_LIFE:
			return script->CreateRealValue(obj->GetActiveStepTotalLife());
		case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
		{
			std::vector<long double> listLD;
			std::vector<double> listD = obj->GetActiveStepLifeRateList();
			for(int iLife = 0 ; iLife < listD.size() ; iLife++)
				listLD.push_back(listD[iLife]);
			return script->CreateRealArrayValue(listLD);
		}
		case INFO_IS_LAST_STEP:
		{
			bool res = obj->GetRemainStepCount() == 0;
			res &= (obj->GetActiveStepTotalLife() == 0);
			return value(machine->get_engine()->get_boolean_type(), res);
		}
		case INFO_PLAYER_SHOOTDOWN_COUNT:
		{
			long double res = 0;
			if(sceneData != NULL)
				res = sceneData->GetPlayerShootDownCount();
			return script->CreateRealValue(res);
		}
		case INFO_PLAYER_SPELL_COUNT:
		{
			long double res = 0;
			if(sceneData != NULL)
				res = sceneData->GetPlayerSpellCount();
			return script->CreateRealValue(res);
		}
		case INFO_CURRENT_LIFE:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				int dataIndex = obj->GetDataIndex();
				res = obj->GetActiveStepLife(dataIndex);
			}
			return script->CreateRealValue(res);
		}
		case INFO_CURRENT_LIFE_MAX:
		{
			long double res = 0;
			if(sceneData != NULL)
			{
				int dataIndex = obj->GetDataIndex();
				std::vector<double>& listLife = sceneData->GetLifeList();
				for(int iLife = 0 ; iLife < listLife.size(); iLife++)
				{
					res += listLife[iLife];
				}
			}
			return script->CreateRealValue(res);
		}

	}

	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_SetSpellTimer(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<StgEnemyBossSceneData>::unsync sceneData = obj->GetActiveData();
	if(sceneData == NULL)return value();

	int timer = (int)argv[1].as_real();
	sceneData->SetSpellTimer(timer);
	return value();
}
gstd::value StgStageScript::Func_ObjEnemyBossScene_StartSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgEnemyBossSceneObject* obj = dynamic_cast<StgEnemyBossSceneObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<StgEnemyBossSceneData>::unsync sceneData = obj->GetActiveData();
	if(sceneData == NULL)return value();

	sceneData->SetSpellCard(true);
	ScriptManager* scriptManager = script->scriptManager_;
	scriptManager->RequestEventAll(EV_START_BOSS_SPELL);
	return value();
}

//STG共通関数：弾オブジェクト操作
gstd::value StgStageScript::Func_ObjShot_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	int type = (int)argv[0].as_real();
	ref_count_ptr<StgShotObject>::unsync obj;
	if(type == OBJ_SHOT)
	{
		obj = new StgNormalShotObject(stageController);
	}
	else if(type == OBJ_LOOSE_LASER)
	{
		obj = new StgLooseLaserObject(stageController);
	}
	else if(type == OBJ_STRAIGHT_LASER)
	{
		obj = new StgStraightLaserObject(stageController);
	}
	else if(type == OBJ_CURVE_LASER)
	{
		obj = new StgCurveLaserObject(stageController);
	}

	int id = ID_INVALID;
	if(obj != NULL)
	{
		int typeOwner = script->GetScriptType() == TYPE_PLAYER ? 
			StgShotObject::OWNER_PLAYER : StgShotObject::OWNER_ENEMY;
		obj->SetOwnerType(typeOwner);

		obj->SetObjectManager(script->objManager_.GetPointer());
		id = script->AddObject(obj, false);
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}
gstd::value StgStageScript::Func_ObjShot_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	ref_count_ptr<StgShotObject>::unsync objShot =
		ref_count_ptr<StgShotObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objShot != NULL)
	{
		if(script->GetScriptType() == TYPE_PLAYER)
		{
			ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
			if(objPlayer != NULL && !objPlayer->IsPermitShot())return value();
		}

		StgShotManager* shotManager = stageController->GetShotManager();
		shotManager->AddShot(objShot);
		objShot->Activate();

		script->ActivateObject(objShot->GetObjectID(), true);
	}

	return value();
}

gstd::value StgStageScript::Func_ObjShot_SetAutoDelete(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bAutoDelete = argv[1].as_boolean();
	obj->SetAutoDelete(bAutoDelete);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_FadeDelete(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->SetFadeDelete();

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDeleteFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	obj->SetAutoDeleteFrame(frame);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDelay(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int delay = (int)argv[1].as_real();
	obj->SetDelay(delay);

	return value();
}

gstd::value StgStageScript::Func_ObjShot_SetSpellResist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bRegist = argv[1].as_boolean();
	double life = obj->GetLife();
	if(bRegist)
	{
		life = StgShotObject::LIFE_SPELL_REGIST;
	}
	else
	{
		if(life >= StgShotObject::LIFE_SPELL_UNREGIST)
			life = StgShotObject::LIFE_SPELL_UNREGIST;
	}
	obj->SetLife(life);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetGraphic(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int grf = (int)argv[1].as_real();
	obj->SetShotDataID(grf);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetSourceBlendType(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int typeBlend = (int)argv[1].as_real();
	obj->SetSourceBlendType(typeBlend);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double damage = argv[1].as_real();
	obj->SetDamage(damage);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double life = argv[1].as_real();
	obj->SetLife(life);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetEraseShot(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bErase = argv[1].as_boolean();
	obj->SetEraseShot(bErase);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetSpellFactor(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bErase = argv[1].as_boolean();
	obj->SetSpellFactor(bErase);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_ToItem(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->ConvertToItem();

	return value();
}
gstd::value StgStageScript::Func_ObjShot_AddShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int idShot = (int)argv[1].as_real();
	int frame = (int)argv[2].as_real();
	obj->AddShot(frame, idShot, 0, 0);
	return value();
}
gstd::value StgStageScript::Func_ObjShot_AddShotA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int idShot = (int)argv[1].as_real();
	int frame = (int)argv[2].as_real();
	double radius = argv[3].as_real();
	double angle = argv[4].as_real();
	obj->AddShot(frame, idShot, radius, angle);
	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgShotObject>::unsync obj = ref_count_ptr<StgShotObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	if(obj->GetDelay() > 0)return value();

	obj->SetUserIntersectionMode(true);
	int typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ? 
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px = (int)obj->GetPositionX();
	int py = (int)obj->GetPositionY();
	int radius = (int)argv[1].as_real();
	DxCircle circle(px, py, radius);
	ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;

	//当たり判定
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(typeTarget);
	target->SetCircle(circle);
	target->SetObject(wObj);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgShotObject>::unsync obj = ref_count_ptr<StgShotObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	if(obj->GetDelay() > 0)return value();

	obj->SetUserIntersectionMode(true);
	int typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ? 
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	int radius = (int)argv[3].as_real();
	DxCircle circle(px, py, radius);
	ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;

	//当たり判定
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(typeTarget);
	target->SetCircle(circle);
	target->SetObject(wObj);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgShotObject>::unsync obj = ref_count_ptr<StgShotObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	if(obj->GetDelay() > 0)return value();

	obj->SetUserIntersectionMode(true);
	int typeTarget = obj->GetOwnerType() == StgShotObject::OWNER_PLAYER ? 
		StgIntersectionTarget::TYPE_PLAYER_SHOT : StgIntersectionTarget::TYPE_ENEMY_SHOT;

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px1 = (int)argv[1].as_real();
	int py1 = (int)argv[2].as_real();
	int px2 = (int)argv[3].as_real();
	int py2 = (int)argv[4].as_real();
	int width = (int)argv[5].as_real();
	DxWidthLine line(px1, py1, px2, py2, width);

	//当たり判定
	ref_count_weak_ptr<StgShotObject>::unsync wObjShot = obj;
	ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
	target->SetTargetType(typeTarget);
	target->SetObject(wObjShot);
	target->SetLine(line);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetIntersectionEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = argv[1].as_boolean();
	obj->SetIntersectionEnable(bEnable);

	return value();
}
gstd::value StgStageScript::Func_ObjShot_SetItemChange(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = argv[1].as_boolean();
	obj->SetItemChangeEnable(bEnable);

	return value();
}

gstd::value StgStageScript::Func_ObjShot_GetDelay(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetDelay();
	return value(machine->get_engine()->get_real_type(),(long double)res);
}
gstd::value StgStageScript::Func_ObjShot_GetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetDamage();
	return value(machine->get_engine()->get_real_type(),(long double)res);
}
gstd::value StgStageScript::Func_ObjShot_GetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetLife();
	return value(machine->get_engine()->get_real_type(),(long double)res);
}
gstd::value StgStageScript::Func_ObjShot_IsSpellResist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool res = obj->GetLife() == StgShotObject::LIFE_SPELL_REGIST;
	return value(machine->get_engine()->get_boolean_type(), res);
}

gstd::value StgStageScript::Func_ObjShot_GetImageID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	int id = (int)argv[0].as_real();
	StgShotObject* obj = dynamic_cast<StgShotObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetShotDataID();
	return value(machine->get_engine()->get_real_type(),(long double)res);
}


gstd::value StgStageScript::Func_ObjLaser_SetLength(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int length = (int)argv[1].as_real();
	obj->SetLength(length);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetRenderWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int width = (int)argv[1].as_real();
	obj->SetRenderWidth(width);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetIntersectionWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int width = (int)argv[1].as_real()/2;
	obj->SetIntersectionWidth(width);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetInvalidLength(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int start = (int)argv[1].as_real();
	int end = (int)argv[2].as_real();
	obj->SetInvalidLength(start, end);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetGrazeInvalidFrame(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int frame = (int)argv[1].as_real();
	obj->SetGrazeInvalidFrame(frame);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_SetItemDistance(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double dist = argv[1].as_real();
	obj->SetItemDistance(dist);

	return value();
}
gstd::value StgStageScript::Func_ObjLaser_GetLength(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	int length = obj->GetLength();
	return value(machine->get_engine()->get_real_type(),(long double)length);
}
gstd::value StgStageScript::Func_ObjLaser_GetRenderWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	int width = obj->GetRenderWidth();
	return value(machine->get_engine()->get_real_type(),(long double)width);
}
gstd::value StgStageScript::Func_ObjLaser_GetIntersectionWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgLaserObject* obj = dynamic_cast<StgLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	int width = obj->GetIntersectionWidth();
	return value(machine->get_engine()->get_real_type(),(long double)width);
}
gstd::value StgStageScript::Func_ObjStLaser_SetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double angle = argv[1].as_real();
	obj->SetLaserAngle(angle);

	return value();
}
gstd::value StgStageScript::Func_ObjStLaser_GetAngle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return script->CreateRealValue(0);

	double angle = obj->GetLaserAngle();
	return value(machine->get_engine()->get_real_type(),(long double)angle);
}
gstd::value StgStageScript::Func_ObjStLaser_SetSource(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgStraightLaserObject* obj = dynamic_cast<StgStraightLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = argv[1].as_boolean();
	obj->SetSourceEnable(bEnable);

	return value();
}
gstd::value StgStageScript::Func_ObjCrLaser_SetTipDecrement(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgCurveLaserObject* obj = dynamic_cast<StgCurveLaserObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	double dec = argv[1].as_real();
	dec = min(dec, 1);
	dec = max(dec, 0);
	obj->SetTipDecrement(dec);

	return value();
}

//STG共通関数：アイテムオブジェクト操作
gstd::value StgStageScript::Func_ObjItem_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	int type = (int)argv[0].as_real();
	ref_count_ptr<StgItemObject>::unsync obj;
	if(type == StgItemObject::ITEM_USER)
	{
		obj = new StgItemObject_User(stageController);
	}

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->SetObjectManager(script->objManager_.GetPointer());
		id = script->AddObject(obj, false);
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}
gstd::value StgStageScript::Func_ObjItem_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	ref_count_ptr<StgItemObject>::unsync objItem =
		ref_count_ptr<StgItemObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objItem != NULL)
	{
		StgItemManager* itemManager = stageController->GetItemManager();
		itemManager->AddItem(objItem);
		objItem->Activate();

		script->ActivateObject(objItem->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetItemID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject_User* obj = dynamic_cast<StgItemObject_User*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int grf = (int)argv[1].as_real();
	obj->SetImageID(grf);

	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetRenderScoreEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = (bool)argv[1].as_boolean();
	obj->SetChangeItemScore(bEnable);

	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetAutoCollectEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = (bool)argv[1].as_boolean();
	obj->SetPermitMoveToPlayer(bEnable);

	return value();
}
gstd::value StgStageScript::Func_ObjItem_SetDefinedMovePatternA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageScript* script = (StgStageScript*)machine->data;
	int id = (int)argv[0].as_real();
	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int type = (int)argv[1].as_real();
	ref_count_ptr<StgMovePattern_Item>::unsync move = new StgMovePattern_Item(obj);
	move->SetItemMoveType(type);
	obj->SetPattern(move);

	return value();
}
gstd::value StgStageScript::Func_ObjItem_GetInfo(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int type = (int)argv[1].as_real();

	StgItemObject* obj = dynamic_cast<StgItemObject*>(script->GetObjectPointer(id));
	if(obj == NULL)
	{
		switch(type)
		{
			case INFO_ITEM_SCORE:
				return value(machine->get_engine()->get_real_type(), (long double)0);
		}
		return value();
	}

	switch(type)
	{
		case INFO_ITEM_SCORE:
			return value(machine->get_engine()->get_real_type(), (long double)obj->GetScore());
	}

	return value();
}

//STG共通関数：自機オブジェクト操作
gstd::value StgStageScript::Func_ObjPlayer_AddIntersectionCircleA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerObject>::unsync obj = 
		ref_count_ptr<StgPlayerObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();

	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	double rHit = argv[3].as_real();
	double rGraze = argv[4].as_real();

	DxCircle circle(px, py, rHit);

	//当たり判定
	ref_count_weak_ptr<StgPlayerObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Player>::unsync targetHit = new StgIntersectionTarget_Player(false);
	targetHit->SetObject(wObj);
	targetHit->SetCircle(circle);
	obj->AddIntersectionRelativeTarget(targetHit);

	//Graze判定
	circle.SetR(rHit + rGraze);
	ref_count_ptr<StgIntersectionTarget_Player>::unsync targetGraze = new StgIntersectionTarget_Player(true);
	targetGraze->SetObject(wObj);
	targetGraze->SetCircle(circle);
	obj->AddIntersectionRelativeTarget(targetGraze);

	return value();
}
gstd::value StgStageScript::Func_ObjPlayer_AddIntersectionCircleA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerObject>::unsync obj = 
		ref_count_ptr<StgPlayerObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();

	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	double rGraze = argv[3].as_real();

	DxCircle circle(px, py, 0);

	//Graze判定
	ref_count_weak_ptr<StgPlayerObject>::unsync wObj = obj;
	circle.SetR(rGraze);
	ref_count_ptr<StgIntersectionTarget_Player>::unsync targetGraze = new StgIntersectionTarget_Player(true);
	targetGraze->SetObject(wObj);
	targetGraze->SetCircle(circle);
	obj->AddIntersectionRelativeTarget(targetGraze);

	return value();
}
gstd::value StgStageScript::Func_ObjPlayer_ClearIntersection(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerObject>::unsync obj = 
		ref_count_ptr<StgPlayerObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	obj->ClearIntersectionRelativeTarget();

	return value();
}

//STG共通関数：当たり判定オブジェクト操作
gstd::value StgStageScript::Func_ObjCol_IsIntersected(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* objBase = dynamic_cast<DxScriptObjectBase*>(script->GetObjectPointer(id));
	if(objBase == NULL)return value();

	ref_count_ptr<StgIntersectionObject>::unsync obj = 
		ref_count_ptr<StgIntersectionObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	
	bool res = obj->IsIntersected();
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStageScript::Func_ObjCol_GetListOfIntersectedEnemyID(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* objBase = dynamic_cast<DxScriptObjectBase*>(script->GetObjectPointer(id));
	if(objBase == NULL)return value();

	ref_count_ptr<StgIntersectionObject>::unsync obj = 
		ref_count_ptr<StgIntersectionObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)return value();
	
	std::vector<int>& list = obj->GetIntersectedIdList();
	std::vector<long double> listLD;
	for(int iList = 0 ; iList < list.size() ; iList++)
	{
		int idObject = list[iList];
		ref_count_ptr<StgEnemyObject>::unsync objEnemy = 
			ref_count_ptr<StgEnemyObject>::unsync::DownCast(script->GetObject(idObject));
		if(objEnemy != NULL)
			listLD.push_back(idObject);
	}

	gstd::value res = script->CreateRealArrayValue(listLD);
	return res;
}
gstd::value StgStageScript::Func_ObjCol_GetIntersectedCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	ref_count_ptr<StgIntersectionObject>::unsync obj = 
		ref_count_ptr<StgIntersectionObject>::unsync::DownCast(script->GetObject(id));
	if(obj == NULL)
		return value(machine->get_engine()->get_real_type(), (long double)0);
	
	std::vector<int>& list = obj->GetIntersectedIdList();
	long double res = list.size();
	return value(machine->get_engine()->get_real_type(), res);
}

/**********************************************************
//StgSystemScript
**********************************************************/
function const stgSystemFunction[] =  
{
	//関数：


	//定数
	{"__stgSystemFunction__",constant<0>::func, 0},
};
StgStageSystemScript::StgStageSystemScript(StgStageController* stageController) : StgStageScript(stageController)
{
	typeScript_ = TYPE_SYSTEM;
	_AddFunction(stgSystemFunction, sizeof(stgSystemFunction) / sizeof(function));
}
StgStageSystemScript::~StgStageSystemScript()
{
}

/**********************************************************
//StgStageItemScript
**********************************************************/
function const stgItemFunction[] =  
{
	//定数
	{"EV_GET_ITEM", constant<StgStageItemScript::EV_GET_ITEM>::func, 0},
	{"EV_DELETE_SHOT_TO_ITEM", constant<StgStageItemScript::EV_DELETE_SHOT_TO_ITEM>::func, 0},

	{"EV_GRAZE", constant<StgStagePlayerScript::EV_GRAZE>::func, 0},
};
StgStageItemScript::StgStageItemScript(StgStageController* stageController) : StgStageScript(stageController)
{
	typeScript_ = TYPE_ITEM;
	_AddFunction(stgItemFunction, sizeof(stgItemFunction) / sizeof(function));
}
StgStageItemScript::~StgStageItemScript()
{
}

/**********************************************************
//StgStageShotScript
**********************************************************/
function const stgShotFunction[] =  
{
	//定数
	{"EV_DELETE_SHOT_IMMEDIATE", constant<StgStageScript::EV_DELETE_SHOT_IMMEDIATE>::func, 0},
	{"EV_DELETE_SHOT_TO_ITEM", constant<StgStageScript::EV_DELETE_SHOT_TO_ITEM>::func, 0},
	{"EV_DELETE_SHOT_FADE", constant<StgStageScript::EV_DELETE_SHOT_FADE>::func, 0},

	{"SetShotDeleteEventEnable", StgStageShotScript::Func_SetShotDeleteEventEnable, 2},
};
StgStageShotScript::StgStageShotScript(StgStageController* stageController) : StgStageScript(stageController)
{
	typeScript_ = TYPE_SHOT;
	_AddFunction(stgShotFunction, sizeof(stgShotFunction) / sizeof(function));
}
StgStageShotScript::~StgStageShotScript()
{
}

gstd::value StgStageShotScript::Func_SetShotDeleteEventEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStageShotScript* script = (StgStageShotScript*)machine->data;
	int type = (int)argv[0].as_real();
	bool bEnable = argv[1].as_boolean();

	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();
	shotManager->SetDeleteEventEnableByType(type, bEnable);

	return value();
}

/**********************************************************
//StgPlayerScript
**********************************************************/
function const stgPlayerFunction[] =  
{
	//関数：
	{"CreatePlayerShotA1", StgStagePlayerScript::Func_CreatePlayerShotA1, 7},
	{"CallSpell", StgStagePlayerScript::Func_CallSpell, 0},
	{"LoadPlayerShotData", StgStagePlayerScript::Func_LoadPlayerShotData, 1},
	{"ReloadPlayerShotData", StgStagePlayerScript::Func_ReloadPlayerShotData, 1},
	{"GetSpellManageObject", StgStagePlayerScript::Func_GetSpellManageObject, 0},

	//自機専用関数：スペルオブジェクト操作
	{"ObjSpell_Create", StgStagePlayerScript::Func_ObjSpell_Create, 0},
	{"ObjSpell_Regist", StgStagePlayerScript::Func_ObjSpell_Regist, 1},
	{"ObjSpell_SetDamage", StgStagePlayerScript::Func_ObjSpell_SetDamage, 2},
	{"ObjSpell_SetPenetration", StgStagePlayerScript::Func_ObjSpell_SetPenetration, 2},
	{"ObjSpell_SetEraseShot", StgStagePlayerScript::Func_ObjSpell_SetEraseShot, 2},
	{"ObjSpell_SetIntersectionCircle", StgStagePlayerScript::Func_ObjSpell_SetIntersectionCircle, 4},
	{"ObjSpell_SetIntersectionLine", StgStagePlayerScript::Func_ObjSpell_SetIntersectionLine, 6},

	//定数
	{"EV_REQUEST_SPELL", constant<StgStagePlayerScript::EV_REQUEST_SPELL>::func, 0},
	{"EV_GRAZE", constant<StgStagePlayerScript::EV_GRAZE>::func, 0},
	{"EV_HIT", constant<StgStagePlayerScript::EV_HIT>::func, 0},

	{"EV_GET_ITEM", constant<StgStageItemScript::EV_GET_ITEM>::func, 0},
};
StgStagePlayerScript::StgStagePlayerScript(StgStageController* stageController) : StgStageScript(stageController)
{
	typeScript_ = TYPE_PLAYER;
	_AddFunction(stgPlayerFunction, sizeof(stgPlayerFunction) / sizeof(function));
}
StgStagePlayerScript::~StgStagePlayerScript()
{
}

//自機専用関数
gstd::value StgStagePlayerScript::Func_CreatePlayerShotA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
	if(objPlayer != NULL && !objPlayer->IsPermitShot())return value();

	ref_count_ptr<StgNormalShotObject>::unsync obj = new StgNormalShotObject(stageController);
	obj->SetObjectManager(script->objManager_.GetPointer());
	int id = script->AddObject(obj);
	if(id != ID_INVALID)
	{
		stageController->GetShotManager()->AddShot(obj);
		long double posX = argv[0].as_real();
		long double posY = argv[1].as_real();
		long double speed = argv[2].as_real();
		long double angle = argv[3].as_real();
		double damage = (double)argv[4].as_real();
		double life = (double)argv[5].as_real();
		int idShot = (int)argv[6].as_real();

		obj->SetOwnerType(StgShotObject::OWNER_PLAYER);
		obj->SetX(posX);
		obj->SetY(posY);
		obj->SetSpeed(speed);
		obj->SetDirectionAngle(angle);
		obj->SetLife(life);
		obj->SetDamage(damage);
		obj->SetShotDataID(idShot);
	}
	
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value StgStagePlayerScript::Func_CallSpell(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
	if(objPlayer == NULL)return value();

	objPlayer->CallSpell();
	
	return value();
}
gstd::value StgStagePlayerScript::Func_LoadPlayerShotData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadPlayerShotData(path);
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStagePlayerScript::Func_ReloadPlayerShotData(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;
	StgShotManager* shotManager = stageController->GetShotManager();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	bool res = shotManager->LoadPlayerShotData(path, true);
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value StgStagePlayerScript::Func_GetSpellManageObject(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgPlayerObject>::unsync obj = stageController->GetPlayerObject();
	int id = ID_INVALID;
	if(obj != NULL)
	{
		ref_count_ptr<StgPlayerSpellManageObject>::unsync objManage = obj->GetSpellManageObject();
		if(objManage != NULL)
		{
			id = objManage->GetObjectID();
		}
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}

//自機専用関数：スペルオブジェクト操作
gstd::value StgStagePlayerScript::Func_ObjSpell_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	script->CheckRunInMainThread();
	StgStageController* stageController = script->stageController_;

	ref_count_ptr<StgPlayerSpellObject>::unsync obj = new StgPlayerSpellObject(stageController);

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->SetObjectManager(script->objManager_.GetPointer());
		id = script->AddObject(obj, false);
	}
	return value(machine->get_engine()->get_real_type(), (long double)id);
}
gstd::value StgStagePlayerScript::Func_ObjSpell_Regist(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell = 
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell != NULL)
	{
		script->ActivateObject(objSpell->GetObjectID(), true);
	}

	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetDamage(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell = 
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell == NULL)return value();

	double damage = argv[1].as_real();
	objSpell->SetDamage(damage);
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetPenetration(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell = 
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell == NULL)return value();

	double life = argv[1].as_real();
	objSpell->SetLife(life);
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetEraseShot(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();
	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell = 
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell == NULL)return value();

	bool bEraseShot = argv[1].as_boolean();
	objSpell->SetEraseShot(bEraseShot);
	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetIntersectionCircle(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell =
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell == NULL)return value();

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px = (int)argv[1].as_real();
	int py = (int)argv[2].as_real();
	int radius = (int)argv[3].as_real();
	DxCircle circle(px, py, radius);

	//当たり判定
	ref_count_weak_ptr<StgPlayerSpellObject>::unsync wObjSpell = objSpell;
	ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
	target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SPELL);
	target->SetObject(wObjSpell);
	target->SetCircle(circle);

	intersectionManager->AddTarget(target);

	return value();
}
gstd::value StgStagePlayerScript::Func_ObjSpell_SetIntersectionLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	StgStagePlayerScript* script = (StgStagePlayerScript*)machine->data;
	StgStageController* stageController = script->stageController_;

	int id = (int)argv[0].as_real();

	ref_count_ptr<StgPlayerSpellObject>::unsync objSpell =
		ref_count_ptr<StgPlayerSpellObject>::unsync::DownCast(stageController->GetMainRenderObject(id));
	if(objSpell == NULL)return value();

	StgIntersectionManager* intersectionManager = stageController->GetIntersectionManager();
	int px1 = (int)argv[1].as_real();
	int py1 = (int)argv[2].as_real();
	int px2 = (int)argv[3].as_real();
	int py2 = (int)argv[4].as_real();
	int width = (int)argv[5].as_real();
	DxWidthLine line(px1, py1, px2, py2, width);

	//当たり判定
	ref_count_weak_ptr<StgPlayerSpellObject>::unsync wObjSpell = objSpell;
	ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
	target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SPELL);
	target->SetObject(wObjSpell);
	target->SetLine(line);

	intersectionManager->AddTarget(target);

	return value();
}

