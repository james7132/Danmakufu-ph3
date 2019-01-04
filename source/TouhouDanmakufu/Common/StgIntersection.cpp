#include"StgIntersection.hpp"
#include"StgShot.hpp"
#include"StgPlayer.hpp"
#include"StgEnemy.hpp"

/**********************************************************
//StgIntersectionManager
**********************************************************/
StgIntersectionManager::StgIntersectionManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int screenWidth = graphics->GetScreenWidth();
	int screenHeight = graphics->GetScreenWidth();

	_CreatePool(2);
	listSpace_.resize(3);
	for(int iSpace = 0 ; iSpace < listSpace_.size() ; iSpace++)
	{
		StgIntersectionSpace* space = new StgIntersectionSpace();
		space->Initialize(4, -100, -100, screenWidth + 100, screenHeight + 100);
		listSpace_[iSpace] = space;
	}
}
StgIntersectionManager::~StgIntersectionManager()
{
}
void StgIntersectionManager::Work()
{
	listEnemyTargetPoint_ = listEnemyTargetPointNext_;
	listEnemyTargetPointNext_.clear();

	int totalCheck = 0;
	std::vector<ref_count_ptr<StgIntersectionSpace> >::iterator itr = listSpace_.begin();
	for(; itr != listSpace_.end() ; itr++)
	{
		StgIntersectionSpace* space = (*itr).GetPointer();
		ref_count_ptr<StgIntersectionCheckList>::unsync listCheck = space->CreateIntersectionCheckList();
		int countCheck = listCheck->GetCheckCount();
		for(int iCheck = 0 ; iCheck < countCheck ; iCheck++)
		{
			//Getは1回しか使用できません
			ref_count_ptr<StgIntersectionTarget>::unsync targetA = listCheck->GetTargetA(iCheck);
			ref_count_ptr<StgIntersectionTarget>::unsync targetB = listCheck->GetTargetB(iCheck);

			bool bIntersected = IsIntersected(targetA, targetB);
			if(!bIntersected)continue;

			//Grazeの関係で、先に自機の当たり判定をする必要がある。
			ref_count_weak_ptr<StgIntersectionObject>::unsync objA = targetA->GetObject();
			ref_count_weak_ptr<StgIntersectionObject>::unsync objB = targetB->GetObject();
			if(objA != NULL)
			{
				objA->Intersect(targetA, targetB);
				objA->SetIntersected();

				if(objB != NULL)
				{
					int idObject = objB->GetDxScriptObjectID();
					objA->AddIntersectedId(idObject);
				}
			}
			
			if(objB != NULL)
			{
				objB->Intersect(targetB, targetA);
				objB->SetIntersected();
				if(objA != NULL)
				{
					int idObject = objA->GetDxScriptObjectID();
					objB->AddIntersectedId(idObject);
				}
			}
		}

		totalCheck += countCheck;
		space->ClearTarget();
	}

	_ArrangePool();

	ELogger* logger = ELogger::GetInstance();
	if(logger->IsWindowVisible())
	{
		int countUsed = GetUsedPoolObjectCount();
		int countCache = GetCachePoolObjectCount();
		logger->SetInfo(9, L"stg intersection_count", 
			StringUtility::Format(L"used=%4d, cache=%4d, total=%4d check=%4d", countUsed, countCache, countUsed+countCache, totalCheck ));
	}
}
void StgIntersectionManager::AddTarget(ref_count_ptr<StgIntersectionTarget>::unsync target)
{
	//SPACE_PLAYER_ENEMY = 0,//自機-敵、敵弾
	//SPACE_PLAYERSOHT_ENEMY,//自弾,スペル-敵
	//SPACE_PLAYERSHOT_ENEMYSHOT,//自弾,スペル-敵弾

	target->SetMortonNumber(-1);
	//target->ClearObjectIntersectedIdList();

	int type = target->GetTargetType();
	switch(type)
	{
		case StgIntersectionTarget::TYPE_PLAYER:
		{
			listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetA(target);
			break;
		}
		
		case StgIntersectionTarget::TYPE_PLAYER_SHOT:
		case StgIntersectionTarget::TYPE_PLAYER_SPELL:
		{
			listSpace_[SPACE_PLAYERSOHT_ENEMY]->RegistTargetA(target);

			//弾消し能力付加なら
			bool bEraseShot = false;
			if(type == StgIntersectionTarget::TYPE_PLAYER_SHOT)
			{
				StgShotObject* shot = (StgShotObject*)target->GetObject().GetPointer();
				if(shot != NULL)
					bEraseShot = shot->IsEraseShot();
			}
			else if(type == StgIntersectionTarget::TYPE_PLAYER_SPELL)
			{
				StgPlayerSpellObject* spell = (StgPlayerSpellObject*)target->GetObject().GetPointer();
				if(spell != NULL)
					bEraseShot = spell->IsEraseShot();
			}

			if(bEraseShot)
			{
				listSpace_[SPACE_PLAYERSHOT_ENEMYSHOT]->RegistTargetA(target);
			}
			break;
		}

		case StgIntersectionTarget::TYPE_ENEMY:
		{
			listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetB(target);
			listSpace_[SPACE_PLAYERSOHT_ENEMY]->RegistTargetB(target);

			ref_count_ptr<StgIntersectionTarget_Circle>::unsync circle =
				ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target);
			if(circle != NULL)
			{
				ref_count_weak_ptr<StgEnemyObject>::unsync objEnemy = 
					ref_count_weak_ptr<StgEnemyObject>::unsync::DownCast(target->GetObject());
				if(objEnemy != NULL)
				{
					int idObject = objEnemy->GetObjectID();
					POINT pos = {(int)circle->GetCircle().GetX(), (int)circle->GetCircle().GetY()};
					StgIntersectionTargetPoint tp;
					tp.SetObjectID(idObject);
					tp.SetPoint(pos);
					listEnemyTargetPointNext_.push_back(tp);
				}
			}

			break;
		}

		case StgIntersectionTarget::TYPE_ENEMY_SHOT:
		{
			listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetB(target);
			listSpace_[SPACE_PLAYERSHOT_ENEMYSHOT]->RegistTargetB(target);
			break;
		}
	}
}
void StgIntersectionManager::AddEnemyTargetToShot(ref_count_ptr<StgIntersectionTarget>::unsync target)
{
	target->SetMortonNumber(-1);
	//target->ClearObjectIntersectedIdList();

	int type = target->GetTargetType();
	switch(type)
	{
		case StgIntersectionTarget::TYPE_ENEMY:
		{
			listSpace_[SPACE_PLAYERSOHT_ENEMY]->RegistTargetB(target);

			ref_count_ptr<StgIntersectionTarget_Circle>::unsync circle = 
				ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target);
			if(circle != NULL)
			{
				ref_count_weak_ptr<StgEnemyObject>::unsync objEnemy = 
					ref_count_weak_ptr<StgEnemyObject>::unsync::DownCast(target->GetObject());
				if(objEnemy != NULL)
				{
					int idObject = objEnemy->GetObjectID();
					POINT pos = { (int)circle->GetCircle().GetX(), (int)circle->GetCircle().GetY()};
					StgIntersectionTargetPoint tp;
					tp.SetObjectID(idObject);
					tp.SetPoint(pos);
					listEnemyTargetPointNext_.push_back(tp);
				}
			}

			break;
		}
	}
}
void StgIntersectionManager::AddEnemyTargetToPlayer(ref_count_ptr<StgIntersectionTarget>::unsync target)
{
	target->SetMortonNumber(-1);
	//target->ClearObjectIntersectedIdList();

	int type = target->GetTargetType();
	switch(type)
	{
		case StgIntersectionTarget::TYPE_ENEMY:
		{
			listSpace_[SPACE_PLAYER_ENEMY]->RegistTargetB(target);
			break;
		}
	}
}

