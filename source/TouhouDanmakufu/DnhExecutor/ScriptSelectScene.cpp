#include"ScriptSelectScene.hpp"
#include"System.hpp"

/**********************************************************
//ScriptSelectScene
**********************************************************/
ScriptSelectScene::ScriptSelectScene()
{
	std::wstring pathBack = EPathProperty::GetSystemImageDirectory() + L"System_ScriptSelect_Background.png";
	ref_count_ptr<Texture> textureBack = new Texture();
	textureBack->CreateFromFile(pathBack);

	DirectGraphics* graphics = DirectGraphics::GetBase();
	int screenWidth = graphics->GetScreenWidth();
	int screenHeight = graphics->GetScreenHeight();
	RECT_D srcBack = {0., 0., 640., 480.};
	RECT_D destBack = {0., 0., (double)screenWidth, (double)screenHeight};
	spriteBack_ = new Sprite2D();
	spriteBack_->SetTexture(textureBack);
	spriteBack_->SetVertex(srcBack, destBack);

	spriteImage_ = new Sprite2D();

	pageMaxY_ = 9;
	objMenuText_.resize(COUNT_MENU_TEXT);
	bPageChangeX_ = true;

	frameSelect_ = 0;
}
ScriptSelectScene::~ScriptSelectScene()
{
	ClearModel();
}
void ScriptSelectScene::Clear()
{
	MenuTask::Clear();
	objMenuText_.clear();
	objMenuText_.resize(COUNT_MENU_TEXT);
}
void ScriptSelectScene::_ChangePage()
{
	DxText dxText;
	dxText.SetFontBorderType(directx::DxFont::BORDER_FULL);
	dxText.SetFontBorderWidth(2);
	dxText.SetFontSize(16);
	dxText.SetFontBold(true);

	int top = (pageCurrent_ - 1) * (pageMaxY_ + 1);
	for(int iItem = 0 ; iItem <= pageMaxY_ ; iItem++)
	{
		int index = top + iItem;
		if(index < item_.size() && item_[index] != NULL)
		{
			ScriptSelectSceneMenuItem* pItem = (ScriptSelectSceneMenuItem*)item_[index].GetPointer();
			if(pItem->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR)
			{
				dxText.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
				dxText.SetFontColorBottom(D3DCOLOR_ARGB(255,255,64,64));
				dxText.SetFontBorderColor(D3DCOLOR_ARGB(255,128,32,32));
				std::wstring path = pItem->GetPath();
				std::wstring text = L"[DIR.] ";
				text += PathProperty::GetDirectoryName(path);
				dxText.SetText(text);
			}
			else
			{
				dxText.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
				dxText.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
				dxText.SetFontBorderColor(D3DCOLOR_ARGB(255,32,32,128));
				ref_count_ptr<ScriptInformation> info = pItem->GetScriptInformation();
				dxText.SetText(info->GetTitle());
			}

			objMenuText_[iItem] = dxText.CreateRenderObject();
		}
		else
		{
			objMenuText_[iItem] = NULL;
		}
	}

	frameSelect_ = 0;
}
void ScriptSelectScene::Work()
{
	{
		ref_count_ptr<ScriptSelectFileModel> model = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
		if(model != NULL && model->GetStatus() == Thread::RUN)
		{
			if(model->GetWaitPath().size() > 0)return;
		}
	}
	if(!bActive_)return;

	int lastCursorY = cursorY_;

	MenuTask::Work();
	if(!_IsWaitedKeyFree())return;

	EDirectInput* input = EDirectInput::GetInstance();
	if(input->GetVirtualKeyState(EDirectInput::KEY_OK) == KEY_PUSH)
	{
		ref_count_ptr<MenuItem> tItem = GetSelectedMenuItem();
		ref_count_ptr<ScriptSelectSceneMenuItem> item = ref_count_ptr<ScriptSelectSceneMenuItem>::DownCast(tItem);
		if(item != NULL)
		{
			bool bDir = item->GetType() == ScriptSelectSceneMenuItem::TYPE_DIR;
			if(bDir)
			{
				//�f�B���N�g��
				ref_count_ptr<ScriptSelectModel> model = NULL;
				int index = GetSelectedItemIndex();
				ScriptSelectSceneMenuItem* pItem = (ScriptSelectSceneMenuItem*)item_[index].GetPointer();
				std::wstring dir = pItem->GetPath();
				
				//�y�[�W���Z�b�g
				cursorX_ = 0;
				cursorY_ = 0;
				pageCurrent_ = 1;

				model = new ScriptSelectFileModel(TYPE_DIR, dir);
				SetModel(model);

				SystemController::GetInstance()->GetSystemInformation()->SetLastScriptSearchDirectory(dir);
				bWaitedKeyFree_ = false;
			}
			else
			{
				//�X�N���v�g
				SetActive(false);

				int index = GetSelectedItemIndex();
				ScriptSelectSceneMenuItem* pItem = (ScriptSelectSceneMenuItem*)item_[index].GetPointer();
				ref_count_ptr<ScriptInformation> info = pItem->GetScriptInformation();

				std::wstring pathLastSelected = info->GetScriptPath();
				SystemController::GetInstance()->GetSystemInformation()->SetLastSelectedScriptPath(pathLastSelected);

				ETaskManager* taskManager = ETaskManager::GetInstance();
				ref_count_ptr<PlayTypeSelectScene> task = new PlayTypeSelectScene(info);
				taskManager->AddTask(task);
				taskManager->AddWorkFunction(TTaskFunction<PlayTypeSelectScene>::Create(
					task, &PlayTypeSelectScene::Work), PlayTypeSelectScene::TASK_PRI_WORK);
				taskManager->AddRenderFunction(TTaskFunction<PlayTypeSelectScene>::Create(
					task, &PlayTypeSelectScene::Render), PlayTypeSelectScene::TASK_PRI_RENDER);				

				return;
			}
		}
	}
	else if(input->GetVirtualKeyState(EDirectInput::KEY_CANCEL) == KEY_PUSH)
	{
		bool bTitle = true;

		if(GetType() == TYPE_DIR)
		{
			ref_count_ptr<ScriptSelectFileModel> fileModel = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
			std::wstring dir = fileModel->GetDirectory();
			std::wstring root = EPathProperty::GetStgScriptRootDirectory();
			if(!File::IsEqualsPath(dir, root))
			{
				//�L�����Z��
				bTitle = false;
				std::wstring dirOld = SystemController::GetInstance()->GetSystemInformation()->GetLastScriptSearchDirectory();
				std::wstring::size_type pos = dirOld.find_last_of(L"/", dirOld.size() - 2);
				std::wstring dir = dirOld.substr(0, pos) + L"/";
				ref_count_ptr<ScriptSelectFileModel> model = new ScriptSelectFileModel(TYPE_DIR, dir);
				model->SetWaitPath(dirOld);

				//SetWaitPath�őI�𒆂̃p�X�Ɉړ������邽�߂�
				//�y�[�W���Z�b�g
				cursorX_ = 0;
				cursorY_ = 0;
				pageCurrent_ = 1;
				SetModel(model);

				SystemController::GetInstance()->GetSystemInformation()->SetLastScriptSearchDirectory(dir);
				bWaitedKeyFree_ = false;
				return;
			}
		}
		
		if(bTitle)
		{
			SceneManager* sceneManager = SystemController::GetInstance()->GetSceneManager();
			sceneManager->TransTitleScene();
			return;
		}
	}

	if(lastCursorY != cursorY_)frameSelect_ = 0;
	else frameSelect_++;
}
void ScriptSelectScene::Render()
{
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();
	graphics->SetRenderStateFor2D(DirectGraphics::MODE_BLEND_ALPHA);

	spriteBack_->Render();

	std::wstring strType;
	switch(GetType())
	{
		case TYPE_SINGLE: strType = L"Single"; break;
		case TYPE_PLURAL: strType = L"Plural"; break;
		case TYPE_STAGE: strType = L"Stage"; break;
		case TYPE_PACKAGE: strType = L"Package"; break;
		case TYPE_DIR: strType = L"Directory"; break;
		case TYPE_ALL: strType = L"All"; break;
	}

	std::wstring strDescription = StringUtility::Format(
		L"�t�@�C����I�����Ă������� (%s�F%d/%d)", strType.c_str(), pageCurrent_, GetPageCount() );

	DxText dxTextDescription;
	dxTextDescription.SetFontColorTop(D3DCOLOR_ARGB(255,128,128,255));
	dxTextDescription.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
	dxTextDescription.SetFontBorderType(directx::DxFont::BORDER_FULL);
	dxTextDescription.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
	dxTextDescription.SetFontBorderWidth(2);
	dxTextDescription.SetFontSize(20);
	dxTextDescription.SetFontBold(true);
	dxTextDescription.SetText(strDescription);
	dxTextDescription.SetPosition(32,8);
	dxTextDescription.Render();

	//�f�B���N�g����
	if(GetType() == TYPE_DIR)
	{
		ref_count_ptr<ScriptSelectFileModel> fileModel = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
		std::wstring dir = fileModel->GetDirectory();
		std::wstring root = EPathProperty::GetStgScriptRootDirectory();

		dir = PathProperty::ReplaceYenToSlash(dir);
		root = PathProperty::ReplaceYenToSlash(root);
		std::wstring textDir = L"script/" + StringUtility::ReplaceAll(dir, root, L"");

		DxText dxTextDir;
		dxTextDir.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
		dxTextDir.SetFontColorBottom(D3DCOLOR_ARGB(255,255,128,128));
		dxTextDir.SetFontBorderType(directx::DxFont::BORDER_NONE);
		dxTextDir.SetFontBorderWidth(0);
		dxTextDir.SetFontSize(14);
		dxTextDir.SetFontBold(true);
		dxTextDir.SetText(strDescription);
		dxTextDir.SetPosition(40,32);
		dxTextDir.SetText(textDir);
		dxTextDir.Render();
	}

	{
		//�^�C�g��
		Lock lock(cs_);
		for(int iItem = 0 ; iItem <= pageMaxY_ ; iItem++)
		{
			ref_count_ptr<DxTextRenderObject> obj = objMenuText_[iItem];
			if(obj == NULL)continue;
			int alphaText = bActive_ ? 255 : 128;
			obj->SetVertexColor(D3DCOLOR_ARGB(255, alphaText, alphaText, alphaText));
			obj->SetPosition(32, 48 + iItem * 18);
			obj->Render();

			if(iItem == cursorY_)
			{
				graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ADD_RGB);
				int cycle = 60;
				int alpha = frameSelect_ % cycle;
				if(alpha < cycle / 2)alpha = 255 * (float)((float)(cycle / 2 - alpha) / (float)(cycle / 2));
				else alpha = 255 * (float)((float)(alpha - cycle / 2) / (float)(cycle / 2));
				obj->SetVertexColor(D3DCOLOR_ARGB(255, alpha, alpha, alpha ));
				obj->Render();
				obj->SetVertexColor(D3DCOLOR_ARGB(255, 255, 255, 255));
				graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
			}
		}

		ref_count_ptr<ScriptSelectSceneMenuItem> item = 
			ref_count_ptr<ScriptSelectSceneMenuItem>::DownCast(GetSelectedMenuItem());
		if(bActive_ && item != NULL && item->GetType() != ScriptSelectSceneMenuItem::TYPE_DIR)
		{
			ref_count_ptr<ScriptInformation> info = item->GetScriptInformation();

			DxText dxTextInfo;
			dxTextInfo.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
			dxTextInfo.SetFontBorderType(directx::DxFont::BORDER_NONE);
			dxTextInfo.SetFontBorderWidth(0);
			dxTextInfo.SetFontSize(16);
			dxTextInfo.SetFontBold(true);

			//�C���[�W
			ref_count_ptr<Texture> texture = spriteImage_->GetTexture();
			std::wstring pathImage1 = L"";
			if(texture != NULL)pathImage1 = texture->GetName();
			std::wstring pathImage2 = info->GetImagePath();
			if(pathImage1 != pathImage2)
			{
				texture = new Texture();
				File file(pathImage2);
				if(file.IsExists())
				{
					texture->CreateFromFileInLoadThread(pathImage2);
					spriteImage_->SetTexture(texture);
				}
				else
					spriteImage_->SetTexture(NULL);
					
			}

			texture = spriteImage_->GetTexture();
			if(texture != NULL && texture->IsLoad())
			{
				RECT_D rcSrc = {0., 0., (double)texture->GetWidth(), (double)texture->GetHeight()};
				RECT_D rcDest = {340., 250., (double)(rcDest.left + 280), (double)(rcDest.top + 210)};
				spriteImage_->SetSourceRect(rcSrc);
				spriteImage_->SetDestinationRect(rcDest);
				spriteImage_->Render();
			}

			//�X�N���v�g�p�X
			std::wstring path = info->GetScriptPath();
			std::wstring root = EPathProperty::GetStgScriptRootDirectory();
			root = PathProperty::ReplaceYenToSlash(root);
			path = PathProperty::ReplaceYenToSlash(path);
			path = StringUtility::ReplaceAll(path, root, L"");
			
			std::wstring archive = info->GetArchivePath();
			if(archive.size() > 0)
			{
				archive = PathProperty::ReplaceYenToSlash(archive);
				archive = StringUtility::ReplaceAll(archive, root, L"");
				path += StringUtility::Format(L" [%s]", archive.c_str());
			}

			dxTextInfo.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
			dxTextInfo.SetFontColorBottom(D3DCOLOR_ARGB(255,255,64,64));
			dxTextInfo.SetText(path);
			dxTextInfo.SetPosition(16, 240);
			dxTextInfo.Render();

			//�X�N���v�g���
			int type = info->GetType();
			std::wstring strType = L"";
			switch(type)
			{
			case ScriptInformation::TYPE_SINGLE:strType = L"(Single)";break;
			case ScriptInformation::TYPE_PLURAL:strType = L"(Plural)";break;
			case ScriptInformation::TYPE_STAGE:strType = L"(Stage)";break;
			case ScriptInformation::TYPE_PACKAGE:strType = L"(Package)";break;
			}
			dxTextInfo.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
			dxTextInfo.SetFontColorBottom(D3DCOLOR_ARGB(255,255,64,255));
			dxTextInfo.SetText(strType);
			dxTextInfo.SetPosition(32, 256);
			dxTextInfo.Render();

			//�e�L�X�g
			std::wstring text = info->GetText();
			dxTextInfo.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
			dxTextInfo.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
			dxTextInfo.SetFontSize(18);
			dxTextInfo.SetLinePitch(9);
			dxTextInfo.SetText(text);
			dxTextInfo.SetPosition(24, 288);
			dxTextInfo.SetMaxWidth(320);
			dxTextInfo.Render();

		}

	}

	if(!model_->IsCreated())
	{
		//���[�h��
		std::wstring text = L"Now Loading...";
		DxText dxTextNowLoad;
		dxTextNowLoad.SetFontColorTop(D3DCOLOR_ARGB(255,128,128,128));
		dxTextNowLoad.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,64));
		dxTextNowLoad.SetFontBorderType(directx::DxFont::BORDER_FULL);
		dxTextNowLoad.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
		dxTextNowLoad.SetFontBorderWidth(2);
		dxTextNowLoad.SetFontSize(18);
		dxTextNowLoad.SetFontBold(true);
		dxTextNowLoad.SetText(strDescription);
		dxTextNowLoad.SetPosition(24, 452);
		dxTextNowLoad.SetText(text);
		dxTextNowLoad.Render();
	}
}

