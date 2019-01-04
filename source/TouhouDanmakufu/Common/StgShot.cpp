#include"StgShot.hpp"
#include"StgSystem.hpp"
#include"StgIntersection.hpp"
#include"StgItem.hpp"

/**********************************************************
//StgShotManager
**********************************************************/
StgShotManager::StgShotManager(StgStageController* stageController)
{
	stageController_ = stageController;

	listPlayerShotData_ = new StgShotDataList();
	listEnemyShotData_ = new StgShotDataList();
}
StgShotManager::~StgShotManager()
{
	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj != NULL)
		{
			obj->ClearShotObject();
		}
	}
}
void StgShotManager::Work()
{
	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); )
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())
		{
			obj->ClearShotObject();
			itr = listObj_.erase(itr);
		}
		else if(!obj->IsActive())
		{
			itr = listObj_.erase(itr);
		}
		else itr++;
	}

}
void StgShotManager::Render(int targetPriority)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetZBufferEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetCullingMode(D3DCULL_NONE);
	graphics->SetLightingEnable(false);
	//graphics->SetTextureFilter(DirectGraphics::MODE_TEXTURE_FILTER_POINT);
	//			MODE_TEXTURE_FILTER_POINT,//補間なし
	//			MODE_TEXTURE_FILTER_LINEAR,//線形補間
	//フォグを解除する
	DWORD bEnableFog = FALSE;
	graphics->GetDevice()->GetRenderState(D3DRS_FOGENABLE, &bEnableFog);
	if(bEnableFog)
		graphics->SetFogEnable(false);

	DxCamera2D* camera = graphics->GetCamera2D().GetPointer();
	D3DXMATRIX matCamera = camera->GetMatrix();

	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;
		if(!obj->IsActive())continue;
		if(obj->GetRenderPriorityI() != targetPriority)continue;
		obj->RenderOnShotManager(matCamera);
	}

	//描画
	int countBlendType = StgShotDataList::RENDER_TYPE_COUNT;
	int blendMode[] = 
	{
		DirectGraphics::MODE_BLEND_ADD_ARGB,
		DirectGraphics::MODE_BLEND_ADD_RGB,
		DirectGraphics::MODE_BLEND_MULTIPLY,
		DirectGraphics::MODE_BLEND_SUBTRACT,
		DirectGraphics::MODE_BLEND_INV_DESTRGB,
		DirectGraphics::MODE_BLEND_ALPHA
	};
	int typeRender[] = 
	{
		StgShotDataList::RENDER_ADD_ARGB, 
		StgShotDataList::RENDER_ADD_RGB,
		StgShotDataList::RENDER_MULTIPLY,
		StgShotDataList::RENDER_SUBTRACT,
		StgShotDataList::RENDER_INV_DESTRGB,
		StgShotDataList::RENDER_ALPHA
	};
	for(int iBlend = 0 ; iBlend < countBlendType ; iBlend++)
	{
		graphics->SetBlendMode(blendMode[iBlend]);
		std::vector<ref_count_ptr<StgShotRenderer>::unsync >* listPlayer = 
			listPlayerShotData_->GetRendererList(typeRender[iBlend]);
		int iRender = 0;
		for(iRender = 0 ; iRender < listPlayer->size() ; iRender++)
			(listPlayer->at(iRender))->Render();

		std::vector<ref_count_ptr<StgShotRenderer>::unsync >* listEnemy = 
			listEnemyShotData_->GetRendererList(typeRender[iBlend]);
		for(iRender = 0 ; iRender < listEnemy->size() ; iRender++)
			(listEnemy->at(iRender))->Render();
	}

	if(bEnableFog)
		graphics->SetFogEnable(true);
}
void StgShotManager::RegistIntersectionTarget()
{
	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(!obj->IsDeleted() && obj->IsActive())
		{
			obj->ClearIntersectedIdList();
			obj->RegistIntersectionTarget();
		}
	}
}

bool StgShotManager::LoadPlayerShotData(std::wstring path, bool bReload)
{
	return listPlayerShotData_->AddShotDataList(path, bReload);
}
bool StgShotManager::LoadEnemyShotData(std::wstring path, bool bReload)
{
	return listEnemyShotData_->AddShotDataList(path, bReload);
}
RECT StgShotManager::GetShotAutoDeleteClipRect()
{
	ref_count_ptr<StgStageInformation> stageInfo = stageController_->GetStageInformation();
	RECT rcStgFrame = stageInfo->GetStgFrameRect();
	RECT rcClip = stageInfo->GetShotAutoDeleteClip();

	int width = rcStgFrame.right - rcStgFrame.left;
	int height = rcStgFrame.bottom - rcStgFrame.top;
	rcClip.right += width;
	rcClip.bottom += height;
	return rcClip;
}

void StgShotManager::DeleteInCircle(int typeDelete, int typeTo, int typeOnwer, int cx, int cy, double radius)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();

	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;

		if(typeOnwer != StgShotObject::OWNER_NULL && 
			obj->GetOwnerType() != typeOnwer)continue;

		if(typeDelete == DEL_TYPE_SHOT && obj->GetLife() == StgShotObject::LIFE_SPELL_REGIST)continue;

		double sx = obj->GetPositionX();
		double sy = obj->GetPositionY();

		double tr = pow(pow(cx-sx, 2) + pow(cy-sy, 2), 0.5);
		if(tr <= radius)
		{
			if(typeTo == TO_TYPE_IMMEDIATE)
			{
				obj->DeleteImmediate();
			}
			else if(typeTo == TO_TYPE_FADE)
			{
				obj->SetFadeDelete();
			}
			else if(typeTo == TO_TYPE_ITEM)
			{
				obj->ConvertToItem();
			}
		}

	}
}

std::vector<int> StgShotManager::GetShotIdInCircle(int typeOnwer, int cx, int cy, int radius)
{
	std::vector<int> res;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();

	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;

		if(typeOnwer != StgShotObject::OWNER_NULL && 
			obj->GetOwnerType() != typeOnwer)continue;

		double sx = obj->GetPositionX();
		double sy = obj->GetPositionY();

		double tr = pow(pow(cx-sx, 2) + pow(cy-sy, 2), 0.5);
		if(tr <= radius)
		{
			int id = obj->GetObjectID();
			res.push_back(id);
		}

	}
	return res;
}
int StgShotManager::GetShotCount(int typeOnwer)
{
	int res = 0;
	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;

		if(typeOnwer != StgShotObject::OWNER_NULL && 
			obj->GetOwnerType() != typeOnwer)continue;

		res++;
	}
	return res;
}
std::vector<bool> StgShotManager::GetValidRenderPriorityList()
{
	std::vector<bool> res;
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	res.resize(objectManager->GetRenderBucketCapacity());

	std::list<ref_count_ptr<StgShotObject>::unsync >::iterator itr = listObj_.begin();
	for(; itr != listObj_.end(); itr++)
	{
		ref_count_ptr<StgShotObject>::unsync obj = (*itr);
		if(obj->IsDeleted())continue;

		int pri = obj->GetRenderPriorityI();
		res[pri] = true;
	}

	return res;
}
void StgShotManager::SetDeleteEventEnableByType(int type, bool bEnable)
{
	int bit = 0;
	switch(type)
	{
	case StgStageShotScript::EV_DELETE_SHOT_IMMEDIATE:
		bit = StgShotManager::BIT_EV_DELETE_IMMEDIATE;
		break;
	case StgStageShotScript::EV_DELETE_SHOT_TO_ITEM:
		bit = StgShotManager::BIT_EV_DELETE_TO_ITEM;
		break;
	case StgStageShotScript::EV_DELETE_SHOT_FADE:
		bit = StgShotManager::BIT_EV_DELETE_FADE;
		break;
	}

	if(bEnable)
	{
		listDeleteEventEnable_.set(bit);
	}
	else
	{
		listDeleteEventEnable_.reset(bit);
	}
}
bool StgShotManager::IsDeleteEventEnable(int bit)
{
	bool res = listDeleteEventEnable_[bit];
	return res;
}

