#include"StgItem.hpp"
#include"StgSystem.hpp"
#include"StgStageScript.hpp"
#include"StgPlayer.hpp"

/**********************************************************
//StgItemManager
**********************************************************/
StgItemManager::StgItemManager(StgStageController* stageController)
{
	stageController_ = stageController;
	listItemData_ = new StgItemDataList();

	std::wstring dir = EPathProperty::GetSystemImageDirectory();

	listSpriteItem_ = new SpriteList2D();
	std::wstring pathItem = dir + L"System_Stg_Item.png";
	ref_count_ptr<Texture> textureItem = new Texture();
	textureItem->CreateFromFile(pathItem);
	listSpriteItem_->SetTexture(textureItem);
	
	listSpriteDigit_ = new SpriteList2D();
	std::wstring pathDigit = dir + L"System_Stg_Digit.png";
	ref_count_ptr<Texture> textureDigit = new Texture();
	textureDigit->CreateFromFile(pathDigit);
	listSpriteDigit_->SetTexture(textureDigit);

	bDefaultBonusItemEnable_ = true;
	bAllItemToPlayer_ = false;

}
StgItemManager::~StgItemManager()
{
}
void StgItemManager::Work()
{
	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController_->GetPlayerObject();
	double px = objPlayer->GetX();
	double py = objPlayer->GetY();
	double pr = objPlayer->GetItemIntersectionRadius();
	int pAutoItemCollectY = objPlayer->GetAutoItemCollectY();

	std::list<ref_count_ptr<StgItemObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); )
	{
		ref_count_ptr<StgItemObject>::unsync obj = (*itr);
		if(obj->IsDeleted())
		{
			//obj->Clear();
			itr = listObj_.erase(itr);
		}
		else 
		{
			double ix = obj->GetPositionX();
			double iy = obj->GetPositionY();
			if(objPlayer->GetState() == StgPlayerObject::STATE_NORMAL)
			{
				double radius = pow(pow(px - ix, 2) + pow(py - iy, 2), 0.5);
				if(radius <= pr)
				{
					obj->Intersect(NULL, NULL);
				}

				if(bCancelToPlayer_)
				{
					//自動回収キャンセル
					obj->SetMoveToPlayer(false);
				}
				else if(obj->IsPermitMoveToPlayer())
				{
					bool bMoveToPlayer = false;
					if(pAutoItemCollectY >= 0)
					{
						//上部自動回収
						int typeMove = obj->GetMoveType();
						if(!obj->IsMoveToPlayer() && py <= pAutoItemCollectY)
							bMoveToPlayer = true;
					}

					if(listItemTypeToPlayer_.size() > 0)
					{
						//自機にアイテムを集める
						int typeItem = obj->GetItemType();
						bool bFind = listItemTypeToPlayer_.find(typeItem) != listItemTypeToPlayer_.end();
						if(bFind)
							bMoveToPlayer = true;
					}

					if(listCircleToPlayer_.size() > 0)
					{
						std::list<DxCircle>::iterator itr = listCircleToPlayer_.begin();
						for(; itr != listCircleToPlayer_.end() ;itr++)
						{
							DxCircle circle = *itr;
							double cr = pow(pow(ix - circle.GetX(), 2) + pow(iy - circle.GetY(), 2), 0.5);
							if(cr <= circle.GetR())
							{
								bMoveToPlayer = true;
								break;
							}
						}
					}

					if(bAllItemToPlayer_)
						bMoveToPlayer = true;

					if(bMoveToPlayer) 
						obj->SetMoveToPlayer(true);
				}

			}
			else
			{
				obj->SetMoveToPlayer(false);
			}

			itr++;
		}
	}
	listItemTypeToPlayer_.clear();
	listCircleToPlayer_.clear();
	bAllItemToPlayer_ = false;
	bCancelToPlayer_ = false;

}
void StgItemManager::Render(int targetPriority)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetCullingMode(D3DCULL_NONE);
	graphics->SetLightingEnable(false);

	//フォグを解除する
	DWORD bEnableFog = FALSE;
	graphics->GetDevice()->GetRenderState(D3DRS_FOGENABLE, &bEnableFog);
	if(bEnableFog)
		graphics->SetFogEnable(false);

	//拡大率など計算
	DxCamera2D* camera = graphics->GetCamera2D().GetPointer();
	D3DXMATRIX matCamera = camera->GetMatrix();

	std::list<ref_count_ptr<StgItemObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgItemObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;
		if(obj->GetRenderPriorityI() != targetPriority)continue;

		obj->RenderOnItemManager(matCamera);
	}

	int countBlendType = StgItemDataList::RENDER_TYPE_COUNT;
	int blendMode[] = {DirectGraphics::MODE_BLEND_ADD_ARGB, DirectGraphics::MODE_BLEND_ADD_RGB, DirectGraphics::MODE_BLEND_ALPHA};
	int typeRender[] = {StgShotDataList::RENDER_ADD_ARGB, StgShotDataList::RENDER_ADD_RGB, StgShotDataList::RENDER_ALPHA};
	ref_count_ptr<SpriteList2D>::unsync listSprite[] = {listSpriteDigit_, listSpriteItem_};
	for(int iBlend = 0 ; iBlend < countBlendType ; iBlend++)
	{
		graphics->SetBlendMode(blendMode[iBlend]);
		if(blendMode[iBlend] == DirectGraphics::MODE_BLEND_ADD_ARGB)
		{
			listSpriteDigit_->Render();
			listSpriteDigit_->ClearVertexCount();
		}
		else if(blendMode[iBlend] == DirectGraphics::MODE_BLEND_ALPHA)
		{
			listSpriteItem_->Render();
			listSpriteItem_->ClearVertexCount();
		}

		std::vector<ref_count_ptr<StgItemRenderer>::unsync >* listRenderer = 
			listItemData_->GetRendererList(typeRender[iBlend]);
		int iRender = 0;
		for(iRender = 0 ; iRender < listRenderer->size() ; iRender++)
			(listRenderer->at(iRender))->Render();
	}

	if(bEnableFog)
		graphics->SetFogEnable(true);
}
std::vector<bool> StgItemManager::GetValidRenderPriorityList()
{
	std::vector<bool> res;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	res.resize(objectManager->GetRenderBucketCapacity());

	std::list<ref_count_ptr<StgItemObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgItemObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;

		int pri = obj->GetRenderPriorityI();
		res[pri] = true;
	}

	return res;
}

