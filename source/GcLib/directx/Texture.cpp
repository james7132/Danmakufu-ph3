#include"Texture.hpp"
#include"DirectGraphics.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//TextureData
**********************************************************/
TextureData::TextureData()
{
	manager_ = NULL;
	pTexture_ = NULL;
	lpRenderSurface_ = NULL;
	lpRenderZ_ = NULL;
	bLoad_ = true;

	ZeroMemory(&infoImage_, sizeof(D3DXIMAGE_INFO));
	type_ = TYPE_TEXTURE;
}
TextureData::~TextureData()
{
	if(pTexture_ != NULL)pTexture_->Release();
	if(lpRenderSurface_ != NULL)lpRenderSurface_->Release();
	if(lpRenderZ_ != NULL) lpRenderZ_->Release();
}

/**********************************************************
//Texture
**********************************************************/
Texture::Texture()
{
}
Texture::Texture(Texture* texture)
{
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		data_ = texture->data_;
	}
}
Texture::~Texture()
{
	Release();
}
void Texture::Release()
{
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)
		{
			TextureManager* manager = data_->manager_;
			if(manager != NULL && manager->IsDataExists(data_->name_))
			{
				int countRef = data_.GetReferenceCount();
				//自身とTextureManager内の数だけになったら削除
				if(countRef == 2)
				{
					manager->_ReleaseTextureData(data_->name_);
				}
			}
			data_ = NULL;
		}
	}
}
std::wstring Texture::GetName()
{
	std::wstring res = L"";
	{
		Lock lock(TextureManager::GetBase()->GetLock());	
		if(data_!=NULL)res=data_->GetName();
	}
	return res;
}
bool Texture::CreateFromFile(std::wstring path)
{
	path = PathProperty::GetUnique(path);

	bool res = false;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		TextureManager* manager = TextureManager::GetBase();
		ref_count_ptr<Texture> texture = manager->CreateFromFile(path);
		if(texture != NULL)
		{
			data_ = texture->data_;
		}
		res = data_ != NULL;
	}

	return res;
}

bool Texture::CreateRenderTarget(std::wstring name)
{
	bool res = false;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		TextureManager* manager = TextureManager::GetBase();
		ref_count_ptr<Texture> texture = manager->CreateRenderTarget(name);
		if(texture != NULL)
		{
			data_ = texture->data_;
		}
		res = data_ != NULL;
	}
	return res;
}
bool Texture::CreateFromFileInLoadThread(std::wstring path, bool bLoadImageInfo)
{
	path = PathProperty::GetUnique(path);

	bool res = false;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		TextureManager* manager = TextureManager::GetBase();
		ref_count_ptr<Texture> texture = manager->CreateFromFileInLoadThread(path, bLoadImageInfo);
		if(texture != NULL)
		{
			data_ = texture->data_;
		}
		res = data_ != NULL;
	}

	return res;
}
void Texture::SetTexture(IDirect3DTexture9 *pTexture)
{
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		TextureData* textureData = new TextureData(); 
		textureData->pTexture_ = pTexture;
		D3DSURFACE_DESC desc;
		pTexture->GetLevelDesc(0, &desc);

		D3DXIMAGE_INFO* infoImage = &textureData->infoImage_;
		infoImage->Width = desc.Width;
		infoImage->Height = desc.Height;
		infoImage->Format = desc.Format;
		infoImage->ImageFileFormat = D3DXIFF_BMP;
		infoImage->ResourceType = D3DRTYPE_TEXTURE;
		data_ = textureData;
	}
}