/**********************************************************
//StgShotDataList
**********************************************************/
StgShotDataList::StgShotDataList()
{
	listRenderer_.resize(RENDER_TYPE_COUNT);
	defaultDelayColor_ = D3DCOLOR_ARGB(255, 128, 128, 128);
}
StgShotDataList::~StgShotDataList()
{
}
bool StgShotDataList::AddShotDataList(std::wstring path, bool bReload)
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
		std::vector<ref_count_ptr<StgShotData>::unsync > listData;
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
				if(element == L"ShotData")
				{
					_ScanShot(listData, scanner);
				}
				else if(element == L"shot_image")
				{
					scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
					pathImage = scanner.Next().GetString();
				}
				else if(element == L"delay_color")
				{
					std::vector<std::wstring> list = _GetArgumentList(scanner);
					defaultDelayColor_ = D3DCOLOR_ARGB(255, 
						StringUtility::ToInteger(list[0]),
						StringUtility::ToInteger(list[1]),
						StringUtility::ToInteger(list[2]));
				}
				else if(element == L"delay_rect")
				{
					std::vector<std::wstring> list = _GetArgumentList(scanner);
					RECT rect;
					rect.left = StringUtility::ToInteger(list[0]);
					rect.top = StringUtility::ToInteger(list[1]);
					rect.right = StringUtility::ToInteger(list[2]);
					rect.bottom = StringUtility::ToInteger(list[3]);
					rcDelay = rect;
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
				ref_count_ptr<StgShotRenderer>::unsync render = new StgShotRenderer();
				render->SetTexture(texture);
				listRenderer_[iRender].push_back(render);
			}
		}

		if(listData_.size() < listData.size())
			listData_.resize(listData.size());
		for(int iData = 0 ; iData < listData.size() ; iData++)
		{
			ref_count_ptr<StgShotData>::unsync data = listData[iData];
			if(data == NULL)continue;
			data->indexTexture_ = textureIndex;
			if(data->rcDelay_.left < 0)
				data->rcDelay_ = rcDelay;
			listData_[iData] = data;
		}

		listReadPath_.insert(path);
		Logger::WriteTop(StringUtility::Format(L"弾データを読み込みました:%s", path.c_str()));
		res = true;
	}
	catch(gstd::wexception& e)
	{
		std::wstring log = StringUtility::Format(L"弾データ読み込み失敗:%d行目(%s)", scanner.GetCurrentLine(), e.what());
		Logger::WriteTop(log);
		res = NULL;
	}
	catch(...)
	{
		std::wstring log = StringUtility::Format(L"弾データ読み込み失敗:%d行目", scanner.GetCurrentLine());
		Logger::WriteTop(log);
		res = NULL;
	}

	return res;
}
void StgShotDataList::_ScanShot(std::vector<ref_count_ptr<StgShotData>::unsync >& listData, Scanner& scanner)
{
	Token& tok = scanner.Next();
	if(tok.GetType() == Token::TK_NEWLINE)tok = scanner.Next();
	scanner.CheckType(tok, Token::TK_OPENC);

	ref_count_ptr<StgShotData>::unsync data = new StgShotData(this);
	data->colorDelay_ = defaultDelayColor_;
	int id = -1;

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
			else if(element == L"delay_color")
			{
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				data->colorDelay_ = D3DCOLOR_ARGB(255, 
					StringUtility::ToInteger(list[0]),
					StringUtility::ToInteger(list[1]),
					StringUtility::ToInteger(list[2]));
			}
			else if(element == L"delay_rect")
			{
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				RECT rect;
				rect.left = StringUtility::ToInteger(list[0]);
				rect.top = StringUtility::ToInteger(list[1]);
				rect.right = StringUtility::ToInteger(list[2]);
				rect.bottom = StringUtility::ToInteger(list[3]);
				data->rcDelay_ = rect;
			}
			else if(element == L"collision")
			{
				DxCircle circle;
				std::vector<std::wstring> list = _GetArgumentList(scanner);
				if(list.size() == 1)
				{
					circle.SetR(StringUtility::ToInteger(list[0]));
				}
				else if(list.size() == 3)
				{
					circle.SetR(StringUtility::ToInteger(list[0]));
					circle.SetX(StringUtility::ToInteger(list[1]));
					circle.SetY(StringUtility::ToInteger(list[2]));
				}

				data->listCol_.push_back(circle);
			}
			else if(element == L"render" || element == L"delay_render")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				std::wstring strRender = scanner.Next().GetElement();
				int typeRender = DirectGraphics::MODE_BLEND_ALPHA;

				if(strRender == L"ADD" || strRender == L"ADD_RGB")
					typeRender = DirectGraphics::MODE_BLEND_ADD_RGB;
				else if(strRender == L"ADD_ARGB")
					typeRender = DirectGraphics::MODE_BLEND_ADD_ARGB;
				else if(strRender == L"MULTIPLY")
					typeRender = DirectGraphics::MODE_BLEND_MULTIPLY;
				else if(strRender == L"SUBTRACT")
					typeRender = DirectGraphics::MODE_BLEND_SUBTRACT;
				else if(strRender == L"INV_DESTRGB")
					typeRender = DirectGraphics::MODE_BLEND_INV_DESTRGB;

				if(element == L"render")data->typeRender_ = typeRender;
				else if(element == L"delay_render")data->typeDelayRender_ = typeRender;
			}
			else if(element == L"alpha")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				data->alpha_ = scanner.Next().GetInteger();
			}
			else if(element == L"angular_velocity")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				tok = scanner.Next();
				if(tok.GetElement() == L"rand")
				{
					scanner.CheckType(scanner.Next(), Token::TK_OPENP);
					data->angularVelocityMin_ = scanner.Next().GetReal();
					scanner.CheckType(scanner.Next(), Token::TK_COMMA);
					data->angularVelocityMax_ = scanner.Next().GetReal();
					scanner.CheckType(scanner.Next(), Token::TK_CLOSEP);
				}
				else
				{
					data->angularVelocityMin_ = tok.GetReal();
					data->angularVelocityMax_ = data->angularVelocityMin_;
				}
			}
			else if(element == L"fixed_angle")
			{
				scanner.CheckType(scanner.Next(), Token::TK_EQUAL);
				tok = scanner.Next();
				data->bFixedAngle_ = tok.GetElement() == L"true";
			}
			else if(element == L"AnimationData")
			{
				_ScanAnimation(data, scanner);
			}
		}
	}

	if(id >= 0) 
	{
		if(data->listCol_.size() == 0)
		{
			RECT rect = data->rcSrc_;
			int rx = abs(rect.right-rect.left);
			int ry = abs(rect.bottom-rect.top);
			int r = min(rx, ry);
			r = r / 3 - 3;
			if(r <= 3)r = 3;
			DxCircle circle(0, 0, r);
			data->listCol_.push_back(circle);
		}
		if(listData.size() <= id) 
			listData.resize(id + 1);

		listData[id] = data;
	}
}
void StgShotDataList::_ScanAnimation(ref_count_ptr<StgShotData>::unsync shotData, Scanner& scanner)
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
					StgShotData::AnimationData anime;
					int frame = StringUtility::ToInteger(list[0]);
					RECT rcSrc = {
						StringUtility::ToInteger(list[1]),
						StringUtility::ToInteger(list[2]),
						StringUtility::ToInteger(list[3]),
						StringUtility::ToInteger(list[4]),
					};

					anime.frame_ = frame;
					anime.rcSrc_ = rcSrc;

					shotData->listAnime_.push_back(anime);
					shotData->totalAnimeFrame_ += frame;
				}
			}
		}
	}
}
std::vector<std::wstring> StgShotDataList::_GetArgumentList(Scanner& scanner)
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

//StgShotData
StgShotData::StgShotData(StgShotDataList* listShotData)
{
	listShotData_ = listShotData;
	typeRender_ = DirectGraphics::MODE_BLEND_ALPHA;
	typeDelayRender_ = DirectGraphics::MODE_BLEND_ADD_ARGB;
	colorDelay_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	SetRect(&rcSrc_ , 0, 0, 0, 0);
	SetRect(&rcDelay_, -1, -1, -1, -1);
	alpha_ = 255;
	totalAnimeFrame_ = 0;
	angularVelocityMin_ = 0;
	angularVelocityMax_ = 0;
	bFixedAngle_ = false;
}
StgShotData::~StgShotData()
{
}
RECT StgShotData::GetRect(int frame)
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
ref_count_ptr<Texture> StgShotData::GetTexture()
{
	ref_count_ptr<Texture> res = listShotData_->GetTexture(indexTexture_);
	return res;
}
StgShotRenderer* StgShotData::GetRenderer()
{
	StgShotRenderer* res = GetRendererFromGraphicsBlendType(typeRender_);
	return res;
}
StgShotRenderer* StgShotData::GetRenderer(int type)
{
	return listShotData_->GetRenderer(indexTexture_, type).GetPointer();
}
StgShotRenderer* StgShotData::GetRendererFromGraphicsBlendType(int blendType)
{
	StgShotRenderer* res = NULL;
	if(blendType == DirectGraphics::MODE_BLEND_ALPHA)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_ALPHA).GetPointer();
	else if(blendType == DirectGraphics::MODE_BLEND_ADD_RGB)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_ADD_RGB).GetPointer();
	else if(blendType == DirectGraphics::MODE_BLEND_ADD_ARGB)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_ADD_ARGB).GetPointer();
	else if(blendType == DirectGraphics::MODE_BLEND_MULTIPLY)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_MULTIPLY).GetPointer();
	else if(blendType == DirectGraphics::MODE_BLEND_SUBTRACT)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_SUBTRACT).GetPointer();
	else if(blendType == DirectGraphics::MODE_BLEND_INV_DESTRGB)
		res = listShotData_->GetRenderer(indexTexture_, StgShotDataList::RENDER_INV_DESTRGB).GetPointer();
	return res;
}
bool StgShotData::IsAlphaBlendValidType(int blendType)
{
	bool bValidAlpha = false;
	if(blendType == DirectGraphics::MODE_BLEND_ALPHA ||
		blendType == DirectGraphics::MODE_BLEND_ADD_ARGB ||
		blendType == DirectGraphics::MODE_BLEND_SUBTRACT)
	{
		bValidAlpha = true;
	}
	return bValidAlpha;
}

/**********************************************************
//StgShotRenderer
**********************************************************/
StgShotRenderer::StgShotRenderer()
{
	countRenderVertex_ = 0;
	SetVertexCount(256 * 256);

}
int StgShotRenderer::GetVertexCount()
{
	int res = countRenderVertex_;
	res = min(countRenderVertex_, vertex_.GetSize() / strideVertexStreamZero_);
	return res;
}
void StgShotRenderer::Render()
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
void StgShotRenderer::AddVertex(VERTEX_TLX& vertex)
{
	SetVertex(countRenderVertex_, vertex);
	countRenderVertex_++;
}
void StgShotRenderer::AddSquareVertex(VERTEX_TLX* listVertex)
{
	AddVertex(listVertex[0]);
	AddVertex(listVertex[2]);
	AddVertex(listVertex[1]);
	AddVertex(listVertex[1]);
	AddVertex(listVertex[2]);
	AddVertex(listVertex[3]);
}