bool StgItemManager::LoadItemData(std::wstring path, bool bReload)
{
	return listItemData_->AddItemDataList(path, bReload);
}
ref_count_ptr<StgItemObject>::unsync StgItemManager::CreateItem(int type)
{
	ref_count_ptr<StgItemObject>::unsync res;
	switch(type)
	{
	case StgItemObject::ITEM_1UP:
	case StgItemObject::ITEM_1UP_S:
		res = new StgItemObject_1UP(stageController_);
		break;
	case StgItemObject::ITEM_SPELL:
	case StgItemObject::ITEM_SPELL_S:
		res = new StgItemObject_Bomb(stageController_);
		break;
	case StgItemObject::ITEM_POWER:
	case StgItemObject::ITEM_POWER_S:
		res = new StgItemObject_Power(stageController_);
		break;
	case StgItemObject::ITEM_POINT:
	case StgItemObject::ITEM_POINT_S:
		res = new StgItemObject_Point(stageController_);
		break;
	case StgItemObject::ITEM_USER:
		res = new StgItemObject_User(stageController_);
		break;
	}
	res->SetItemType(type);

	return res;
}
void StgItemManager::CollectItemsAll()
{
	bAllItemToPlayer_ = true;
}
void StgItemManager::CollectItemsByType(int type)
{
	listItemTypeToPlayer_.insert(type);
}
void StgItemManager::CollectItemsInCircle(DxCircle circle)
{
	listCircleToPlayer_.push_back(circle);
}
void StgItemManager::CancelCollectItems()
{
	bCancelToPlayer_ = true;
}

