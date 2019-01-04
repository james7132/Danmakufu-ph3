#include"StgPlayer.hpp"
#include"StgSystem.hpp"


/**********************************************************
//StgPlayerObject
**********************************************************/
StgPlayerObject::StgPlayerObject(StgStageController* stageController) : StgMoveObject(stageController)
{
	stageController_ = stageController;
	typeObject_ = StgStageScript::OBJ_PLAYER;

	infoPlayer_ = new StgPlayerInformation();
	RECT rcStgFrame = stageController_->GetStageInformation()->GetStgFrameRect();
	int stgWidth = rcStgFrame.right - rcStgFrame.left;
	int stgHeight = rcStgFrame.bottom - rcStgFrame.top;
	
	SetRenderPriority(0.30);
	speedFast_ = 4;
	speedSlow_ = 1.6;
	SetRect(&rcClip_, 0, 0, stgWidth, stgHeight);

	state_ = STATE_NORMAL;
	frameStateDown_ = 120;
	frameRebirthMax_ = 15;
	infoPlayer_->frameRebirth_ = frameRebirthMax_;
	frameRebirthDiff_ = 3;

	infoPlayer_->life_ = 2;
	infoPlayer_->countBomb_ = 3;
	infoPlayer_->power_ = 1.0;

	itemCircle_ = 24;
	frameInvincibility_ = 0;
	bForbidShot_ = false;
	bForbidSpell_ = false;
	yAutoItemCollect_ = -256 * 256;

	hitObjectID_ = DxScript::ID_INVALID;

	_InitializeRebirth();
}
StgPlayerObject::~StgPlayerObject()
{
}
void StgPlayerObject::_InitializeRebirth()
{
	RECT rcStgFrame = stageController_->GetStageInformation()->GetStgFrameRect();
	int stgWidth = rcStgFrame.right - rcStgFrame.left;
	int stgHeight = rcStgFrame.bottom - rcStgFrame.top;
	
	SetX(stgWidth / 2);
	SetY(rcStgFrame.bottom - 48);
}
void StgPlayerObject::Work()
{
	EDirectInput* input = EDirectInput::GetInstance();
	StgStageScriptManager* scriptManager = stageController_->GetScriptManagerP();
	StgEnemyManager* enemyManager = stageController_->GetEnemyManager();

	//当たり判定クリア
	ClearIntersected();

	if(state_ == STATE_NORMAL)
	{
		//通常時
		if(hitObjectID_ != DxScript::ID_INVALID)
		{
			state_ = STATE_HIT;
			frameState_ = infoPlayer_->frameRebirth_;

			gstd::value valueHitObjectID = script_->CreateRealValue(hitObjectID_);
			std::vector<gstd::value> listScriptValue;
			listScriptValue.push_back(valueHitObjectID);
			script_->RequestEvent(StgStagePlayerScript::EV_HIT, listScriptValue);
		}
		else
		{
			if(listGrazedShot_.size() > 0)
			{
				std::vector<value> listValPos;
				std::vector<long double> listShotID;
				int grazedShotCount = listGrazedShot_.size();
				for(int iObj = 0 ; iObj < grazedShotCount ; iObj++)
				{
					ref_count_weak_ptr<StgShotObject>::unsync objShot = 
						ref_count_weak_ptr<StgShotObject>::unsync::DownCast(listGrazedShot_[iObj]);
					if(objShot != NULL)
					{
						int id = objShot->GetObjectID();
						listShotID.push_back(id);

						std::vector<long double> listPos;
						listPos.push_back(objShot->GetPositionX());
						listPos.push_back(objShot->GetPositionY());
						listValPos.push_back(script_->CreateRealArrayValue(listPos));
					}
				}

				std::vector<gstd::value> listScriptValue;
				listScriptValue.push_back(script_->CreateRealValue(grazedShotCount));
				listScriptValue.push_back(script_->CreateRealArrayValue(listShotID));
				listScriptValue.push_back(script_->CreateValueArrayValue(listValPos));
				script_->RequestEvent(StgStagePlayerScript::EV_GRAZE, listScriptValue);

				ref_count_ptr<StgStageScriptManager> stageScriptManager = stageController_->GetScriptManagerR();
				ref_count_ptr<ManagedScript> itemScript = stageScriptManager->GetItemScript();
				if(itemScript != NULL)
				{
					itemScript->RequestEvent(StgStagePlayerScript::EV_GRAZE, listScriptValue);
				}
			}
			//_Move();
			if(input->GetVirtualKeyState(EDirectInput::KEY_BOMB) == KEY_PUSH)
				CallSpell();

			_AddIntersection();
		}
	}
	if(state_ == STATE_HIT)
	{
		//くらいボム待機
		if(input->GetVirtualKeyState(EDirectInput::KEY_BOMB) == KEY_PUSH)
			CallSpell();	

		if(state_ == STATE_HIT)
		{
			//くらいボム有効フレーム減少
			frameState_--;
			if(frameState_ < 0)
			{
				//自機ダウン
				bool bEnemyLastSpell = false;
				ref_count_ptr<StgEnemyBossSceneObject>::unsync objBossScene = enemyManager->GetBossSceneObject();
				if(objBossScene != NULL)
				{
					objBossScene->AddPlayerShootDownCount();
					if(objBossScene->GetActiveData()->IsLastSpell())
						bEnemyLastSpell = true;
				}
				if(!bEnemyLastSpell)
					infoPlayer_->life_--;
				scriptManager->RequestEventAll(StgStagePlayerScript::EV_PLAYER_SHOOTDOWN);

				if(infoPlayer_->life_ >= 0)
				{
					bVisible_ = false;
					state_ = STATE_DOWN;
					frameState_ = frameStateDown_;
				}
				else
				{
					state_ = STATE_END;
				}
			}
		}
	}
	if(state_ == STATE_DOWN)
	{
		//ダウン
		frameState_--;
		if(frameState_ <= 0)
		{
			bVisible_ = true;
			_InitializeRebirth();
			state_ = STATE_NORMAL;
			scriptManager->RequestEventAll(StgStageScript::EV_PLAYER_REBIRTH);
		}
	}
	if(state_ == STATE_END)
	{
		bVisible_ = false;
	}

	frameInvincibility_ = max(frameInvincibility_ - 1, 0);
	listGrazedShot_.clear();
	hitObjectID_ = DxScript::ID_INVALID;
}
void StgPlayerObject::Move()
{
	if(state_ == STATE_NORMAL)
	{
		//通常時
		if(hitObjectID_ == DxScript::ID_INVALID)
		{
			_Move();
		}
	}
}
void StgPlayerObject::_Move()
{
	double sx = 0;
	double sy = 0;
	EDirectInput* input = EDirectInput::GetInstance();
	int keyLeft = input->GetVirtualKeyState(EDirectInput::KEY_LEFT);
	int keyRight = input->GetVirtualKeyState(EDirectInput::KEY_RIGHT);
	int keyUp = input->GetVirtualKeyState(EDirectInput::KEY_UP);
	int keyDown = input->GetVirtualKeyState(EDirectInput::KEY_DOWN);
	int keySlow = input->GetVirtualKeyState(EDirectInput::KEY_SLOWMOVE);

	double speed = speedFast_;
	if(keySlow == KEY_PUSH || keySlow == KEY_HOLD)speed = speedSlow_;

	bool bKetLeft = keyLeft == KEY_PUSH || keyLeft == KEY_HOLD;
	bool bKeyRight = keyRight == KEY_PUSH || keyRight == KEY_HOLD;
	bool bKeyUp = keyUp == KEY_PUSH || keyUp == KEY_HOLD;
	bool bKeyDown = keyDown == KEY_PUSH || keyDown == KEY_HOLD;

	if(bKetLeft && !bKeyRight)sx += -speed;
	if(!bKetLeft && bKeyRight)sx += speed;
	if(bKeyUp && !bKeyDown)sy += -speed;
	if(!bKeyUp && bKeyDown)sy += speed;

	if(sx != 0 && sy != 0)
	{
		sx /= 1.41421356;
		sy /= 1.41421356;
	}

	double px = posX_ + sx;
	double py = posY_ + sy;

	//はみ出たときの処理
	px = max(px, rcClip_.left);
	px = min(px, rcClip_.right);
	py = max(py, rcClip_.top);
	py = min(py, rcClip_.bottom);

	SetX(px);
	SetY(py);
}
void StgPlayerObject::_AddIntersection()
{
	if(frameInvincibility_ > 0)return;
	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();

	UpdateIntersectionRelativeTarget(posX_, posY_, 0);
	RegistIntersectionRelativeTarget(intersectionManager);
}
bool StgPlayerObject::_IsValidSpell()
{
	bool res = true;
	res &= (state_ == STATE_NORMAL || (state_ == STATE_HIT && frameState_ > 0));
	res &= (objSpell_ == NULL || objSpell_->IsDeleted());
	return res;
}
void StgPlayerObject::CallSpell()
{
	if(!_IsValidSpell())return;
	if(!IsPermitSpell())return;

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objSpell_ = new StgPlayerSpellManageObject();
	int idSpell = objectManager->AddObject(objSpell_);

	gstd::value vUse = script_->RequestEvent(StgStagePlayerScript::EV_REQUEST_SPELL);
	if(!script_->IsBooleanValue(vUse))
		throw gstd::wexception(L"@Event(EV_REQUEST_SPELL)内でboolean型の値が返されていません。");
	bool bUse = vUse.as_boolean();
	if(!bUse)
	{
		objSpell_ = NULL;
		objectManager->DeleteObject(idSpell);
		return;
	}

	if(state_ == STATE_HIT)
	{
		state_ = STATE_NORMAL;
		infoPlayer_->frameRebirth_ = max(infoPlayer_->frameRebirth_ - frameRebirthDiff_, 0);
	}

	StgEnemyManager* enemyManager = stageController_->GetEnemyManager();
	ref_count_ptr<StgEnemyBossSceneObject>::unsync objBossScene = enemyManager->GetBossSceneObject();
	if(objBossScene != NULL)
	{
		objBossScene->AddPlayerSpellCount();
	}

	StgStageScriptManager* scriptManager = stageController_->GetScriptManagerP();
	scriptManager->RequestEventAll(StgStageScript::EV_PLAYER_SPELL);
}