/**********************************************************
//StgShotObject
**********************************************************/
StgShotObject::StgShotObject(StgStageController* stageController) : StgMoveObject(stageController)
{
	stageController_ = stageController;

	frameWork_ = 0;
	posX_ = 0;
	posY_ = 0;
	idShotData_ = 0;
	typeBlend_ = DirectGraphics::MODE_BLEND_NONE;
	typeSourceBrend_ = DirectGraphics::MODE_BLEND_NONE;

	damage_ = 1;
	life_ = LIFE_SPELL_UNREGIST;
	bAutoDelete_ = true;
	bEraseShot_ = false;
	bSpellFactor_ = false;

	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	delay_ = 0;
	frameGrazeInvalid_ = 0;
	frameFadeDelete_ = -1;
	frameAutoDelete_ = INT_MAX;

	typeOwner_ = OWNER_ENEMY;
	bUserIntersectionMode_ = false;
	bIntersectionEnable_ = true;
	bChangeItemEnable_ = true;

	int priShotI = stageController_->GetStageInformation()->GetShotObjectPriority();
	double priShotD = (double)priShotI / (stageController_->GetMainObjectManager()->GetRenderBucketCapacity() - 1);
	SetRenderPriority(priShotD);
}
StgShotObject::~StgShotObject()
{
	if(listReserveShot_ != NULL)
	{
		listReserveShot_->Clear(stageController_);
	}
}
void StgShotObject::Work()
{
}
void StgShotObject::_Move()
{
	if(delay_ == 0)
		StgMoveObject::_Move();
	SetX(posX_);
	SetY(posY_);

	//弾画像置き換え処理
	if(pattern_ != NULL)
	{
		int idShot = pattern_->GetShotDataID();
		if(idShot != StgMovePattern::NO_CHANGE)
		{
			SetShotDataID(idShot);
		}
	}
}
void StgShotObject::_DeleteInLife()
{
	if(IsDeleted())return;
	if(life_ <= 0)
	{
		_SendDeleteEvent(StgShotManager::BIT_EV_DELETE_IMMEDIATE);
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}
void StgShotObject::_DeleteInAutoClip()
{
	if(IsDeleted())return;
	if(!IsAutoDelete())return;
	StgShotManager* shotManager = stageController_->GetShotManager();
	RECT rect = shotManager->GetShotAutoDeleteClipRect();
	if(posX_ < rect.left  || posX_ > rect.right || posY_ < rect.top || posY_ > rect.bottom)
	{
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}
void StgShotObject::_DeleteInFadeDelete()
{
	if(IsDeleted())return;
	if(frameFadeDelete_ == 0)
	{
		_SendDeleteEvent(StgShotManager::BIT_EV_DELETE_FADE);
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}
void StgShotObject::_DeleteInAutoDeleteFrame()
{
	if(IsDeleted())return;
	if(delay_ > 0)return;

	if(frameAutoDelete_ <= 0)
	{
		_SendDeleteEvent(StgShotManager::BIT_EV_DELETE_IMMEDIATE);
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
		return;
	}
	frameAutoDelete_ = max(0, frameAutoDelete_ - 1);
}
void StgShotObject::_SendDeleteEvent(int bit)
{
	if(typeOwner_ != OWNER_ENEMY)return;

	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	ref_count_ptr<ManagedScript> scriptShot = stageScriptManager->GetShotScript();
	if(scriptShot == NULL)return;

	StgShotManager* shotManager = stageController_->GetShotManager();
	bool bSendEnable = shotManager->IsDeleteEventEnable(bit);
	if(!bSendEnable)return;

	int posX = GetPositionX();
	int posY = GetPositionY();

	std::vector<long double> listPos;
	listPos.push_back(posX);
	listPos.push_back(posY);

	int typeEvent = 0;
	switch(bit)
	{
	case StgShotManager::BIT_EV_DELETE_IMMEDIATE:
		typeEvent = StgStageShotScript::EV_DELETE_SHOT_IMMEDIATE;
		break;
	case StgShotManager::BIT_EV_DELETE_TO_ITEM:
		typeEvent = StgStageShotScript::EV_DELETE_SHOT_TO_ITEM;	
		break;
	case StgShotManager::BIT_EV_DELETE_FADE:
		typeEvent = StgStageShotScript::EV_DELETE_SHOT_FADE;
		break;
	}

	std::vector<gstd::value> listScriptValue;
	listScriptValue.push_back(scriptShot->CreateRealValue(idObject_));
	listScriptValue.push_back(scriptShot->CreateRealArrayValue(listPos));
	scriptShot->RequestEvent(typeEvent, listScriptValue);
}
void StgShotObject::_AddReservedShotWork()
{
	if(IsDeleted())return;
	if(listReserveShot_ == NULL)return;
	ref_count_ptr<ReserveShotList::ListElement>::unsync listData = listReserveShot_->GetNextFrameData();
	if(listData == NULL)return;

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	std::list<ReserveShotListData>* list = listData->GetDataList();
	std::list<ReserveShotListData>::iterator itr = list->begin();
	for(; itr != list->end() ; itr++)
	{
		StgShotObject::ReserveShotListData& data = (*itr);
		int idShot = data.GetShotID();
		ref_count_ptr<StgShotObject>::unsync obj = 
			ref_count_ptr<StgShotObject>::unsync::DownCast(objectManager->GetObject(idShot));
		if(obj == NULL || obj->IsDeleted())continue;

		_AddReservedShot(obj, &data);
	}

}

void StgShotObject::_AddReservedShot(ref_count_ptr<StgShotObject>::unsync obj, StgShotObject::ReserveShotListData* data)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();

	double ownAngle = GetDirectionAngle();
	double ox = GetPositionX();
	double oy = GetPositionY();
	
	double dRadius = data->GetRadius();
	double dAngle = data->GetAngle();
	double sx = obj->GetPositionX();
	double sy = obj->GetPositionY();
	double sAngle = obj->GetDirectionAngle();
	double angle = ownAngle + dAngle;

	double tx = ox + sx + dRadius * cos(Math::DegreeToRadian(angle));
	double ty = oy + sy + dRadius * sin(Math::DegreeToRadian(angle));
	obj->SetX(tx);
	obj->SetY(ty);

	StgShotManager* shotManager = stageController_->GetShotManager();
	shotManager->AddShot(obj);
	obj->Activate();
	objectManager->ActivateObject(obj->GetObjectID(), true);
}

void StgShotObject::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{

}
StgShotData* StgShotObject::_GetShotData()
{
	StgShotData* res = NULL;
	StgShotManager* shotManager = stageController_->GetShotManager();
	StgShotDataList* dataList = (typeOwner_ == OWNER_PLAYER) ?
		shotManager->GetPlayerShotDataList() : shotManager->GetEnemyShotDataList();

	if(dataList != NULL)
	{
		res = dataList->GetData(idShotData_).GetPointer();
	}

	return res;
}
ref_count_ptr<StgShotObject>::unsync StgShotObject::GetOwnObject()
{
	return ref_count_ptr<StgShotObject>::unsync::DownCast(stageController_->GetMainRenderObject(idObject_));
}
void StgShotObject::_SetVertexPosition(VERTEX_TLX& vertex, float x, float y, float z, float w)
{
	float bias = -0.5f;
	vertex.position.x = x + bias;
	vertex.position.y = y + bias;
	vertex.position.z = z;
	vertex.position.w = w;
}
void StgShotObject::_SetVertexUV(VERTEX_TLX& vertex, float u, float v)
{
	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;
	
	ref_count_ptr<Texture> texture = shotData->GetTexture();
	int width = texture->GetWidth();
	int height = texture->GetHeight();
	vertex.texcoord.x = u / width;
	vertex.texcoord.y = v / height;
}
void StgShotObject::_SetVertexColorARGB(VERTEX_TLX& vertex, D3DCOLOR color)
{
	vertex.diffuse_color = color;
}
void StgShotObject::SetAlpha(int alpha)
{
	color_ = ColorAccess::SetColorA(color_, alpha);
}
void StgShotObject::SetColor(int r, int g, int b)
{
	color_ = ColorAccess::SetColorR(color_, r);
	color_ = ColorAccess::SetColorG(color_, g);
	color_ = ColorAccess::SetColorB(color_, b);
}

void StgShotObject::AddShot(int frame, int idShot, int radius, int angle)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->ActivateObject(idShot, false);

	if(listReserveShot_ == NULL)
		listReserveShot_ = new ReserveShotList();
	listReserveShot_->AddData(frame, idShot, radius, angle);
}
void StgShotObject::ConvertToItem()
{
	if(IsDeleted())return;

	if(bChangeItemEnable_)
	{
		_ConvertToItemAndSendEvent();
		_SendDeleteEvent(StgShotManager::BIT_EV_DELETE_TO_ITEM);
	}

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(idObject_);
}
void StgShotObject::DeleteImmediate()
{
	if(IsDeleted())return;

	_SendDeleteEvent(StgShotManager::BIT_EV_DELETE_IMMEDIATE);

	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
	objectManager->DeleteObject(idObject_);
}

//StgShotObject::ReserveShotList
ref_count_ptr<StgShotObject::ReserveShotList::ListElement>::unsync StgShotObject::ReserveShotList::GetNextFrameData()
{
	ref_count_ptr<ListElement>::unsync res = NULL;
	if(mapData_.find(frame_) != mapData_.end())
	{
		res = mapData_[frame_];
		mapData_.erase(frame_);
	}

	frame_++;
	return res;
}
void StgShotObject::ReserveShotList::AddData(int frame, int idShot, int radius, int angle)
{
	ref_count_ptr<ListElement>::unsync list;
	if(mapData_.find(frame) == mapData_.end())
	{
		list = new ListElement();
		mapData_[frame] = list;
	}
	else
	{
		list = mapData_[frame];
	}

	ReserveShotListData data;
	data.idShot_ = idShot;
	data.radius_ = radius;
	data.angle_ = angle;
	list->Add(data);
}
void StgShotObject::ReserveShotList::Clear(StgStageController* stageController)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController->GetMainObjectManager();
	if(objectManager == NULL)return;

	std::map<int, ref_count_ptr<ListElement>::unsync >::iterator itrMap = mapData_.begin();
	for(;itrMap != mapData_.end() ; itrMap++)
	{
		ref_count_ptr<ListElement>::unsync listElement = itrMap->second;
		std::list<ReserveShotListData>* list = listElement->GetDataList();
		std::list<ReserveShotListData>::iterator itr = list->begin();
		for(; itr != list->end() ; itr++)
		{
			StgShotObject::ReserveShotListData& data = (*itr);
			int idShot = data.GetShotID();
			ref_count_ptr<StgShotObject>::unsync objShot = 
				ref_count_ptr<StgShotObject>::unsync::DownCast(objectManager->GetObject(idShot));
			if(objShot != NULL)objShot->ClearShotObject();
			objectManager->DeleteObject(idShot);
		}
	}
}