/**********************************************************
//StgItemDataList
**********************************************************/
StgItemDataList::StgItemDataList()
{
	listRenderer_.resize(RENDER_TYPE_COUNT);
}
StgItemDataList::~StgItemDataList()
{
}
bool StgItemDataList::AddItemDataList(std::wstring path, bool bReload)
{
	if(!bReload && listReadPath_.find(path) != listReadPath_.end())return true;

	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if(reader == NULL) throw gstd::wexception(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());
	if(!reader->Open())throw gstd::wexception(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());
	std::string source = reader->ReadAllString();

	bool res = false;
	Scanner scanner(source);
	try
	{
		std::vector<ref_count_ptr<StgItemData>::unsync > listData;
		std::wstring pathImage = L"";
		RECT rcDelay = {-1, -1, -1, -1};
		while(scanner.HasNext())
		{
			Token& tok = scanner.Next();
			if(tok.GetType() == Token::TK_EOF)//Eofの識別子が来たらファイルの調査終了
			{
				break;
			}
			else if(tok.GetType() == Token::TK_ID)
			{
				std::wstring element = tok.GetElement();
				if(element == L"ItemData")
				{
					_ScanItem(listData, scanner);
				}
				else if(element == L"item_image")
				{
					scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
					pathImage = scanner.Next().GetString();
				}

				if(scanner.HasNext())
					tok = scanner.Next();

			}
		}

		//テクスチャ読み込み
		if(pathImage.size() == 0)throw gstd::wexception(L"画像ファイルが設定されていません。");
		std::wstring dir = PathProperty::GetFileDirectory(path);
		pathImage = StringUtility::Replace(pathImage, L"./", dir);

		ref_count_ptr<Texture> texture = new Texture();
		bool bTexture = texture->CreateFromFile(pathImage);
		if(!bTexture)throw gstd::wexception(L"画像ファイルが見つかりませんでした。");

		int textureIndex = -1;
		for(int iTexture = 0 ;iTexture < listTexture_.size() ; iTexture++)
		{
			ref_count_ptr<Texture> tSearch = listTexture_[iTexture];
			if(tSearch->GetName() == texture->GetName())
			{
				textureIndex = iTexture;
				break;
			}
		}
		if(textureIndex < 0)
		{
			textureIndex = listTexture_.size();
			listTexture_.push_back(texture);
			for(int iRender = 0 ; iRender < listRenderer_.size(); iRender++)
			{
				ref_count_ptr<StgItemRenderer>::unsync render = new StgItemRenderer();
				render->SetTexture(texture);
				listRenderer_[iRender].push_back(render);
			}
		}

		if(listData_.size() < listData.size())
			listData_.resize(listData.size());
		for(int iData = 0 ; iData < listData.size() ; iData++)
		{
			ref_count_ptr<StgItemData>::unsync data = listData[iData];
			if(data == NULL)continue;
			data->indexTexture_ = textureIndex;
			listData_[iData] = data;
		}

		listReadPath_.insert(path);
		Logger::WriteTop(StringUtility::Format(L"アイテムデータを読み込みました:%s", path.c_str()));
		res = true;
	}
	catch(gstd::wexception& e)
	{
		std::wstring log = StringUtility::Format(L"アイテムデータ読み込み失敗:%d行目(%s)", scanner.GetCurrentLine(), e.what());
		Logger::WriteTop(log);
		res = NULL;
	}
	catch(...)
	{
		std::wstring log = StringUtility::Format(L"アイテムデータ読み込み失敗:%d行目", scanner.GetCurrentLine());
		Logger::WriteTop(log);
		res = NULL;
	}

	return res;
}
void StgItemDataList::_ScanItem(std::vector<ref_count_ptr<StgItemData>::unsync >& listData, Scanner& scanner)
{
	Token& tok = scanner.Next();
	if(tok.GetType() == Token::TK_NEWLINE)tok = scanner.Next();
	scanner.CheckType(tok, Token::TK_OPENC);

	ref_count_ptr<StgItemData>::unsync data = new StgItemData(this);
	int id = -1;
	int typeItem = -1;

	while(true)
	{
		tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)
		{
			break;
		}
		else if(tok.GetType() == Token::TK_ID)
		{
			std::wstring element = tok.GetElement();

			if(element == L"id")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				id = scanner.Next().GetInteger();
			}
			else if(element == L"type")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				typeItem = scanner.Next().GetInteger();
			}
			else if(element == L"rect")
			{
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				RECT rect;
				rect.left = StringUtility::ToInteger(list[0]);
				rect.top = StringUtility::ToInteger(list[1]);
				rect.right = StringUtility::ToInteger(list[2]);
				rect.bottom = StringUtility::ToInteger(list[3]);
				data->rcSrc_ = rect;
			}
			else if(element == L"out")
			{
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				RECT rect;
				rect.left = StringUtility::ToInteger(list[0]);
				rect.top = StringUtility::ToInteger(list[1]);
				rect.right = StringUtility::ToInteger(list[2]);
				rect.bottom = StringUtility::ToInteger(list[3]);
				data->rcOut_ = rect;
			}
			else if(element == L"render")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				std::wstring render = scanner.Next().GetElement();
				if(render == L"ADD" || render == L"ADD_RGB")
					data->typeRender_ = DirectGraphics::MODE_BLEND_ADD_RGB;
				else if(render == L"ADD_ARGB")
					data->typeRender_ = DirectGraphics::MODE_BLEND_ADD_ARGB;
			}
			else if(element == L"alpha")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				data->alpha_ = scanner.Next().GetInteger();
			}
			else if(element == L"AnimationData")
			{
				_ScanAnimation(data, scanner);
			}
		}
	}

	if(id >= 0) 
	{
		if(listData.size() <= id) 
			listData.resize(id + 1);

		if(typeItem <0)
			typeItem = id;
		data->typeItem_ = typeItem;

		listData[id] = data;
	}
}
void StgItemDataList::_ScanAnimation(ref_count_ptr<StgItemData>::unsync itemData, Scanner& scanner)
{
	Token& tok = scanner.Next();
	if(tok.GetType() == Token::TK_NEWLINE)tok = scanner.Next();
	scanner.CheckType(tok, Token::TK_OPENC);

	while(true)
	{
		tok = scanner.Next();
		if(tok.GetType() == Token::TK_CLOSEC)
		{
			break;
		}
		else if(tok.GetType() == Token::TK_ID)
		{
			std::wstring element = tok.GetElement();

			if(element == L"animation_data")
			{
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				if(list.size() == 5) 
				{
					StgItemData::AnimationData anime;
					int frame = StringUtility::ToInteger(list[0]);
					RECT rcSrc = {
						StringUtility::ToInteger(list[1]),
						StringUtility::ToInteger(list[2]),
						StringUtility::ToInteger(list[3]),
						StringUtility::ToInteger(list[4]),
					};

					anime.frame_ = frame;
					anime.rcSrc_ = rcSrc;

					itemData->listAnime_.push_back(anime);
					itemData->totalAnimeFrame_ += frame;
				}
			}
		}
	}
}
std::vector<std::wstring> StgItemDataList::_GetArgumentList(Scanner& scanner)
{
	std::vector<std::wstring> res;
	scanner.CheckType(scanner.Next(), Token::TK_EQUAL);

	Token& tok = scanner.Next();

	if(tok.GetType() == Token::TK_OPENP)
	{
		while(true) 
		{
			tok = scanner.Next();
			int type = tok.GetType();
			if(type == Token::TK_CLOSEP)break;
			else if(type != Token::TK_COMMA)
			{
				std::wstring str = tok.GetElement();
				res.push_back(str);
			}
		}
	}
	else
	{
		res.push_back(tok.GetElement());
	}
	return res;
}