int ScriptSelectScene::GetType()
{
	int res = TYPE_SINGLE;
	ref_count_ptr<ScriptSelectFileModel> fileModel = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
	if(fileModel != NULL)
	{
		res = fileModel->GetType();
	}
	return res;
}
void ScriptSelectScene::SetModel(ref_count_ptr<ScriptSelectModel> model)
{
	ClearModel();

	if(model == NULL)return;
	model->scene_ = this;
	model_ = model;

	ref_count_ptr<ScriptSelectFileModel> fileModel = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
	if(fileModel != NULL)
	{
		SystemController::GetInstance()->GetSystemInformation()->SetLastSelectScriptSceneType(
			fileModel->GetType());
	}

	model->CreateMenuItem();
}
void ScriptSelectScene::ClearModel()
{
	Clear();
	if(model_ != NULL)
	{
		ref_count_ptr<ScriptSelectFileModel> fileModel = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
		if(fileModel != NULL)
		{
			fileModel->Stop();
		}
		fileModel->Join();
	}

	model_ = NULL;
}

void ScriptSelectScene::AddMenuItem(std::list<ref_count_ptr<ScriptSelectSceneMenuItem> >& listItem)
{
	if(listItem.size() == 0)return;

	{
		Lock lock(cs_);

		std::list<ref_count_ptr<ScriptSelectSceneMenuItem> >::iterator itr = listItem.begin();
		for(;itr != listItem.end() ; itr++)
		{
			MenuTask::AddMenuItem((*itr));
		}

		//���ݑI�𒆂̃A�C�e��
		ref_count_ptr<MenuItem> item = GetSelectedMenuItem();

		//�\�[�g
		std::sort(item_.begin(), item_.end(), ScriptSelectScene::Sort());

		//���ݑI�𒆂̃A�C�e���Ɉړ�
		if(item != NULL)
		{
			bool bWait = false;
			std::wstring path = L"";
			ref_count_ptr<ScriptSelectFileModel> model = ref_count_ptr<ScriptSelectFileModel>::DownCast(model_);
			if(model != NULL)
			{
				path = model->GetWaitPath();
				if(path.size() > 0)
					bWait = true;
			}
			
			if(path.size() == 0 && (pageCurrent_ > 1 || cursorY_ > 0))
			{
				ScriptSelectSceneMenuItem* pItem = (ScriptSelectSceneMenuItem*)item.GetPointer();
				path = pItem->GetPath();
			}

			for(int iItem = 0 ; iItem < item_.size() ; iItem++)
			{
				ScriptSelectSceneMenuItem* itrItem = (ScriptSelectSceneMenuItem*)item_[iItem].GetPointer();
				if(itrItem == NULL)continue;

				bool bEqualsPath = File::IsEqualsPath(path , itrItem->GetPath());;
				if(!bEqualsPath)continue;

				pageCurrent_ = iItem / (pageMaxY_ + 1) + 1;
				cursorY_ = iItem % (pageMaxY_ + 1);

				if(bWait)
					model->SetWaitPath(L"");

				break;
			}
		}

		_ChangePage();
	}
}