void StgPlayerObject::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	StgIntersectionTarget_Player* own = (StgIntersectionTarget_Player*)ownTarget.GetPointer();
	int otherType = otherTarget->GetTargetType();
	switch(otherType)
	{
		case StgIntersectionTarget::TYPE_ENEMY_SHOT:
		{
			//敵弾
			if(own->IsGraze())
			{
				StgShotObject* objShot = (StgShotObject*)otherTarget->GetObject().GetPointer();
				if(objShot != NULL && objShot->IsValidGraze())
				{
					listGrazedShot_.push_back(otherTarget->GetObject());
					stageController_->GetStageInformation()->AddGraze(1);
				}
			}
			else 
			{
				ref_count_weak_ptr<StgShotObject>::unsync objShot = 
					ref_count_weak_ptr<StgShotObject>::unsync::DownCast(otherTarget->GetObject());
				if(objShot != NULL)
					hitObjectID_ = objShot->GetObjectID();
			}
		}
		break;

		case StgIntersectionTarget::TYPE_ENEMY:
		{
			//敵
			if(!own->IsGraze())
			{
				ref_count_weak_ptr<StgEnemyObject>::unsync objEnemy = 
					ref_count_weak_ptr<StgEnemyObject>::unsync::DownCast(otherTarget->GetObject());
				if(objEnemy != NULL)
					hitObjectID_ = objEnemy->GetObjectID();
			}
		}
		break;
	}

	
}
ref_count_ptr<StgPlayerObject>::unsync StgPlayerObject::GetOwnObject()
{
	return ref_count_ptr<StgPlayerObject>::unsync::DownCast(stageController_->GetMainRenderObject(idObject_));
}
bool StgPlayerObject::IsPermitShot()
{
	//以下のとき不可
	//・会話中
	return !bForbidShot_;
}
bool StgPlayerObject::IsPermitSpell()
{
	//以下のとき不可
	//・会話中
	//・ラストスペル中
	StgEnemyManager* enemyManager = stageController_->GetEnemyManager();
	bool bEnemyLastSpell = false;
	ref_count_ptr<StgEnemyBossSceneObject>::unsync objBossScene = enemyManager->GetBossSceneObject();
	if(objBossScene != NULL)
	{
		ref_count_ptr<StgEnemyBossSceneData>::unsync data = objBossScene->GetActiveData();
		if(data != NULL && data->IsLastSpell())
			bEnemyLastSpell = true;
	}

	return !bEnemyLastSpell && !bForbidSpell_;
}
bool StgPlayerObject::IsWaitLastSpell()
{
	bool res = IsPermitSpell() && state_ == STATE_HIT;
	return res;
}

/**********************************************************
//StgPlayerSpellObject
**********************************************************/
StgPlayerSpellObject::StgPlayerSpellObject(StgStageController* stageController)
{
	stageController_ = stageController;
	damage_ = 0;
	bEraseShot_ = true;
	life_ = 256 * 256 * 256;
}
void StgPlayerSpellObject::Work()
{
	if(IsDeleted())return;
	if(life_ <= 0)
	{
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}
void StgPlayerSpellObject::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	double damage = 0;
	int otherType = otherTarget->GetTargetType();
	switch(otherType)
	{
		case StgIntersectionTarget::TYPE_ENEMY:
		case StgIntersectionTarget::TYPE_ENEMY_SHOT:
		{
			damage = 1;
			break;
		}
	}
	life_ -= damage;
	life_ = max(life_, 0);
}