//StgItemData
StgItemData::StgItemData(StgItemDataList* listItemData)
{
	listItemData_ = listItemData;
	typeRender_ = DirectGraphics::MODE_BLEND_ALPHA;
	SetRect(&rcSrc_ , 0, 0, 0, 0);
	SetRect(&rcOut_ , 0, 0, 0, 0);
	alpha_ = 255;
	totalAnimeFrame_ = 0;
}
StgItemData::~StgItemData()
{
}
RECT StgItemData::GetRect(int frame)
{
	if(totalAnimeFrame_ == 0) 
		return rcSrc_;

	RECT res;
	frame = frame % totalAnimeFrame_;
	int total = 0;
	std::vector<AnimationData>::iterator itr = listAnime_.begin();
	for(;itr != listAnime_.end();itr++) 
	{
		//AnimationData* anime = itr;
		total += itr->frame_;
		if(total >= frame)
		{
			res = itr->rcSrc_;
			break;
		}
	}

	return res;
}
ref_count_ptr<Texture> StgItemData::GetTexture()
{
	ref_count_ptr<Texture> res = listItemData_->GetTexture(indexTexture_);
	return res;
}
StgItemRenderer* StgItemData::GetRenderer()
{
	StgItemRenderer* res = NULL;
	if(typeRender_ == DirectGraphics::MODE_BLEND_ALPHA)
		res = listItemData_->GetRenderer(indexTexture_, StgItemDataList::RENDER_ALPHA).GetPointer();
	else if(typeRender_ == DirectGraphics::MODE_BLEND_ADD_RGB)
		res = listItemData_->GetRenderer(indexTexture_, StgItemDataList::RENDER_ADD_RGB).GetPointer();
	else if(typeRender_ == DirectGraphics::MODE_BLEND_ADD_ARGB)
		res = listItemData_->GetRenderer(indexTexture_, StgItemDataList::RENDER_ADD_ARGB).GetPointer();
	return res;
}
StgItemRenderer* StgItemData::GetRenderer(int type)
{
	return listItemData_->GetRenderer(indexTexture_, type).GetPointer();
}

/**********************************************************
//StgItemRenderer
**********************************************************/
StgItemRenderer::StgItemRenderer()
{
	countRenderVertex_ = 0;
	SetVertexCount(256 * 256);

}
int StgItemRenderer::GetVertexCount()
{
	int res = countRenderVertex_;
	res = min(countRenderVertex_, vertex_.GetSize() / strideVertexStreamZero_);
	return res;
}
void StgItemRenderer::Render()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	IDirect3DDevice9* device = graphics->GetDevice();
	ref_count_ptr<Texture>& texture = texture_[0];
	if(texture != NULL)
		device->SetTexture(0, texture->GetD3DTexture());
	else
		device->SetTexture(0, NULL);
	device->SetFVF(VERTEX_TLX::fvf);

	device->DrawPrimitiveUP(typePrimitive_, _GetPrimitiveCount(), vertex_.GetPointer(), strideVertexStreamZero_);

	//描画対象をクリアする
	countRenderVertex_ = 0;
}
void StgItemRenderer::AddVertex(VERTEX_TLX& vertex)
{
	SetVertex(countRenderVertex_, vertex);
	countRenderVertex_++;
}
void StgItemRenderer::AddSquareVertex(VERTEX_TLX* listVertex)
{
	AddVertex(listVertex[0]);
	AddVertex(listVertex[2]);
	AddVertex(listVertex[1]);
	AddVertex(listVertex[1]);
	AddVertex(listVertex[2]);
	AddVertex(listVertex[3]);
}