//ScriptSelectSceneMenuItem
ScriptSelectSceneMenuItem::ScriptSelectSceneMenuItem(int type, std::wstring path, ref_count_ptr<ScriptInformation> info)
{
	type_ = type;
	path_ = PathProperty::ReplaceYenToSlash(path);
	info_ = info;
}
ScriptSelectSceneMenuItem::~ScriptSelectSceneMenuItem()
{
}

/**********************************************************
//ScriptSelectModel
**********************************************************/
ScriptSelectModel::ScriptSelectModel()
{
	bCreated_ = false;
}
ScriptSelectModel::~ScriptSelectModel()
{
}


//ScriptSelectFileModel
ScriptSelectFileModel::ScriptSelectFileModel(int type, std::wstring dir)
{
	type_ = type;
	dir_ = dir;
}
ScriptSelectFileModel::~ScriptSelectFileModel()
{

}
void ScriptSelectFileModel::_Run()
{
	timeLastUpdate_ = ::timeGetTime() - 1000;
	_SearchScript(dir_);
	if(GetStatus() == RUN)
	{
		scene_->AddMenuItem(listItem_);
		listItem_.clear();
	}
	bCreated_= true;
}
void ScriptSelectFileModel::_SearchScript(std::wstring dir)
{
	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::wstring findDir = dir + L"*.*";
	hFind = FindFirstFile(findDir.c_str(), &data);
	do 
	{
		if(GetStatus() != RUN)return;

		int time = ::timeGetTime();
		if(abs(time - timeLastUpdate_) > 500)
		{
			//500ms���ɍX�V
			timeLastUpdate_ = time;
			scene_->AddMenuItem(listItem_);
			listItem_.clear();
		}

		std::wstring name = data.cFileName;
		if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			(name != L".." && name != L"."))
		{
			//�f�B���N�g��
			std::wstring tDir = dir + name;
			tDir += L"\\";

			if(type_ == TYPE_DIR)
			{
				//�f�B���N�g��
				ref_count_ptr<ScriptSelectSceneMenuItem> item = new ScriptSelectSceneMenuItem(
					ScriptSelectSceneMenuItem::TYPE_DIR, tDir, NULL);
				listItem_.push_back(item);
			}
			else
			{
				_SearchScript(tDir);
			}
			continue;
		}
		if(wcscmp(data.cFileName, L"..")==0 || wcscmp(data.cFileName, L".")==0)
			continue;

		//�t�@�C��
		std::wstring path = dir + name;

		_CreateMenuItem(path);

	}while(FindNextFile(hFind,&data));
	FindClose(hFind);
}
void ScriptSelectFileModel::_CreateMenuItem(std::wstring path)
{
	std::vector<ref_count_ptr<ScriptInformation> >listInfo = 
		ScriptInformation::CreateScriptInformationList(path, true);
	for(int iInfo = 0 ; iInfo < listInfo.size() ; iInfo++)
	{
		ref_count_ptr<ScriptInformation> info = listInfo[iInfo];
		if(!_IsValidScriptInformation(info))continue;

		int typeItem = _ConvertTypeInfoToItem(info->GetType());
		ref_count_ptr<ScriptSelectSceneMenuItem> item = new ScriptSelectSceneMenuItem(
			typeItem, info->GetScriptPath(), info);
		listItem_.push_back(item);
	}

}
bool ScriptSelectFileModel::_IsValidScriptInformation(ref_count_ptr<ScriptInformation> info)
{
	int typeScript = info->GetType();
	bool bTarget = false;
	switch(type_)
	{
		case TYPE_SINGLE:
			bTarget = (typeScript == ScriptInformation::TYPE_SINGLE);
			break;
		case TYPE_PLURAL:
			bTarget = (typeScript == ScriptInformation::TYPE_PLURAL);
			break;
		case TYPE_STAGE:
			bTarget |= (typeScript == ScriptInformation::TYPE_STAGE);
			//bTarget |= (typeScript == ScriptInformation::TYPE_PACKAGE);
			break;
		case TYPE_PACKAGE:
			bTarget = (typeScript == ScriptInformation::TYPE_PACKAGE);
			break;
		case TYPE_DIR:
			bTarget = (typeScript != ScriptInformation::TYPE_PLAYER);
			break;
		case TYPE_ALL:
			bTarget = (typeScript != ScriptInformation::TYPE_PLAYER);
			break;
	}

	return bTarget;
}
int ScriptSelectFileModel::_ConvertTypeInfoToItem(int typeInfo)
{
	int typeItem = ScriptSelectSceneMenuItem::TYPE_SINGLE;
	switch(typeInfo)
	{
		case ScriptInformation::TYPE_SINGLE:
			typeItem = ScriptSelectSceneMenuItem::TYPE_SINGLE;
			break;
		case ScriptInformation::TYPE_PLURAL:
			typeItem = ScriptSelectSceneMenuItem::TYPE_PLURAL;
			break;
		case ScriptInformation::TYPE_STAGE:
			typeItem = ScriptSelectSceneMenuItem::TYPE_STAGE;
			break;
		case ScriptInformation::TYPE_PACKAGE:
			typeItem = ScriptSelectSceneMenuItem::TYPE_PACKAGE;
			break;
	}
	return typeItem;
}
void ScriptSelectFileModel::CreateMenuItem()
{
	bCreated_ = false;
	if(type_ == TYPE_DIR)
	{
		SystemController::GetInstance()->GetSystemInformation()->SetLastScriptSearchDirectory(dir_);
	}
	Start();
}