IDirect3DTexture9* Texture::GetD3DTexture()
{
	IDirect3DTexture9* res = NULL;
	{
		bool bWait = true;
		int time = timeGetTime();
		while(bWait)
		{
			Lock lock(TextureManager::GetBase()->GetLock());
			if(data_ != NULL)
			{
				bWait = !data_->bLoad_;
				if(!bWait)
					res = _GetTextureData()->pTexture_;

				if(bWait && abs((int)(timeGetTime() - time)) > 10000)
				{
					//一定時間たってもだめだったらロック？
					std::wstring path = data_->GetName();
					Logger::WriteTop(
						StringUtility::Format(L"テクスチャ読み込みを行えていません。ロック？ ：%s", path.c_str()));
					data_->bLoad_ = true;
					break;
				}
			}
			else break;

			if(bWait)::Sleep(1);
		}

	}
	return res;
}
IDirect3DSurface9* Texture::GetD3DSurface()
{
	IDirect3DSurface9* res = NULL;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)
			res = _GetTextureData()->lpRenderSurface_;
	}
	return res;
}
IDirect3DSurface9* Texture::GetD3DZBuffer()
{
	IDirect3DSurface9* res = NULL;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		if(data_ != NULL)
			res = _GetTextureData()->lpRenderZ_;
	}
	return res;
}
int Texture::GetWidth()
{
	int res = 0;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		TextureData* data = _GetTextureData();
		if(data != NULL)res = data->infoImage_.Width;
	}
	return res;
}
int Texture::GetHeight()
{
	int res = 0;
	{
		Lock lock(TextureManager::GetBase()->GetLock());
		TextureData* data = _GetTextureData();
		if(data != NULL)res = data->infoImage_.Height;
	}
	return res;
}
/**********************************************************
//TextureManager
**********************************************************/
const std::wstring TextureManager::TARGET_TRANSITION = L"__RENDERTARGET_TRANSITION__";
TextureManager* TextureManager::thisBase_ = NULL;
TextureManager::TextureManager()
{
	
}
TextureManager::~TextureManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);
	this->Clear();

	FileManager::GetBase()->RemoveLoadThreadListener(this);

	panelInfo_ = NULL;
	thisBase_=NULL;
}
bool TextureManager::Initialize()
{
	if(thisBase_ != NULL)return false;

	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);
	
	ref_count_ptr<Texture> texTransition = new Texture();
	bool res = texTransition->CreateRenderTarget(TARGET_TRANSITION);
	Add(TARGET_TRANSITION, texTransition);

	FileManager::GetBase()->AddLoadThreadListener(this);
	
	return res;
}
void TextureManager::Clear()
{
	{
		Lock lock(lock_);
		mapTexture_.clear();
		mapTextureData_.clear();
	}
}
void TextureManager::_ReleaseTextureData(std::wstring name)
{
	{
		Lock lock(lock_);
		if(IsDataExists(name))
		{
			mapTextureData_[name]->bLoad_ = true;//読み込み完了扱い
			mapTextureData_.erase(name);
			Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを解放しました[%s]", name.c_str()));
		}
	}
}
void TextureManager::ReleaseDxResource()
{
	std::map<std::wstring, gstd::ref_count_ptr<TextureData> >::iterator itrMap;
	{
		Lock lock(GetLock());
		for(itrMap = mapTextureData_.begin(); itrMap != mapTextureData_.end(); itrMap++)
		{
			std::wstring name = itrMap->first;
			TextureData* data = (itrMap->second).GetPointer();
			if(data->type_ == TextureData::TYPE_RENDER_TARGET)
			{
				if(data->pTexture_ != NULL)data->pTexture_->Release();
				if(data->lpRenderSurface_ != NULL)data->lpRenderSurface_->Release();
				if(data->lpRenderZ_ != NULL)data->lpRenderZ_->Release();		
			}
		}
	}
}
void TextureManager::RestoreDxResource()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	std::map<std::wstring, gstd::ref_count_ptr<TextureData> >::iterator itrMap;
	{
		Lock lock(GetLock());
		for(itrMap = mapTextureData_.begin(); itrMap != mapTextureData_.end(); itrMap++)
		{
			std::wstring name = itrMap->first;
			TextureData* data = (itrMap->second).GetPointer();
			if(data->type_ == TextureData::TYPE_RENDER_TARGET)
			{
				int width = data->infoImage_.Width;
				int height = data->infoImage_.Height;

				HRESULT hr;
				// Zバッファ生成
				hr = graphics->GetDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &data->lpRenderZ_, NULL);
				//テクスチャ作成
				D3DFORMAT fmt;
				if(graphics->GetScreenMode() == DirectGraphics::SCREENMODE_FULLSCREEN)
					fmt = graphics->GetFullScreenPresentParameter().BackBufferFormat;
				else fmt = graphics->GetWindowPresentParameter().BackBufferFormat;

				hr = graphics->GetDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &data->pTexture_, NULL);
				data->pTexture_->GetSurfaceLevel(0, &data->lpRenderSurface_);	
			}
		}
	}
}