/**********************************************************
//StgItemObject
**********************************************************/
StgItemObject::StgItemObject(StgStageController* stageController) : StgMoveObject(stageController)
{
	stageController_ = stageController;
	typeObject_ = StgStageScript::OBJ_ITEM;

	pattern_ = new StgMovePattern_Item(this);
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	score_ = 0;

	bMoveToPlayer_ = false;
	bPermitMoveToPlayer_ = true;
	bChangeItemScore_ = true;

	int priItemI = stageController_->GetStageInformation()->GetItemObjectPriority();
	double priItemD = (double)priItemI / (stageController_->GetMainObjectManager()->GetRenderBucketCapacity() - 1);
	SetRenderPriority(priItemD);
}
void StgItemObject::Work()
{
	bool bDefaultMovePattern = ref_count_ptr<StgMovePattern_Item>::unsync::DownCast(GetPattern()) != NULL;
	if(!bDefaultMovePattern && IsMoveToPlayer())
	{
		double speed = 8;
		ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController_->GetPlayerObject();
		double angle = atan2(objPlayer->GetY() - GetPositionY(), objPlayer->GetX() - GetPositionX());
		double angDirection = Math::RadianToDegree(angle);
		SetSpeed(speed);
		SetDirectionAngle(angDirection);
	}
	StgMoveObject::_Move();
	SetX(posX_);
	SetY(posY_);

	_DeleteInAutoClip();
}
void StgItemObject::RenderOnItemManager(D3DXMATRIX mat)
{
	StgItemManager* itemManager = stageController_->GetItemManager();
	SpriteList2D* renderer = typeItem_ == ITEM_SCORE ?
		itemManager->GetDigitRenderer() : itemManager->GetItemRenderer();

	if(typeItem_ != ITEM_SCORE)
	{
		double scale = 1.0;
		switch(typeItem_)
		{
		case ITEM_1UP:
		case ITEM_SPELL:
		case ITEM_POWER:
		case ITEM_POINT:
			scale = 1.0;
			break;

		case ITEM_1UP_S:
		case ITEM_SPELL_S:
		case ITEM_POWER_S:
		case ITEM_POINT_S:
		case ITEM_BONUS:
			scale = 0.75;
			break;
		}

		RECT rcSrc;
		switch(typeItem_)
		{
		case ITEM_1UP:
		case ITEM_1UP_S:
			SetRect(&rcSrc, 1, 1, 16, 16);
			break;
		case ITEM_SPELL:
		case ITEM_SPELL_S:
			SetRect(&rcSrc, 20, 1, 35, 16);
			break;
		case ITEM_POWER:
		case ITEM_POWER_S:
			SetRect(&rcSrc, 40, 1, 55, 16);
			break;
		case ITEM_POINT:
		case ITEM_POINT_S:
			SetRect(&rcSrc, 1, 20, 16, 35);
			break;
		case ITEM_BONUS:
			SetRect(&rcSrc, 20, 20, 35, 35);
			break;
		}

		//上にはみ出している
		double posY = posY_;
		D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255);
		if(posY_ <= 0)
		{
			D3DCOLOR colorOver = D3DCOLOR_ARGB(255, 255, 255, 255);
			switch(typeItem_)
			{
			case ITEM_1UP:
			case ITEM_1UP_S:
				colorOver = D3DCOLOR_ARGB(255, 236, 0, 236);
				break;
			case ITEM_SPELL:
			case ITEM_SPELL_S:
				colorOver = D3DCOLOR_ARGB(255, 0, 160, 0);
				break;
			case ITEM_POWER:
			case ITEM_POWER_S:
				colorOver = D3DCOLOR_ARGB(255, 209, 0, 0);
				break;
			case ITEM_POINT:
			case ITEM_POINT_S:
				colorOver = D3DCOLOR_ARGB(255, 0, 0, 160);
				break;
			}
			if(color != colorOver)
			{
				SetRect(&rcSrc, 113, 1, 126, 10);
				posY = 6;
			}
			color = colorOver;
		}

		RECT_D rcSrcD = GetRectD(rcSrc);
		renderer->SetColor(color);
		renderer->SetPosition(posX_, posY, 0);
		renderer->SetScaleXYZ(scale, scale, scale);
		renderer->SetSourceRect(rcSrcD);
		renderer->SetDestinationCenter();
		renderer->AddVertex();
	}
	else
	{
		renderer->SetScaleXYZ(1.0, 1.0, 1.0);
		renderer->SetColor(color_);
		renderer->SetPosition(0, 0, 0);

		int fontSize = 14;
		_int64 score = score_;
		std::vector<int> listNum;
		while(true)
		{
			int tnum = score % 10;
			score /= 10;
			listNum.push_back(tnum);
			if(score == 0)break;
		}		
		for(int iNum = listNum.size() - 1; iNum >= 0 ; iNum--)
		{
			RECT_D rcSrc = {(double)(listNum[iNum]*36), 0.,
				(double)((listNum[iNum]+1)*36-1), 31.};
			RECT_D rcDest = { (double)(posX_+(listNum.size()-1-iNum)*fontSize/2), (double)posY_,
				(double)(posX_+(listNum.size()-iNum)*fontSize/2), (double)(posY_+fontSize)};
			renderer->SetSourceRect(rcSrc);
			renderer->SetDestinationRect(rcDest);
			renderer->AddVertex();
		}
	}
}
void StgItemObject::_DeleteInAutoClip()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();

	RECT rcClip;
	ZeroMemory(&rcClip, sizeof(RECT));
	rcClip.left = -64;
	rcClip.right = graphics->GetScreenWidth() + 64;
	rcClip.bottom = graphics->GetScreenHeight() + 64;
	bool bDelete = (posX_ < rcClip.left || posX_ > rcClip.right || posY_ > rcClip.bottom);
	if(!bDelete)return;

	stageController_->GetMainObjectManager()->DeleteObject(GetObjectID());
}
void StgItemObject::_CreateScoreItem()
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	StgItemManager* itemManager = stageController_->GetItemManager();
	ref_count_ptr<StgItemObject_Score>::unsync obj = new StgItemObject_Score(stageController_);
	obj->SetX(posX_);
	obj->SetY(posY_);
	obj->SetScore(score_);
	objectManager->AddObject(obj);
	itemManager->AddItem(obj);
}
void StgItemObject::_NotifyEventToPlayerScript(std::vector<long double>& listValue)
{
	//自機スクリプトへ通知
	ref_count_ptr<StgPlayerObject>::unsync player = stageController_->GetPlayerObject();
	StgStagePlayerScript* scriptPlayer = player->GetPlayerScript();
	std::vector<gstd::value> listScriptValue;
	for(int iVal = 0 ; iVal < listValue.size() ; iVal++)
	{
		listScriptValue.push_back(scriptPlayer->CreateRealValue(listValue[iVal]));
	}

	scriptPlayer->RequestEvent(StgStageItemScript::EV_GET_ITEM, listScriptValue);
}
void StgItemObject::_NotifyEventToItemScript(std::vector<long double>& listValue)
{
	//アイテムスクリプトへ通知
	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	_int64 idItemScript = stageScriptManager->GetItemScriptID();
	if(idItemScript != StgControlScriptManager::ID_INVALID)
	{
		ref_count_ptr<ManagedScript> scriptItem = stageScriptManager->GetScript(idItemScript);
		if(scriptItem != NULL)
		{
			std::vector<gstd::value> listScriptValue;
			for(int iVal = 0 ; iVal < listValue.size() ; iVal++)
			{
				listScriptValue.push_back(scriptItem->CreateRealValue(listValue[iVal]));
			}
			scriptItem->RequestEvent(StgStageItemScript::EV_GET_ITEM, listScriptValue);
		}
	}
}
void StgItemObject::SetAlpha(int alpha)
{
	color_ = ColorAccess::SetColorA(color_, alpha);
}
void StgItemObject::SetColor(int r, int g, int b)
{
	color_ = ColorAccess::SetColorR(color_, r);
	color_ = ColorAccess::SetColorG(color_, g);
	color_ = ColorAccess::SetColorB(color_, b);
}
void StgItemObject::SetToPosition(POINT pos)
{
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetToPosition(pos);
}
int StgItemObject::GetMoveType()
{
	int res = StgMovePattern_Item::MOVE_NONE;

	StgMovePattern_Item* move = dynamic_cast<StgMovePattern_Item*>(pattern_.GetPointer());
	if(move != NULL)
		res = move->GetItemMoveType();
	return res;
}
void StgItemObject::SetMoveType(int type)
{
	StgMovePattern_Item* move = dynamic_cast<StgMovePattern_Item*>(pattern_.GetPointer());
	if(move != NULL)
		move->SetItemMoveType(type);
}