bool StgIntersectionManager::IsIntersected(ref_count_ptr<StgIntersectionTarget>::unsync& target1, ref_count_ptr<StgIntersectionTarget>::unsync& target2)
{
	bool res = false;
	int shape1 = target1->GetShape();
	int shape2 = target2->GetShape();
	if(shape1 == StgIntersectionTarget::SHAPE_CIRCLE && shape2 == StgIntersectionTarget::SHAPE_CIRCLE)
	{
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync c1 =
			ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target1);
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync c2 = 
			ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target2);
		res = DxMath::IsIntersected(c1->GetCircle(), c2->GetCircle());
	}
	else if((shape1 == StgIntersectionTarget::SHAPE_CIRCLE && shape2 == StgIntersectionTarget::SHAPE_LINE) ||
		(shape1 == StgIntersectionTarget::SHAPE_LINE && shape2 == StgIntersectionTarget::SHAPE_CIRCLE))
	{
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync c = NULL;
		ref_count_ptr<StgIntersectionTarget_Line>::unsync l = NULL;
		if(shape1 == StgIntersectionTarget::SHAPE_CIRCLE && shape2 == StgIntersectionTarget::SHAPE_LINE)
		{
			c = ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target1);
			l = ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(target2);
		}
		else
		{
			c = ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(target2);
			l = ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(target1);
		}

		res = IsIntersected(c->GetCircle(), l->GetLine());
	}
	else if(shape1 == StgIntersectionTarget::SHAPE_LINE && shape2 == StgIntersectionTarget::SHAPE_LINE)
	{
		ref_count_ptr<StgIntersectionTarget_Line>::unsync l1 = 
			ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(target1);
		ref_count_ptr<StgIntersectionTarget_Line>::unsync l2 =
			ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(target2);
		res = IsIntersected(l1->GetLine(), l2->GetLine());
	}
	return res;
}
bool StgIntersectionManager::IsIntersected(DxCircle& circle, DxWidthLine& line)
{
	//先端もしくは終端が円内にあるかを調べる
	{
		double radius = circle.GetR();
		double dist1 = pow(pow(circle.GetX()-line.GetX1(),2) + pow(circle.GetY()-line.GetY1(),2), 0.5);
		double dist2 = pow(pow(circle.GetX()-line.GetX2(),2) + pow(circle.GetY()-line.GetY2(),2), 0.5);
		if(radius >= dist1 || radius >= dist2)
			return true;
	}

	//線分内に円があるかを調べる
	{
		double lx1 = line.GetX2() - line.GetX1();
		double ly1 = line.GetY2() - line.GetY1();
		double cx1 = circle.GetX() - line.GetX1();
		double cy1 = circle.GetY() - line.GetY1();
		double inner1 = lx1 * cx1 + ly1 * cy1;

		double lx2 = line.GetX1() - line.GetX2();
		double ly2 = line.GetY1() - line.GetY2();
		double cx2 = circle.GetX() - line.GetX2();
		double cy2 = circle.GetY() - line.GetY2();
		double inner2 = lx2 * cx2 + ly2 * cy2;

		if(inner1 < 0 || inner2 < 0)
			return false;
	}

	if(false)
	{//tr1:レーザーの長さ、tr2:レーザーの先から判定先までの長さ
		double radius = circle.GetR();//pow(pow(line.GetX2()-line.GetX1(),2) + pow(line.GetY2()-line.GetY1(),2), 0.5);
		double dist1 = pow(pow(circle.GetX()-line.GetX1(),2) + pow(circle.GetY()-line.GetY1(),2), 0.5);
		double dist2 = pow(pow(circle.GetX()-line.GetX2(),2) + pow(circle.GetY()-line.GetY2(),2), 0.5);
		//tr1 -= 18;//端を判定しないための補正
		//if(tr1 < 18)tr1 = 18;
		if(radius < dist1 && radius < dist2)
			return false;
	}

	double lx = line.GetX2()-line.GetX1();
	double ly = line.GetY2()-line.GetY1();
	double px = circle.GetX()-line.GetX1();
	double py = circle.GetY()-line.GetY1();
	double u = pow(pow(lx,2) + pow(ly, 2), 0.5);//直線の距離
	if(u <= 0)return false;

	double ux = lx/u;//直線の単位ベクトルx
	double uy = ly/u;//直線の単位ベクトルz
	double d = px*ux + py*uy;//直線の単位ベクトルと始点から点までベクトルの内積
	double qx = d*ux;
	double qy = d*uy;
	double rx = px-qx;//点から直線までの最短距離ベクトルx
	double ry = py-qy;//点から直線までの最短距離ベクトルz
	double e = pow(pow(rx, 2) + pow(ry, 2),0.5);//直線のと点の距離
	double r = line.GetWidth() + circle.GetR();
	bool res  = e < r;
	return res;
}
bool StgIntersectionManager::IsIntersected(DxWidthLine& line1, DxWidthLine& line2)
{
	return false;
}
void StgIntersectionManager::_ResetPoolObject(gstd::ref_count_ptr<StgIntersectionTarget>::unsync& obj)
{
//	ELogger::WriteTop(StringUtility::Format("_ResetPoolObject:start:%s)", obj->GetInfoAsString().c_str()));
	obj->obj_ = NULL;
//	ELogger::WriteTop("_ResetPoolObject:end");
}
gstd::ref_count_ptr<StgIntersectionTarget>::unsync StgIntersectionManager::_CreatePoolObject(int type)
{
	gstd::ref_count_ptr<StgIntersectionTarget>::unsync res = NULL;
	switch(type)
	{
		case StgIntersectionTarget::SHAPE_CIRCLE:
			res = new StgIntersectionTarget_Circle();
			break;
		case StgIntersectionTarget::SHAPE_LINE:
			res = new StgIntersectionTarget_Line();
			break;
	}
	return res;
}
void StgIntersectionManager::CheckDeletedObject(std::string funcName)
{
	int countType = listUsedPool_.size();
	for(int iType = 0 ; iType < countType ; iType++)
	{
		std::list<gstd::ref_count_ptr<StgIntersectionTarget, false> >* listUsed = &listUsedPool_[iType];
		std::vector<gstd::ref_count_ptr<StgIntersectionTarget, false> >* listCache = &listCachePool_[iType];
		
		std::list<gstd::ref_count_ptr<StgIntersectionTarget, false> >::iterator itr = listUsed->begin();
		for(; itr != listUsed->end() ; itr++)
		{
			gstd::ref_count_ptr<StgIntersectionTarget, false> target = (*itr);
			ref_count_weak_ptr<DxScriptObjectBase>::unsync dxObj =
				ref_count_weak_ptr<DxScriptObjectBase>::unsync::DownCast(target->GetObject());
			if(dxObj != NULL && dxObj->IsDeleted())
			{
				ELogger::WriteTop(StringUtility::Format(L"%s(deleted):%s", funcName.c_str(), target->GetInfoAsString().c_str()));
			}
		}
	}
}