/**********************************************************
//StgNormalShotObject
**********************************************************/
StgNormalShotObject::StgNormalShotObject(StgStageController* stageController) : StgShotObject(stageController)
{
	typeObject_ = StgStageScript::OBJ_SHOT;
	angularVelocity_ = 0;
}
StgNormalShotObject::~StgNormalShotObject()
{

}
void StgNormalShotObject::Work()
{
	_Move();
	if(delay_ == 0)
	{
		_AddReservedShotWork();
	}
	
	delay_ = max(delay_ - 1, 0);
	frameWork_++;

	angle_.z += angularVelocity_;

	if(frameFadeDelete_ >= 0)
	{
		frameFadeDelete_--;
	}

	_DeleteInAutoClip();
	_DeleteInLife();
	_DeleteInFadeDelete();
	_DeleteInAutoDeleteFrame();
}

void StgNormalShotObject::_AddIntersectionRelativeTarget()
{
	if(delay_ > 0)return;
	if(frameFadeDelete_ >= 0)return;
	ClearIntersected();
	if(IsDeleted())return;
	if(bUserIntersectionMode_)return;//ユーザ定義あたり判定モード
	if(!bIntersectionEnable_)return;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;

	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();
	std::vector<DxCircle>* listCircle = shotData->GetIntersectionCircleList();
	if(GetIntersectionRelativeTargetCount() != listCircle->size())
	{
		ClearIntersectionRelativeTarget();

		ref_count_ptr<StgShotObject>::unsync obj = GetOwnObject();
		if(obj == NULL)return;
		ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;

		for(int iTarget = 0 ; iTarget < listCircle->size() ; iTarget++)
		{
			ref_count_ptr<StgIntersectionTarget_Circle>::unsync target =
				ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
			if(typeOwner_ == OWNER_PLAYER)
				target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SHOT);
			else 
				target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY_SHOT);
			target->SetObject(wObj);
			AddIntersectionRelativeTarget(target);
		}
	}

	bool bInvalid = true;
	for(int iCircle = 0 ; iCircle < listCircle->size() ; iCircle++)
	{
		ref_count_ptr<StgIntersectionTarget>::unsync target = GetIntersectionRelativeTarget(iCircle);
		StgIntersectionTarget_Circle* cTarget = (StgIntersectionTarget_Circle*)target.GetPointer();

		DxCircle circle = listCircle->at(iCircle);
		if(circle.GetX() != 0 || circle.GetY() != 0) 
		{
			double angleZ = 0;
			if(!shotData->IsFixedAngle())angleZ = GetDirectionAngle() + 90 + angle_.z;
			else angleZ = angle_.z;

			double px = circle.GetX() * cos(D3DXToRadian(angleZ)) - (-circle.GetY()) * sin(D3DXToRadian(angleZ));
			double py = circle.GetX() * sin(D3DXToRadian(angleZ)) + (-circle.GetY()) * cos(D3DXToRadian(angleZ));
			circle.SetX(px + posX_);
			circle.SetY(py + posY_);
		}
		else
		{
			circle.SetX(circle.GetX() + posX_);
			circle.SetY(circle.GetY() + posY_);
		}
		cTarget->SetCircle(circle);

		RECT rect = cTarget->GetIntersectionSapceRect();
		if(rect.left != rect.right && rect.top != rect.bottom)
			bInvalid = false;
	}

	if(typeOwner_ == OWNER_PLAYER)
	{
		//自弾の場合は登録
		bInvalid = false;
	}
	else
	{
		//自機の移動範囲が負の値が可能であれば敵弾でも登録
		ref_count_ptr<StgPlayerObject>::unsync player = stageController_->GetPlayerObject();
		if(player != NULL)
		{
			RECT rcClip = player->GetClip();
			if(rcClip.left < 0 || rcClip.top < 0)
				bInvalid = false;
		}
	}

	if(!bInvalid)
		RegistIntersectionRelativeTarget(intersectionManager);
}
std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > StgNormalShotObject::GetIntersectionTargetList()
{
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > res;

	if(delay_ > 0)return res;
	if(frameFadeDelete_ >= 0)return res;
	if(IsDeleted())return res;
	if(bUserIntersectionMode_)return res;//ユーザ定義あたり判定モード
	if(!bIntersectionEnable_)return res;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return res;

	std::vector<DxCircle>* listCircle = shotData->GetIntersectionCircleList();
	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();


	for(int iCircle = 0 ; iCircle < listCircle->size() ; iCircle++)
	{
		ref_count_ptr<StgIntersectionTarget_Circle>::unsync target = 
			ref_count_ptr<StgIntersectionTarget_Circle>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_CIRCLE));
		StgIntersectionTarget_Circle* cTarget = (StgIntersectionTarget_Circle*)target.GetPointer();

		DxCircle circle = listCircle->at(iCircle);
		if(circle.GetX() != 0 || circle.GetY() != 0) 
		{
			double angleZ = 0;
			if(!shotData->IsFixedAngle())angleZ = GetDirectionAngle() + 90 + angle_.z;
			else angleZ = angle_.z;

			double px = circle.GetX() * cos(D3DXToRadian(angleZ)) - (-circle.GetY()) * sin(D3DXToRadian(angleZ));
			double py = circle.GetX() * sin(D3DXToRadian(angleZ)) + (-circle.GetY()) * cos(D3DXToRadian(angleZ));
			circle.SetX(px + posX_);
			circle.SetY(py + posY_);
		}
		else
		{
			circle.SetX(circle.GetX() + posX_);
			circle.SetY(circle.GetY() + posY_);
		}
		cTarget->SetCircle(circle);

		res.push_back(target);
	}

	return res;
}