bool TextureManager::_CreateFromFile(std::wstring path)
{
	if(IsDataExists(path))
	{
		return true;
	}

	//まだ作成されていないなら、作成
	try
	{
		ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
		if(reader == NULL)throw gstd::wexception(L"ファイルが見つかりません");
		if(!reader->Open())throw gstd::wexception(L"ファイルが開けません");

		int size = reader->GetFileSize();
		ByteBuffer buf;
		buf.SetSize(size);
		reader->Read(buf.GetPointer(), size);

//		D3DXIMAGE_INFO info;
//		D3DXGetImageInfoFromFileInMemory(buf.GetPointer(), size, &info);

		D3DCOLOR colorKey = D3DCOLOR_ARGB(255,0,0,0);
		if(path.find(L".bmp") == std::wstring::npos)//bmpのみカラーキー適応
			colorKey = 0;
		D3DFORMAT pixelFormat=D3DFMT_A8R8G8B8;

		ref_count_ptr<TextureData> data(new TextureData());

		HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(DirectGraphics::GetBase()->GetDevice(),
										buf.GetPointer(), size,
										D3DX_DEFAULT,D3DX_DEFAULT,
										//info.Width, info.Height,
										0,
										0,
										pixelFormat,
										D3DPOOL_MANAGED,
										D3DX_FILTER_BOX,
										D3DX_DEFAULT,
										colorKey,
										NULL,
										NULL,
										&data->pTexture_);
		if(FAILED(hr))
		{
			throw gstd::wexception(L"D3DXCreateTextureFromFileInMemoryEx失敗");
		}

		mapTextureData_[path] = data;
		data->manager_ = this;
		data->name_ = path;
		D3DXGetImageInfoFromFileInMemory(buf.GetPointer(), size, &data->infoImage_);

		Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを読み込みました[%s]", path.c_str()));
	}
	catch(gstd::wexception& e)
	{
		std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗[%s]\n\t%s", path.c_str(), e.what());
		Logger::WriteTop(str);
		return false;
	}

	return true;
}
bool TextureManager::_CreateRenderTarget(std::wstring name)
{
	if(IsDataExists(name))
	{
		return true;
	}

	bool res = true;
	try
	{
		ref_count_ptr<TextureData> data = new TextureData();
		DirectGraphics* graphics = DirectGraphics::GetBase();
		int screenWidth = graphics->GetScreenWidth();
		int screenHeight = graphics->GetScreenHeight();
		int width = 2;
		int height = 2;
		while(width <= screenWidth)
		{
			width *= 2;
		}
		while(height <= screenHeight)
		{
			height *= 2;
		}

		HRESULT hr;
		hr = graphics->GetDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &data->lpRenderZ_, NULL);
		if(FAILED(hr))throw false;
		D3DFORMAT fmt=D3DFMT_A8R8G8B8;
		hr = graphics->GetDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &data->pTexture_, NULL);
		if(FAILED(hr))
		{
			//テクスチャを正方形にする
			if(width > height)height = width;
			else if(height>width)width=height;
			hr = graphics->GetDevice()->CreateDepthStencilSurface(width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, FALSE, &data->lpRenderZ_, NULL);
			if(FAILED(hr))throw false;
			D3DFORMAT fmt=D3DFMT_A8R8G8B8;
			hr = graphics->GetDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, fmt, D3DPOOL_DEFAULT, &data->pTexture_, NULL);
			if(FAILED(hr))throw false;	
		}
		data->pTexture_->GetSurfaceLevel(0, &data->lpRenderSurface_);

		mapTextureData_[name] = data;
		data->manager_ = this;
		data->name_ = name;
		data->type_ = TextureData::TYPE_RENDER_TARGET;
		data->infoImage_.Width = width;
		data->infoImage_.Height = height;

		Logger::WriteTop(StringUtility::Format(L"TextureManager：レンダリングターゲット作成[%s]", name.c_str()));

	}
	catch(...)
	{
		Logger::WriteTop(StringUtility::Format(L"TextureManager：レンダリングターゲット作成失敗[%s]", name.c_str()));
		res = false;
	}

	return res;
}
gstd::ref_count_ptr<Texture> TextureManager::CreateFromFile(std::wstring path)
{
	path = PathProperty::GetUnique(path);
	gstd::ref_count_ptr<Texture> res;
	{
		Lock lock(lock_);
		bool bExist = mapTexture_.find(path) != mapTexture_.end();
		if(bExist)
		{
			res = mapTexture_[path];
		}
		else
		{
			bool bSuccess = _CreateFromFile(path);
			if(bSuccess)
			{
				res = new Texture();
				res->data_ = mapTextureData_[path];
			}
		}
	}
	return res;
}

