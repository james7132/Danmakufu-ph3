#include"Shader.hpp"
#include"HLSL.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//ShaderData
**********************************************************/
ShaderData::ShaderData()
{
	manager_ = NULL;
	bLoad_ = false;
	effect_ = NULL;
	bText_ = false;
}
ShaderData::~ShaderData()
{
}

/**********************************************************
//ShaderManager
**********************************************************/
const std::wstring NAME_DEFAULT_SKINNED_MESH = L"__NAME_DEFAULT_SKINNED_MESH__";
ShaderManager* ShaderManager::thisBase_ = NULL;
ShaderManager::ShaderManager()
{
}
ShaderManager::~ShaderManager()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->RemoveDirectGraphicsListener(this);

	Clear();
}
bool ShaderManager::Initialize()
{
	if(thisBase_ != NULL)return false;

	bool res = true;
	thisBase_ = this;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->AddDirectGraphicsListener(this);

	ref_count_ptr<Shader> shaderSkinedMesh = new Shader();
	std::string sourceSkinedMesh = HLSL_DEFAULT_SKINED_MESH;
	shaderSkinedMesh->CreateFromText(sourceSkinedMesh);
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderSkinedMesh);

//	shaderSkinedMesh->Begin();
//	shaderSkinedMesh->End();

/*
	ref_count_ptr<Shader> shaderTest = new Shader();
	std::string pathDir = PathProperty::GetModuleDirectory() + "shader\\";
	shaderTest->CreateFromFile(pathDir + "Sharder_SkinnedMesh.txt");
	AddShader(NAME_DEFAULT_SKINNED_MESH, shaderTest);
*/
/*
	File file(pathDir + "test.txt");
	if(file.Open())
	{
		std::string str;
		int size = file.GetSize();
		str.resize(size);
		file.Read(&str[0], size);

		ref_count_ptr<Shader> shaderTestFile = new Shader();
		shaderTestFile->CreateFromText(str);
	}
*/
	return res;
}
void ShaderManager::Clear()
{
	{
		Lock lock(lock_);
		mapShader_.clear();
		mapShaderData_.clear();
	}
}
void ShaderManager::_ReleaseShaderData(std::wstring name)
{
	{
		Lock lock(lock_);
		if(IsDataExists(name))
		{
			mapShaderData_[name]->bLoad_ = true;//読み込み完了扱い
			mapShaderData_.erase(name);
			Logger::WriteTop(StringUtility::Format(L"ShaderManager：Shaderを解放しました(Shader Released)[%s]", name.c_str()));
		}
	}
}
bool ShaderManager::_CreateFromFile(std::wstring path)
{
	lastError_ = L"";
	if(IsDataExists(path))
	{
		return true;
	}

	path = PathProperty::GetUnique(path);
	ref_count_ptr<FileReader> reader = FileManager::GetBase()->GetFileReader(path);
	if(reader == NULL || !reader->Open())
	{
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s", path.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
		return false;
	}

	int size = reader->GetFileSize();
	ByteBuffer buf;
	buf.SetSize(size);
	reader->Read(buf.GetPointer(), size);
	
	std::string source;
	source.resize(size);
	memcpy(&source[0], buf.GetPointer(), size);

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ID3DXBuffer* pErr = NULL;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr
	);

	bool res = true;
	if(FAILED(hr))
	{
		res = false;
		std::wstring err = L"";
		if(pErr != NULL)
		{
			char* cText = (char*)pErr->GetBufferPointer();
			err = StringUtility::ConvertMultiToWide(cText);
		}
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Shader Load Failed)：\r\n%s\r\n[%s]", path.c_str(), err.c_str());
		Logger::WriteTop(log);
		lastError_ = log;
	}
	else
	{
		std::wstring log = StringUtility::Format(L"Shader読み込み(Shader Load Success)：\r\n%s", path.c_str());
		Logger::WriteTop(log);

		mapShaderData_[path] = data;
		data->manager_ = this;
		data->name_ = path;
	}
	return res;
}
bool ShaderManager::_CreateFromText(std::string& source)
{
	lastError_ = L"";
	std::wstring id = _GetTextSourceID(source);
	if(IsDataExists(id))
	{
		return true;
	}

	bool res = true;
	DirectGraphics* graphics = DirectGraphics::GetBase();

	gstd::ref_count_ptr<ShaderData> data(new ShaderData());
	ID3DXBuffer* pErr = NULL;
	HRESULT hr = D3DXCreateEffect(
		graphics->GetDevice(),
		source.c_str(),
		source.size(),
		NULL, NULL,
		0,
		NULL,
		&data->effect_,
		&pErr
	);

	std::string tStr = StringUtility::Slice(source, 128);
	if(FAILED(hr))
	{
		res = false;
		char* err = "";
		if(pErr != NULL)err = (char*)pErr->GetBufferPointer();
		std::wstring log = StringUtility::Format(L"Shader読み込み失敗(Load Shader Failed)：\r\n%s\r\n[%s]", tStr.c_str(), err);
		Logger::WriteTop(log);
		lastError_ = log;
	}
	else
	{
		std::wstring log = L"Shader読み込み(Load Shader Success)：";
		log += StringUtility::FormatToWide("%s", tStr.c_str());
		Logger::WriteTop(log);

		mapShaderData_[id] = data;
		data->manager_ = this;
		data->name_ = id;
		data->bText_ = true;
	}

	return res;
}
std::wstring ShaderManager::_GetTextSourceID(std::string& source)
{
	std::wstring res = StringUtility::ConvertMultiToWide(source);
	res = StringUtility::Slice(res, 64);
	return res;
}
void ShaderManager::_BeginShader(Shader* shader, int pass)
{
	Shader* lastShader = NULL;
	if(listExecuteShader_.size() > 0)
	{
		lastShader = *listExecuteShader_.rbegin();
	}

	if(shader != NULL && shader != lastShader)
	{
		if(lastShader != NULL)
		{
			lastShader->_EndPass();
			lastShader->_End();
		}

		shader->_Begin();
		shader->_BeginPass(pass);
	}
	else if(shader == NULL && lastShader != NULL)
	{
		lastShader->_EndPass();
		lastShader->_End();		
	}

	listExecuteShader_.push_back(shader);
}
void ShaderManager::_EndShader(Shader* shader)
{
	Shader* preShader = NULL;
	if(listExecuteShader_.size() > 0)
	{
		preShader = *listExecuteShader_.rbegin();
		listExecuteShader_.pop_back();
	}

	if(shader != preShader)
		throw gstd::wexception(L"EndShader異常");

	preShader = NULL;
	if(listExecuteShader_.size() > 0)
	{
		preShader = *listExecuteShader_.rbegin();
	}

	//同じShaderなら何もしない
	if(shader == preShader)return;
	shader->_EndPass();
	shader->_End();

	if(preShader != NULL)
	{
		shader->_Begin();
		shader->_BeginPass();
	}

}