void StgNormalShotObject::RenderOnShotManager(D3DXMATRIX mat)
{
	if(!IsVisible())return;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;

	StgShotRenderer* renderer = NULL;

	int shotBlendType = DirectGraphics::MODE_BLEND_ALPHA;
	if(delay_ > 0)
	{
		//遅延時間
		int objDelayBlendType = GetSourceBlendType();
		if(objDelayBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			shotBlendType = shotData->GetDelayRenderType();
			renderer = shotData->GetRendererFromGraphicsBlendType(shotBlendType);
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objDelayBlendType);
		}
	}
	else
	{
		int objBlendType = GetBlendType();
		if(objBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer();
			shotBlendType = shotData->GetRenderType();
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objBlendType);
		}
	}

	if(renderer == NULL)return;

	D3DXMATRIX matScale;
	D3DXMATRIX matRot;
	D3DXMATRIX matTrans;
	RECT rcSrc;
	D3DCOLOR color;
	if(delay_ > 0)
	{
		//遅延時間
		double expa = 0.5f + (double)delay_ / 30.0f * 2;
		if(expa > 2)expa = 2;

		double angleZ = 0;
		if(!shotData->IsFixedAngle())angleZ = GetDirectionAngle() + 90 + angle_.z;
		else angleZ = angle_.z;

		D3DXMatrixScaling(&matScale, expa, expa, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angleZ));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		mat = matScale * matRot * matTrans * mat;

		rcSrc = shotData->GetDelayRect();
		color = shotData->GetDelayColor();
	}
	else
	{
		double angleZ = 0;
		if(!shotData->IsFixedAngle())angleZ = GetDirectionAngle() + 90 + angle_.z;
		else angleZ = angle_.z;

		D3DXMatrixScaling(&matScale, scale_.x, scale_.y, scale_.z);
		D3DXMatrixRotationYawPitchRoll(&matRot, D3DXToRadian(angle_.y), D3DXToRadian(angle_.x), D3DXToRadian(angleZ));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		mat = matScale * matRot * matTrans * mat;

		rcSrc = shotData->GetRect(frameWork_);
		color = color_;
		double alpha = shotData->GetAlpha() / 255.0;
		if(frameFadeDelete_ >= 0) 
			alpha = (double)frameFadeDelete_ / (double)FRAME_FADEDELETE;

		bool bValidAlpha = StgShotData::IsAlphaBlendValidType(shotBlendType);
		if(bValidAlpha)
		{
			//α有効
			int colorA = ColorAccess::GetColorA(color);
			color = ColorAccess::SetColorA(color, alpha * colorA);
		}
		else
		{
			//α無効
			color = ColorAccess::ApplyAlpha(color, alpha);
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

void StgNormalShotObject::ClearShotObject()
{
	ClearIntersectionRelativeTarget();
}
void StgNormalShotObject::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	double damage = 0;
	int otherType = otherTarget->GetTargetType();
	switch(otherType)
	{
		case StgIntersectionTarget::TYPE_PLAYER:
		{
			//自機
			frameGrazeInvalid_ = INT_MAX;
			break;
		}
		case StgIntersectionTarget::TYPE_PLAYER_SHOT:
		{
			//自機弾
			StgShotObject* shot = (StgShotObject*)otherTarget->GetObject().GetPointer();
			if(shot != NULL)
			{
				bool bEraseShot = shot->IsEraseShot();
				if(bEraseShot && life_ != LIFE_SPELL_REGIST)
					ConvertToItem();
			}
			break;
		}
		case StgIntersectionTarget::TYPE_PLAYER_SPELL:
		{
			//自機スペル
			StgPlayerSpellObject* spell = (StgPlayerSpellObject*)otherTarget->GetObject().GetPointer();
			if(spell != NULL)
			{
				bool bEraseShot = spell->IsEraseShot();
				if(bEraseShot && life_ != LIFE_SPELL_REGIST)
					ConvertToItem();
			}
			break;
		}
		case StgIntersectionTarget::TYPE_ENEMY:
		case StgIntersectionTarget::TYPE_ENEMY_SHOT:
		{
			damage = 1;
			break;
		}
	}

	if(life_ != LIFE_SPELL_REGIST)
		life_ = max(life_ - damage, 0);
}
void StgNormalShotObject::_ConvertToItemAndSendEvent()
{
	StgItemManager* itemManager = stageController_->GetItemManager();
	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	ref_count_ptr<ManagedScript> scriptItem = stageScriptManager->GetItemScript();

	int posX = GetPositionX();
	int posY = GetPositionY();
	if(scriptItem != NULL)
	{
		std::vector<long double> listPos;
		listPos.push_back(posX);
		listPos.push_back(posY);

		std::vector<gstd::value> listScriptValue;
		listScriptValue.push_back(scriptItem->CreateRealValue(idObject_));
		listScriptValue.push_back(scriptItem->CreateRealArrayValue(listPos));
		scriptItem->RequestEvent(StgStageScript::EV_DELETE_SHOT_TO_ITEM, listScriptValue);
	}

	if(itemManager->IsDefaultBonusItemEnable())
	{
		ref_count_ptr<StgItemObject>::unsync obj = new StgItemObject_Bonus(stageController_);
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		int id = objectManager->AddObject(obj);
		if(id != DxScript::ID_INVALID)
		{
			//弾の座標にアイテムを作成する
			StgItemManager* itemManager = stageController_->GetItemManager();
			itemManager->AddItem(obj);
			obj->SetPositionX(posX);
			obj->SetPositionY(posY);
		}
	}
}
void StgNormalShotObject::RegistIntersectionTarget()
{
	if(!bUserIntersectionMode_)
	{
		_AddIntersectionRelativeTarget();
	}

}
void StgNormalShotObject::SetShotDataID(int id)
{
	StgShotData* oldData = _GetShotData();
	StgShotObject::SetShotDataID(id);

	//角速度更新
	StgShotData* shotData = _GetShotData();
	if(shotData != NULL && oldData != shotData)
	{
		if(angularVelocity_ != 0)
		{
			angularVelocity_ = 0;
			angle_.z = 0;
		}

		double avMin = shotData->GetAngularVelocityMin();
		double avMax = shotData->GetAngularVelocityMax();
		if(avMin != 0 || avMax != 0)
		{
			ref_count_ptr<StgStageInformation> stageInfo = stageController_->GetStageInformation();
			ref_count_ptr<MersenneTwister> rand = stageInfo->GetMersenneTwister();
			angularVelocity_ = rand->GetReal(avMin, avMax);
		}

	}

}

/**********************************************************
//StgLaserObject(レーザー基本部)
**********************************************************/
StgLaserObject::StgLaserObject(StgStageController* stageController) : StgShotObject(stageController)
{
	life_ = LIFE_SPELL_REGIST;
	length_ = 0;
	widthRender_ = 0;
	widthIntersection_ = -1;
	invalidLengthStart_ = -1;
	invalidLengthEnd_ = -1;
	frameGrazeInvalidStart_ = 20;
	itemDistance_ = 24;
}
void StgLaserObject::SetLength(int length)
{
	length_ = length;
	if(invalidLengthStart_<0)invalidLengthStart_=length_*0.1;
	if(invalidLengthEnd_<0)invalidLengthEnd_=length_*0.1;
}
void StgLaserObject::SetRenderWidth(int width)
{
	widthRender_ = width;
	if(widthIntersection_ < 0)widthIntersection_=width/4;
}
void StgLaserObject::ClearShotObject()
{
	ClearIntersectionRelativeTarget();
}
void StgLaserObject::_AddIntersectionRelativeTarget()
{
	if(delay_ > 0)return;
	if(frameFadeDelete_ >= 0)return;
	ClearIntersected();

	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > listTarget = GetIntersectionTargetList();
	for(int iTarget = 0 ; iTarget < listTarget.size() ; iTarget++)
		intersectionManager->AddTarget(listTarget[iTarget]);
}
void StgLaserObject::Intersect(ref_count_ptr<StgIntersectionTarget>::unsync ownTarget, ref_count_ptr<StgIntersectionTarget>::unsync otherTarget)
{
	double damage = 0;
	int otherType = otherTarget->GetTargetType();
	switch(otherType)
	{
		case StgIntersectionTarget::TYPE_PLAYER:
		{
			//自機
			if(frameGrazeInvalid_ <= 0)
			{
				frameGrazeInvalid_ = frameGrazeInvalidStart_ > 0 ? frameGrazeInvalidStart_ : INT_MAX;
			}
			break;
		}
		case StgIntersectionTarget::TYPE_PLAYER_SHOT:
		{
			//自機弾弾
			StgShotObject* shot = (StgShotObject*)otherTarget->GetObject().GetPointer();
			if(shot != NULL)
			{
				bool bEraseShot = shot->IsEraseShot();
				if(bEraseShot && life_ != LIFE_SPELL_REGIST)
				{
					damage = shot->GetDamage();
					ConvertToItem();
				}
			}
			break;
		}
		case StgIntersectionTarget::TYPE_PLAYER_SPELL:
		{
			//自機スペル
			StgPlayerSpellObject* spell = (StgPlayerSpellObject*)otherTarget->GetObject().GetPointer();
			if(spell != NULL)
			{
				bool bEraseShot = spell->IsEraseShot();
				if(bEraseShot && life_ != LIFE_SPELL_REGIST)
				{
					damage = spell->GetDamage();
					ConvertToItem();
				}
			}
			break;
		}
	}
	if(life_ != LIFE_SPELL_REGIST)
		life_ = max(life_ - damage, 0);
}


/**********************************************************
//StgLooseLaserObject(射出型レーザー)
**********************************************************/
StgLooseLaserObject::StgLooseLaserObject(StgStageController* stageController) : StgLaserObject(stageController)
{
	typeObject_ = StgStageScript::OBJ_LOOSE_LASER;
}
void StgLooseLaserObject::Work()
{
	//1フレーム目は移動しない
	if(frameWork_ == 0)
	{
		posXE_ = posX_;
		posYE_ = posY_;
	}

	_Move();
	if(delay_ == 0)
	{
		_AddReservedShotWork();
	}
	
	delay_ = max(delay_ - 1, 0);
	frameWork_++;

	if(frameFadeDelete_ >= 0)
	{
		frameFadeDelete_--;
	}

	_DeleteInAutoClip();
	_DeleteInLife();
	_DeleteInFadeDelete();
	_DeleteInAutoDeleteFrame();
//	_AddIntersectionRelativeTarget();
	frameGrazeInvalid_--;
}
void StgLooseLaserObject::_Move()
{
	if(delay_ == 0)
		StgMoveObject::_Move();
	DxScriptRenderObject::SetX(posX_);
	DxScriptRenderObject::SetY(posY_);

	if(delay_ > 0)return;
	double radius = pow(pow(posXE_ - posX_,2)+pow(posYE_ - posY_,2), 0.5);
	if(radius > length_)
	{
		double speed = GetSpeed();
		double ang = GetDirectionAngle();
		posXE_ += speed * cos(Math::DegreeToRadian(ang));
		posYE_ += speed * sin(Math::DegreeToRadian(ang));
	}
}
void StgLooseLaserObject::_DeleteInAutoClip()
{
	if(IsDeleted())return;
	if(!IsAutoDelete())return;
	StgShotManager* shotManager = stageController_->GetShotManager();
	RECT rect = shotManager->GetShotAutoDeleteClipRect();
	if((posX_ < rect.left && posXE_ < rect.left) || (posX_ > rect.right && posXE_ > rect.right) || 
		(posY_ < rect.top && posYE_ < rect.top) || (posY_ > rect.bottom && posYE_ > rect.bottom))
	{
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}

std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > StgLooseLaserObject::GetIntersectionTargetList()
{
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > res;

	if(delay_ > 0)return res;
	if(frameFadeDelete_ >= 0)return res;
	if(IsDeleted())return res;
	if(bUserIntersectionMode_)return res;//ユーザ定義あたり判定モード
	if(!bIntersectionEnable_)return res;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return res;

	ref_count_ptr<StgShotObject>::unsync obj = GetOwnObject();
	if(obj == NULL)return res;

	//当たり判定
	double ang = GetDirectionAngle();
	int length = pow(pow(posXE_ - posX_, 2) + pow(posYE_ - posY_, 2) , 0.5);
	int invalidLengthS = min(length , invalidLengthStart_);
	int posXS = posX_ - invalidLengthS * cos(Math::DegreeToRadian(ang));
	int posYS = posY_ - invalidLengthS * sin(Math::DegreeToRadian(ang));
	int invalidLengthE = min(length , invalidLengthEnd_);
	int posXE = posXE_ + invalidLengthE * cos(Math::DegreeToRadian(ang));
	int posYE = posYE_ + invalidLengthE * sin(Math::DegreeToRadian(ang));

	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();
	DxWidthLine line(posXS, posYS, posXE, posYE, widthIntersection_);

	ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
	if(typeOwner_ == OWNER_PLAYER)
		target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SHOT);
	else 
		target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY_SHOT);
	target->SetObject(wObj);
	target->SetLine(line);

	res.push_back(target);
	return res;
}