gstd::ref_count_ptr<Texture> TextureManager::CreateRenderTarget(std::wstring name)
{
	gstd::ref_count_ptr<Texture> res;
	{
		Lock lock(lock_);
		bool bExist = mapTexture_.find(name) != mapTexture_.end();
		if(bExist)
		{
			res = mapTexture_[name];
		}
		else
		{
			bool bSuccess = _CreateRenderTarget(name);
			if(bSuccess)
			{
				res = new Texture();
				res->data_ = mapTextureData_[name];
			}
		}
	}
	return res;
}
gstd::ref_count_ptr<Texture> TextureManager::CreateFromFileInLoadThread(std::wstring path, bool bLoadImageInfo)
{
	path = PathProperty::GetUnique(path);
	gstd::ref_count_ptr<Texture> res;
	{
		Lock lock(lock_);
		bool bExist = mapTexture_.find(path) != mapTexture_.end();

		if(bExist)
		{
			res = mapTexture_[path];
		}
		else
		{
			bool bLoadTarget = true;
			res = new Texture();
			if(!IsDataExists(path))
			{
				ref_count_ptr<TextureData> data(new TextureData());
				mapTextureData_[path] = data;
				data->manager_ = this;
				data->name_ = path;
				data->bLoad_ = false;

				//画像情報だけ事前に読み込み
				if(bLoadImageInfo)
				{
					try
					{
						ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
						if(reader == NULL)throw gstd::wexception(L"ファイルが見つかりません");
						if(!reader->Open())throw gstd::wexception(L"ファイルが開けません");

						int size = reader->GetFileSize();
						ByteBuffer buf;
						buf.SetSize(size);
						reader->Read(buf.GetPointer(), size);

						D3DXIMAGE_INFO info;
						HRESULT hr = D3DXGetImageInfoFromFileInMemory(buf.GetPointer(), size, &info);
						if(FAILED(hr))
						{
							throw gstd::wexception(L"D3DXGetImageInfoFromFileInMemory失敗");
						}

						data->infoImage_ = info;
					}
					catch(gstd::wexception& e)
					{
						std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗[%s]\n\t%s", path.c_str(), e.what());
						Logger::WriteTop(str);
						data->bLoad_ = true;//読み込み完了扱い
						bLoadTarget = false;
					}
				}
			}
			else bLoadTarget = false;

			res->data_ = mapTextureData_[path];
			if(bLoadTarget)
			{
				ref_count_ptr<FileManager::LoadObject> source = res;
				ref_count_ptr<FileManager::LoadThreadEvent> event = new FileManager::LoadThreadEvent(this, path, res);
				FileManager::GetBase()->AddLoadThreadEvent(event);

			}
		}
	}
	return res;
}
void TextureManager::CallFromLoadThread(ref_count_ptr<FileManager::LoadThreadEvent> event)
{
	std::wstring path = event->GetPath();
	{
		Lock lock(lock_);
		ref_count_ptr<Texture> texture = ref_count_ptr<Texture>::DownCast(event->GetSource());
		if(texture == NULL)return;

		ref_count_ptr<TextureData> data = texture->data_;
		if(data == NULL || data->bLoad_)return;

		int countRef = data.GetReferenceCount();
		//自身とTextureManager内の数だけになったら読み込まない。
		if(countRef <= 2)
		{
			data->bLoad_ = true;//念のため読み込み完了扱い
			return;
		}

		try
		{
			ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
			if(reader == NULL)throw gstd::wexception(L"ファイルが見つかりません");
			if(!reader->Open())throw gstd::wexception(L"ファイルが開けません");

			int size = reader->GetFileSize();
			ByteBuffer buf;
			buf.SetSize(size);
			reader->Read(buf.GetPointer(), size);

			D3DCOLOR colorKey = D3DCOLOR_ARGB(255,0,0,0);
			if(path.find(L".bmp") == std::wstring::npos)//bmpのみカラーキー適応
				colorKey = 0;

			D3DFORMAT pixelFormat=D3DFMT_A8R8G8B8;

			HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(DirectGraphics::GetBase()->GetDevice(),
											buf.GetPointer(), size,
											D3DX_DEFAULT,D3DX_DEFAULT,
											0,
											0,
											pixelFormat,
											D3DPOOL_MANAGED,
											D3DX_FILTER_BOX,
											D3DX_DEFAULT,
											colorKey,
											NULL,
											NULL,
											&data->pTexture_);
			if(FAILED(hr))
			{
				throw gstd::wexception(L"D3DXCreateTextureFromFileInMemoryEx失敗");
			}

			D3DXGetImageInfoFromFileInMemory(buf.GetPointer(), size, &data->infoImage_);

			Logger::WriteTop(StringUtility::Format(L"TextureManager：テクスチャを読み込みました(LT)[%s]", path.c_str()));
		}
		catch(gstd::wexception& e)
		{
			std::wstring str = StringUtility::Format(L"TextureManager：テクスチャ読み込み失敗(LT)[%s]\n\t%s", path.c_str(), e.what());
			Logger::WriteTop(str);
		}
		data->bLoad_ = true;
	}
}