/**********************************************************
//PlayTypeSelectScene
**********************************************************/
PlayTypeSelectScene::PlayTypeSelectScene(ref_count_ptr<ScriptInformation> info)
{
	pageMaxY_ = 10;
	bPageChangeX_ = true;

	info_ = info;
	int mx = 24;
	int my = 256;
	AddMenuItem(new PlayTypeSelectMenuItem(L"Play", mx, my));

	//���v���C
	if(info->GetType() != ScriptInformation::TYPE_PACKAGE)
	{
		int itemCount = 1;
		std::wstring pathScript = info->GetScriptPath();
		replayInfoManager_ = new ReplayInformationManager();
		replayInfoManager_->UpdateInformationList(pathScript);
		std::vector<int> listReplayIndex = replayInfoManager_->GetIndexList();
		for(int iList = 0 ; iList < listReplayIndex.size() ; iList++)
		{
			int index = listReplayIndex[iList];
			ref_count_ptr<ReplayInformation> replay = replayInfoManager_->GetInformation(index);
			int itemY = 256 + (itemCount % pageMaxY_) * 20;

			std::wstring text = StringUtility::Format(L"No.%02d %-8s %012I64d %-8s (%2.2ffps) <%s>",
				index, 	
				replay->GetUserName().c_str(),
				replay->GetTotalScore(),
				replay->GetPlayerScriptReplayName().c_str(),
				replay->GetAvarageFps(),
				replay->GetDateAsString().c_str()
			);
			AddMenuItem(new PlayTypeSelectMenuItem(text, mx, itemY));
			itemCount++;
		}
	}

}
void PlayTypeSelectScene::Work()
{
	MenuTask::Work();
	if(!_IsWaitedKeyFree())return;

	EDirectInput* input = EDirectInput::GetInstance();
	if(input->GetVirtualKeyState(EDirectInput::KEY_OK) == KEY_PUSH)
	{
		if(info_->GetType() == ScriptInformation::TYPE_PACKAGE)
		{
			//�p�b�P�[�W���[�h
			SceneManager* sceneManager = SystemController::GetInstance()->GetSceneManager();
			sceneManager->TransPackageScene(info_);
		}
		else
		{
			//�p�b�P�[�W�ȊO
			int indexSelect = GetSelectedItemIndex();
			if(indexSelect == 0) 
			{
				//���@�I��
				TransitionManager* transitionManager = SystemController::GetInstance()->GetTransitionManager();
				transitionManager->DoFadeOut();

				ETaskManager* taskManager = ETaskManager::GetInstance();
				taskManager->SetRenderFunctionEnable(false, typeid(ScriptSelectScene));
				taskManager->SetWorkFunctionEnable(false, typeid(PlayTypeSelectScene));
				taskManager->SetRenderFunctionEnable(false, typeid(PlayTypeSelectScene));

				ref_count_ptr<PlayerSelectScene> task = new PlayerSelectScene(info_);
				taskManager->AddTask(task);
				taskManager->AddWorkFunction(TTaskFunction<PlayerSelectScene>::Create(
					task, &PlayerSelectScene::Work), PlayerSelectScene::TASK_PRI_WORK);
				taskManager->AddRenderFunction(TTaskFunction<PlayerSelectScene>::Create(
					task, &PlayerSelectScene::Render), PlayerSelectScene::TASK_PRI_RENDER);	
			}
			else
			{
				//���v���C
				std::vector<int> listReplayIndex = replayInfoManager_->GetIndexList();
				int replayIndex = listReplayIndex[indexSelect-1];
				ref_count_ptr<ReplayInformation> replay = replayInfoManager_->GetInformation(replayIndex);

				SceneManager* sceneManager = SystemController::GetInstance()->GetSceneManager();
				sceneManager->TransStgScene(info_, replay);
			}
		}

		return;
	}
	else if(input->GetVirtualKeyState(EDirectInput::KEY_CANCEL) == KEY_PUSH)
	{
		ETaskManager* taskManager = ETaskManager::GetInstance();
		ref_count_ptr<ScriptSelectScene> scriptSelectScene = 
			ref_count_ptr<ScriptSelectScene>::DownCast(taskManager->GetTask(typeid(ScriptSelectScene)));
		scriptSelectScene->SetActive(true);

		taskManager->RemoveTask(typeid(PlayTypeSelectScene));
		return;
	}

}
void PlayTypeSelectScene::Render()
{
	MenuTask::Render();
}