void StgLooseLaserObject::RenderOnShotManager(D3DXMATRIX mat)
{
	if(!IsVisible())return;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;

	int shotBlendType = StgShotDataList::RENDER_ADD_ARGB;
	StgShotRenderer* renderer = NULL;
	if(delay_ > 0)
	{
		//遅延時間
		int objDelayBlendType = GetSourceBlendType();
		if(objDelayBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer(StgShotDataList::RENDER_ADD_ARGB);
			shotBlendType = DirectGraphics::MODE_BLEND_ADD_ARGB;
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objDelayBlendType);
		}
	}
	else
	{
		int objBlendType = GetBlendType();
		int shotBlendType = objBlendType;
		if(objBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer(StgShotDataList::RENDER_ADD_ARGB);
			shotBlendType = DirectGraphics::MODE_BLEND_ADD_ARGB;
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objBlendType);
		}
	}

	if(renderer == NULL)return;

	D3DXMATRIX matScale;
	D3DXMATRIX matRot;
	D3DXMATRIX matTrans;
	RECT rcSrc;
	RECT rcDest;
	D3DCOLOR color;
	if(delay_ > 0)
	{
		//遅延時間
		double expa = 0.5f + (double)delay_ / 30.0f * 2;
		if(expa > 3.5)expa = 3.5;

		D3DXMatrixScaling(&matScale, expa, expa, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(GetDirectionAngle() + 90));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		mat = matScale * matRot * matTrans * mat;

		rcSrc = shotData->GetDelayRect();
		color = shotData->GetDelayColor();

		int width = rcSrc.right - rcSrc.left;
		int height = rcSrc.bottom - rcSrc.top;
		SetRect(&rcDest, -width / 2, -height / 2, width / 2, height / 2);
		if(width % 2 == 1)rcDest.right += 1;
		if(height % 2 == 1)rcDest.bottom += 1;
	}
	else
	{
		D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(GetDirectionAngle() + 90));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		mat = matScale * matRot * matTrans * mat;

		double radius = pow(pow(posXE_ - posX_,2)+pow(posYE_-posY_,2), 0.5);
		rcSrc = shotData->GetRect(frameWork_);
		color = color_;
		double alpha = shotData->GetAlpha() / 255.0;
		if(frameFadeDelete_ >= 0) 
			alpha = (double)frameFadeDelete_ / (double)FRAME_FADEDELETE;

		bool bValidAlpha = StgShotData::IsAlphaBlendValidType(shotBlendType);
		if(bValidAlpha)
		{
			//α有効
			int colorA = ColorAccess::GetColorA(color);
			color = ColorAccess::SetColorA(color, alpha * colorA);
		}
		else
		{
			//α無効
			color = ColorAccess::ApplyAlpha(color, alpha);
		}

		color = ColorAccess::ApplyAlpha(color, alpha);
		SetRect(&rcDest, -widthRender_/2, 0, widthRender_/2, radius);
	}


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
void StgLooseLaserObject::_ConvertToItemAndSendEvent()
{
	StgItemManager* itemManager = stageController_->GetItemManager();
	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	ref_count_ptr<ManagedScript> scriptItem = stageScriptManager->GetItemScript();

	int ex = GetPositionX();
	int ey = GetPositionY();
	double angle = GetDirectionAngle();

	int length = pow(pow(posXE_ - posX_, 2) + pow(posYE_ - posY_, 2) , 0.5);
	for(double itemPos = 0 ; itemPos < length ; itemPos += itemDistance_)
	{
		double posX = ex - itemPos * cos(Math::DegreeToRadian(angle));
		double posY = ey - itemPos * sin(Math::DegreeToRadian(angle));
		if(scriptItem != NULL)
		{
			std::vector<long double> listPos;
			listPos.push_back(posX);
			listPos.push_back(posY);

			std::vector<gstd::value> listScriptValue;
			listScriptValue.push_back(scriptItem->CreateRealValue(idObject_));
			listScriptValue.push_back(scriptItem->CreateRealArrayValue(listPos));
			scriptItem->RequestEvent(StgStageScript::EV_DELETE_SHOT_TO_ITEM, listScriptValue);
		}

		if(itemManager->IsDefaultBonusItemEnable() && delay_ == 0)
		{
			ref_count_ptr<StgItemObject>::unsync obj = new StgItemObject_Bonus(stageController_);
			int id = stageController_->GetMainObjectManager()->AddObject(obj);
			if(id != DxScript::ID_INVALID)
			{
				//弾の座標にアイテムを作成する
				itemManager->AddItem(obj);
				obj->SetPositionX(posX);
				obj->SetPositionY(posY);
			}
		
		}
	}
}
void StgLooseLaserObject::RegistIntersectionTarget()
{
	if(!bUserIntersectionMode_)
	{
		_AddIntersectionRelativeTarget();
	}
}

/**********************************************************
//StgStraightLaserObject(設置型レーザー)
**********************************************************/
StgStraightLaserObject::StgStraightLaserObject(StgStageController* stageController) : StgLaserObject(stageController)
{
	typeObject_ = StgStageScript::OBJ_STRAIGHT_LASER;
	angLaser_ = 270;
	frameFadeDelete_ = -1;
	bUseSouce_ = true;
	scaleX_ = 0.05;
}
void StgStraightLaserObject::Work()
{
	_Move();
	if(delay_ == 0)
	{
		_AddReservedShotWork();
	}
	
	delay_ = max(delay_ - 1, 0);
	if(delay_ <= 0)scaleX_ = min(1.0, scaleX_ + 0.1);

	frameWork_++;

	if(frameFadeDelete_ >= 0)
	{
		frameFadeDelete_--;
	}

	_DeleteInAutoClip();
	_DeleteInLife();
	_DeleteInFadeDelete();
	_DeleteInAutoDeleteFrame();
//	_AddIntersectionRelativeTarget();
	frameGrazeInvalid_--;
}