/**********************************************************
//StgIntersectionSpace
**********************************************************/
StgIntersectionSpace::StgIntersectionSpace()
{
	spaceLeft_ = 0;
	spaceTop_ = 0;
	spaceWidth_ = 0;
	spaceHeight_ = 0;
	unitWidth_ = 0;
	unitHeight_ = 0;
	countCell_ = 0;
	unitLevel_ = 0;

	// 各レベルでの空間数を算出
	listCountLevel_[0] = 1;
	for(int iLevel = 1 ; iLevel < MAX_LEVEL + 1 ; iLevel++)
		listCountLevel_[iLevel] = listCountLevel_[iLevel - 1] * 4;

	listCheck_ = new StgIntersectionCheckList();

}
StgIntersectionSpace::~StgIntersectionSpace()
{
}
bool StgIntersectionSpace::Initialize(int level, int left, int top, int right, int bottom)
{
	// 設定最高レベル以上の空間は作れない
	if(level >= MAX_LEVEL)
		return false;

	countCell_ = (listCountLevel_[level + 1] - 1) / 3;
	listCell_.resize(countCell_);
	for(int iCell = 0 ; iCell < listCell_.size() ; iCell++) 
	{
		listCell_[iCell].resize(2);
	}

	spaceLeft_ = left;
	spaceTop_ = top;
	spaceWidth_ = right - left;
	spaceHeight_ = bottom - top;

	unitWidth_ = spaceWidth_ / (1 << level);
	unitHeight_ = spaceHeight_ / (1 << level);
	unitLevel_ = level;

	return true;
}
bool StgIntersectionSpace::RegistTarget(int type, ref_count_ptr<StgIntersectionTarget>::unsync& target)
{
	RECT rect = target->GetIntersectionSapceRect();
	if(rect.right < spaceLeft_ || rect.bottom < spaceTop_ ||
		rect.left > (spaceLeft_ + spaceWidth_) || rect.top > (spaceTop_ + spaceHeight_))
		return false;

	// オブジェクトの境界範囲から登録モートン番号を算出
	bool res = false;
	int index = target->GetMortonNumber();
	if(index < 0)
	{
		index = _GetMortonNumber(rect.left, rect.top, rect.right, rect.bottom);
		target->SetMortonNumber(index);
	}

	if(index >= 0 && index < countCell_ )
	{
		listCell_[index][type].push_back(target);
		res = true;
	}

	return res;
}