gstd::ref_count_ptr<TextureData> TextureManager::GetTextureData(std::wstring name)
{
	gstd::ref_count_ptr<TextureData> res;
	{
		Lock lock(lock_);
		bool bExist = mapTextureData_.find(name) != mapTextureData_.end();
		if(bExist)
		{
			res = mapTextureData_[name];
		}
	}
	return res;
}

gstd::ref_count_ptr<Texture> TextureManager::GetTexture(std::wstring name)
{
	gstd::ref_count_ptr<Texture> res;
	{
		Lock lock(lock_);
		bool bExist = mapTexture_.find(name) != mapTexture_.end();
		if(bExist)
		{
			res = mapTexture_[name];
		}
	}
	return res;
}

void TextureManager::Add(std::wstring name, gstd::ref_count_ptr<Texture> texture)
{
	{
		Lock lock(lock_);
		bool bExist = mapTexture_.find(name) != mapTexture_.end();
		if(!bExist)
		{
			mapTexture_[name] = texture;
		}
	}
}
void TextureManager::Release(std::wstring name)
{
	{
		Lock lock(lock_);
		mapTexture_.erase(name);
	}
}
bool TextureManager::IsDataExists(std::wstring name)
{
	bool res = false;
	{
		Lock lock(lock_);
		res = mapTextureData_.find(name) != mapTextureData_.end();
	}
	return res;
}