void StgStraightLaserObject::_DeleteInAutoClip()
{
	if(IsDeleted())return;
	if(!IsAutoDelete())return;
	StgShotManager* shotManager = stageController_->GetShotManager();
	RECT rect = shotManager->GetShotAutoDeleteClipRect();
	int posXE = posX_ + (int)(length_ * cos(Math::DegreeToRadian(angLaser_)));
	int posYE = posY_ + (int)(length_ * sin(Math::DegreeToRadian(angLaser_)));

	if((posX_ < rect.left && posXE < rect.left) || (posX_ > rect.right && posXE > rect.right) || 
		(posY_ < rect.top && posYE < rect.top) || (posY_ > rect.bottom && posYE > rect.bottom))
	{
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}
}
void StgStraightLaserObject::_DeleteInAutoDeleteFrame()
{
	if(IsDeleted())return;
	if(delay_ > 0)return;

	if(frameAutoDelete_ <= 0)
	{
		SetFadeDelete();
	}
	frameAutoDelete_ = max(0, frameAutoDelete_ - 1);
}
std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > StgStraightLaserObject::GetIntersectionTargetList()
{
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > res;
	if(delay_ > 0)return res;
	if(frameFadeDelete_ >= 0)return res;
	if(IsDeleted())return res;
	if(bUserIntersectionMode_)return res;//ユーザ定義あたり判定モード
	if(!bIntersectionEnable_)return res;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return res;
	if(scaleX_ < 1.0 && typeOwner_ != OWNER_PLAYER)return res;

	//当たり判定
	int posXS = posX_ + invalidLengthStart_ * cos(Math::DegreeToRadian(angLaser_));
	int posYS = posY_ + invalidLengthStart_ * sin(Math::DegreeToRadian(angLaser_));
	int length = max((length_ - invalidLengthEnd_), 0);
	int posXE = posX_ + (int)(length * cos(Math::DegreeToRadian(angLaser_)));
	int posYE = posY_ + (int)(length * sin(Math::DegreeToRadian(angLaser_)));

	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();
	DxWidthLine line(posXS, posYS, posXE, posYE, widthIntersection_);
	ref_count_ptr<StgShotObject>::unsync obj = GetOwnObject();
	if(obj == NULL)return res;

	ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;
	ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
		ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
	if(typeOwner_ == OWNER_PLAYER)
		target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SHOT);
	else 
		target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY_SHOT);
	target->SetObject(wObj);
	target->SetLine(line);

	res.push_back(target);
	return res;
}
void StgStraightLaserObject::_AddReservedShot(ref_count_ptr<StgShotObject>::unsync obj, StgShotObject::ReserveShotListData* data)
{
	ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();

	double ownAngle = GetDirectionAngle();
	double ox = GetPositionX();
	double oy = GetPositionY();
	
	double dRadius = data->GetRadius();
	double dAngle = data->GetAngle();
	double sx = obj->GetPositionX();
	double sy = obj->GetPositionY();
	double objAngle = obj->GetDirectionAngle();
	double sAngle = angLaser_;
	double angle = sAngle + dAngle;

	double tx = ox + sx + dRadius * cos(Math::DegreeToRadian(angle));
	double ty = oy + sy + dRadius * sin(Math::DegreeToRadian(angle));
	obj->SetPositionX(tx);
	obj->SetPositionY(ty);
	obj->SetDirectionAngle(angle + objAngle);

	StgShotManager* shotManager = stageController_->GetShotManager();
	shotManager->AddShot(obj);
	obj->Activate();
	objectManager->ActivateObject(obj->GetObjectID(), true);
}
void StgStraightLaserObject::RenderOnShotManager(D3DXMATRIX mat)
{
	if(!IsVisible())return;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;

	int objBlendType = GetBlendType();
	int shotBlendType = objBlendType;
	{
		StgShotRenderer* renderer = NULL;
		if(objBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer(StgShotDataList::RENDER_ADD_ARGB);
			shotBlendType = DirectGraphics::MODE_BLEND_ADD_ARGB;
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objBlendType);
		}
		if(renderer == NULL)return;

		//レーザー
		D3DXMATRIX matScale;
		D3DXMATRIX matRot;
		D3DXMATRIX matTrans;
		RECT rcSrc;
		RECT rcDest;
		D3DCOLOR color;
		D3DXMatrixScaling(&matScale, scaleX_, 1.0f, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(angLaser_ + 270));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		D3DXMATRIX matLaser = matScale * matRot * matTrans * mat;

		rcSrc = shotData->GetRect(frameWork_);
		color = color_;
		double alpha = shotData->GetAlpha() / 255.0;
		if(frameFadeDelete_ >= 0) 
			alpha = (double)frameFadeDelete_ / (double)FRAME_FADEDELETE_LASER;

		bool bValidAlpha = StgShotData::IsAlphaBlendValidType(shotBlendType);
		if(bValidAlpha)
		{
			//α有効
			int colorA = ColorAccess::GetColorA(color);
			color = ColorAccess::SetColorA(color, alpha * colorA);
		}
		else
		{
			//α無効
			color = ColorAccess::ApplyAlpha(color, alpha);
		}		

//		if(delay_ > 0)
//			SetRect(&rcDest, -16, length_, 16, 0);
//		else
			SetRect(&rcDest, -widthRender_/2, length_, widthRender_/2, 0);

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
			verts[iVert].position = DxMath::VectMatMulti(verts[iVert].position, matLaser);
		}

		renderer->AddVertex(verts[0]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[3]);
	}

	if(bUseSouce_ && frameFadeDelete_ < 0)
	{
		StgShotRenderer* renderer = NULL;
		int objSourceBlendType = GetSourceBlendType();
		int sourceBlendType = shotBlendType;
		if(objSourceBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			//未設定の場合は、レーザー描画種別を引き継ぐ
			renderer = shotData->GetRendererFromGraphicsBlendType(sourceBlendType);
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objSourceBlendType);
			sourceBlendType = objSourceBlendType;
		}
		if(renderer == NULL)return;

		//光源
		D3DXMATRIX matScale;
		D3DXMATRIX matRot;
		D3DXMATRIX matTrans;
		RECT rcSrc;
		RECT rcDest;
		D3DCOLOR color;
		D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(angLaser_ + 270));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		D3DXMATRIX matSource = matScale * matRot * matTrans * mat;

		rcSrc = shotData->GetDelayRect();
		color = shotData->GetDelayColor();

		int sourceWidth = widthRender_ * 2 / 3;
		SetRect(&rcDest, -sourceWidth, -sourceWidth, sourceWidth, sourceWidth);

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
			verts[iVert].position = DxMath::VectMatMulti(verts[iVert].position, matSource);
		}

		renderer->AddVertex(verts[0]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[3]);
	}

}
void StgStraightLaserObject::_ConvertToItemAndSendEvent()
{
	StgItemManager* itemManager = stageController_->GetItemManager();
	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	ref_count_ptr<ManagedScript> scriptItem = stageScriptManager->GetItemScript();

	int ex = GetPositionX();
	int ey = GetPositionY();
	double angle = angLaser_;

	for(double itemPos = 0 ; itemPos < length_ ; itemPos += itemDistance_)
	{
		double posX = ex + itemPos * cos(Math::DegreeToRadian(angle));
		double posY = ey + itemPos * sin(Math::DegreeToRadian(angle));

		if(scriptItem != NULL)
		{
			std::vector<long double> listPos;
			listPos.push_back(posX);
			listPos.push_back(posY);

			std::vector<gstd::value> listScriptValue;
			listScriptValue.push_back(scriptItem->CreateRealValue(idObject_));
			listScriptValue.push_back(scriptItem->CreateRealArrayValue(listPos));
			scriptItem->RequestEvent(StgStageScript::EV_DELETE_SHOT_TO_ITEM, listScriptValue);
		}

		if(itemManager->IsDefaultBonusItemEnable() && delay_ == 0)
		{
			ref_count_ptr<StgItemObject>::unsync obj = new StgItemObject_Bonus(stageController_);
			int id = stageController_->GetMainObjectManager()->AddObject(obj);
			if(id != DxScript::ID_INVALID)
			{
				//弾の座標にアイテムを作成する
				itemManager->AddItem(obj);
				obj->SetPositionX(posX);
				obj->SetPositionY(posY);
			}
		}
	}
}
void StgStraightLaserObject::RegistIntersectionTarget()
{
	if(!bUserIntersectionMode_)
	{
		_AddIntersectionRelativeTarget();
	}
}
/**********************************************************
//StgCurveLaserObject(曲がる型レーザー)
**********************************************************/
StgCurveLaserObject::StgCurveLaserObject(StgStageController* stageController) : StgLaserObject(stageController)
{
	typeObject_ = StgStageScript::OBJ_CURVE_LASER;
	tipDecrement_ = 1.0;
}
void StgCurveLaserObject::Work()
{
	_Move();
	if(delay_ == 0)
	{
		_AddReservedShotWork();
	}
	
	delay_ = max(delay_ - 1, 0);
	frameWork_++;

	if(frameFadeDelete_ >= 0)
	{
		frameFadeDelete_--;
	}

	_DeleteInAutoClip();
	_DeleteInLife();
	_DeleteInFadeDelete();
	_DeleteInAutoDeleteFrame();
//	_AddIntersectionRelativeTarget();
	frameGrazeInvalid_--;
}
void StgCurveLaserObject::_Move()
{
	StgMoveObject::_Move();
	DxScriptRenderObject::SetX(posX_);
	DxScriptRenderObject::SetY(posY_);

	Position pos;
	pos.x = posX_;
	pos.y = posY_;

	listPosition_.push_front(pos);
	if(listPosition_.size() > length_)
	{
		listPosition_.pop_back();
	}
}
void StgCurveLaserObject::_DeleteInAutoClip()
{
	if(IsDeleted())return;
	if(!IsAutoDelete())return;
	StgShotManager* shotManager = stageController_->GetShotManager();
	RECT rect = shotManager->GetShotAutoDeleteClipRect();

	bool bDelete = listPosition_.size() > 0;
	std::list<Position>::iterator itr = listPosition_.begin();
	for(; itr != listPosition_.end() ; itr++)
	{
		Position& pos = (*itr);
		bool bXOut = pos.x < rect.left || pos.x > rect.right;
		bool bYOut = pos.y < rect.top || pos.y > rect.bottom;
		if(!bXOut && !bYOut)
		{
			bDelete = false;
			break;
		}
	}

	if(bDelete)
	{
		ref_count_ptr<StgStageScriptObjectManager> objectManager = stageController_->GetMainObjectManager();
		objectManager->DeleteObject(idObject_);
	}

}
std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > StgCurveLaserObject::GetIntersectionTargetList()
{
	std::vector<ref_count_ptr<StgIntersectionTarget>::unsync > res;
	if(delay_ > 0)return res;
	if(frameFadeDelete_ >= 0)return res;
	if(IsDeleted())return res;
	if(bUserIntersectionMode_)return res;//ユーザ定義あたり判定モード
	if(!bIntersectionEnable_)return res;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return res;

	ref_count_ptr<StgShotObject>::unsync obj = GetOwnObject();
	if(obj == NULL)return res;
	ref_count_weak_ptr<StgShotObject>::unsync wObj = obj;

	//当たり判定
	std::vector<Position> listPos;
	std::list<Position>::iterator itr = listPosition_.begin();
	for(; itr != listPosition_.end() ; itr++)
	{
		listPos.push_back(*itr);
	}

	StgIntersectionManager* intersectionManager = stageController_->GetIntersectionManager();
	int countPos = listPos.size();
	for(int iPos = 0 ; iPos < countPos - 1; iPos++)
	{
		double posXS = listPos[iPos].x;
		double posYS = listPos[iPos].y;
		double posXE = listPos[iPos + 1].x;
		double posYE = listPos[iPos + 1].y;
/*
		if(iPos == 0)
		{
			double ang = atan2(posYE - posYS, posXE - posXS);
			int length = min(0 , invalidLengthStart_);
			posXS = posXS + length * cos(ang);
			posYS = posYS + length * sin(ang);
		}
		if(iPos == countPos - 2)
		{
			double ang = atan2(posYE - posYS, posXE - posXS);
			int length = max(invalidLengthEnd_, 0);
			posXE = posXE - length * cos(ang);
			posYE = posYE - length * sin(ang);
		}
*/
		DxWidthLine line(posXS, posYS, posXE, posYE, widthIntersection_);
		ref_count_ptr<StgIntersectionTarget_Line>::unsync target = 
			ref_count_ptr<StgIntersectionTarget_Line>::unsync::DownCast(intersectionManager->GetPoolObject(StgIntersectionTarget::SHAPE_LINE));
		if(typeOwner_ == OWNER_PLAYER)
			target->SetTargetType(StgIntersectionTarget::TYPE_PLAYER_SHOT);
		else 
			target->SetTargetType(StgIntersectionTarget::TYPE_ENEMY_SHOT);
		target->SetObject(wObj);
		target->SetLine(line);

		res.push_back(target);
	}
	return res;
}