//PlayTypeSelectMenuItem
PlayTypeSelectMenuItem::PlayTypeSelectMenuItem(std::wstring text, int x, int y)
{
	pos_.x = x;
	pos_.y = y;

	DxText dxText;
	dxText.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
	dxText.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,64));
	dxText.SetFontBorderType(directx::DxFont::BORDER_FULL);
	dxText.SetFontBorderColor(D3DCOLOR_ARGB(255,32,32,128));
	dxText.SetFontBorderWidth(1);
	dxText.SetFontSize(14);
	dxText.SetFontBold(true);
	dxText.SetText(text);
	objText_ = dxText.CreateRenderObject();
}
PlayTypeSelectMenuItem::~PlayTypeSelectMenuItem()
{
}
void PlayTypeSelectMenuItem::Work()
{
	_WorkSelectedItem();
}
void PlayTypeSelectMenuItem::Render()
{
	objText_->SetPosition(pos_);
	objText_->Render();

	if(menu_->GetSelectedMenuItem() == this)
	{
		DirectGraphics* graphics = DirectGraphics::GetBase();
		graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ADD_RGB);

		int alpha = _GetSelectedItemAlpha();
		objText_->SetVertexColor(D3DCOLOR_ARGB(255, alpha, alpha, alpha));
		objText_->Render();
		objText_->SetVertexColor(D3DCOLOR_ARGB(255, 255, 255, 255));
		graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
	}
}