//StgItemObject_1UP
StgItemObject_1UP::StgItemObject_1UP(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_1UP;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
}
void StgItemObject_1UP::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	std::vector<long double> listValue;
	listValue.push_back(typeItem_);
	listValue.push_back(idObject_);
	_NotifyEventToPlayerScript(listValue);
	_NotifyEventToItemScript(listValue);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}

//StgItemObject_Bomb
StgItemObject_Bomb::StgItemObject_Bomb(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_SPELL;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
}
void StgItemObject_Bomb::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	std::vector<long double> listValue;
	listValue.push_back(typeItem_);
	listValue.push_back(idObject_);
	_NotifyEventToPlayerScript(listValue);
	_NotifyEventToItemScript(listValue);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}

//StgItemObject_Power
StgItemObject_Power::StgItemObject_Power(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_POWER;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
	score_ = 10;
}
void StgItemObject_Power::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	if(bChangeItemScore_)
		_CreateScoreItem();
	stageController_->GetStageInformation()->AddScore(score_);

	std::vector<long double> listValue;
	listValue.push_back(typeItem_);
	listValue.push_back(idObject_);
	_NotifyEventToPlayerScript(listValue);
	_NotifyEventToItemScript(listValue);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}

//StgItemObject_Point
StgItemObject_Point::StgItemObject_Point(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_POINT;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_TOPOSITION_A);
}
void StgItemObject_Point::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	if(bChangeItemScore_)
		_CreateScoreItem();
	stageController_->GetStageInformation()->AddScore(score_);

	std::vector<long double> listValue;
	listValue.push_back(typeItem_);
	listValue.push_back(idObject_);
	_NotifyEventToPlayerScript(listValue);
	_NotifyEventToItemScript(listValue);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}