void StgCurveLaserObject::RenderOnShotManager(D3DXMATRIX mat)
{
	if(!IsVisible())return;

	StgShotData* shotData = _GetShotData();
	if(shotData == NULL)return;

	int shotBlendType = StgShotDataList::RENDER_ADD_ARGB;
	StgShotRenderer* renderer = NULL;
	if(delay_ > 0)
	{
		//遅延時間
		int objDelayBlendType = GetSourceBlendType();
		if(objDelayBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer(StgShotDataList::RENDER_ADD_ARGB);
			shotBlendType = DirectGraphics::MODE_BLEND_ADD_ARGB;
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objDelayBlendType);
		}
	}
	else
	{
		int objBlendType = GetBlendType();
		int shotBlendType = objBlendType;
		if(objBlendType == DirectGraphics::MODE_BLEND_NONE)
		{
			renderer = shotData->GetRenderer(StgShotDataList::RENDER_ADD_ARGB);
			shotBlendType = DirectGraphics::MODE_BLEND_ADD_ARGB;
		}
		else
		{
			renderer = shotData->GetRendererFromGraphicsBlendType(objBlendType);
		}
	}
	if(renderer == NULL)return;

	D3DXMATRIX matScale;
	D3DXMATRIX matRot;
	D3DXMATRIX matTrans;
	D3DCOLOR color;

	std::vector<RECT_D> listRcSrc;
	std::vector<RECT_D> listRcDest;
	std::vector<D3DXMATRIX> listMatrix;
	if(delay_ > 0)
	{
		RECT rcSrc = shotData->GetDelayRect();
		RECT rcDest;

		//遅延時間
		double expa = 0.5f + (double)delay_ / 30.0f * 2;
		if(expa > 3.5)expa = 3.5;

		D3DXMatrixScaling(&matScale, expa, expa, 1.0f);
		D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, D3DXToRadian(GetDirectionAngle() + 90));
		D3DXMatrixTranslation(&matTrans, position_.x, position_.y, position_.z);
		D3DXMATRIX tMat = matScale * matRot * matTrans * mat;
		color = shotData->GetDelayColor();

		int width = rcSrc.right - rcSrc.left;
		int height = rcSrc.bottom - rcSrc.top;
		SetRect(&rcDest, -width / 2, -height / 2, width / 2, height / 2);
		if(width % 2 == 1)rcDest.right += 1;
		if(height % 2 == 1)rcDest.bottom += 1;

		RECT_D rcSrcD = GetRectD(rcSrc);
		RECT_D rcDestD = GetRectD(rcDest);
		listRcSrc.push_back(rcSrcD);
		listRcDest.push_back(rcDestD);
		listMatrix.push_back(tMat);
	}
	else
	{
		std::vector<Position> listPos;
		std::list<Position>::iterator itr = listPosition_.begin();
		for(; itr != listPosition_.end() ; itr++)
		{
			listPos.push_back(*itr);
		}

		int countPos = listPos.size();
		RECT rcSrcOrg = shotData->GetRect(frameWork_);
		double rate = (double)(rcSrcOrg.bottom-rcSrcOrg.top) / (double)max(countPos, 1);

		color = color_;
		double alpha = shotData->GetAlpha() / 255.0;
		if(frameFadeDelete_ >= 0) 
			alpha = (double)frameFadeDelete_ / (double)FRAME_FADEDELETE;

		bool bValidAlpha = StgShotData::IsAlphaBlendValidType(shotBlendType);
		if(bValidAlpha)
		{
			//α有効
			int colorA = ColorAccess::GetColorA(color);
			color = ColorAccess::SetColorA(color, alpha * colorA);
		}
		else
		{
			//α無効
			color = ColorAccess::ApplyAlpha(color, alpha);
		}	

		RECT_D rcSrcD = GetRectD(rcSrcOrg);
		rcSrcD.bottom = rcSrcD.top;
		double posSrc = 0;
		for(int iPos = 0 ; iPos  < countPos - 1; iPos++)
		{
			double posXS = listPos[iPos].x;
			double posYS = listPos[iPos].y;
			double posXE = listPos[iPos + 1].x;
			double posYE = listPos[iPos + 1].y;
			double ang = atan2(posYE - posYS, posXE - posXS);

			RECT_D rcDestD;
			D3DXMatrixScaling(&matScale, 1.0f, 1.0f, 1.0f);
			D3DXMatrixRotationYawPitchRoll(&matRot, 0, 0, ang + D3DXToRadian(90));
			D3DXMatrixTranslation(&matTrans, posXS, posYS, position_.z);
			D3DXMATRIX tMat = matScale * matRot * matTrans * mat;

			rcSrcD.top = rcSrcOrg.top + posSrc;
			double bottom = rcSrcD.top + rate;
			if(rcSrcD.top == bottom)bottom = bottom + 1;
			rcSrcD.bottom = bottom;
			posSrc += rate;

			double radius = pow(pow(posXE - posXS,2) + pow(posYE - posYS,2), 0.5);
			SetRectD(&rcDestD, -widthRender_/2, radius, widthRender_/2, 0);

			listRcSrc.push_back(rcSrcD);
			listRcDest.push_back(rcDestD);
			listMatrix.push_back(tMat);
		}

	}


	int countRect = listRcSrc.size();
	double tAlpha = ColorAccess::GetColorA(color);
	double dAlpha = tAlpha / (double)listRcSrc.size() * 2. * tipDecrement_;
	VERTEX_TLX oldVerts[4];
	for(int iRect = 0 ; iRect < countRect ; iRect++)
	{
		if(iRect > countRect / 2)
			tAlpha -= dAlpha;
		if(iRect < listRcSrc.size() / 2)
			tAlpha = iRect * 256 / (listRcSrc.size() / 2) + (255 - tipDecrement_ * 255.);
		tAlpha = max(0, tAlpha);

		D3DCOLOR tColor = color;
		bool bValidAlpha = StgShotData::IsAlphaBlendValidType(shotBlendType);
		if(bValidAlpha)
		{
			//α有効
			int colorA = ColorAccess::GetColorA(tColor);
			tColor = ColorAccess::SetColorA(tColor, tAlpha * colorA);
		}
		else
		{
			//α無効
			tColor = ColorAccess::ApplyAlpha(tColor, tAlpha / 255.0);
		}	


		RECT_D& rcSrc = listRcSrc[iRect];
		RECT_D& rcDest = listRcDest[iRect];
		D3DXMATRIX& tMat = listMatrix[iRect];

		VERTEX_TLX verts[4];
		double srcX[] = {rcSrc.left, rcSrc.right, rcSrc.left, rcSrc.right};
		double srcY[] = {rcSrc.top, rcSrc.top, rcSrc.bottom, rcSrc.bottom};
		double destX[] = {rcDest.left, rcDest.right, rcDest.left, rcDest.right};
		double destY[] = {rcDest.top, rcDest.top, rcDest.bottom, rcDest.bottom};
		for(int iVert = 0 ;iVert < 4 ; iVert++)
		{
			_SetVertexUV(verts[iVert], srcX[iVert], srcY[iVert]);
			_SetVertexPosition(verts[iVert], destX[iVert], destY[iVert]);
			_SetVertexColorARGB(verts[iVert], tColor);
			verts[iVert].position = DxMath::VectMatMulti(verts[iVert].position, tMat);
		}

		if(iRect > 0)
		{
			verts[0] = oldVerts[2];
			verts[1] = oldVerts[3];
		}
		memcpy(&oldVerts, verts, sizeof(VERTEX_TLX) * 4);

		renderer->AddVertex(verts[0]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[1]);
		renderer->AddVertex(verts[2]);
		renderer->AddVertex(verts[3]);
	}

}
void StgCurveLaserObject::_ConvertToItemAndSendEvent()
{
	StgItemManager* itemManager = stageController_->GetItemManager();
	StgStageScriptManager* stageScriptManager = stageController_->GetScriptManagerP();
	ref_count_ptr<ManagedScript> scriptItem = stageScriptManager->GetItemScript();

	std::list<Position>::iterator itr = listPosition_.begin();
	for(; itr != listPosition_.end() ; itr++)
	{
		Position pos = (*itr);
		double posX = pos.x;
		double posY = pos.y;

		if(scriptItem != NULL)
		{
			std::vector<long double> listPos;
			listPos.push_back(posX);
			listPos.push_back(posY);

			std::vector<gstd::value> listScriptValue;
			listScriptValue.push_back(scriptItem->CreateRealValue(idObject_));
			listScriptValue.push_back(scriptItem->CreateRealArrayValue(listPos));
			scriptItem->RequestEvent(StgStageScript::EV_DELETE_SHOT_TO_ITEM, listScriptValue);
		}

		if(itemManager->IsDefaultBonusItemEnable() && delay_ == 0)
		{
			ref_count_ptr<StgItemObject>::unsync obj = new StgItemObject_Bonus(stageController_);
			int id = stageController_->GetMainObjectManager()->AddObject(obj);
			if(id != DxScript::ID_INVALID)
			{
				//弾の座標にアイテムを作成する
				itemManager->AddItem(obj);
				obj->SetPositionX(posX);
				obj->SetPositionY(posY);
			}
		
		}
	}
}
void StgCurveLaserObject::RegistIntersectionTarget()
{
	if(!bUserIntersectionMode_)
	{
		_AddIntersectionRelativeTarget();
	}
}