void StgIntersectionSpace::ClearTarget()
{
	for(int iCell = 0 ; iCell < listCell_.size() ; iCell++) 
	{
		for(int iType = 0 ; iType < listCell_[iCell].size() ; iType++)
		{
			listCell_[iCell][iType].clear();
		}
	}
}
ref_count_ptr<StgIntersectionCheckList>::unsync StgIntersectionSpace::CreateIntersectionCheckList()
{
	ref_count_ptr<StgIntersectionCheckList>::unsync res = listCheck_;
	res->Clear();
	std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > > listStack;
	listStack.resize(listCell_[0].size());

	_WriteIntersectionCheckList(0, res, listStack);

	return res;
}
void StgIntersectionSpace::_WriteIntersectionCheckList(int indexSpace, ref_count_ptr<StgIntersectionCheckList>::unsync& listCheck, std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > > &listStack)
{
	std::vector<std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > >& listCell = listCell_[indexSpace];
	int typeCount = listCell.size();
	for(int iType1 = 0 ; iType1 < typeCount ; iType1++)
	{
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& list1 = listCell[iType1];
		int iType2 =0;
		for(iType2 = iType1 + 1 ; iType2 < typeCount ; iType2++)
		{
			std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& list2 = listCell[iType2];

			// ① 空間内のオブジェクト同士の衝突リスト作成
			std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >::iterator itr1 = list1.begin();
			for(; itr1 != list1.end() ; itr1++)
			{
				std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >::iterator itr2 = list2.begin();
				for(; itr2 != list2.end() ; itr2++)
				{
					ref_count_ptr<StgIntersectionTarget>::unsync target1 = (*itr1);
					ref_count_ptr<StgIntersectionTarget>::unsync target2 = (*itr2);
					listCheck->Add(target1, target2);
				}
			}

		}

		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& stack = listStack[iType1];
		for(iType2 = 0; iType2 < typeCount ; iType2++)
		{
			if(iType1 == iType2)continue;
			std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& list2 = listCell[iType2];

			// ② 衝突スタックとの衝突リスト作成
			std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >::iterator itrStack = stack.begin();
			for(; itrStack != stack.end() ; itrStack++)
			{
				std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >::iterator itr2 = list2.begin();	
				for(; itr2 != list2.end() ; itr2++)
				{
					ref_count_ptr<StgIntersectionTarget>::unsync target2 = (*itr2);
					ref_count_ptr<StgIntersectionTarget>::unsync targetStack = (*itrStack);
					if(iType1 < iType2)
						listCheck->Add(targetStack, target2);
					else
						listCheck->Add(target2, targetStack);
				}
			}
		}
	}

	//空間内のオブジェクトをスタックに追加
	int iType = 0;
	for(iType = 0 ; iType < typeCount ; iType++)
	{
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& list = listCell[iType];
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& stack = listStack[iType];
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >::iterator itr = list.begin();
		for(; itr != list.end() ; itr++)
		{
			ref_count_ptr<StgIntersectionTarget>::unsync target = (*itr);
			stack.push_back(target);
		}
	}

	// ③ 子空間に移動
	for(int iChild = 0 ; iChild < 4 ; iChild++)
	{
		int indexChild = indexSpace * 4 + 1 + iChild;
		if(indexChild < countCell_)
		{
			_WriteIntersectionCheckList(indexChild, listCheck, listStack);
		}
	}

	//スタックから解除
	for(iType = 0 ; iType < typeCount ; iType++)
	{
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& list = listCell[iType];
		std::vector<ref_count_ptr<StgIntersectionTarget>::unsync >& stack = listStack[iType];
		int count = list.size();
		for(int iCount = 0 ; iCount < count ; iCount++)
		{
			stack.pop_back();
		}
	}
}
unsigned int StgIntersectionSpace::_GetMortonNumber( float left, float top, float right, float bottom )
{
	// 座標から空間番号を算出
	// 最小レベルにおける各軸位置を算出
	unsigned int  LT = _GetPointElem(left, top);
	unsigned int  RB = _GetPointElem(right, bottom );

	// 空間番号の排他的論理和から
	// 所属レベルを算出
	unsigned int def = RB ^ LT;
	unsigned int hiLevel = 0;
	for(int iLevel = 0; iLevel<unitLevel_; iLevel++)
	{
		DWORD Check = (def>>(iLevel*2)) & 0x3;
		if( Check != 0 )
			hiLevel = iLevel+1;
	}
	DWORD spaceIndex = RB>>(hiLevel*2);
	DWORD addIndex = (listCountLevel_[unitLevel_ - hiLevel] - 1) / 3;
	spaceIndex += addIndex;

	if(spaceIndex > countCell_)
		return 0xffffffff;

	return spaceIndex;
}
unsigned int StgIntersectionSpace::_BitSeparate32(unsigned int n )
{
	// ビット分割関数
	n = (n|(n<<8)) & 0x00ff00ff;
	n = (n|(n<<4)) & 0x0f0f0f0f;
	n = (n|(n<<2)) & 0x33333333;
	return (n|(n<<1)) & 0x55555555;
}
unsigned short StgIntersectionSpace::_Get2DMortonNumber( unsigned short x, unsigned short y )
{
	// 2Dモートン空間番号算出関数
	return (unsigned short)(_BitSeparate32(x) | (_BitSeparate32(y)<<1));
}
unsigned int  StgIntersectionSpace::_GetPointElem(float pos_x, float pos_y )
{
	// 座標→線形4分木要素番号変換関数
	float val1 = max(pos_x-spaceLeft_, 0);
	float val2 = max(pos_y-spaceTop_, 0);
	return _Get2DMortonNumber(
		(unsigned short)(val1/unitWidth_), (unsigned short)(val2/unitHeight_) );
}