void ShaderManager::ReleaseDxResource()
{
	std::map<std::wstring, gstd::ref_count_ptr<Shader> >::iterator itrMap;
	{
		for(itrMap = mapShader_.begin(); itrMap != mapShader_.end(); itrMap++)
		{
			std::wstring name = itrMap->first;
			Shader* data = (itrMap->second).GetPointer();
			data->ReleaseDxResource();
		}
	}
}
void ShaderManager::RestoreDxResource()
{
	std::map<std::wstring, gstd::ref_count_ptr<Shader> >::iterator itrMap;
	{
		for(itrMap = mapShader_.begin(); itrMap != mapShader_.end(); itrMap++)
		{
			std::wstring name = itrMap->first;
			Shader* data = (itrMap->second).GetPointer();
			data->RestoreDxResource();
		}
	}
}

bool ShaderManager::IsDataExists(std::wstring name)
{
	bool res = false;
	{
		Lock lock(lock_);
		res = mapShaderData_.find(name) != mapShaderData_.end();
	}
	return res;
}
gstd::ref_count_ptr<ShaderData> ShaderManager::GetShaderData(std::wstring name)
{
	gstd::ref_count_ptr<ShaderData> res;
	{
		Lock lock(lock_);
		bool bExist = mapShaderData_.find(name) != mapShaderData_.end();
		if(bExist)
		{
			res = mapShaderData_[name];
		}
	}
	return res;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromFile(std::wstring path)
{
	path = PathProperty::GetUnique(path);
	gstd::ref_count_ptr<Shader> res;
	{
		Lock lock(lock_);
		bool bExist = mapShader_.find(path) != mapShader_.end();
		if(bExist)
		{
			res = mapShader_[path];
		}
		else
		{
			bool bSuccess = _CreateFromFile(path);
			if(bSuccess)
			{
				res = new Shader();
				res->data_ = mapShaderData_[path];
			}
		}
	}
	return res;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromText(std::string source)
{
	gstd::ref_count_ptr<Shader> res;
	{
		Lock lock(lock_);
		std::wstring id = _GetTextSourceID(source);
		bool bExist = mapShader_.find(id) != mapShader_.end();
		if(bExist)
		{
			res = mapShader_[id];
		}
		else
		{
			bool bSuccess = _CreateFromText(source);
			if(bSuccess)
			{
				res = new Shader();
				res->data_ = mapShaderData_[id];
			}
		}
	}
	return res;
}
gstd::ref_count_ptr<Shader> ShaderManager::CreateFromFileInLoadThread(std::wstring path)
{
	return false;
}
void ShaderManager::CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event)
{
}

void ShaderManager::AddShader(std::wstring name, gstd::ref_count_ptr<Shader> shader)
{
	{
		Lock lock(lock_);
		mapShader_[name] = shader;
	}
}
void ShaderManager::DeleteShader(std::wstring name)
{
	{
		Lock lock(lock_);
		mapShader_.erase(name);
	}
}
gstd::ref_count_ptr<Shader> ShaderManager::GetShader(std::wstring name)
{
	gstd::ref_count_ptr<Shader> res;
	{
		Lock lock(lock_);
		bool bExist = mapShader_.find(name) != mapShader_.end();
		if(bExist)
		{
			res = mapShader_[name];
		}
	}
	return res;
}
gstd::ref_count_ptr<Shader> ShaderManager::GetDefaultSkinnedMeshShader()
{
	gstd::ref_count_ptr<Shader> res = GetShader(NAME_DEFAULT_SKINNED_MESH);
	return res;
}
void ShaderManager::CheckExecutingShaderZero()
{
	if(listExecuteShader_.size() > 0)
		throw gstd::wexception(L"CheckExecutingShaderZero");
}
std::wstring ShaderManager::GetLastError()
{
	std::wstring res;
	{
		Lock lock(lock_);
		res = lastError_;
	}
	return res;
}

/**********************************************************
//ShaderParameter
**********************************************************/
ShaderParameter::ShaderParameter()
{
	type_ = TYPE_UNKNOWN;
	value_ = new ByteBuffer();
}
ShaderParameter::~ShaderParameter()
{
}
void ShaderParameter::SetMatrix(D3DXMATRIX& matrix)
{
	type_ = TYPE_MATRIX;
	int size = sizeof(D3DXMATRIX);
	value_->Seek(0);
	value_->Write(&matrix, size);
}
D3DXMATRIX ShaderParameter::GetMatrix()
{
	D3DXMATRIX res = (D3DXMATRIX&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetMatrixArray(std::vector<D3DXMATRIX>& listMatrix)
{
	type_ = TYPE_MATRIX_ARRAY;
	value_->Seek(0);
	for(int iMatrix = 0 ; iMatrix < listMatrix.size() ; iMatrix++)
	{
		int size = sizeof(D3DMATRIX);
		D3DXMATRIX& matrix = listMatrix[iMatrix];
		value_->Write(&matrix.m, size);
	}

}
std::vector<D3DXMATRIX> ShaderParameter::GetMatrixArray()
{
	std::vector<D3DXMATRIX> res;
	int count = value_->GetSize() / sizeof(D3DMATRIX);
	res.resize(count);

	value_->Seek(0);
	for(int iMatrix = 0 ; iMatrix < res.size() ; iMatrix++)
	{
		value_->Read(&res[iMatrix].m, sizeof(D3DMATRIX));
	}

	return res;
}
void ShaderParameter::SetVector(D3DXVECTOR4& vector)
{
	type_ = TYPE_VECTOR;
	int size = sizeof(D3DXVECTOR4);
	value_->Seek(0);
	value_->Write(&vector, size);
}
D3DXVECTOR4 ShaderParameter::GetVector()
{
	D3DXVECTOR4 res = (D3DXVECTOR4&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetFloat(float value)
{
	type_ = TYPE_FLOAT;
	int size = sizeof(float);
	value_->Seek(0);
	value_->Write(&value, size);
}
float ShaderParameter::GetFloat()
{
	float res = (float&)*value_->GetPointer();
	return res;
}
void ShaderParameter::SetFloatArray(std::vector<float>& values)
{
	type_ = TYPE_FLOAT_ARRAY;
	int size = sizeof(float) * values.size();
	value_->Seek(0);
	value_->Write(&values[0], size);
}
std::vector<float> ShaderParameter::GetFloatArray()
{
	std::vector<float> res;
	int count = value_->GetSize() / sizeof(float);
	res.resize(count);

	value_->Seek(0);
	value_->Read(&res[0], value_->GetSize());
	return res;
}
void ShaderParameter::SetTexture(gstd::ref_count_ptr<Texture> texture)
{
	type_ = TYPE_TEXTURE;
	texture_ = texture;
}
gstd::ref_count_ptr<Texture> ShaderParameter::GetTexture()
{
	return texture_;
}


/**********************************************************
//Shader
**********************************************************/
Shader::Shader()
{
	data_ = NULL;
	//bLoadShader_ = false;
	//pVertexShader_ = NULL;
	//pPixelShader_ = NULL;
}
Shader::Shader(Shader* shader)
{
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		data_ = shader->data_;
	}
}
Shader::~Shader()
{
	Release();
}
void Shader::Release()
{
	//if(pVertexShader_ != NULL)
	//	pVertexShader_->Release();
	//if(pPixelShader_ != NULL)
	//	pPixelShader_->Release();
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		if(data_ != NULL)
		{
			ShaderManager* manager = data_->manager_;
			if(manager != NULL && manager->IsDataExists(data_->name_))
			{
				int countRef = data_.GetReferenceCount();
				//自身とTextureManager内の数だけになったら削除
				if(countRef == 2)
				{
					manager->_ReleaseShaderData(data_->name_);
				}
			}
			data_ = NULL;
		}
	}
}

int Shader::Begin(int pass)
{
	ShaderManager* manager = ShaderManager::GetBase();
	manager->_BeginShader(this, pass);
	return 1;
}
void Shader::End()
{
	ShaderManager* manager = ShaderManager::GetBase();
	manager->_EndShader(this);
}

ID3DXEffect* Shader::GetEffect()
{
	ID3DXEffect* res = NULL;
	if(data_ != NULL)res = data_->effect_;
	return res;
}
void Shader::ReleaseDxResource()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return;
	effect->OnLostDevice();
}
void Shader::RestoreDxResource()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return;
	effect->OnResetDevice();
}
bool Shader::CreateFromFile(std::wstring path)
{
	path = PathProperty::GetUnique(path);

	bool res = false;
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		ShaderManager* manager = ShaderManager::GetBase();
		ref_count_ptr<Shader> shader = manager->CreateFromFile(path);
		if(shader != NULL)
		{
			data_ = shader->data_;
		}
		res = data_ != NULL;
	}

	return res;
}
bool Shader::CreateFromText(std::string& source)
{
	bool res = false;
	{
		Lock lock(ShaderManager::GetBase()->GetLock());
		if(data_ != NULL)Release();
		ShaderManager* manager = ShaderManager::GetBase();
		ref_count_ptr<Shader> shader = manager->CreateFromText(source);
		if(shader != NULL)
		{
			data_ = shader->data_;
		}
		res = data_ != NULL;
	}

	return res;
}