//StgItemObject_Bonus
StgItemObject_Bonus::StgItemObject_Bonus(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_BONUS;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_TOPLAYER);

	int graze = stageController->GetStageInformation()->GetGraze();
	score_ = (int)(graze / 40) * 10 + 300;
}
void StgItemObject_Bonus::Work()
{
	StgItemObject::Work();

	ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController_->GetPlayerObject();
	if(objPlayer->GetState() != StgPlayerObject::STATE_NORMAL)
	{
		_CreateScoreItem();
		stageController_->GetStageInformation()->AddScore(score_);

		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(GetObjectID());
	}
}
void StgItemObject_Bonus::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	_CreateScoreItem();
	stageController_->GetStageInformation()->AddScore(score_);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}

//StgItemObject_Score
StgItemObject_Score::StgItemObject_Score(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_SCORE;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_SCORE);

	bPermitMoveToPlayer_ = false;

	frameDelete_ = 0;
}
void StgItemObject_Score::Work()
{
	StgItemObject::Work();
	int alpha = 255 - frameDelete_ * 8;
	color_ = D3DCOLOR_ARGB(alpha, alpha, alpha, alpha);

	if(frameDelete_ > 30)
	{
		stageController_->GetMainObjectManager()->DeleteObject(GetObjectID());
		return;
	}
	frameDelete_++;
}
void StgItemObject_Score::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
}

//StgItemObject_User
StgItemObject_User::StgItemObject_User(StgStageController* stageController) : StgItemObject(stageController)
{
	typeItem_ = ITEM_USER;
	idImage_ = -1;
	frameWork_ = 0;
	StgMovePattern_Item* move = (StgMovePattern_Item*)pattern_.GetPointer();
	move->SetItemMoveType(StgMovePattern_Item::MOVE_DOWN);

	bChangeItemScore_ = true;
}
void StgItemObject_User::SetImageID(int id)
{
	idImage_ = id;
	StgItemData* data = _GetItemData();
	if(data != NULL)
	{
		typeItem_ = data->GetItemType();
	}
}
StgItemData* StgItemObject_User::_GetItemData()
{
	StgItemData* res = NULL;
	StgItemManager* itemManager = stageController_->GetItemManager();
	StgItemDataList* dataList = itemManager->GetItemDataList();

	if(dataList != NULL)
	{
		res = dataList->GetData(idImage_).GetPointer();
	}

	return res;
}
void StgItemObject_User::_SetVertexPosition(VERTEX_TLX& vertex, float x, float y, float z, float w)
{
	float bias = -0.5f;
	vertex.position.x = x + bias;
	vertex.position.y = y + bias;
	vertex.position.z = z;
	vertex.position.w = w;
}
void StgItemObject_User::_SetVertexUV(VERTEX_TLX& vertex, float u, float v)
{
	StgItemData* itemData = _GetItemData();
	if(itemData == NULL)return;
	
	ref_count_ptr<Texture> texture = itemData->GetTexture();
	int width = texture->GetWidth();
	int height = texture->GetHeight();
	vertex.texcoord.x = u / width;
	vertex.texcoord.y = v / height;
}
void StgItemObject_User::_SetVertexColorARGB(VERTEX_TLX& vertex, D3DCOLOR color)
{
	vertex.diffuse_color = color;
}
void StgItemObject_User::Work()
{
	StgItemObject::Work();
	frameWork_++;
}
void StgItemObject_User::RenderOnItemManager(D3DXMATRIX mat)
{
	if(!IsVisible())return;

	StgItemData* itemData = _GetItemData();
	if(itemData == NULL)return;

	int objBlendType = GetBlendType();
	StgItemRenderer* renderer = itemData->GetRenderer();
	if(renderer == NULL)return;

	D3DXMATRIX matScale;
	D3DXMATRIX matRot;
	D3DXMATRIX matTrans;
	RECT rcSrc;
	D3DCOLOR color;
	{
		//上にはみ出している
		bool bOutY = false;
		rcSrc = itemData->GetRect(frameWork_);
		if(position_.y + (rcSrc.bottom - rcSrc.top) / 2 <= 0)
		{
			bOutY = true;
			rcSrc = itemData->GetOut();
		}

		double angleZ = angle_.z;
		if(!bOutY)
		{
			D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
			D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angleZ));
			D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		}
		else
		{
			D3DXMatrixIdentity(&matScale);
			D3DXMatrixIdentity(&matRot);
			D3DXMatrixTranslation(&matTrans, position_.x, (rcSrc.bottom - rcSrc.top) / 2, position_.z);
		}

		mat = matScale * matRot * matTrans * mat;

		bool bBlendAddRGB = (objBlendType == DirectGraphics::MODE_BLEND_ADD_RGB);

		color = color_;
		double alpha = itemData->GetAlpha() / 255.0;
		if(bBlendAddRGB)
			color = ColorAccess::ApplyAlpha(color, alpha);
		else
		{
			int colorA = ColorAccess::GetColorA(color);
			color = ColorAccess::SetColorA(color, alpha * colorA);
		}
	}

	int width = rcSrc.right - rcSrc.left;
	int height = rcSrc.bottom - rcSrc.top;
	RECT rcDest = {-width / 2, -height / 2, width / 2, height / 2};
	if(width % 2 == 1)rcDest.right += 1;
	if(height % 2 == 1)rcDest.bottom += 1;

	//if(bIntersected_)color = D3DCOLOR_ARGB(255, 255, 0, 0);//接触テスト

	VERTEX_TLX verts[4];
	int srcX[] = {rcSrc.left, rcSrc.right, rcSrc.left, rcSrc.right};
	int srcY[] = {rcSrc.top, rcSrc.top, rcSrc.bottom, rcSrc.bottom};
	int destX[] = {rcDest.left, rcDest.right, rcDest.left, rcDest.right};
	int destY[] = {rcDest.top, rcDest.top, rcDest.bottom, rcDest.bottom};
	for(int iVert = 0 ;iVert < 4 ; iVert++)
	{
		_SetVertexUV(verts[iVert], srcX[iVert], srcY[iVert]);
		_SetVertexPosition(verts[iVert], destX[iVert], destY[iVert]);
		_SetVertexColorARGB(verts[iVert], color);
		verts[iVert].position = DxMath::VectMatMulti(verts[iVert].position, mat);
	}

	renderer->AddVertex(verts[0]);
	renderer->AddVertex(verts[2]);
	renderer->AddVertex(verts[1]);
	renderer->AddVertex(verts[1]);
	renderer->AddVertex(verts[2]);
	renderer->AddVertex(verts[3]);
}
void StgItemObject_User::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	if(bChangeItemScore_)
		_CreateScoreItem();
	stageController_->GetStageInformation()->AddScore(score_);

	std::vector<long double> listValue;
	listValue.push_back(typeItem_);
	listValue.push_back(idObject_);
	_NotifyEventToItemScript(listValue);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(GetObjectID());
}