/**********************************************************
//TextureInfoPanel
**********************************************************/
TextureInfoPanel::TextureInfoPanel()
{
	timeUpdateInterval_ = 500;
}
TextureInfoPanel::~TextureInfoPanel()
{
	Stop();
	Join(1000);
}
bool TextureInfoPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);

	gstd::WListView::Style styleListView;
	styleListView.SetStyle(WS_CHILD | WS_VISIBLE | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL| LVS_NOSORTHEADER);
	styleListView.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListView.SetListViewStyleEx(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	wndListView_.Create(hWnd_, styleListView);

	wndListView_.AddColumn(64, ROW_ADDRESS, L"Address");
	wndListView_.AddColumn(96, ROW_NAME, L"Name");
	wndListView_.AddColumn(48, ROW_FULLNAME, L"FullName");
	wndListView_.AddColumn(32, ROW_COUNT_REFFRENCE, L"Ref");
	wndListView_.AddColumn(48, ROW_WIDTH_IMAGE, L"Width");
	wndListView_.AddColumn(48, ROW_HEIGHT_IMAGE, L"Height");

	Start();

	return true;
}
void TextureInfoPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	wndListView_.SetBounds(wx, wy, wWidth, wHeight);
}
void TextureInfoPanel::_Run()
{
	while(GetStatus() == RUN)
	{
		TextureManager* manager = TextureManager::GetBase();
		if(manager != NULL)
			Update(manager);
		Sleep(timeUpdateInterval_);
	}
}
void TextureInfoPanel::Update(TextureManager* manager)
{
	if(!IsWindowVisible())return;
	std::set<std::wstring> setKey;
	std::map<std::wstring, gstd::ref_count_ptr<TextureData> >::iterator itrMap;
	{
		Lock lock(manager->GetLock());

		std::map<std::wstring, gstd::ref_count_ptr<TextureData> >& mapData = manager->mapTextureData_;
		for(itrMap=mapData.begin(); itrMap!=mapData.end(); itrMap++)
		{
			std::wstring name = itrMap->first;
			TextureData* data = (itrMap->second).GetPointer();

			int address = (int)data;
			std::wstring key = StringUtility::Format(L"%08x", address);
			int index = wndListView_.GetIndexInColumn(key, ROW_ADDRESS);
			if(index == -1)
			{
				index = wndListView_.GetRowCount();
				wndListView_.SetText(index, ROW_ADDRESS, key);
			}

			int countRef = (itrMap->second).GetReferenceCount();
			D3DXIMAGE_INFO* infoImage = &data->infoImage_;

			wndListView_.SetText(index, ROW_NAME, PathProperty::GetFileName(name));
			wndListView_.SetText(index, ROW_FULLNAME, name);
			wndListView_.SetText(index, ROW_COUNT_REFFRENCE, StringUtility::Format(L"%d", countRef));
			wndListView_.SetText(index, ROW_WIDTH_IMAGE, StringUtility::Format(L"%d", infoImage->Width));
			wndListView_.SetText(index, ROW_HEIGHT_IMAGE, StringUtility::Format(L"%d", infoImage->Height));

			setKey.insert(key);
		}
	}

	for(int iRow=0; iRow<wndListView_.GetRowCount();)
	{
		std::wstring key = wndListView_.GetText(iRow, ROW_ADDRESS);
		if(setKey.find(key) != setKey.end())iRow++;
		else wndListView_.DeleteRow(iRow);
	}
}