int Shader::_Begin()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return 0;
	_SetupParameter();

	unsigned int res = S_OK;
	HRESULT hr = effect->Begin( &res, 0 );
	return res;
}
void Shader::_End()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return;
	effect->End();
}
void Shader::_BeginPass(int pass)
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return;
	effect->BeginPass(pass);
/*
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
	if(!bLoadShader_)
	{
		//http://www.gamedev.net/topic/646178-given-an-effect-technique-pass-handle-how-to-get-the-pixelshader/
		D3DXHANDLE hTechnique = effect->GetCurrentTechnique();
		D3DXHANDLE hPass = effect->GetPass(hTechnique, pass);

		D3DXPASS_DESC passDesc;
		effect->GetPassDesc(hPass, &passDesc);
		if(passDesc.pVertexShaderFunction != NULL)
			device->CreateVertexShader(passDesc.pVertexShaderFunction, &pVertexShader_);
		if(passDesc.pPixelShaderFunction != NULL)
			device->CreatePixelShader(passDesc.pPixelShaderFunction, &pPixelShader_);
		bLoadShader_ = true;
	}

//	device->SetVertexShader(pVertexShader_);
	device->SetPixelShader(pPixelShader_);
*/	
}
void Shader::_EndPass()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return;
	effect->EndPass();