//StgIntersectionObject
void StgIntersectionObject::ClearIntersectionRelativeTarget()
{
	for(int iTarget = 0 ; iTarget < listRelativeTarget_.size() ; iTarget++)
	{
		ref_count_weak_ptr<StgIntersectionTarget>::unsync target = listRelativeTarget_[iTarget];
		target->SetObject(NULL);
	}
	listRelativeTarget_.clear();
}
void StgIntersectionObject::AddIntersectionRelativeTarget(ref_count_ptr<StgIntersectionTarget>::unsync target)
{
	listRelativeTarget_.push_back(target);
	int shape = target->GetShape();
	if(shape == StgIntersectionTarget::SHAPE_CIRCLE)
	{
		StgIntersectionTarget_Circle* tTarget = (StgIntersectionTarget_Circle*)target.GetPointer();
		listOrgCircle_.push_back(tTarget->GetCircle());
	}
	else if(shape == StgIntersectionTarget::SHAPE_LINE)
	{
		StgIntersectionTarget_Line* tTarget = (StgIntersectionTarget_Line*)target.GetPointer();
		listOrgLine_.push_back(tTarget->GetLine());
	}
}
void StgIntersectionObject::UpdateIntersectionRelativeTarget(int posX, int posY, double angle)
{
	int iCircle = 0;
	int iLine = 0;
	for(int iTarget = 0 ; iTarget < listRelativeTarget_.size() ; iTarget++)
	{
		ref_count_ptr<StgIntersectionTarget>::unsync target = listRelativeTarget_[iTarget];
		int shape = target->GetShape();
		if(shape == StgIntersectionTarget::SHAPE_CIRCLE)
		{
			StgIntersectionTarget_Circle* tTarget = (StgIntersectionTarget_Circle*)target.GetPointer();
			DxCircle org = listOrgCircle_[iCircle];
			int px = org.GetX() + posX;
			int py = org.GetY() + posY;

			DxCircle circle = tTarget->GetCircle();
			circle.SetX(px);
			circle.SetY(py);
			tTarget->SetCircle(circle);
			iCircle++;
		}
		else if(shape == StgIntersectionTarget::SHAPE_LINE)
		{
			StgIntersectionTarget_Line* tTarget = (StgIntersectionTarget_Line*)target.GetPointer();
			iLine++;
		}
	}
}
void StgIntersectionObject::RegistIntersectionRelativeTarget(StgIntersectionManager* manager)
{
	for(int iTarget = 0 ; iTarget < listRelativeTarget_.size() ; iTarget++)
	{
		ref_count_ptr<StgIntersectionTarget>::unsync target = listRelativeTarget_[iTarget];
		manager->AddTarget(target);
	}
}
int StgIntersectionObject::GetDxScriptObjectID()
{
	int res = DxScript::ID_INVALID;
	StgEnemyObject* objEnemy = dynamic_cast<StgEnemyObject*>(this);
	if(objEnemy != NULL)
	{
		res = objEnemy->GetObjectID();
	}

	return res;
}