/**********************************************************
//StgMovePattern_Item
**********************************************************/
StgMovePattern_Item::StgMovePattern_Item(StgMoveObject* target) : StgMovePattern(target)
{
	frame_ = 0;
	typeMove_ = MOVE_DOWN;
	speed_ = 3;
	angDirection_ = 270;
	ZeroMemory(&posTo_, sizeof(POINT));
}
void StgMovePattern_Item::Move()
{
	StgItemObject* itemObject = (StgItemObject*)target_;
	StgStageController* stageController = itemObject->GetStageController();

	double px = target_->GetPositionX();
	double py = target_->GetPositionY();
	if(typeMove_ == MOVE_TOPLAYER || itemObject->IsMoveToPlayer())
	{
		speed_ = 8;
		ref_count_ptr<StgPlayerObject>::unsync objPlayer = stageController->GetPlayerObject();
		double angle = atan2(objPlayer->GetY() - py, objPlayer->GetX() - px);
		angDirection_ = Math::RadianToDegree(angle);
	}
	else if(typeMove_ == MOVE_TOPOSITION_A)
	{
		double tarX = px;
		double tarY = py;

		double toX = posTo_.x;
		double toY = posTo_.y;
		speed_ = pow(pow(toX - tarX, 2) + pow(toY - tarY, 2), 0.5) / 16;

		double angle = atan2(toY - tarY, toX - tarX);
		angDirection_ = Math::RadianToDegree(angle);
		if(frame_ == 60)
		{
			speed_ = 0;
			angDirection_ = 90;
			typeMove_ = MOVE_DOWN;
		}
	}
	else if(typeMove_ == MOVE_DOWN)
	{
		speed_ += 3.0f/60.0f;
		if(speed_ > 2.5f)speed_ = 2.5f;
		angDirection_ = 90;
	}
	else if(typeMove_ == MOVE_SCORE)
	{
		speed_ = 1;
		angDirection_ = 270;
	}

	if(typeMove_ != MOVE_NONE)
	{
		double sx = speed_ * cos(Math::DegreeToRadian(angDirection_));
		double sy = speed_ * sin(Math::DegreeToRadian(angDirection_));
		px = target_->GetPositionX() + sx;
		py = target_->GetPositionY() + sy;
		target_->SetPositionX(px);
		target_->SetPositionY(py);
	}

	frame_++;
}