/*
	IDirect3DDevice9* device = DirectGraphics::GetBase()->GetDevice();
//	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
*/	
}

bool Shader::_SetupParameter()
{
	ID3DXEffect* effect = GetEffect();
	if(effect == NULL)return false;
	HRESULT hr = effect->SetTechnique(technique_.c_str());
	if(FAILED(hr))return false;

	std::map<std::string, gstd::ref_count_ptr<ShaderParameter> >::iterator itrParam;
	for(itrParam = mapParam_.begin() ; itrParam != mapParam_.end() ; itrParam++)
	{
		std::string name = itrParam->first;
		gstd::ref_count_ptr<ShaderParameter> param = itrParam->second;
		int type = param->GetType();
		switch(type)
		{
			case ShaderParameter::TYPE_MATRIX:
			{
				D3DXMATRIX matrix = param->GetMatrix();
				hr = effect->SetMatrix(name.c_str(), &matrix);
				break;
			}
			case ShaderParameter::TYPE_MATRIX_ARRAY:
			{
				std::vector<D3DXMATRIX> matrixArray = param->GetMatrixArray();
				hr = effect->SetMatrixArray(name.c_str(), &matrixArray[0], matrixArray.size());
				break;
			}
			case ShaderParameter::TYPE_VECTOR:
			{
				D3DXVECTOR4 vect = param->GetVector();
				hr = effect->SetVector(name.c_str(), &vect);
				break;
			}
			case ShaderParameter::TYPE_FLOAT:
			{
				float value = param->GetFloat();
				hr = effect->SetFloat(name.c_str(), value);
				break;
			}
			case ShaderParameter::TYPE_FLOAT_ARRAY:
			{
				std::vector<float> value = param->GetFloatArray();
				hr = effect->SetFloatArray(name.c_str(), &value[0], value.size());
				break;
			}
			case ShaderParameter::TYPE_TEXTURE:
			{
				gstd::ref_count_ptr<Texture> texture = param->GetTexture();
				IDirect3DTexture9 *pTex = texture->GetD3DTexture();
				hr = effect->SetTexture(name.c_str(), pTex);
				break;
			}
		}
		//if(FAILED(hr))return false;
	}
	return true;
}
gstd::ref_count_ptr<ShaderParameter> Shader::_GetParameter(std::string name, bool bCreate)
{
	bool bFind = mapParam_.find(name) != mapParam_.end();
	if(!bFind && !bCreate)return NULL;

	gstd::ref_count_ptr<ShaderParameter> res = NULL;
	if(!bFind)
	{
		res = new ShaderParameter();
		mapParam_[name] = res;
	}
	else
	{
		res = mapParam_[name];
	}

	return res;
}
bool Shader::SetTechnique(std::string name)
{
	//ID3DXEffect* effect = GetEffect();
	//if(effect == NULL)return false;
	//effect->SetTechnique(name.c_str());

	technique_ = name;
	return true;
}
bool Shader::SetMatrix(std::string name, D3DXMATRIX& matrix)
{
	//ID3DXEffect* effect = GetEffect();
	//if(effect == NULL)return false;
	//effect->SetMatrix(name.c_str(), &matrix);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrix(matrix);

	return true;
}
bool Shader::SetMatrixArray(std::string name, std::vector<D3DXMATRIX>& matrix)
{
	//ID3DXEffect* effect = GetEffect();
	//if(effect == NULL)return false;
	//effect->SetMatrixArray(name.c_str(), &matrix[0], matrix.size());

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetMatrixArray(matrix);

	return true;
}
bool Shader::SetVector(std::string name, D3DXVECTOR4& vector)
{
	//ID3DXEffect* effect = GetEffect();
	//if(effect == NULL)return false;
	//effect->SetVector(name.c_str(), &vector);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetVector(vector);
	return true;
}
bool Shader::SetFloat(std::string name, float value)
{
	//ID3DXEffect* effect = GetEffect();
	//if(effect == NULL)return false;
	//effect->SetFloat(name.c_str(), value);

	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloat(value);
	return true;
}
bool Shader::SetFloatArray(std::string name, std::vector<float>& values)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetFloatArray(values);
	return true;
}
bool Shader::SetTexture(std::string name, gstd::ref_count_ptr<Texture> texture)
{
	gstd::ref_count_ptr<ShaderParameter> param = _GetParameter(name, true);
	param->SetTexture(texture);
	return true;
}