/**********************************************************
//PlayerSelectScene
**********************************************************/
PlayerSelectScene::PlayerSelectScene(ref_count_ptr<ScriptInformation> info)
{
	pageMaxY_ = 4;
	bPageChangeX_ = true;
	frameSelect_ = 0;

	info_ = info;

	std::wstring pathBack = EPathProperty::GetSystemImageDirectory() + L"System_ScriptSelect_Background.png";
	ref_count_ptr<Texture> textureBack = new Texture();
	textureBack->CreateFromFile(pathBack);

	DirectGraphics* graphics = DirectGraphics::GetBase();
	int screenWidth = graphics->GetScreenWidth();
	int screenHeight = graphics->GetScreenHeight();
	RECT_D srcBack = {0., 0., 640., 480.};
	RECT_D destBack = {0., 0., (double)screenWidth, (double)screenHeight};
	
	spriteBack_ = new Sprite2D();
	spriteBack_->SetTexture(textureBack);
	spriteBack_->SetVertex(srcBack, destBack);

	spriteImage_ = new Sprite2D();

	SystemInformation* systemInfo = SystemController::GetInstance()->GetSystemInformation();

	//���@�ꗗ���쐬
	std::vector<std::wstring> listPlayerPath = info_->GetPlayerList();
	if(listPlayerPath.size() == 0)
	{
		listPlayer_ = systemInfo->GetFreePlayerScriptInformationList();
	}
	else
	{
		listPlayer_ = info_->CreatePlayerScriptInformationList();
	}

	//���j���[�쐬
	for(int iMenu = 0 ; iMenu < listPlayer_.size(); iMenu++)
	{
		AddMenuItem(new PlayerSelectMenuItem(listPlayer_[iMenu]));
	}

	std::vector<ref_count_ptr<ScriptInformation> > listLastPlayerSelect = systemInfo->GetLastPlayerSelectedList();
	bool bSameList = false;
	if(listPlayer_.size() == listLastPlayerSelect.size()) 
	{
		bSameList = true;
		for(int iList = 0 ; iList < listPlayer_.size() ; iList++)
		{
			bSameList &= listPlayer_[iList] == listLastPlayerSelect[iList];
		}
	}
	if(bSameList)
	{
		int lastIndex = systemInfo->GetLastSelectedPlayerIndex();
		cursorY_ = lastIndex % (pageMaxY_ + 1);
		pageCurrent_ = lastIndex / (pageMaxY_ + 1) + 1;
	}

}
void PlayerSelectScene::Work()
{
	int lastCursorY = cursorY_;

	MenuTask::Work();
	if(!_IsWaitedKeyFree())return;

	EDirectInput* input = EDirectInput::GetInstance();
	if(input->GetVirtualKeyState(EDirectInput::KEY_OK) == KEY_PUSH)
	{
		int index = GetSelectedItemIndex();
		ref_count_ptr<ScriptInformation> infoPlayer = listPlayer_[index];
		SystemController::GetInstance()->GetSystemInformation()->SetLastSelectedPlayerIndex(index, listPlayer_);

		SceneManager* sceneManager = SystemController::GetInstance()->GetSceneManager();
		sceneManager->TransStgScene(info_, infoPlayer, NULL);

		return;
	}
	else if(input->GetVirtualKeyState(EDirectInput::KEY_CANCEL) == KEY_PUSH)
	{
		ETaskManager* taskManager = ETaskManager::GetInstance();
		taskManager->SetRenderFunctionEnable(true, typeid(ScriptSelectScene));
		taskManager->SetWorkFunctionEnable(true, typeid(PlayTypeSelectScene));
		taskManager->SetRenderFunctionEnable(true, typeid(PlayTypeSelectScene));

		taskManager->RemoveTask(typeid(PlayerSelectScene));

		return;
	}

	if(lastCursorY != cursorY_)frameSelect_ = 0;
	else frameSelect_++;
}
void PlayerSelectScene::Render()
{
	EDirectGraphics* graphics = EDirectGraphics::GetInstance();
	graphics->SetRenderStateFor2D(DirectGraphics::MODE_BLEND_ALPHA);

	spriteBack_->Render();

	int top = (pageCurrent_ - 1) * (pageMaxY_ + 1);
	ref_count_ptr<ScriptInformation> infoSelected = NULL;
	{
		Lock lock(cs_);
		for(int iItem = 0 ; iItem <= pageMaxY_ ; iItem++)
		{
			int index = top + iItem;
			if(index < item_.size() && item_[index] != NULL)
			{
				PlayerSelectMenuItem* pItem = (PlayerSelectMenuItem*)item_[index].GetPointer();
				ref_count_ptr<ScriptInformation> info = pItem->GetScriptInformation();
				if(GetSelectedItemIndex() == index)
					infoSelected = info;
			}
		}
	}

	if(infoSelected != NULL)
	{
		DxText dxTextInfo;
		dxTextInfo.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
		dxTextInfo.SetFontBorderType(directx::DxFont::BORDER_NONE);
		dxTextInfo.SetFontBold(true);

		//�C���[�W
		ref_count_ptr<Texture> texture = spriteImage_->GetTexture();
		std::wstring pathImage1 = L"";
		if(texture != NULL)pathImage1 = texture->GetName();
		std::wstring pathImage2 = infoSelected->GetImagePath();
		if(pathImage1 != pathImage2)
		{
			texture = new Texture();
			File file(pathImage2);
			if(file.IsExists())
			{
				texture->CreateFromFileInLoadThread(pathImage2);
				spriteImage_->SetTexture(texture);
			}
			else
				spriteImage_->SetTexture(NULL);
				
		}

		texture = spriteImage_->GetTexture();
		if(texture != NULL && texture->IsLoad())
		{
			RECT_D rcSrc = {0., 0., (double)texture->GetWidth(), (double)texture->GetHeight()};
			RECT_D rcDest = {0., 0., (double)texture->GetWidth(), (double)texture->GetHeight()};
			spriteImage_->SetSourceRect(rcSrc);
			spriteImage_->SetDestinationRect(rcDest);
			spriteImage_->Render();
		}

		//�X�N���v�g�p�X
		std::wstring path = infoSelected->GetScriptPath();
		std::wstring root = EPathProperty::GetStgScriptRootDirectory();
		root = PathProperty::ReplaceYenToSlash(root);
		path = PathProperty::ReplaceYenToSlash(path);
		path = StringUtility::ReplaceAll(path, root, L"");
		
		std::wstring archive = infoSelected->GetArchivePath();
		if(archive.size() > 0)
		{
			archive = PathProperty::ReplaceYenToSlash(archive);
			archive = StringUtility::ReplaceAll(archive, root, L"");
			path += StringUtility::Format(L" [%s]", archive.c_str());
		}

		dxTextInfo.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
		dxTextInfo.SetFontColorBottom(D3DCOLOR_ARGB(255,255,128,128));
		dxTextInfo.SetFontSize(14);
		dxTextInfo.SetText(path);
		dxTextInfo.SetPosition(40,32);
		dxTextInfo.Render();

		//�e�L�X�g
		dxTextInfo.SetFontBorderType(directx::DxFont::BORDER_SHADOW);
		dxTextInfo.SetFontBold(false);
		dxTextInfo.SetFontBorderWidth(2);
		dxTextInfo.SetFontSize(16);
		dxTextInfo.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
		dxTextInfo.SetFontColorTop(D3DCOLOR_ARGB(255,160,160,160));
		dxTextInfo.SetFontColorBottom(D3DCOLOR_ARGB(255,255,64,64));
		dxTextInfo.SetText(infoSelected->GetText());
		dxTextInfo.SetPosition(320, 240);
		dxTextInfo.Render();

	}

	std::wstring strDescription = StringUtility::Format(
		L"�U�����@��I�����Ă������� (%d/%d)", pageCurrent_, GetPageCount() );

	DxText dxTextDescription;
	dxTextDescription.SetFontColorTop(D3DCOLOR_ARGB(255,128,128,255));
	dxTextDescription.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
	dxTextDescription.SetFontBorderType(directx::DxFont::BORDER_FULL);
	dxTextDescription.SetFontBorderColor(D3DCOLOR_ARGB(255,255,255,255));
	dxTextDescription.SetFontBorderWidth(2);
	dxTextDescription.SetFontSize(20);
	dxTextDescription.SetFontBold(true);
	dxTextDescription.SetText(strDescription);
	dxTextDescription.SetPosition(32,8);
	dxTextDescription.Render();

	{
		Lock lock(cs_);
		for(int iItem = 0 ; iItem <= pageMaxY_ ; iItem++)
		{
			int index = top + iItem;
			if(index < item_.size() && item_[index] != NULL)
			{
				int mx = 320;
				int my = 48 + (iItem % (pageMaxY_ + 1)) * 18;

				PlayerSelectMenuItem* pItem = (PlayerSelectMenuItem*)item_[index].GetPointer();
				ref_count_ptr<ScriptInformation> info = pItem->GetScriptInformation();

				DxText dxText;
				dxText.SetPosition(mx, my);
				dxText.SetFontBorderType(directx::DxFont::BORDER_FULL);
				dxText.SetFontBorderWidth(2);
				dxText.SetFontSize(16);
				dxText.SetFontBold(true);
				dxText.SetFontColorTop(D3DCOLOR_ARGB(255,255,255,255));
				dxText.SetFontColorBottom(D3DCOLOR_ARGB(255,64,64,255));
				dxText.SetFontBorderColor(D3DCOLOR_ARGB(255,32,32,128));
				dxText.SetText(info->GetTitle());
				dxText.Render();

				if(GetSelectedItemIndex() == index)
				{
					graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ADD_RGB);
					int cycle = 60;
					int alpha = frameSelect_ % cycle;
					if(alpha < cycle / 2)alpha = 255 * (float)((float)(cycle / 2 - alpha) / (float)(cycle / 2));
					else alpha = 255 * (float)((float)(alpha - cycle / 2) / (float)(cycle / 2));
					dxText.SetVertexColor(D3DCOLOR_ARGB(255, alpha, alpha, alpha ));
					dxText.Render();
					graphics->SetBlendMode(DirectGraphics::MODE_BLEND_ALPHA);
				}
			}
		}

	}
}

//PlayerSelectMenuItem
PlayerSelectMenuItem::PlayerSelectMenuItem(ref_count_ptr<ScriptInformation> info)
{
	info_ = info;
}
PlayerSelectMenuItem::~PlayerSelectMenuItem()
{
}
void PlayerSelectMenuItem::Work()
{
	_WorkSelectedItem();
}
void PlayerSelectMenuItem::Render()
{

}