/**********************************************************
//StgIntersectionTarget
**********************************************************/
void StgIntersectionTarget::ClearObjectIntersectedIdList()
{
	if(obj_ != NULL)
	{
		obj_->ClearIntersectedIdList();
	}
}
std::wstring StgIntersectionTarget::GetInfoAsString()
{
	std::wstring res;
	res += L"type[";
	switch(typeTarget_)
	{
	case TYPE_PLAYER:res += L"PLAYER";break;
	case TYPE_PLAYER_SHOT:res += L"PLAYER_SHOT";break;
	case TYPE_PLAYER_SPELL:res += L"PLAYER_SPELL";break;
	case TYPE_ENEMY:res += L"ENEMY";break;
	case TYPE_ENEMY_SHOT:res += L"ENEMY_SHOT";break;
	}
	res += L"] ";

	res += L"shape[";
	switch(shape_)
	{
	case SHAPE_CIRCLE:res += L"CIRCLE";break;
	case SHAPE_LINE:res += L"LINE";break;
	}
	res += L"] ";

	res += StringUtility::Format(L"address[%08x] ", (int)this);

	res += L"obj[";
	if(obj_ == NULL)
	{
		res += L"NULL";
	}
	else
	{
		ref_count_weak_ptr<DxScriptObjectBase>::unsync dxObj =
			ref_count_weak_ptr<DxScriptObjectBase>::unsync::DownCast(obj_);
		if(dxObj == NULL)
			res += L"UNKNOWN";
		else
		{
			int address = (int)dxObj.GetPointer();
			char* className = (char*)typeid(*this).name();
			res += StringUtility::Format(L"ref=%d, delete=%s, active=%s, class=%s[%08x]",
				dxObj.GetReferenceCount(),
				dxObj->IsDeleted() ? L"true" : L"false",
				dxObj->IsActive() ? L"true" : L"false",
				className, address);
		}
	}
	res += L"] ";

	return res;
}

