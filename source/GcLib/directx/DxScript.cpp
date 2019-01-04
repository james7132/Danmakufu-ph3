#include"DxScript.hpp"
#include"DxUtility.hpp"
#include"DirectGraphics.hpp"
#include"DirectInput.hpp"
#include"MetasequoiaMesh.hpp"
#include"ElfreinaMesh.hpp"

using namespace gstd;
using namespace directx;

/**********************************************************
//DxScriptObjectBase
**********************************************************/
DxScriptObjectBase::DxScriptObjectBase()
{
	bVisible_ = true;
	priRender_ = 0.5;
	bDeleted_ = false;
	bActive_ = false;
	manager_ = NULL;
	idObject_ = DxScript::ID_INVALID;
	idScript_ = ScriptClientBase::ID_SCRIPT_FREE;
	typeObject_ = DxScript::OBJ_INVALID;
}
DxScriptObjectBase::~DxScriptObjectBase()
{
	if(manager_ != NULL && idObject_ != DxScript::ID_INVALID)
		manager_->listUnusedIndex_.push_back(idObject_);
}
int DxScriptObjectBase::GetRenderPriorityI()
{
	int res = (int)(priRender_ * (manager_->GetRenderBucketCapacity() - 1) + 0.5);
	return res;
}

/**********************************************************
//DxScriptRenderObject
**********************************************************/
DxScriptRenderObject::DxScriptRenderObject()
{
	bZWrite_ = false;
	bZTest_ = false;
	bFogEnable_ = false;
	typeBlend_ = DirectGraphics::MODE_BLEND_ALPHA;
	modeCulling_ = D3DCULL_NONE;
	position_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	angle_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	scale_ = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

/**********************************************************
//DxScriptShaderObject
**********************************************************/
DxScriptShaderObject::DxScriptShaderObject()
{
	typeObject_ = DxScript::OBJ_SHADER;
}

/**********************************************************
//DxScriptPrimitiveObject
**********************************************************/
DxScriptPrimitiveObject::DxScriptPrimitiveObject()
{
	idRelative_ = -1;
}
void DxScriptPrimitiveObject::SetPrimitiveType(D3DPRIMITIVETYPE type)
{
	objRender_->SetPrimitiveType(type);
}
void DxScriptPrimitiveObject::SetVertexCount(int count)
{
	objRender_->SetVertexCount(count);
}
int DxScriptPrimitiveObject::GetVertexCount()
{
	return objRender_->GetVertexCount();
}
gstd::ref_count_ptr<Texture> DxScriptPrimitiveObject::GetTexture()
{
	return objRender_->GetTexture();
}
void DxScriptPrimitiveObject::SetTexture(gstd::ref_count_ptr<Texture> texture)
{
	objRender_->SetTexture(texture);
}
gstd::ref_count_ptr<Shader> DxScriptPrimitiveObject::GetShader()
{
	return objRender_->GetShader();
}
void DxScriptPrimitiveObject::SetShader(gstd::ref_count_ptr<Shader> shader)
{
	objRender_->SetShader(shader);
}

/**********************************************************
//DxScriptPrimitiveObject2D
**********************************************************/
DxScriptPrimitiveObject2D::DxScriptPrimitiveObject2D()
{
	typeObject_ = DxScript::OBJ_PRIMITIVE_2D;

	objRender_ = new RenderObjectTLX();
	bZWrite_ = false;
	bZTest_ = false;
}
void DxScriptPrimitiveObject2D::Render()
{
	RenderObjectTLX* obj = GetObjectPointer();

	//フォグを解除する
	DirectGraphics* graphics = DirectGraphics::GetBase();
	DWORD bEnableFog = FALSE;
	graphics->GetDevice()->GetRenderState(D3DRS_FOGENABLE, &bEnableFog);
	if(bEnableFog)
		graphics->SetFogEnable(false);

	SetRenderState();
	obj->Render();

	if(bEnableFog)
		graphics->SetFogEnable(true);
}
void DxScriptPrimitiveObject2D::SetRenderState()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	RenderObjectTLX* obj = GetObjectPointer();
	obj->SetPosition(position_);
	obj->SetAngle(angle_);
	obj->SetScale(scale_);
	graphics->SetLightingEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetZBufferEnable(false);
	graphics->SetBlendMode(typeBlend_);
	graphics->SetCullingMode(D3DCULL_NONE);
}
bool DxScriptPrimitiveObject2D::IsValidVertexIndex(int index)
{
	RenderObjectTLX* obj = GetObjectPointer();
	int count = obj->GetVertexCount();
	return index>=0 && index<count;
}
void DxScriptPrimitiveObject2D::SetColor(int r, int g, int b)
{
	RenderObjectTLX* obj = GetObjectPointer();

	int count = obj->GetVertexCount();
	for(int iVert = 0 ; iVert < count ; iVert++)
	{
		VERTEX_TLX* vert = obj->GetVertex(iVert);
		D3DCOLOR& color = vert->diffuse_color;
		color = ColorAccess::SetColorR(color, r);
		color = ColorAccess::SetColorG(color, g);
		color = ColorAccess::SetColorB(color, b);
	}
}
void DxScriptPrimitiveObject2D::SetAlpha(int alpha)
{
	RenderObjectTLX* obj = GetObjectPointer();

	int count = obj->GetVertexCount();
	for(int iVert = 0 ; iVert < count ; iVert++)
	{
		VERTEX_TLX* vert = obj->GetVertex(iVert);
		D3DCOLOR& color = vert->diffuse_color;
		color = ColorAccess::SetColorA(color, alpha);
	}
}
void DxScriptPrimitiveObject2D::SetVertexPosition(int index, float x, float y, float z)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectTLX* obj = GetObjectPointer();
	obj->SetVertexPosition(index, x, y, z);
}
void DxScriptPrimitiveObject2D::SetVertexUV(int index, float u, float v)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectTLX* obj = GetObjectPointer();
	obj->SetVertexUV(index, u, v);
}
void DxScriptPrimitiveObject2D::SetVertexAlpha(int index, int alpha)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectTLX* obj = GetObjectPointer();
	VERTEX_TLX* vert = obj->GetVertex(index);
	D3DCOLOR& color = vert->diffuse_color;
	color = ColorAccess::SetColorA(color, alpha);
}
void DxScriptPrimitiveObject2D::SetVertexColor(int index, int r, int g, int b)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectTLX* obj = GetObjectPointer();
	VERTEX_TLX* vert = obj->GetVertex(index);
	D3DCOLOR& color = vert->diffuse_color;
	color = ColorAccess::SetColorR(color, r);
	color = ColorAccess::SetColorG(color, g);
	color = ColorAccess::SetColorB(color, b);
}
void DxScriptPrimitiveObject2D::SetPermitCamera(bool bPermit)
{
	RenderObjectTLX* obj = GetObjectPointer();
	obj->SetPermitCamera(bPermit);
}
D3DXVECTOR3 DxScriptPrimitiveObject2D::GetVertexPosition(int index)
{
	D3DXVECTOR3 res(0, 0, 0);
	if(!IsValidVertexIndex(index))return res;
	RenderObjectTLX* obj = GetObjectPointer();
	VERTEX_TLX* vert = obj->GetVertex(index);

	float bias = 0.5f;
	res.x = vert->position.x + bias;
	res.y = vert->position.y + bias;
	res.z = 0;

	return res;
}

/**********************************************************
//DxScriptSpriteObject2D
**********************************************************/
DxScriptSpriteObject2D::DxScriptSpriteObject2D()
{
	typeObject_ = DxScript::OBJ_SPRITE_2D;
	objRender_ = new Sprite2D();
}
void DxScriptSpriteObject2D::Copy(DxScriptSpriteObject2D* src)
{
	priRender_ = src->priRender_;
	bZWrite_ = src->bZWrite_;
	bZTest_ = src->bZTest_;
	modeCulling_ = src->modeCulling_;
	bVisible_ = src->bVisible_;
	manager_ = src->manager_;
	position_ = src->position_;
	angle_ = src->angle_;
	scale_ = src->scale_;
	typeBlend_ = src->typeBlend_;

	Sprite2D* destSprite2D = (Sprite2D*)objRender_.GetPointer();
	Sprite2D* srcSprite2D = (Sprite2D*)src->objRender_.GetPointer();
	destSprite2D->Copy(srcSprite2D);
}

/**********************************************************
//DxScriptSpriteListObject2D
**********************************************************/
DxScriptSpriteListObject2D::DxScriptSpriteListObject2D()
{
	typeObject_ = DxScript::OBJ_SPRITE_LIST_2D;
	objRender_ = new SpriteList2D();
}
void DxScriptSpriteListObject2D::SetColor(int r, int g, int b)
{
	D3DCOLOR color = GetSpritePointer()->GetColor();
	color = ColorAccess::SetColorR(color, r);
	color = ColorAccess::SetColorG(color, g);
	color = ColorAccess::SetColorB(color, b);
	GetSpritePointer()->SetColor(color);
}
void DxScriptSpriteListObject2D::SetAlpha(int alpha)
{
	D3DCOLOR color = GetSpritePointer()->GetColor();
	color = ColorAccess::SetColorA(color, alpha);
	GetSpritePointer()->SetColor(color);
}
void DxScriptSpriteListObject2D::AddVertex()
{
	SpriteList2D* obj = GetSpritePointer();
	obj->SetPosition(position_);
	obj->SetAngle(angle_);
	obj->SetScale(scale_);
	obj->AddVertex();
}
void DxScriptSpriteListObject2D::CloseVertex()
{
	SpriteList2D* obj = GetSpritePointer();
	obj->CloseVertex();

	position_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	angle_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	scale_ =  D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

/**********************************************************
//DxScriptPrimitiveObject3D
**********************************************************/
DxScriptPrimitiveObject3D::DxScriptPrimitiveObject3D()
{
	typeObject_ = DxScript::OBJ_PRIMITIVE_3D;
	objRender_ = new RenderObjectLX();
	bZWrite_ = false;
	bZTest_ = true;
	bFogEnable_ = true;
}
void DxScriptPrimitiveObject3D::Render()
{
	RenderObjectLX* obj = GetObjectPointer();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool bEnvFogEnable = graphics->IsFogEnable();
	SetRenderState();
	obj->Render();

	//フォグの状態をリセット
	if(bEnvFogEnable)
		graphics->SetFogEnable(true);
}
void DxScriptPrimitiveObject3D::SetRenderState()
{
	if(idRelative_ >= 0)
	{
		ref_count_ptr<DxScriptObjectBase>::unsync objRelative = manager_->GetObject(idRelative_);
		if(objRelative != NULL)
		{
			objRelative->SetRenderState();
			DxScriptMeshObject* objMesh = dynamic_cast<DxScriptMeshObject*>(objRelative.GetPointer());
			if(objMesh != NULL)
			{
				int frameAnime = objMesh->GetAnimeFrame();
				std::wstring nameAnime = objMesh->GetAnimeName();
				ref_count_ptr<DxMesh> mesh = objMesh->GetMesh();
				D3DXMATRIX mat = mesh->GetAnimationMatrix(nameAnime, frameAnime, nameRelativeBone_);
				objRender_->SetRalativeMatrix(mat);
			}
		}
	}

	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool bEnvFogEnable = graphics->IsFogEnable();

	RenderObjectLX* obj = GetObjectPointer();
	obj->SetPosition(position_);
	obj->SetAngle(angle_);
	obj->SetScale(scale_);
	graphics->SetLightingEnable(false);
	graphics->SetZWriteEnalbe(bZWrite_);
	graphics->SetZBufferEnable(bZTest_);
	if(bEnvFogEnable)
		graphics->SetFogEnable(bFogEnable_);
	graphics->SetBlendMode(typeBlend_);
	graphics->SetCullingMode(modeCulling_);

}
bool DxScriptPrimitiveObject3D::IsValidVertexIndex(int index)
{
	RenderObjectLX* obj = GetObjectPointer();
	int count = obj->GetVertexCount();
	return index>=0 && index<count;
}
void DxScriptPrimitiveObject3D::SetColor(int r, int g, int b)
{
	RenderObjectLX* obj = GetObjectPointer();

	int count = obj->GetVertexCount();
	for(int iVert = 0 ; iVert < count ; iVert++)
	{
		VERTEX_LX* vert = obj->GetVertex(iVert);
		D3DCOLOR& color = vert->diffuse_color;
		color = ColorAccess::SetColorR(color, r);
		color = ColorAccess::SetColorG(color, g);
		color = ColorAccess::SetColorB(color, b);
	}
}
void DxScriptPrimitiveObject3D::SetAlpha(int alpha)
{
	RenderObjectLX* obj = GetObjectPointer();

	int count = obj->GetVertexCount();
	for(int iVert = 0 ; iVert < count ; iVert++)
	{
		VERTEX_LX* vert = obj->GetVertex(iVert);
		D3DCOLOR& color = vert->diffuse_color;
		color = ColorAccess::SetColorA(color, alpha);
	}
}
void DxScriptPrimitiveObject3D::SetVertexPosition(int index, float x, float y, float z)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectLX* obj = GetObjectPointer();
	VERTEX_LX* vert = obj->GetVertex(index);
	vert->position.x = x;
	vert->position.y = y;
	vert->position.z = z;
}
void DxScriptPrimitiveObject3D::SetVertexUV(int index, float u, float v)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectLX* obj = GetObjectPointer();

	VERTEX_LX* vert = obj->GetVertex(index);
	vert->texcoord.x = u;
	vert->texcoord.y = v;
}
void DxScriptPrimitiveObject3D::SetVertexAlpha(int index, int alpha)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectLX* obj = GetObjectPointer();
	VERTEX_LX* vert = obj->GetVertex(index);
	D3DCOLOR& color = vert->diffuse_color;
	color = ColorAccess::SetColorA(color, alpha);
}
void DxScriptPrimitiveObject3D::SetVertexColor(int index, int r, int g, int b)
{
	if(!IsValidVertexIndex(index))return;
	RenderObjectLX* obj = GetObjectPointer();
	VERTEX_LX* vert = obj->GetVertex(index);
	D3DCOLOR& color = vert->diffuse_color;
	color = ColorAccess::SetColorR(color, r);
	color = ColorAccess::SetColorG(color, g);
	color = ColorAccess::SetColorB(color, b);
}
D3DXVECTOR3 DxScriptPrimitiveObject3D::GetVertexPosition(int index)
{
	D3DXVECTOR3 res(0, 0, 0);
	if(!IsValidVertexIndex(index))return res;
	RenderObjectLX* obj = GetObjectPointer();
	VERTEX_LX* vert = obj->GetVertex(index);

	res.x = vert->position.x;
	res.y = vert->position.y;
	res.z = vert->position.z;

	return res;
}
/**********************************************************
//DxScriptSpriteObject3D
**********************************************************/
DxScriptSpriteObject3D::DxScriptSpriteObject3D()
{
	typeObject_ = DxScript::OBJ_SPRITE_3D;
	objRender_ = new Sprite3D();
}
/**********************************************************
//DxScriptTrajectoryObject3D
**********************************************************/
DxScriptTrajectoryObject3D::DxScriptTrajectoryObject3D()
{
	typeObject_ = DxScript::OBJ_TRAJECTORY_3D;
	objRender_ = new TrajectoryObject3D();
}
void DxScriptTrajectoryObject3D::Work()
{
	if(idRelative_ >= 0)
	{
		ref_count_ptr<DxScriptObjectBase>::unsync objRelative = manager_->GetObject(idRelative_);
		if(objRelative != NULL)
		{
			objRelative->SetRenderState();
			DxScriptMeshObject* objMesh = dynamic_cast<DxScriptMeshObject*>(objRelative.GetPointer());
			if(objMesh != NULL)
			{
				int frameAnime = objMesh->GetAnimeFrame();
				std::wstring nameAnime = objMesh->GetAnimeName();
				ref_count_ptr<DxMesh> mesh = objMesh->GetMesh();
				D3DXMATRIX matAnime = mesh->GetAnimationMatrix(nameAnime, frameAnime, nameRelativeBone_);

				TrajectoryObject3D* objRender = GetObjectPointer();
				objRender->AddPoint(matAnime);
			}
		}
	}

	TrajectoryObject3D* obj = GetObjectPointer();
	obj->Work();
}
void DxScriptTrajectoryObject3D::Render()
{
	TrajectoryObject3D* obj = GetObjectPointer();
	SetRenderState();
	obj->Render();
}
void DxScriptTrajectoryObject3D::SetRenderState()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	TrajectoryObject3D* obj = GetObjectPointer();
	graphics->SetLightingEnable(false);
	graphics->SetZWriteEnalbe(bZWrite_);
	graphics->SetZBufferEnable(bZTest_);
	graphics->SetBlendMode(typeBlend_);
	graphics->SetCullingMode(modeCulling_);
}
void DxScriptTrajectoryObject3D::SetColor(int r, int g, int b)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	TrajectoryObject3D* obj = GetObjectPointer();
	obj->SetColor(D3DCOLOR_ARGB(255, r, g, b));
}

/**********************************************************
//DxScriptMeshObject
**********************************************************/
DxScriptMeshObject::DxScriptMeshObject()
{
	typeObject_ = DxScript::OBJ_MESH;
	bZWrite_ = true;
	bZTest_ = true;
	bFogEnable_ = true;
	time_ = 0;
	anime_ = L"";
	color_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	bCoordinate2D_ = false;
}

void DxScriptMeshObject::Render()
{
	if(mesh_ == NULL)return;
	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool bEnvFogEnable = graphics->IsFogEnable();
	SetRenderState();
	mesh_->Render(anime_, time_);

	//フォグの状態をリセット
	if(bEnvFogEnable)
		graphics->SetFogEnable(true);
}
void DxScriptMeshObject::SetRenderState()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool bEnvFogEnable = graphics->IsFogEnable();
	
	mesh_->SetPosition(position_);
	mesh_->SetAngle(angle_);
	mesh_->SetScale(scale_);
	mesh_->SetColor(color_);
	mesh_->SetCoordinate2D(bCoordinate2D_);

	graphics->SetLightingEnable(true);
	graphics->SetZWriteEnalbe(bZWrite_);
	graphics->SetZBufferEnable(bZTest_);
	if(bEnvFogEnable)
		graphics->SetFogEnable(bFogEnable_);
	graphics->SetBlendMode(typeBlend_);
	graphics->SetCullingMode(modeCulling_);
}
void DxScriptMeshObject::SetColor(int r, int g, int b)
{
	ColorAccess::SetColorR(color_, r);
	ColorAccess::SetColorG(color_, g);
	ColorAccess::SetColorB(color_, b);
}
void DxScriptMeshObject::SetAlpha(int alpha)
{
	ColorAccess::SetColorA(color_, alpha);
}
void DxScriptMeshObject::_UpdateMeshState()
{
	if(mesh_ == NULL)return;
	mesh_->SetPosition(position_);
	mesh_->SetAngle(angle_);
	mesh_->SetScale(scale_);
	mesh_->SetColor(color_);
}
void DxScriptMeshObject::SetShader(gstd::ref_count_ptr<Shader> shader)
{
	if(mesh_ == NULL)return;
	mesh_->SetShader(shader);
}

/**********************************************************
//DxScriptTextObject
**********************************************************/
DxScriptTextObject::DxScriptTextObject()
{
	typeObject_ = DxScript::OBJ_TEXT;
	bChange_ = true;
	bAutoCenter_ = true;
	center_ = D3DXVECTOR2(0, 0);
}
void DxScriptTextObject::Render()
{
	//フォグを解除する
	DirectGraphics* graphics = DirectGraphics::GetBase();
	DWORD bEnableFog = FALSE;
	graphics->GetDevice()->GetRenderState(D3DRS_FOGENABLE, &bEnableFog);
	if(bEnableFog)
		graphics->SetFogEnable(false);

	SetRenderState();
	text_.SetPosition(position_.x, position_.y);
	_UpdateRenderer();
	objRender_->SetPosition(position_.x, position_.y);
	objRender_->SetAngle(angle_);
	objRender_->SetScale(scale_);
	objRender_->SetVertexColor(text_.GetVertexColor());
	objRender_->SetTransCenter(center_);
	objRender_->SetAutoCenter(bAutoCenter_);
	objRender_->Render();
	bChange_ = false;

	if(bEnableFog)
		graphics->SetFogEnable(true);
}
void DxScriptTextObject::SetRenderState()
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetLightingEnable(false);
	graphics->SetZWriteEnalbe(false);
	graphics->SetZBufferEnable(false);
	graphics->SetBlendMode(typeBlend_);
	graphics->SetCullingMode(D3DCULL_NONE);
}
void DxScriptTextObject::_UpdateRenderer()
{
	if(bChange_)
	{
		textInfo_ = text_.GetTextInfo();
		objRender_ = text_.CreateRenderObject(textInfo_);
	}
	bChange_ = false;
}

std::vector<int> DxScriptTextObject::GetTextCountCU()
{
	_UpdateRenderer();
	int lineCount = textInfo_->GetLineCount();
	std::vector<int> listCount;
	for(int iLine = 0 ; iLine < lineCount ; iLine++)
	{
		gstd::ref_count_ptr<DxTextLine> textLine = textInfo_->GetTextLine(iLine);
		int count = textLine->GetTextCodeCount();
		listCount.push_back(count);
	}
	return listCount;
}
void DxScriptTextObject::SetAlpha(int alpha)
{
	D3DCOLOR color = text_.GetVertexColor();
	int r = ColorAccess::GetColorR(color);
	int g = ColorAccess::GetColorG(color);
	int b = ColorAccess::GetColorB(color);

	alpha = max(alpha, 0);
	alpha = min(alpha, 255);

	SetVertexColor(D3DCOLOR_ARGB(alpha, r, g, b));
}
void DxScriptTextObject::SetColor(int r, int g, int b)
{
	D3DCOLOR color = text_.GetVertexColor();
	int a = ColorAccess::GetColorA(color);
	r = max(r, 0);
	r = min(r, 255);
	g = max(g, 0);
	g = min(g, 255);
	b = max(b, 0);
	b = min(b, 255);	

	SetVertexColor(D3DCOLOR_ARGB(a, r, g, b));
}
int DxScriptTextObject::GetTotalWidth()
{
	_UpdateRenderer();
	int res = textInfo_->GetTotalWidth();
	return res;
}
int DxScriptTextObject::GetTotalHeight()
{
	_UpdateRenderer();
	int res = textInfo_->GetTotalHeight();
	return res;
}
void DxScriptTextObject::SetShader(gstd::ref_count_ptr<Shader> shader)
{
	text_.SetShader(shader);
	bChange_ = true;
}

/**********************************************************
//DxSoundObject
**********************************************************/
DxSoundObject::DxSoundObject()
{
	typeObject_ = DxScript::OBJ_SOUND;
}
DxSoundObject::~DxSoundObject()
{
	if(player_ == NULL)return;
	player_->Delete();
}
bool DxSoundObject::Load(std::wstring path)
{
	DirectSoundManager* manager = DirectSoundManager::GetBase();
	player_ = manager->GetPlayer(path);
	if(player_ == NULL)return false;

	return true;
}
void DxSoundObject::Play()
{
	if(player_ != NULL)
		player_->Play(style_);
}

/**********************************************************
//DxFileObject
**********************************************************/
DxFileObject::DxFileObject()
{
}
DxFileObject::~DxFileObject()
{
}
bool DxFileObject::OpenR(std::wstring path)
{
	file_ = new File(path);
	bool res = file_->Open();
	if(!res)file_ = NULL;
	return res;
}
bool DxFileObject::OpenW(std::wstring path)
{
	path = PathProperty::Canonicalize(path);
	path = PathProperty::ReplaceYenToSlash(path);

	std::wstring dir = PathProperty::GetFileDirectory(path);
	File fDir(dir);
	bool bDir = fDir.CreateDirectory();
	if(!bDir)return false;

	std::wstring dirModule = PathProperty::GetFileDirectory(path);
	if(dir.find(dirModule) == std::wstring::npos)return false;

	file_ = new File(path);
	bool res = file_->Create();
	if(!res)file_ = NULL;
	return res;
}
void DxFileObject::Close()
{
	if(file_ == NULL)return;
	file_->Close();
}

/**********************************************************
//DxTextFileObject
**********************************************************/
DxTextFileObject::DxTextFileObject()
{
	typeObject_ = DxScript::OBJ_FILE_TEXT;
}
DxTextFileObject::~DxTextFileObject()
{
}
bool DxTextFileObject::OpenR(std::wstring path)
{
	listLine_.clear();
	bool res = DxFileObject::OpenR(path);
	if(!res)return false;
	
	int size = file_->GetSize();
	if(size == 0)return true;

	std::string text;
	text.resize(size+1);
	file_->Read(&text[0], size);
	text[size] = '\0';

	class TextScanner
	{
		private:
			std::string* text_;
			int pos_;
		public:
			TextScanner(std::string* text){text_ = text;pos_=0;}
			int GetPosition(){return pos_;}
			char GetCurrentCharactor(){return (*text_)[pos_];}
			void Advance()
			{
				pos_++;
				if(pos_ >= text_->size())throw gstd::wexception();
			}
	};
	TextScanner scan(&text);

	try 
	{
		int pointer = 0;
		int posStart = 0;
		while(true)
		{
			if(scan.GetPosition() >= size)break;
			if(IsDBCSLeadByte(scan.GetCurrentCharactor()))
			{
				while(IsDBCSLeadByte(scan.GetCurrentCharactor()))scan.Advance();
			}
			else
			{
				char ch = scan.GetCurrentCharactor();
				if(ch == '\r' || ch == '\n')
				{
					int posEnd = scan.GetPosition();
					if(ch == '\r')scan.Advance();
					scan.Advance();

					std::string str = text.substr(posStart, posEnd - posStart);
					listLine_.push_back(str);
					posStart = scan.GetPosition();
				}
				else
				{
					scan.Advance();
				}
			}

		};
		std::string str = text.substr(posStart, size - posStart);
		listLine_.push_back(str);
	}
	catch(...)
	{
		return false;
	}

	return true;
}
bool DxTextFileObject::OpenW(std::wstring path)
{
	bool res = DxFileObject::OpenW(path);
	return res;
}
bool DxTextFileObject::Store()
{
	if(file_ == NULL)return false;
	for(int iLine = 0 ; iLine < listLine_.size() ; iLine++)
	{
		std::string str = listLine_[iLine];
		if(iLine < listLine_.size() - 1)
			str += "\r\n";

		const char* text = str.c_str();
		int length = str.size();
		file_->Write((void *)text, length);
	}
	return true;
}
std::string DxTextFileObject::GetLine(int line)
{
	line--; //行数は1開始
	if(line >= listLine_.size())return "";

	std::string res = listLine_[line];
	return res;
}

/**********************************************************
//DxBinaryFileObject
**********************************************************/
DxBinaryFileObject::DxBinaryFileObject()
{
	typeObject_ = DxScript::OBJ_FILE_BINARY;
	byteOrder_ = ByteOrder::ENDIAN_LITTLE;
	codePage_ = CP_ACP;
}
DxBinaryFileObject::~DxBinaryFileObject()
{
}
bool DxBinaryFileObject::OpenR(std::wstring path)
{
	bool res = DxFileObject::OpenR(path);
	if(!res)return false;
	
	int size = file_->GetSize();
	buffer_ = new gstd::ByteBuffer();
	buffer_->SetSize(size);

	file_->Read(buffer_->GetPointer(), size);

	return true;
}
bool DxBinaryFileObject::OpenW(std::wstring path)
{
	return false;
}
bool DxBinaryFileObject::Store()
{
	return false;
}
bool DxBinaryFileObject::IsReadableSize(int size)
{
	int pos = buffer_->GetOffset();
	bool res = pos + size <= buffer_->GetSize();
	return res;
}

/**********************************************************
//DxScriptObjectManager
**********************************************************/
DxScriptObjectManager::DxScriptObjectManager()
{
	SetMaxObject(256*256);
	SetRenderBucketCapacity(101);
	totalObjectCreateCount_ = 0;

	bFogEnable_ = false;
	fogColor_ = D3DCOLOR_ARGB(255, 255, 255, 255);
	fogStart_ = 0;
	fogEnd_ = 0;
}
DxScriptObjectManager::~DxScriptObjectManager()
{
}
void DxScriptObjectManager::SetMaxObject(int max)
{
	if(obj_.size() == max)return;

	if(obj_.size() < max)
	{
		listUnusedIndex_.clear();
		for(int iObj = 0 ; iObj < obj_.size() ; iObj++)
		{
			if(obj_[iObj] != NULL)continue;
			listUnusedIndex_.push_back(iObj);
		}
	}
	for(int iObj = obj_.size() ; iObj < max ; iObj++)
	{
		listUnusedIndex_.push_back(iObj);
	}
	obj_.resize(max);
}
void DxScriptObjectManager::SetRenderBucketCapacity(int capacity)
{
	objRender_.resize(capacity);
	listShader_.resize(capacity);
}
void DxScriptObjectManager::_ArrangeActiveObjectList()
{
	std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
	for(itr = listActiveObject_.begin() ; itr != listActiveObject_.end() ;)
	{
		gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = (*itr);
		if(obj == NULL || obj->IsDeleted() || !obj->IsActive()) itr = listActiveObject_.erase(itr);
		else itr++;
	}
}
int DxScriptObjectManager::AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate)
{
	int res = DxScript::ID_INVALID;
	if(listUnusedIndex_.size() <= obj_.size() / 2)
	{
		//空きがない(念のために空きが半分以下)ので拡張
		int oldSize = obj_.size();
		int newSize = oldSize * 1.5;
		SetMaxObject(newSize);
		Logger::WriteTop(StringUtility::Format(L"DxScriptObjectManagerサイズ拡張[%d->%d]", oldSize, newSize));
	}

	if(listUnusedIndex_.size() != 0)
	{
		res = listUnusedIndex_.front();
		listUnusedIndex_.pop_front();

		obj_[res] = obj;
		if(bActivate)
		{
			obj->bActive_ = true;
			listActiveObject_.push_back(obj);
		}
		obj->idObject_ = res;
		obj->manager_ = this;

		totalObjectCreateCount_++;
	}

	return res;
}
void DxScriptObjectManager::AddObject(int id, gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate)
{
	obj_[id] = obj;
	obj->idObject_ = id;
	obj->manager_ = this;
	if(bActivate)
	{
		obj->bActive_ = true;
		listActiveObject_.push_back(obj);
	}

	std::list<int>::iterator itr = listUnusedIndex_.begin();
	for(; itr != listUnusedIndex_.end(); itr++)
	{
		if((*itr) == id)
		{
			listUnusedIndex_.erase(itr);
			totalObjectCreateCount_++;
			break;
		}
	}
}
void DxScriptObjectManager::ActivateObject(int id, bool bActivate)
{
	gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = GetObject(id);
	if(obj == NULL || obj->IsDeleted())return;

	if(bActivate && !obj->IsActive())
	{
		obj->bActive_ = true;
		listActiveObject_.push_back(obj);
	}
	else if(!bActivate)
	{
		obj->bActive_ = false;
	}
}
std::vector<int> DxScriptObjectManager::GetValidObjectIdentifier()
{
	std::vector<int> res;
	for(int iObj = 0 ; iObj < obj_.size() ; iObj++)
	{
		if(obj_[iObj] == NULL)continue;
		res.push_back(obj_[iObj]->idObject_);
	}
	return res;
}
DxScriptObjectBase* DxScriptObjectManager::GetObjectPointer(int id)
{
	if(id < 0 || id >= obj_.size())return NULL;
	return obj_[id].GetPointer();
}
void DxScriptObjectManager::DeleteObject(int id)
{
	if(id < 0 || id >= obj_.size())return;
	if(obj_[id]  == NULL)return;

	obj_[id]->bDeleted_ = true;
	obj_[id] = NULL;
	//listUnusedIndex_.push_back(id);
}
void DxScriptObjectManager::ClearObject()
{
	int size = obj_.size();
	obj_.clear();
	obj_.resize(size);
	listActiveObject_.clear();

	listUnusedIndex_.clear();
	for(int iObj = 0 ; iObj < size ; iObj++)
	{
		listUnusedIndex_.push_back(iObj);
	}
}
gstd::ref_count_ptr<Shader> DxScriptObjectManager::GetShader(int index)
{
	if(index < 0 || index >= listShader_.size())return NULL;
	ref_count_ptr<Shader> shader = listShader_[index];
	return shader;
}
void DxScriptObjectManager::DeleteObjectByScriptID(_int64 idScript)
{
	if(idScript == ScriptClientBase::ID_SCRIPT_FREE)return;

	for(int iObj = 0 ; iObj < obj_.size() ; iObj++)
	{
		if(obj_[iObj] == NULL)continue;
		if(obj_[iObj]->GetScriptID() != idScript)continue;
		DeleteObject(obj_[iObj]->GetObjectID());
	}
}
void DxScriptObjectManager::AddRenderObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj)
{
	if(!obj->IsVisible())return;

	int renderSize = objRender_.size();
	double pri = obj->priRender_;
	int tPri = (int)(pri * (objRender_.size() - 1) + 0.5);
	if(tPri > objRender_.size()-1)tPri = objRender_.size() - 1;
	objRender_[tPri].push_back(obj);
}
void DxScriptObjectManager::WorkObject()
{
	_ArrangeActiveObjectList();
	std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
	for(itr = listActiveObject_.begin() ; itr != listActiveObject_.end() ; itr++)
	{
		gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = (*itr);
		if(obj == NULL || obj->IsDeleted())continue;
		obj->Work();
	}

	//音声再生
	DirectSoundManager* soundManager = DirectSoundManager::GetBase();
	std::map<std::wstring, ref_count_ptr<SoundInfo> >::iterator itrSound = mapReservedSound_.begin();
	for(; itrSound != mapReservedSound_.end() ; itrSound++) 
	{
		gstd::ref_count_ptr<SoundInfo> info = itrSound->second;
		gstd::ref_count_ptr<SoundPlayer> player = info->player_;
		SoundPlayer::PlayStyle style = info->style_;
		player->Play(style);
	}
	mapReservedSound_.clear();

}
void DxScriptObjectManager::RenderObject()
{
	PrepareRenderObject();

	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->SetVertexFog(bFogEnable_, fogColor_, fogStart_, fogEnd_);

	for(int iPri = 0 ; iPri < objRender_.size() ; iPri++)
	{
		ref_count_ptr<Shader> shader = listShader_[iPri];
		if(shader != NULL)
		{
			shader->Begin();
		}

		std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
		for(itr = objRender_[iPri].begin() ; itr != objRender_[iPri].end() ; itr++)
		{
			(*itr)->Render();
		}
		objRender_[iPri].clear();

		if(shader != NULL)
		{
			shader->End();
		}
	}

}
void DxScriptObjectManager::PrepareRenderObject()
{
	std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync >::iterator itr;
	for(itr = listActiveObject_.begin() ; itr != listActiveObject_.end() ; itr++)
	{
		gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj = (*itr);
		if(obj == NULL || obj->IsDeleted())continue;
		if(!obj->IsVisible())continue;
		AddRenderObject(obj);
	}
}
void DxScriptObjectManager::ClearRenderObject()
{
	for(int iPri = 0 ; iPri < objRender_.size() ; iPri++)
	{
		objRender_[iPri].clear();
	}
}
void DxScriptObjectManager::SetShader(gstd::ref_count_ptr<Shader> shader, double min, double max)
{
	int tPriMin = (int)(min * (listShader_.size() - 1) + 0.5);
	int tPriMax = (int)(max * (listShader_.size() - 1) + 0.5);
	for(int iPri = tPriMin ; iPri <= tPriMax ; iPri++)
	{
		if(iPri < 0 || iPri >= listShader_.size())break;
		listShader_[iPri] = shader;
	}
}
void DxScriptObjectManager::ResetShader()
{
	ResetShader(0, listShader_.size());
}
void DxScriptObjectManager::ResetShader(double min, double max)
{
	SetShader(NULL, min, max);
}

void DxScriptObjectManager::ReserveSound(ref_count_ptr<SoundPlayer> player, SoundPlayer::PlayStyle& style)
{
	ref_count_ptr<SoundInfo> info = new SoundInfo();
	info->player_ = player;
	info->style_ = style;

	std::wstring path = player->GetPath();
	mapReservedSound_[path] = info;
}
void DxScriptObjectManager::DeleteReservedSound(gstd::ref_count_ptr<SoundPlayer> player)
{
	std::wstring path = player->GetPath();
	mapReservedSound_.erase(path);
}
void DxScriptObjectManager::SetFogParam(bool bEnable, D3DCOLOR fogColor, float start, float end)
{
	bFogEnable_ = bEnable;
	fogColor_ = fogColor;
	fogStart_ = start;
	fogEnd_ = end;
}

/**********************************************************
//DxScript
**********************************************************/
function const dxFunction[] =  
{
	//Dx関数：システム系系
	{"InstallFont", DxScript::Func_InstallFont, 1},

	//Dx関数：音声系
	{"LoadSound", DxScript::Func_LoadSound, 1},
	{"RemoveSound", DxScript::Func_RemoveSound, 1},
	{"PlayBGM", DxScript::Func_PlayBGM, 3},
	{"PlaySE", DxScript::Func_PlaySE, 1},
	{"StopSound", DxScript::Func_StopSound, 1},

	//Dx関数：キー系
	{"GetKeyState", DxScript::Func_GetKeyState, 1},
	{"GetMouseX", DxScript::Func_GetMouseX, 0},
	{"GetMouseY", DxScript::Func_GetMouseY, 0},
	{"GetMouseMoveZ", DxScript::Func_GetMouseMoveZ, 0},
	{"GetMouseState", DxScript::Func_GetMouseState, 1},
	{"GetVirtualKeyState", DxScript::Func_GetVirtualKeyState, 1},
	{"SetVirtualKeyState", DxScript::Func_SetVirtualKeyState, 2},

	//Dx関数：描画系
	{"GetScreenWidth", DxScript::Func_GetScreenWidth, 0},
	{"GetScreenHeight", DxScript::Func_GetScreenHeight, 0},
	{"LoadTexture", DxScript::Func_LoadTexture, 1},
	{"LoadTextureInLoadThread", DxScript::Func_LoadTextureInLoadThread, 1},
	{"RemoveTexture", DxScript::Func_RemoveTexture, 1},
	{"GetTextureWidth", DxScript::Func_GetTextureWidth, 1},
	{"GetTextureHeight", DxScript::Func_GetTextureHeight, 1},
	{"SetFogEnable", DxScript::Func_SetFogEnable, 1},
	{"SetFogParam", DxScript::Func_SetFogParam, 5},
	{"CreateRenderTarget", DxScript::Func_CreateRenderTarget, 1},
	{"SetRenderTarget", DxScript::Func_SetRenderTarget, 1},
	{"GetTransitionRenderTargetName", DxScript::Func_GetTransitionRenderTargetName, 0},
	{"SaveRenderedTextureA1", DxScript::Func_SaveRenderedTextureA1, 2},
	{"SaveRenderedTextureA2", DxScript::Func_SaveRenderedTextureA2, 6},
	{"IsPixelShaderSupported", DxScript::Func_IsPixelShaderSupported, 2},
	{"SetShader", DxScript::Func_SetShader, 3},
	{"SetShaderI", DxScript::Func_SetShaderI, 3},
	{"ResetShader", DxScript::Func_ResetShader, 2},
	{"ResetShaderI", DxScript::Func_ResetShaderI, 2},

	//Dx関数：カメラ3D
	{"SetCameraFocusX", DxScript::Func_SetCameraFocusX, 1},
	{"SetCameraFocusY", DxScript::Func_SetCameraFocusY, 1},
	{"SetCameraFocusZ", DxScript::Func_SetCameraFocusZ, 1},
	{"SetCameraFocusXYZ", DxScript::Func_SetCameraFocusXYZ, 3},
	{"SetCameraRadius", DxScript::Func_SetCameraRadius, 1},
	{"SetCameraAzimuthAngle", DxScript::Func_SetCameraAzimuthAngle, 1},
	{"SetCameraElevationAngle", DxScript::Func_SetCameraElevationAngle, 1},
	{"SetCameraYaw", DxScript::Func_SetCameraYaw, 1},
	{"SetCameraPitch", DxScript::Func_SetCameraPitch, 1},
	{"SetCameraRoll", DxScript::Func_SetCameraRoll, 1},
	{"GetCameraX", DxScript::Func_GetCameraX, 0},
	{"GetCameraY", DxScript::Func_GetCameraY, 0},
	{"GetCameraZ", DxScript::Func_GetCameraZ, 0},
	{"GetCameraFocusX", DxScript::Func_GetCameraFocusX, 0},
	{"GetCameraFocusY", DxScript::Func_GetCameraFocusY, 0},
	{"GetCameraFocusZ", DxScript::Func_GetCameraFocusZ, 0},
	{"GetCameraRadius", DxScript::Func_GetCameraRadius, 0},
	{"GetCameraAzimuthAngle", DxScript::Func_GetCameraAzimuthAngle, 0},
	{"GetCameraElevationAngle", DxScript::Func_GetCameraElevationAngle, 0},
	{"GetCameraYaw", DxScript::Func_GetCameraYaw, 0},
	{"GetCameraPitch", DxScript::Func_GetCameraPitch, 0},
	{"GetCameraRoll", DxScript::Func_GetCameraRoll, 0},
	{"SetCameraPerspectiveClip", DxScript::Func_SetCameraPerspectiveClip, 2},

	//Dx関数：カメラ2D
	{"Set2DCameraFocusX", DxScript::Func_Set2DCameraFocusX, 1},
	{"Set2DCameraFocusY", DxScript::Func_Set2DCameraFocusY, 1},
	{"Set2DCameraAngleZ", DxScript::Func_Set2DCameraAngleZ, 1},
	{"Set2DCameraRatio", DxScript::Func_Set2DCameraRatio, 1},
	{"Set2DCameraRatioX", DxScript::Func_Set2DCameraRatioX, 1},
	{"Set2DCameraRatioY", DxScript::Func_Set2DCameraRatioY, 1},
	{"Reset2DCamera", DxScript::Func_Reset2DCamera, 0},
	{"Get2DCameraX", DxScript::Func_Get2DCameraX, 0},
	{"Get2DCameraY", DxScript::Func_Get2DCameraY, 0},
	{"Get2DCameraAngleZ", DxScript::Func_Get2DCameraAngleZ, 0},
	{"Get2DCameraRatio", DxScript::Func_Get2DCameraRatio, 0},
	{"Get2DCameraRatioX", DxScript::Func_Get2DCameraRatioX, 0},
	{"Get2DCameraRatioY", DxScript::Func_Get2DCameraRatioY, 0},

	//Dx関数：その他
	{"GetObjectDistance", DxScript::Func_GetObjectDistance, 2},
	{"GetObject2dPosition", DxScript::Func_GetObject2dPosition, 1},
	{"Get2dPosition", DxScript::Func_Get2dPosition, 3},

	//Dx関数：オブジェクト操作(共通)
	{"Obj_Delete", DxScript::Func_Obj_Delete, 1},
	{"Obj_IsDeleted", DxScript::Func_Obj_IsDeleted, 1},
	{"Obj_SetVisible", DxScript::Func_Obj_SetVisible, 2},
	{"Obj_IsVisible", DxScript::Func_Obj_IsVisible, 1},
	{"Obj_SetRenderPriority", DxScript::Func_Obj_SetRenderPriority, 2},
	{"Obj_SetRenderPriorityI", DxScript::Func_Obj_SetRenderPriorityI, 2},
	{"Obj_GetRenderPriority", DxScript::Func_Obj_GetRenderPriority, 1},
	{"Obj_GetRenderPriorityI", DxScript::Func_Obj_GetRenderPriorityI, 1},
	{"Obj_GetValue", DxScript::Func_Obj_GetValue, 2},
	{"Obj_GetValueD", DxScript::Func_Obj_GetValueD, 3},
	{"Obj_SetValue", DxScript::Func_Obj_SetValue, 3},
	{"Obj_DeleteValue", DxScript::Func_Obj_DeleteValue, 2},
	{"Obj_IsValueExists", DxScript::Func_Obj_IsValueExists, 2},
	{"Obj_GetType", DxScript::Func_Obj_GetType, 1},

	//Dx関数：オブジェクト操作(RenderObject)
	{"ObjRender_SetX", DxScript::Func_ObjRender_SetX, 2},
	{"ObjRender_SetY", DxScript::Func_ObjRender_SetY, 2},
	{"ObjRender_SetZ", DxScript::Func_ObjRender_SetZ, 2},
	{"ObjRender_SetPosition", DxScript::Func_ObjRender_SetPosition, 4},
	{"ObjRender_SetAngleX", DxScript::Func_ObjRender_SetAngleX, 2},
	{"ObjRender_SetAngleY", DxScript::Func_ObjRender_SetAngleY, 2},
	{"ObjRender_SetAngleZ", DxScript::Func_ObjRender_SetAngleZ, 2},
	{"ObjRender_SetAngleXYZ", DxScript::Func_ObjRender_SetAngleXYZ, 4},
	{"ObjRender_SetScaleX", DxScript::Func_ObjRender_SetScaleX, 2},
	{"ObjRender_SetScaleY", DxScript::Func_ObjRender_SetScaleY, 2},
	{"ObjRender_SetScaleZ", DxScript::Func_ObjRender_SetScaleZ, 2},
	{"ObjRender_SetScaleXYZ", DxScript::Func_ObjRender_SetScaleXYZ, 4},
	{"ObjRender_SetColor", DxScript::Func_ObjRender_SetColor, 4},
	{"ObjRender_SetColorHSV", DxScript::Func_ObjRender_SetColorHSV, 4},
	{"ObjRender_SetAlpha", DxScript::Func_ObjRender_SetAlpha, 2},
	{"ObjRender_SetBlendType", DxScript::Func_ObjRender_SetBlendType, 2},
	{"ObjRender_GetX", DxScript::Func_ObjRender_GetX, 1},
	{"ObjRender_GetY", DxScript::Func_ObjRender_GetY, 1},
	{"ObjRender_GetZ", DxScript::Func_ObjRender_GetZ, 1},
	{"ObjRender_GetAngleX", DxScript::Func_ObjRender_GetAngleX, 1},
	{"ObjRender_GetAngleY", DxScript::Func_ObjRender_GetAngleY, 1},
	{"ObjRender_GetAngleZ", DxScript::Func_ObjRender_GetAngleZ, 1},
	{"ObjRender_GetScaleX", DxScript::Func_ObjRender_GetScaleX, 1},
	{"ObjRender_GetScaleY", DxScript::Func_ObjRender_GetScaleY, 1},
	{"ObjRender_GetScaleZ", DxScript::Func_ObjRender_GetScaleZ, 1},
	{"ObjRender_SetZWrite", DxScript::Func_ObjRender_SetZWrite, 2},
	{"ObjRender_SetZTest", DxScript::Func_ObjRender_SetZTest, 2},
	{"ObjRender_SetFogEnable", DxScript::Func_ObjRender_SetFogEnable, 2},
	{"ObjRender_SetCullingMode", DxScript::Func_ObjRender_SetCullingMode, 2},
	{"ObjRender_SetRalativeObject", DxScript::Func_ObjRender_SetRalativeObject, 3},
	{"ObjRender_SetPermitCamera", DxScript::Func_ObjRender_SetPermitCamera, 2},
	{"ObjRender_GetBlendType", DxScript::Func_ObjRender_GetBlendType, 1},

	//Dx関数：オブジェクト操作(ShaderObject)
	{"ObjShader_Create", DxScript::Func_ObjShader_Create, 0},
	{"ObjShader_SetShaderF", DxScript::Func_ObjShader_SetShaderF, 2},
	{"ObjShader_SetShaderO", DxScript::Func_ObjShader_SetShaderO, 2},
	{"ObjShader_ResetShader", DxScript::Func_ObjShader_ResetShader, 1},
	{"ObjShader_SetTechnique", DxScript::Func_ObjShader_SetTechnique, 2},
	{"ObjShader_SetMatrix", DxScript::Func_ObjShader_SetMatrix, 3},
	{"ObjShader_SetMatrixArray", DxScript::Func_ObjShader_SetMatrixArray, 3},
	{"ObjShader_SetVector", DxScript::Func_ObjShader_SetVector, 6},
	{"ObjShader_SetFloat", DxScript::Func_ObjShader_SetFloat, 3},
	{"ObjShader_SetFloatArray", DxScript::Func_ObjShader_SetFloatArray, 3},
	{"ObjShader_SetTexture", DxScript::Func_ObjShader_SetTexture, 3},

	//Dx関数：オブジェクト操作(PrimitiveObject)
	{"ObjPrim_Create", DxScript::Func_ObjPrimitive_Create, 1},
	{"ObjPrim_SetPrimitiveType", DxScript::Func_ObjPrimitive_SetPrimitiveType, 2},
	{"ObjPrim_SetVertexCount", DxScript::Func_ObjPrimitive_SetVertexCount, 2},
	{"ObjPrim_SetTexture", DxScript::Func_ObjPrimitive_SetTexture, 2},
	{"ObjPrim_GetVertexCount", DxScript::Func_ObjPrimitive_GetVertexCount, 1},
	{"ObjPrim_SetVertexPosition", DxScript::Func_ObjPrimitive_SetVertexPosition, 5},
	{"ObjPrim_SetVertexUV", DxScript::Func_ObjPrimitive_SetVertexUV, 4},
	{"ObjPrim_SetVertexUVT", DxScript::Func_ObjPrimitive_SetVertexUVT, 4},
	{"ObjPrim_SetVertexColor", DxScript::Func_ObjPrimitive_SetVertexColor, 5},
	{"ObjPrim_SetVertexAlpha", DxScript::Func_ObjPrimitive_SetVertexAlpha, 3},
	{"ObjPrim_GetVertexPosition", DxScript::Func_ObjPrimitive_GetVertexPosition, 2},

	//Dx関数：オブジェクト操作(Sprite2D)
	{"ObjSprite2D_SetSourceRect", DxScript::Func_ObjSprite2D_SetSourceRect, 5},
	{"ObjSprite2D_SetDestRect", DxScript::Func_ObjSprite2D_SetDestRect, 5},
	{"ObjSprite2D_SetDestCenter", DxScript::Func_ObjSprite2D_SetDestCenter, 1},

	//Dx関数：オブジェクト操作(SpriteList2D)
	{"ObjSpriteList2D_SetSourceRect", DxScript::Func_ObjSpriteList2D_SetSourceRect, 5},
	{"ObjSpriteList2D_SetDestRect", DxScript::Func_ObjSpriteList2D_SetDestRect, 5},
	{"ObjSpriteList2D_SetDestCenter", DxScript::Func_ObjSpriteList2D_SetDestCenter, 1},
	{"ObjSpriteList2D_AddVertex", DxScript::Func_ObjSpriteList2D_AddVertex, 1},
	{"ObjSpriteList2D_CloseVertex", DxScript::Func_ObjSpriteList2D_CloseVertex, 1},
	{"ObjSpriteList2D_ClearVertexCount", DxScript::Func_ObjSpriteList2D_ClearVertexCount, 1},

	//Dx関数：オブジェクト操作(Sprite3D)
	{"ObjSprite3D_SetSourceRect", DxScript::Func_ObjSprite3D_SetSourceRect, 5},
	{"ObjSprite3D_SetDestRect", DxScript::Func_ObjSprite3D_SetDestRect, 5},
	{"ObjSprite3D_SetSourceDestRect", DxScript::Func_ObjSprite3D_SetSourceDestRect, 5},
	{"ObjSprite3D_SetBillboard", DxScript::Func_ObjSprite3D_SetBillboard, 2},

	//Dx関数：オブジェクト操作(TrajectoryObject3D)
	{"ObjTrajectory3D_SetInitialPoint", DxScript::Func_ObjTrajectory3D_SetInitialPoint, 7},
	{"ObjTrajectory3D_SetAlphaVariation", DxScript::Func_ObjTrajectory3D_SetAlphaVariation, 2},
	{"ObjTrajectory3D_SetComplementCount", DxScript::Func_ObjTrajectory3D_SetComplementCount, 2},

	//Dx関数：オブジェクト操作(DxMesh)
	{"ObjMesh_Create", DxScript::Func_ObjMesh_Create, 0},
	{"ObjMesh_Load", DxScript::Func_ObjMesh_Load, 2},
	{"ObjMesh_SetColor", DxScript::Func_ObjMesh_SetColor, 4},
	{"ObjMesh_SetAlpha", DxScript::Func_ObjMesh_SetAlpha, 2},
	{"ObjMesh_SetAnimation", DxScript::Func_ObjMesh_SetAnimation, 3},
	{"ObjMesh_SetCoordinate2D", DxScript::Func_ObjMesh_SetCoordinate2D, 2},
	{"ObjMesh_GetPath", DxScript::Func_ObjMesh_GetPath, 1},

	//Dx関数：テキスト操作(DxText)
	{"ObjText_Create", DxScript::Func_ObjText_Create, 0},
	{"ObjText_SetText", DxScript::Func_ObjText_SetText, 2},
	{"ObjText_SetFontType", DxScript::Func_ObjText_SetFontType, 2},
	{"ObjText_SetFontSize", DxScript::Func_ObjText_SetFontSize, 2},
	{"ObjText_SetFontBold", DxScript::Func_ObjText_SetFontBold, 2},
	{"ObjText_SetFontColorTop", DxScript::Func_ObjText_SetFontColorTop, 4},
	{"ObjText_SetFontColorBottom", DxScript::Func_ObjText_SetFontColorBottom, 4},
	{"ObjText_SetFontBorderWidth", DxScript::Func_ObjText_SetFontBorderWidth, 2},
	{"ObjText_SetFontBorderType", DxScript::Func_ObjText_SetFontBorderType, 2},
	{"ObjText_SetFontBorderColor", DxScript::Func_ObjText_SetFontBorderColor, 4},
	{"ObjText_SetMaxWidth", DxScript::Func_ObjText_SetMaxWidth, 2},
	{"ObjText_SetMaxHeight", DxScript::Func_ObjText_SetMaxHeight, 2},
	{"ObjText_SetLinePitch", DxScript::Func_ObjText_SetLinePitch, 2},
	{"ObjText_SetSidePitch", DxScript::Func_ObjText_SetSidePitch, 2},
	{"ObjText_SetVertexColor", DxScript::Func_ObjText_SetVertexColor, 5},
	{"ObjText_SetTransCenter", DxScript::Func_ObjText_SetTransCenter, 3},
	{"ObjText_SetAutoTransCenter", DxScript::Func_ObjText_SetAutoTransCenter, 2},
	{"ObjText_SetHorizontalAlignment", DxScript::Func_ObjText_SetHorizontalAlignment, 2},
	{"ObjText_SetSyntacticAnalysis", DxScript::Func_ObjText_SetSyntacticAnalysis, 2},
	{"ObjText_GetTextLength", DxScript::Func_ObjText_GetTextLength, 1},
	{"ObjText_GetTextLengthCU", DxScript::Func_ObjText_GetTextLengthCU, 1},
	{"ObjText_GetTextLengthCUL", DxScript::Func_ObjText_GetTextLengthCUL, 1},
	{"ObjText_GetTotalWidth", DxScript::Func_ObjText_GetTotalWidth, 1},
	{"ObjText_GetTotalHeight", DxScript::Func_ObjText_GetTotalHeight, 1},

	//Dx関数：音声操作(DxSoundObject)
	{"ObjSound_Create", DxScript::Func_ObjSound_Create, 0},
	{"ObjSound_Load", DxScript::Func_ObjSound_Load, 2},
	{"ObjSound_Play", DxScript::Func_ObjSound_Play, 1},
	{"ObjSound_Stop", DxScript::Func_ObjSound_Stop, 1},
	{"ObjSound_SetVolumeRate", DxScript::Func_ObjSound_SetVolumeRate, 2},
	{"ObjSound_SetPanRate", DxScript::Func_ObjSound_SetPanRate, 2},
	{"ObjSound_SetFade", DxScript::Func_ObjSound_SetFade, 2},
	{"ObjSound_SetLoopEnable", DxScript::Func_ObjSound_SetLoopEnable, 2},
	{"ObjSound_SetLoopTime", DxScript::Func_ObjSound_SetLoopTime, 3},
	{"ObjSound_SetLoopSampleCount", DxScript::Func_ObjSound_SetLoopSampleCount, 3},
	{"ObjSound_SetRestartEnable", DxScript::Func_ObjSound_SetRestartEnable, 2},
	{"ObjSound_SetSoundDivision", DxScript::Func_ObjSound_SetSoundDivision, 2},
	{"ObjSound_IsPlaying", DxScript::Func_ObjSound_IsPlaying, 1},
	{"ObjSound_GetVolumeRate", DxScript::Func_ObjSound_GetVolumeRate, 1},

	//Dx関数：ファイル操作(DxFileObject)
	{"ObjFile_Create", DxScript::Func_ObjFile_Create, 1},
	{"ObjFile_Open", DxScript::Func_ObjFile_Open, 2},
	{"ObjFile_OpenNW", DxScript::Func_ObjFile_OpenNW, 2},
	{"ObjFile_Store", DxScript::Func_ObjFile_Store, 1},
	{"ObjFile_GetSize", DxScript::Func_ObjFile_GetSize, 1},

	//Dx関数：ファイル操作(DxTextFileObject)
	{"ObjFileT_GetLineCount", DxScript::Func_ObjFileT_GetLineCount, 1},
	{"ObjFileT_GetLineText", DxScript::Func_ObjFileT_GetLineText, 2},
	{"ObjFileT_SplitLineText", DxScript::Func_ObjFileT_SplitLineText, 3},
	{"ObjFileT_AddLine", DxScript::Func_ObjFileT_AddLine, 2},
	{"ObjFileT_ClearLine", DxScript::Func_ObjFileT_ClearLine, 1},

	////Dx関数：ファイル操作(DxBinalyFileObject)
	{"ObjFileB_SetByteOrder", DxScript::Func_ObjFileB_SetByteOrder, 2},
	{"ObjFileB_SetCharacterCode", DxScript::Func_ObjFileB_SetCharacterCode, 2},
	{"ObjFileB_GetPointer", DxScript::Func_ObjFileB_GetPointer, 1},
	{"ObjFileB_Seek", DxScript::Func_ObjFileB_Seek, 2},
	{"ObjFileB_ReadBoolean", DxScript::Func_ObjFileB_ReadBoolean, 1},
	{"ObjFileB_ReadByte", DxScript::Func_ObjFileB_ReadByte, 1},
	{"ObjFileB_ReadShort", DxScript::Func_ObjFileB_ReadShort, 1},
	{"ObjFileB_ReadInteger", DxScript::Func_ObjFileB_ReadInteger, 1},
	{"ObjFileB_ReadLong", DxScript::Func_ObjFileB_ReadLong, 1},
	{"ObjFileB_ReadFloat", DxScript::Func_ObjFileB_ReadFloat, 1},
	{"ObjFileB_ReadDouble", DxScript::Func_ObjFileB_ReadDouble, 1},
	{"ObjFileB_ReadString", DxScript::Func_ObjFileB_ReadString, 2},

	//定数
	{"ID_INVALID",constant<DxScript::ID_INVALID>::func,0},
	{"OBJ_PRIMITIVE_2D",constant<DxScript::OBJ_PRIMITIVE_2D>::func,0},
	{"OBJ_SPRITE_2D",constant<DxScript::OBJ_SPRITE_2D>::func,0},
	{"OBJ_SPRITE_LIST_2D",constant<DxScript::OBJ_SPRITE_LIST_2D>::func,0},
	{"OBJ_PRIMITIVE_3D",constant<DxScript::OBJ_PRIMITIVE_3D>::func,0},
	{"OBJ_SPRITE_3D",constant<DxScript::OBJ_SPRITE_3D>::func,0},
	{"OBJ_TRAJECTORY_3D",constant<DxScript::OBJ_TRAJECTORY_3D>::func,0},
	{"OBJ_SHADER",constant<DxScript::OBJ_SHADER>::func,0},
	{"OBJ_MESH",constant<DxScript::OBJ_MESH>::func,0},
	{"OBJ_TEXT",constant<DxScript::OBJ_TEXT>::func,0},
	{"OBJ_SOUND",constant<DxScript::OBJ_SOUND>::func,0},
	{"OBJ_FILE_TEXT",constant<DxScript::OBJ_FILE_TEXT>::func,0},
	{"OBJ_FILE_BINARY",constant<DxScript::OBJ_FILE_BINARY>::func,0},

	{"BLEND_NONE",constant<DirectGraphics::MODE_BLEND_NONE>::func,0},
	{"BLEND_ALPHA",constant<DirectGraphics::MODE_BLEND_ALPHA>::func,0},
	{"BLEND_ADD_RGB",constant<DirectGraphics::MODE_BLEND_ADD_RGB>::func,0},
	{"BLEND_ADD_ARGB",constant<DirectGraphics::MODE_BLEND_ADD_ARGB>::func,0},
	{"BLEND_MULTIPLY",constant<DirectGraphics::MODE_BLEND_MULTIPLY>::func,0},
	{"BLEND_SUBTRACT",constant<DirectGraphics::MODE_BLEND_SUBTRACT>::func,0},
	{"BLEND_SHADOW",constant<DirectGraphics::MODE_BLEND_SHADOW>::func,0},
	{"BLEND_INV_DESTRGB",constant<DirectGraphics::MODE_BLEND_INV_DESTRGB>::func,0},
	{"CULL_NONE",constant<D3DCULL_NONE>::func,0},
	{"CULL_CW",constant<D3DCULL_CW>::func,0},
	{"CULL_CCW",constant<D3DCULL_CCW>::func,0},

	{"PRIMITIVE_POINT_LIST",constant<D3DPT_POINTLIST>::func,0},
	{"PRIMITIVE_LINELIST",constant<D3DPT_LINELIST>::func,0},
	{"PRIMITIVE_LINESTRIP",constant<D3DPT_LINESTRIP>::func,0},
	{"PRIMITIVE_TRIANGLELIST",constant<D3DPT_TRIANGLELIST>::func,0},
	{"PRIMITIVE_TRIANGLESTRIP",constant<D3DPT_TRIANGLESTRIP>::func,0},
	{"PRIMITIVE_TRIANGLEFAN",constant<D3DPT_TRIANGLEFAN>::func,0},

	{"BORDER_NONE",constant<DxFont::BORDER_NONE>::func,0},
	{"BORDER_FULL",constant<DxFont::BORDER_FULL>::func,0},
	{"BORDER_SHADOW",constant<DxFont::BORDER_SHADOW>::func,0},

	{"SOUND_BGM",constant<SoundDivision::DIVISION_BGM>::func,0},
	{"SOUND_SE",constant<SoundDivision::DIVISION_SE>::func,0},
	{"SOUND_VOICE",constant<SoundDivision::DIVISION_VOICE>::func,0},

	{"ALIGNMENT_LEFT",constant<DxText::ALIGNMENT_LEFT>::func,0},
	{"ALIGNMENT_RIGHT",constant<DxText::ALIGNMENT_RIGHT>::func,0},
	{"ALIGNMENT_CENTER",constant<DxText::ALIGNMENT_CENTER>::func,0},

	{"CODE_ACP",constant<DxScript::CODE_ACP>::func,0},
	{"CODE_UTF8",constant<DxScript::CODE_UTF8>::func,0},
	{"CODE_UTF16LE",constant<DxScript::CODE_UTF16LE>::func,0},
	{"CODE_UTF16BE",constant<DxScript::CODE_UTF16BE>::func,0},

	{"ENDIAN_LITTLE",constant<ByteOrder::ENDIAN_LITTLE>::func,0},
	{"ENDIAN_BIG",constant<ByteOrder::ENDIAN_BIG>::func,0},

	//DirectInput
	{"KEY_FREE",constant<KEY_FREE>::func,0},
	{"KEY_PUSH",constant<KEY_PUSH>::func,0},
	{"KEY_PULL",constant<KEY_PULL>::func,0},
	{"KEY_HOLD",constant<KEY_HOLD>::func,0},

	{"MOUSE_LEFT",constant<DI_MOUSE_LEFT>::func,0},
	{"MOUSE_RIGHT",constant<DI_MOUSE_RIGHT>::func,0},
	{"MOUSE_MIDDLE",constant<DI_MOUSE_MIDDLE>::func,0},

	{"KEY_ESCAPE",constant<DIK_ESCAPE>::func,0},
	{"KEY_1",constant<DIK_1>::func,0},
	{"KEY_2",constant<DIK_2>::func,0},
	{"KEY_3",constant<DIK_3>::func,0},
	{"KEY_4",constant<DIK_4>::func,0},
	{"KEY_5",constant<DIK_5>::func,0},
	{"KEY_6",constant<DIK_6>::func,0},
	{"KEY_7",constant<DIK_7>::func,0},
	{"KEY_8",constant<DIK_8>::func,0},
	{"KEY_9",constant<DIK_9>::func,0},
	{"KEY_0",constant<DIK_0>::func,0},
	{"KEY_MINUS",constant<DIK_MINUS>::func,0},
	{"KEY_EQUALS",constant<DIK_EQUALS>::func,0},
	{"KEY_BACK",constant<DIK_BACK>::func,0},
	{"KEY_TAB",constant<DIK_TAB>::func,0},
	{"KEY_Q",constant<DIK_Q>::func,0},
	{"KEY_W",constant<DIK_W>::func,0},
	{"KEY_E",constant<DIK_E>::func,0},
	{"KEY_R",constant<DIK_R>::func,0},
	{"KEY_T",constant<DIK_T>::func,0},
	{"KEY_Y",constant<DIK_Y>::func,0},
	{"KEY_U",constant<DIK_U>::func,0},
	{"KEY_I",constant<DIK_I>::func,0},
	{"KEY_O",constant<DIK_O>::func,0},
	{"KEY_P",constant<DIK_P>::func,0},
	{"KEY_LBRACKET",constant<DIK_LBRACKET>::func,0},
	{"KEY_RBRACKET",constant<DIK_RBRACKET>::func,0},
	{"KEY_RETURN",constant<DIK_RETURN>::func,0},
	{"KEY_LCONTROL",constant<DIK_LCONTROL>::func,0},
	{"KEY_A",constant<DIK_A>::func,0},
	{"KEY_S",constant<DIK_S>::func,0},
	{"KEY_D",constant<DIK_D>::func,0},
	{"KEY_F",constant<DIK_F>::func,0},
	{"KEY_G",constant<DIK_G>::func,0},
	{"KEY_H",constant<DIK_H>::func,0},
	{"KEY_J",constant<DIK_J>::func,0},
	{"KEY_K",constant<DIK_K>::func,0},
	{"KEY_L",constant<DIK_L>::func,0},
	{"KEY_SEMICOLON",constant<DIK_SEMICOLON>::func,0},
	{"KEY_APOSTROPHE",constant<DIK_APOSTROPHE>::func,0},
	{"KEY_GRAVE",constant<DIK_GRAVE>::func,0},
	{"KEY_LSHIFT",constant<DIK_LSHIFT>::func,0},
	{"KEY_BACKSLASH",constant<DIK_BACKSLASH>::func,0},
	{"KEY_Z",constant<DIK_Z>::func,0},
	{"KEY_X",constant<DIK_X>::func,0},
	{"KEY_C",constant<DIK_C>::func,0},
	{"KEY_V",constant<DIK_V>::func,0},
	{"KEY_B",constant<DIK_B>::func,0},
	{"KEY_N",constant<DIK_N>::func,0},
	{"KEY_M",constant<DIK_M>::func,0},
	{"KEY_COMMA",constant<DIK_COMMA>::func,0},
	{"KEY_PERIOD",constant<DIK_PERIOD>::func,0},
	{"KEY_SLASH",constant<DIK_SLASH>::func,0},
	{"KEY_RSHIFT",constant<DIK_RSHIFT>::func,0},
	{"KEY_MULTIPLY",constant<DIK_MULTIPLY>::func,0},
	{"KEY_LMENU",constant<DIK_LMENU>::func,0},
	{"KEY_SPACE",constant<DIK_SPACE>::func,0},
	{"KEY_CAPITAL",constant<DIK_CAPITAL>::func,0},
	{"KEY_F1",constant<DIK_F1>::func,0},
	{"KEY_F2",constant<DIK_F2>::func,0},
	{"KEY_F3",constant<DIK_F3>::func,0},
	{"KEY_F4",constant<DIK_F4>::func,0},
	{"KEY_F5",constant<DIK_F5>::func,0},
	{"KEY_F6",constant<DIK_F6>::func,0},
	{"KEY_F7",constant<DIK_F7>::func,0},
	{"KEY_F8",constant<DIK_F8>::func,0},
	{"KEY_F9",constant<DIK_F9>::func,0},
	{"KEY_F10",constant<DIK_F10>::func,0},
	{"KEY_NUMLOCK",constant<DIK_NUMLOCK>::func,0},
	{"KEY_SCROLL",constant<DIK_SCROLL>::func,0},
	{"KEY_NUMPAD7",constant<DIK_NUMPAD7>::func,0},
	{"KEY_NUMPAD8",constant<DIK_NUMPAD8>::func,0},
	{"KEY_NUMPAD9",constant<DIK_NUMPAD9>::func,0},
	{"KEY_SUBTRACT",constant<DIK_SUBTRACT>::func,0},
	{"KEY_NUMPAD4",constant<DIK_NUMPAD4>::func,0},
	{"KEY_NUMPAD5",constant<DIK_NUMPAD5>::func,0},
	{"KEY_NUMPAD6",constant<DIK_NUMPAD6>::func,0},
	{"KEY_ADD",constant<DIK_ADD>::func,0},
	{"KEY_NUMPAD1",constant<DIK_NUMPAD1>::func,0},
	{"KEY_NUMPAD2",constant<DIK_NUMPAD2>::func,0},
	{"KEY_NUMPAD3",constant<DIK_NUMPAD3>::func,0},
	{"KEY_NUMPAD0",constant<DIK_NUMPAD0>::func,0},
	{"KEY_DECIMAL",constant<DIK_DECIMAL>::func,0},
	{"KEY_F11",constant<DIK_F11>::func,0},
	{"KEY_F12",constant<DIK_F12>::func,0},
	{"KEY_F13",constant<DIK_F13>::func,0},
	{"KEY_F14",constant<DIK_F14>::func,0},
	{"KEY_F15",constant<DIK_F15>::func,0},
	{"KEY_KANA",constant<DIK_KANA>::func,0},
	{"KEY_CONVERT",constant<DIK_CONVERT>::func,0},
	{"KEY_NOCONVERT",constant<DIK_NOCONVERT>::func,0},
	{"KEY_YEN",constant<DIK_YEN>::func,0},
	{"KEY_NUMPADEQUALS",constant<DIK_NUMPADEQUALS>::func,0},
	{"KEY_CIRCUMFLEX",constant<DIK_CIRCUMFLEX>::func,0},
	{"KEY_AT",constant<DIK_AT>::func,0},
	{"KEY_COLON",constant<DIK_COLON>::func,0},
	{"KEY_UNDERLINE",constant<DIK_UNDERLINE>::func,0},
	{"KEY_KANJI",constant<DIK_KANJI>::func,0},
	{"KEY_STOP",constant<DIK_STOP>::func,0},
	{"KEY_AX",constant<DIK_AX>::func,0},
	{"KEY_UNLABELED",constant<DIK_UNLABELED>::func,0},
	{"KEY_NUMPADENTER",constant<DIK_NUMPADENTER>::func,0},
	{"KEY_RCONTROL",constant<DIK_RCONTROL>::func,0},
	{"KEY_NUMPADCOMMA",constant<DIK_NUMPADCOMMA>::func,0},
	{"KEY_DIVIDE",constant<DIK_DIVIDE>::func,0},
	{"KEY_SYSRQ",constant<DIK_SYSRQ>::func,0},
	{"KEY_RMENU",constant<DIK_RMENU>::func,0},
	{"KEY_PAUSE",constant<DIK_PAUSE>::func,0},
	{"KEY_HOME",constant<DIK_HOME>::func,0},
	{"KEY_UP",constant<DIK_UP>::func,0},
	{"KEY_PRIOR",constant<DIK_PRIOR>::func,0},
	{"KEY_LEFT",constant<DIK_LEFT>::func,0},
	{"KEY_RIGHT",constant<DIK_RIGHT>::func,0},
	{"KEY_END",constant<DIK_END>::func,0},
	{"KEY_DOWN",constant<DIK_DOWN>::func,0},
	{"KEY_NEXT",constant<DIK_NEXT>::func,0},
	{"KEY_INSERT",constant<DIK_INSERT>::func,0},
	{"KEY_DELETE",constant<DIK_DELETE>::func,0},
	{"KEY_LWIN",constant<DIK_LWIN>::func,0},
	{"KEY_RWIN",constant<DIK_RWIN>::func,0},
	{"KEY_APPS",constant<DIK_APPS>::func,0},
	{"KEY_POWER",constant<DIK_POWER>::func,0},
	{"KEY_SLEEP",constant<DIK_SLEEP>::func,0},

};


DxScript::DxScript()
{
	_AddFunction(dxFunction, sizeof(dxFunction) / sizeof(function));
	objManager_ = new DxScriptObjectManager();
}
DxScript::~DxScript()
{
	_ClearResource();
}
void DxScript::_ClearResource()
{
	mapTexture_.clear();
	mapMesh_.clear();

	std::map<std::wstring, gstd::ref_count_ptr<SoundPlayer> >::iterator itrSound;
	for(itrSound=mapSoundPlayer_.begin(); itrSound!=mapSoundPlayer_.end(); itrSound++)
	{
		SoundPlayer* player = (itrSound->second).GetPointer();
		player->Delete();
	}
	mapSoundPlayer_.clear();
}
int DxScript::AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate)
{
	obj->idScript_ = idScript_;
	return objManager_->AddObject(obj, bActivate);
}
gstd::ref_count_ptr<Texture> DxScript::_GetTexture(std::wstring name)
{
	gstd::ref_count_ptr<Texture> res;
	if(mapTexture_.find(name) != mapTexture_.end())
	{
		res = mapTexture_[name];
	}
	return res;
}

//Dx関数：システム系系
gstd::value DxScript::Func_InstallFont(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	DxTextRenderer* renderer = DxTextRenderer::GetBase();
	bool res = false;
	try 
	{
		res = renderer->AddFontFromFile(path);
	}
	catch(gstd::wexception e)
	{
		Logger::WriteTop(e.what());
	}

	return value(machine->get_engine()->get_boolean_type(), res);
}

//Dx関数：音声系
value DxScript::Func_LoadSound(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapSoundPlayer_.find(path) != script->mapSoundPlayer_.end())
			return value(machine->get_engine()->get_boolean_type(), true);

	ref_count_ptr<SoundPlayer> player = manager->GetPlayer(path, true);
	if(player != NULL)
	{
		script->mapSoundPlayer_[path] = player;
	}
	return value(machine->get_engine()->get_boolean_type(), player != NULL);
}
value DxScript::Func_RemoveSound(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapSoundPlayer_.find(path) == script->mapSoundPlayer_.end())return value(); 

	ref_count_ptr<SoundPlayer> player = script->mapSoundPlayer_[path];
	player->Delete();
	script->mapSoundPlayer_.erase(path);
	
	return value(); 
}
value DxScript::Func_PlayBGM(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapSoundPlayer_.find(path) == script->mapSoundPlayer_.end())return value();

	double loopStart = argv[1].as_real();
	double loopEnd = argv[2].as_real();

	ref_count_ptr<SoundPlayer> player = script->mapSoundPlayer_[path];
	player->SetSoundDivision(SoundDivision::DIVISION_BGM);

	SoundPlayer::PlayStyle style;
	style.SetLoopEnable(true);
	style.SetLoopStartTime(loopStart);
	style.SetLoopEndTime(loopEnd);
	//player->Play(style);
	script->GetObjectManager()->ReserveSound(player, style);

	return value();
}
gstd::value DxScript::Func_PlaySE(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapSoundPlayer_.find(path) == script->mapSoundPlayer_.end())return value();

	ref_count_ptr<SoundPlayer> player = script->mapSoundPlayer_[path];
	player->SetSoundDivision(SoundDivision::DIVISION_SE);

	SoundPlayer::PlayStyle style;
	style.SetLoopEnable(false);
	//player->Play(style);
	script->GetObjectManager()->ReserveSound(player, style);
	return value();
}

value DxScript::Func_StopSound(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapSoundPlayer_.find(path) == script->mapSoundPlayer_.end())return value();

	ref_count_ptr<SoundPlayer> player = script->mapSoundPlayer_[path];
	player->Stop();
	script->GetObjectManager()->DeleteReservedSound(player);

	return value();
}

//Dx関数：キー系
gstd::value DxScript::Func_GetKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectInput* input = DirectInput::GetBase();
	int key = (int)argv[0].as_real();
	long double res = input->GetKeyState(key);
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetMouseX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetMousePosition().x;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetMouseY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetMousePosition().y;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetMouseMoveZ(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectInput* input = DirectInput::GetBase();
	long double res = input->GetMouseMoveZ();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetMouseState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectInput* input = DirectInput::GetBase();
	long double res = input->GetMouseState(argv[0].as_real());
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetVirtualKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = KEY_FREE;
	VirtualKeyManager* input = dynamic_cast<VirtualKeyManager*>(DirectInput::GetBase());
	if(input != NULL)
	{
		int id = (int)(argv[0].as_real());
		res = input->GetVirtualKeyState(id);
	}

	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_SetVirtualKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	VirtualKeyManager* input = dynamic_cast<VirtualKeyManager*>(DirectInput::GetBase());
	if(input != NULL)
	{
		int id = (int)(argv[0].as_real());
		int state = (int)(argv[1].as_real());
		ref_count_ptr<VirtualKey> vkey = input->GetVirtualKey(id);
		if(vkey != NULL)
		{
			vkey->SetKeyState(state);
		}
	}

	return value();
}
//Dx関数：描画系
gstd::value DxScript::Func_GetScreenWidth(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int res = graphics->GetScreenWidth();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value DxScript::Func_GetScreenHeight(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	int res = graphics->GetScreenHeight();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value DxScript::Func_LoadTexture(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	bool res = true;
	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	if(script->mapTexture_.find(path) == script->mapTexture_.end())
	{
		ref_count_ptr<Texture> texture = new Texture();
		bool res = texture->CreateFromFile(path);
		if(res)
		{
			Lock lock(script->criticalSection_);
			script->mapTexture_[path] = texture;
		}
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_LoadTextureInLoadThread(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	bool res = true;
	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);

	if(script->mapTexture_.find(path) == script->mapTexture_.end())
	{
		ref_count_ptr<Texture> texture = new Texture();
		bool res = texture->CreateFromFileInLoadThread(path);
		if(res)
		{
			Lock lock(script->criticalSection_);
			script->mapTexture_[path] = texture;
		}
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_RemoveTexture(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	{
		Lock lock(script->criticalSection_);
		script->mapTexture_.erase(path);
	}
	return value();
}
value DxScript::Func_GetTextureWidth(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	TextureManager* textureManager = TextureManager::GetBase();

	gstd::ref_count_ptr<TextureData> textureData = textureManager->GetTextureData(path);
	if(textureData != NULL)
	{
		D3DXIMAGE_INFO imageInfo = textureData->GetImageInfo();
		res = imageInfo.Width;
	}

	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetTextureHeight(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	std::wstring path = argv[0].as_string();
	path = PathProperty::GetUnique(path);
	TextureManager* textureManager = TextureManager::GetBase();

	gstd::ref_count_ptr<TextureData> textureData = textureManager->GetTextureData(path);
	if(textureData != NULL)
	{
		D3DXIMAGE_INFO imageInfo = textureData->GetImageInfo();
		res = imageInfo.Height;
	}

	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_SetFogEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	bool bEnable = argv[0].as_boolean();
	script->GetObjectManager()->SetFogParam(bEnable, 0, 0, 0);
	return value();
}
gstd::value DxScript::Func_SetFogParam(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	double start = argv[0].as_real();
	double end = argv[1].as_real();
	int r = (int)argv[2].as_real();
	int g = (int)argv[3].as_real();
	int b = (int)argv[4].as_real();
	D3DCOLOR color = D3DCOLOR_ARGB(255, r, g, b);
	script->GetObjectManager()->SetFogParam(true, color, start, end);

	return value();
}
gstd::value DxScript::Func_CreateRenderTarget(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	bool res = true;
	std::wstring name = argv[0].as_string();

	if(script->mapTexture_.find(name) == script->mapTexture_.end())
	{
		ref_count_ptr<Texture> texture = new Texture();
		bool res = texture->CreateRenderTarget(name);
		if(res)
		{
			Lock lock(script->criticalSection_);
			script->mapTexture_[name] = texture;
		}
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value DxScript::Func_SetRenderTarget(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	TextureManager* textureManager = TextureManager::GetBase();
	
	std::wstring name = argv[0].as_string();
	ref_count_ptr<Texture> texture = script->_GetTexture(name);
	if(texture == NULL)
		texture = textureManager->GetTexture(name);
	if(texture == NULL)return value();

	DirectGraphics* graphics = DirectGraphics::GetBase();
	ref_count_ptr<Texture> current = graphics->GetRenderTarget();
	graphics->SetRenderTarget(texture);
	graphics->ClearRenderTarget();
	graphics->SetRenderTarget(current);

	return value();
}
gstd::value DxScript::Func_GetTransitionRenderTargetName(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	std::wstring res = TextureManager::TARGET_TRANSITION;
	return value(machine->get_engine()->get_string_type(), res);
}
gstd::value DxScript::Func_SaveRenderedTextureA1(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	std::wstring nameTexture = argv[0].as_string();
	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);

	TextureManager* textureManager = TextureManager::GetBase();
	DirectGraphics* graphics = DirectGraphics::GetBase();

	ref_count_ptr<Texture> texture = script->_GetTexture(nameTexture);
	if(texture == NULL)
		texture = textureManager->GetTexture(nameTexture);

	if(texture != NULL)
	{
		//フォルダ生成
		std::wstring dir = PathProperty::GetFileDirectory(path);
		File fDir(dir);
		fDir.CreateDirectory();

		//保存
		IDirect3DSurface9* pSurface = texture->GetD3DSurface();
		RECT rect = {0, 0, graphics->GetScreenWidth(), graphics->GetScreenHeight()};
		D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP, 
								pSurface, NULL, &rect );
	}

	return value();
}
gstd::value DxScript::Func_SaveRenderedTextureA2(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;

	std::wstring nameTexture = argv[0].as_string();
	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);

	int rcLeft = (int)argv[2].as_real();
	int rcTop = (int)argv[3].as_real();
	int rcRight = (int)argv[4].as_real();
	int rcBottom = (int)argv[5].as_real();

	TextureManager* textureManager = TextureManager::GetBase();
	DirectGraphics* graphics = DirectGraphics::GetBase();

	ref_count_ptr<Texture> texture = script->_GetTexture(nameTexture);
	if(texture == NULL)
		texture = textureManager->GetTexture(nameTexture);
	if(texture != NULL)
	{
		//フォルダ生成
		std::wstring dir = PathProperty::GetFileDirectory(path);
		File fDir(dir);
		fDir.CreateDirectory();

		//保存
		IDirect3DSurface9* pSurface = texture->GetD3DSurface();
		RECT rect = {rcLeft, rcTop, rcRight, rcBottom};
		D3DXSaveSurfaceToFile(path.c_str(), D3DXIFF_BMP, 
								pSurface, NULL, &rect );
	}

	return value();
}
gstd::value DxScript::Func_IsPixelShaderSupported(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	ref_count_ptr<Shader> shader = NULL;

	int major = (int)(argv[0].as_real() + 0.5);
	int minor = (int)(argv[1].as_real() + 0.5);

	DirectGraphics* graphics = DirectGraphics::GetBase();
	bool res = graphics->IsPixelShaderSupported(major, minor);

	return value(machine->get_engine()->get_boolean_type(), res);
}

gstd::value DxScript::Func_SetShader(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();

	double min = (double)argv[1].as_real();
	double max = (double)argv[2].as_real();
	gstd::ref_count_ptr<DxScriptObjectManager> objectManager = script->GetObjectManager();
	objectManager->SetShader(shader, min, max);

	return value();
}
gstd::value DxScript::Func_SetShaderI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();

	int size = script->GetObjectManager()->GetRenderBucketCapacity();
	int min = (int)(argv[1].as_real() + 0.5);
	int max = (int)(argv[2].as_real() + 0.5);

	double priMin = (double)min / (double)(size - 1);
	double priMax = (double)max / (double)(size - 1);

	gstd::ref_count_ptr<DxScriptObjectManager> objectManager = script->GetObjectManager();
	objectManager->SetShader(shader, priMin, priMax);

	return value();
}
gstd::value DxScript::Func_ResetShader(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	ref_count_ptr<Shader> shader = NULL;

	double min = (double)argv[0].as_real();
	double max = (double)argv[1].as_real();
	gstd::ref_count_ptr<DxScriptObjectManager> objectManager = script->GetObjectManager();
	objectManager->SetShader(shader, min, max);

	return value();
}
gstd::value DxScript::Func_ResetShaderI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	ref_count_ptr<Shader> shader = NULL;

	int size = script->GetObjectManager()->GetRenderBucketCapacity();
	int min = (int)(argv[0].as_real() + 0.5);
	int max = (int)(argv[1].as_real() + 0.5);

	double priMin = (double)min / (double)(size - 1);
	double priMax = (double)max / (double)(size - 1);

	gstd::ref_count_ptr<DxScriptObjectManager> objectManager = script->GetObjectManager();
	objectManager->SetShader(shader, priMin, priMax);

	return value();
}

//Dx関数：カメラ3D
value DxScript::Func_SetCameraFocusX(script_machine* machine, int argc, value const * argv)
{
	float x = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetFocusX(x);
	return value();
}
value DxScript::Func_SetCameraFocusY(script_machine* machine, int argc, value const * argv)
{
	float y = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetFocusY(y);
	return value();
}
value DxScript::Func_SetCameraFocusZ(script_machine* machine, int argc, value const * argv)
{
	float z = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetFocusZ(z);
	return value();
}
value DxScript::Func_SetCameraFocusXYZ(script_machine* machine, int argc, value const * argv)
{
	float x = argv[0].as_real();
	float y = argv[1].as_real();
	float z = argv[2].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetFocusX(x);
	graphics->GetCamera()->SetFocusY(y);
	graphics->GetCamera()->SetFocusZ(z);
	return value();
}
value DxScript::Func_SetCameraRadius(script_machine* machine, int argc, value const * argv)
{
	float r = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetRadius(r);
	return value();
}
value DxScript::Func_SetCameraAzimuthAngle(script_machine* machine, int argc, value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetAzimuthAngle(angle);
	return value();
}
value DxScript::Func_SetCameraElevationAngle(script_machine* machine, int argc, value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetElevationAngle(angle);
	return value();
}
value DxScript::Func_SetCameraYaw(script_machine* machine, int argc, value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetYaw(angle);
	return value();
}
value DxScript::Func_SetCameraPitch(script_machine* machine, int argc, value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetPitch(angle);
	return value();
}
value DxScript::Func_SetCameraRoll(script_machine* machine, int argc, value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera()->SetRoll(angle);
	return value();
}
value DxScript::Func_GetCameraX(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetCameraPosition().x;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraY(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetCameraPosition().y;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraZ(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetCameraPosition().z;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraFocusX(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetFocusPosition().x;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraFocusY(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetFocusPosition().y;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraFocusZ(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetFocusPosition().z;
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraRadius(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetRadius();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraAzimuthAngle(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetAzimuthAngle();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraElevationAngle(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetElevationAngle();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraYaw(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetYaw();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraPitch(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetPitch();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_GetCameraRoll(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera()->GetRoll();
	return value(machine->get_engine()->get_real_type(), res);
}
value DxScript::Func_SetCameraPerspectiveClip(script_machine* machine, int argc, value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	double clipNear = argv[0].as_real();
	double clipFar = argv[1].as_real();
	int width = graphics->GetScreenWidth();
	int height = graphics->GetScreenHeight();

	graphics->GetCamera()->SetProjectionMatrix(width, height, clipNear, clipFar);
	return value();
}

//Dx関数：カメラ2D
gstd::value DxScript::Func_Set2DCameraFocusX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float x = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetFocusX(x);
	return gstd::value();
}
gstd::value DxScript::Func_Set2DCameraFocusY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float y = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetFocusY(y);
	return gstd::value();
}
gstd::value DxScript::Func_Set2DCameraAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float angle = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetAngleZ(angle);
	return gstd::value();
}
gstd::value DxScript::Func_Set2DCameraRatio(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float ratio = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetRatio(ratio);
	return gstd::value();
}
gstd::value DxScript::Func_Set2DCameraRatioX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float ratio = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetRatioX(ratio);
	return gstd::value();
}
gstd::value DxScript::Func_Set2DCameraRatioY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	float ratio = argv[0].as_real();
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->SetRatioY(ratio);
	return gstd::value();
}
gstd::value DxScript::Func_Reset2DCamera(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	graphics->GetCamera2D()->Reset();
	return gstd::value();
}
gstd::value DxScript::Func_Get2DCameraX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetFocusX();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Get2DCameraY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetFocusY();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Get2DCameraAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetAngleZ();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Get2DCameraRatio(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetRatio();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Get2DCameraRatioX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetRatioX();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Get2DCameraRatioY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DirectGraphics* graphics = DirectGraphics::GetBase();
	long double res = graphics->GetCamera2D()->GetRatioY();
	return value(machine->get_engine()->get_real_type(), res);
}

//Dx関数：その他
gstd::value DxScript::Func_GetObjectDistance(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id1 = (int)argv[0].as_real();
	int id2 = (int)argv[1].as_real();

	DxScriptRenderObject* obj1 = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id1));
	if(obj1 == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);

	DxScriptRenderObject* obj2 = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id2));
	if(obj2 == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);

	int tx = obj1->GetPosition().x - obj2->GetPosition().x;
	int ty = obj1->GetPosition().y - obj2->GetPosition().y;
	int tz = obj1->GetPosition().z - obj2->GetPosition().z;

	long double res = pow(pow(tx, 2) + pow(ty, 2) + pow(tz, 2), 0.5);
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_GetObject2dPosition(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();

	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)script->RaiseError(L"error invalid object");

	DirectGraphics* graphics = DirectGraphics::GetBase();
	gstd::ref_count_ptr<DxCamera> camera = graphics->GetCamera();
	D3DXVECTOR3 pos = obj->GetPosition();

	D3DXVECTOR2 point = camera->TransformCoordinateTo2D(pos);
	std::vector<long double> listRes;
	listRes.push_back(point.x);
	listRes.push_back(point.y);

	gstd::value res = script->CreateRealArrayValue(listRes);
	return res;
}
gstd::value DxScript::Func_Get2dPosition(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	double px = argv[0].as_real();
	double py = argv[1].as_real();
	double pz = argv[2].as_real();

	D3DXVECTOR3 pos(px, py, pz);

	DirectGraphics* graphics = DirectGraphics::GetBase();
	gstd::ref_count_ptr<DxCamera> camera = graphics->GetCamera();
	D3DXVECTOR2 point = camera->TransformCoordinateTo2D(pos);
	std::vector<long double> listRes;
	listRes.push_back(point.x);
	listRes.push_back(point.y);

	gstd::value res = script->CreateRealArrayValue(listRes);
	return res;
}

//Dx関数：オブジェクト操作(共通)
value DxScript::Func_Obj_Delete(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	script->CheckRunInMainThread();
	int id = (int)argv[0].as_real();
	script->DeleteObject(id);
	return value();
}
value DxScript::Func_Obj_IsDeleted(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	bool res = obj == NULL;
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_Obj_SetVisible(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();
	obj->bVisible_ = argv[1].as_boolean();
	return value();
}
value DxScript::Func_Obj_IsVisible(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	bool res = false;
	if(obj != NULL)
		res = obj->bVisible_;
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_Obj_SetRenderPriority(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();
	double pri = argv[1].as_real();
	if(pri < 0)pri = 0;
	else if(pri > 1)pri = 1;
	obj->priRender_ = pri;
	return value();
}
value DxScript::Func_Obj_SetRenderPriorityI(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();
	
	int pos = (int)(argv[1].as_real() + 0.5);
	int size = script->GetObjectManager()->GetRenderBucketCapacity();
	double pri = (double)pos / (double)(size - 1);

	if(pri < 0)pri = 0;
	else if(pri > 1)pri = 1;
	obj->priRender_ = pri;
	return value();
}
gstd::value DxScript::Func_Obj_GetRenderPriority(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj != NULL)
		res = obj->GetRenderPriority();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_Obj_GetRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj != NULL)
		res = obj->GetRenderPriorityI();
	return value(machine->get_engine()->get_real_type(), res);
}

gstd::value DxScript::Func_Obj_GetValue(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::wstring key = argv[1].as_string();

	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();

	gstd::value res = obj->GetObjectValue(key);
	return res;
}
gstd::value DxScript::Func_Obj_GetValueD(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::wstring key = argv[1].as_string();
	gstd::value def = argv[2];

	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return def;

	gstd::value res = def;
	bool bExists = obj->IsObjectValueExists(key);
	if(bExists)
		res = obj->GetObjectValue(key);

	return res;
}
gstd::value DxScript::Func_Obj_SetValue(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::wstring key = argv[1].as_string();
	gstd::value val = argv[2];

	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();

	obj->SetObjectValue(key, val);
	return value();
}
gstd::value DxScript::Func_Obj_DeleteValue(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::wstring key = argv[1].as_string();

	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();

	obj->DeleteObjectValue(key);
	return value();
}
gstd::value DxScript::Func_Obj_IsValueExists(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	std::wstring key = argv[1].as_string();

	DxScriptObjectBase* obj = script->GetObjectPointer(id);
	if(obj == NULL)return value();

	bool res = obj->IsObjectValueExists(key);
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_Obj_GetType(script_machine* machine, int argc, value const * argv)
{
	long double res = DxScript::OBJ_INVALID;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptObjectBase* obj = dynamic_cast<DxScriptObjectBase*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->GetObjectType();
	return value(machine->get_engine()->get_real_type(),res);
}


//Dx関数：オブジェクト操作(RenderObject)
value DxScript::Func_ObjRender_SetX(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetX(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetY(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetY(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetZ(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetZ(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetPosition(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetX(argv[1].as_real());
	obj->SetY(argv[2].as_real());
	obj->SetZ(argv[3].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetAngleX(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAngleX(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetAngleY(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAngleY(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetAngleZ(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAngleZ(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetAngleXYZ(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAngleX(argv[1].as_real());
	obj->SetAngleY(argv[2].as_real());
	obj->SetAngleZ(argv[3].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetScaleX(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetScaleX(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetScaleY(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetScaleY(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetScaleZ(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetScaleZ(argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetScaleXYZ(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetScaleX(argv[1].as_real());
	obj->SetScaleY(argv[2].as_real());
	obj->SetScaleZ(argv[3].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetColor(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetColor((int)argv[1].as_real(), (int)argv[2].as_real(), (int)argv[3].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetColorHSV(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int hue = (int)argv[1].as_real();
	int sat = (int)argv[2].as_real();
	int val = (int)argv[3].as_real();

	D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255);
	color = ColorAccess::SetColorHSV(color, hue, sat, val);

	int red = ColorAccess::GetColorR(color);
	int green = ColorAccess::GetColorG(color);
	int blue = ColorAccess::GetColorB(color);

	obj->SetColor(red, green, blue);
	return value();
}
value DxScript::Func_ObjRender_SetAlpha(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAlpha((int)argv[1].as_real());
	return value();
}
value DxScript::Func_ObjRender_SetBlendType(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->typeBlend_ = (int)argv[1].as_real();
	return value();
}
value DxScript::Func_ObjRender_GetX(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->position_.x;
	return value(machine->get_engine()->get_real_type(),res);
}
value DxScript::Func_ObjRender_GetY(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->position_.y;
	return value(machine->get_engine()->get_real_type(),res);
}
value DxScript::Func_ObjRender_GetZ(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->position_.z;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetAngleX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->angle_.x;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetAngleY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->angle_.y;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->angle_.z;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetScaleX(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->scale_.x;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetScaleY(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->scale_.y;
	return value(machine->get_engine()->get_real_type(),res);
}
gstd::value DxScript::Func_ObjRender_GetScaleZ(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->scale_.z;
	return value(machine->get_engine()->get_real_type(),res);
}

value DxScript::Func_ObjRender_SetZWrite(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->bZWrite_ = argv[1].as_boolean();
	return value();
}
value DxScript::Func_ObjRender_SetZTest(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->bZTest_ = argv[1].as_boolean();
	return value();
}
value DxScript::Func_ObjRender_SetFogEnable(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->bFogEnable_ = argv[1].as_boolean();
	return value();
}
value DxScript::Func_ObjRender_SetCullingMode(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->modeCulling_ = (int)argv[1].as_boolean();
	return value();
}
value DxScript::Func_ObjRender_SetRalativeObject(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int idRelative = (int)argv[1].as_real();
	std::wstring nameBone = argv[2].as_string();
	obj->SetRelativeObject(idRelative, nameBone);
	DxScriptObjectBase* objRelative = dynamic_cast<DxScriptObjectBase*>(script->GetObjectPointer(idRelative));
	if(objRelative == NULL)return value();	

	return value();
}
value DxScript::Func_ObjRender_SetPermitCamera(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	bool bEnable = argv[1].as_boolean();

	DxScriptPrimitiveObject2D* obj2D = dynamic_cast<DxScriptPrimitiveObject2D*>(script->GetObjectPointer(id));
	DxScriptTextObject* objText = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj2D != NULL)
		obj2D->SetPermitCamera(bEnable);
	else if(objText != NULL)
		objText->SetPermitCamera(bEnable);

	return value();
}

gstd::value DxScript::Func_ObjRender_GetBlendType(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	int res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->GetBlendType();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}

//Dx関数：オブジェクト操作(ShaderObject)
gstd::value DxScript::Func_ObjShader_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	script->CheckRunInMainThread();
	int type = (int)argv[0].as_real();
	ref_count_ptr<DxScriptShaderObject>::unsync obj = new DxScriptShaderObject();

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->Initialize();
		obj->manager_ = script->objManager_.GetPointer();
		id = script->AddObject(obj);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value DxScript::Func_ObjShader_SetShaderF(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)	return value(machine->get_engine()->get_boolean_type(), false);

	bool res = false;
	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	if(false)
	{

	}
	else
	{
		ref_count_ptr<Shader> shader = new Shader();
		res = shader->CreateFromFile(path);
		obj->SetShader(shader);

		if(!res)
		{
			std::wstring error = ShaderManager::GetBase()->GetLastError();
			script->RaiseError(error);
		}
	}

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value DxScript::Func_ObjShader_SetShaderO(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id1 = (int)argv[0].as_real();
	DxScriptRenderObject* obj1 = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id1));
	if(obj1 == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	int id2 = (int)argv[1].as_real();
	DxScriptRenderObject* obj2 = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id2));
	if(obj2 == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	ref_count_ptr<Shader> shader = obj2->GetShader();
	if(shader == NULL)	return value(machine->get_engine()->get_boolean_type(), false);
	obj1->SetShader(shader);

	return value(machine->get_engine()->get_boolean_type(), true);
}
gstd::value DxScript::Func_ObjShader_ResetShader(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetShader(NULL);

	return value();
}
gstd::value DxScript::Func_ObjShader_SetTechnique(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string aPath = StringUtility::ConvertWideToMulti(argv[1].as_string());
	shader->SetTechnique(aPath);

	return value();
}
gstd::value DxScript::Func_ObjShader_SetMatrix(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	gstd::value sMatrix = argv[2];
	type_data::type_kind kind = sMatrix.get_type()->get_kind();
	if(kind != type_data::tk_array)return value();
	int arrayLength = sMatrix.length_as_array();
	if(arrayLength != 16)return value();

	D3DXMATRIX matrix;
	for(int iRow = 0 ; iRow < 4 ; iRow++)
	{
		for(int iCol = 0 ; iCol < 4 ; iCol++)
		{
			int index = iRow * 4 + iCol;
			value& arrayValue = sMatrix.index_as_array(index);
			float fValue = (float)arrayValue.as_real();
			matrix.m[iRow][iCol] = fValue;
		}
	}
	shader->SetMatrix(name, matrix);

	return value();
}
gstd::value DxScript::Func_ObjShader_SetMatrixArray(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	gstd::value array = argv[2];
	type_data::type_kind kind = array.get_type()->get_kind();
	if(kind != type_data::tk_array)return value();

	int dataLength = array.length_as_array();
	std::vector<D3DXMATRIX> listMatrix;
	for(int iArray = 0 ; iArray < dataLength ; iArray++)
	{
		value& sMatrix = array.index_as_array(iArray);
		type_data::type_kind kind = sMatrix.get_type()->get_kind();
		if(kind != type_data::tk_array)return value();
		int arrayLength = sMatrix.length_as_array();
		if(arrayLength != 16)return value();

		D3DXMATRIX matrix;
		for(int iRow = 0 ; iRow < 4 ; iRow++)
		{
			for(int iCol = 0 ; iCol < 4 ; iCol++)
			{
				int index = iRow * 4 + iCol;
				value& arrayValue = sMatrix.index_as_array(index);
				float fValue = (float)arrayValue.as_real();
				matrix.m[iRow][iCol] = fValue;
			}
		}
		listMatrix.push_back(matrix);
	}
	shader->SetMatrixArray(name, listMatrix);

	return value();
}
gstd::value DxScript::Func_ObjShader_SetVector(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	D3DXVECTOR4 vect4;
	vect4.x = (float)argv[2].as_real();
	vect4.y = (float)argv[3].as_real();
	vect4.z = (float)argv[4].as_real();
	vect4.w = (float)argv[5].as_real();
	shader->SetVector(name, vect4);
	return value();
}
gstd::value DxScript::Func_ObjShader_SetFloat(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	float vValue = (float)argv[2].as_real();
	shader->SetFloat(name, vValue);
	return value();
}
gstd::value DxScript::Func_ObjShader_SetFloatArray(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	gstd::value array = argv[2];
	type_data::type_kind kind = array.get_type()->get_kind();
	if(kind != type_data::tk_array)return value();

	int dataLength = array.length_as_array();
	std::vector<float> listFloat;
	for(int iArray = 0 ; iArray < dataLength ; iArray++)
	{
		value& aValue = array.index_as_array(iArray);
		float fValue = (float)aValue.as_real();
		listFloat.push_back(fValue);
	}
	shader->SetFloatArray(name, listFloat);

	return value();
}
gstd::value DxScript::Func_ObjShader_SetTexture(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptRenderObject* obj = dynamic_cast<DxScriptRenderObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Shader> shader = obj->GetShader();
	if(shader == NULL)return value();

	std::string name = StringUtility::ConvertWideToMulti(argv[1].as_string());
	std::wstring path = argv[2].as_string();
	path = PathProperty::GetUnique(path);

	if(script->mapTexture_.find(path) != script->mapTexture_.end())
	{
		shader->SetTexture(name, script->mapTexture_[path]);
	}
	else
	{
		ref_count_ptr<Texture> texture = new Texture();
		texture->CreateFromFile(path);
		shader->SetTexture(name, texture);
	}
	return value();
}

//Dx関数：オブジェクト操作(PrimitiveObject)
value DxScript::Func_ObjPrimitive_Create(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	script->CheckRunInMainThread();
	int type = (int)argv[0].as_real();
	ref_count_ptr<DxScriptPrimitiveObject>::unsync obj;
	if(type == OBJ_PRIMITIVE_2D)
	{
		obj = new DxScriptPrimitiveObject2D();
	}
	else if(type == OBJ_SPRITE_2D)
	{
		obj = new DxScriptSpriteObject2D();
	}
	else if(type == OBJ_SPRITE_LIST_2D)
	{
		obj = new DxScriptSpriteListObject2D();
	}
	else if(type == OBJ_PRIMITIVE_3D)
	{
		obj = new DxScriptPrimitiveObject3D();
	}
	else if(type == OBJ_SPRITE_3D)
	{
		obj = new DxScriptSpriteObject3D();
	}
	else if(type == OBJ_TRAJECTORY_3D)
	{
		obj = new DxScriptTrajectoryObject3D();
	}

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->Initialize();
		obj->manager_ = script->objManager_.GetPointer();
		id = script->AddObject(obj);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
value DxScript::Func_ObjPrimitive_SetPrimitiveType(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetPrimitiveType((D3DPRIMITIVETYPE)(int)argv[1].as_real());
	return value();
}
value DxScript::Func_ObjPrimitive_SetVertexCount(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetVertexCount((int)argv[1].as_real());
	return value();
}
value DxScript::Func_ObjPrimitive_SetTexture(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	if(script->mapTexture_.find(path) != script->mapTexture_.end())
	{
		obj->SetTexture(script->mapTexture_[path]);
	}
	else
	{
		ref_count_ptr<Texture> texture = new Texture();
		texture->CreateFromFile(path);
		obj->SetTexture(texture);
	}
	return value();
}
value DxScript::Func_ObjPrimitive_GetVertexCount(script_machine* machine, int argc, value const * argv)
{
	long double res = 0;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		res = obj->GetVertexCount();
	return value(machine->get_engine()->get_real_type(),res);
}
value DxScript::Func_ObjPrimitive_SetVertexPosition(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetVertexPosition((int)argv[1].as_real(), argv[2].as_real(), argv[3].as_real(), argv[4].as_real());
	return value();
}
value DxScript::Func_ObjPrimitive_SetVertexUV(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetVertexUV((int)argv[1].as_real(), argv[2].as_real(), argv[3].as_real());
	return value();
}
gstd::value DxScript::Func_ObjPrimitive_SetVertexUVT(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<Texture> texture = obj->GetTexture();
	if(texture == NULL)return value();

	int width = texture->GetWidth();
	int height = texture->GetHeight();
	obj->SetVertexUV((int)argv[1].as_real(), argv[2].as_real() / (double)width, argv[3].as_real() / (double)height);

	return value();
}
value DxScript::Func_ObjPrimitive_SetVertexColor(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetVertexColor((int)argv[1].as_real(), (int)argv[2].as_real(), (int)argv[3].as_real(), (int)argv[4].as_real());
	return value();
}
value DxScript::Func_ObjPrimitive_SetVertexAlpha(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetVertexAlpha((int)argv[1].as_real(), (int)argv[2].as_real());
	return value();
}
value DxScript::Func_ObjPrimitive_GetVertexPosition(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	int index = (int)argv[1].as_real();

	D3DXVECTOR3 pos = D3DXVECTOR3(0 ,0, 0);
	DxScriptPrimitiveObject* obj = dynamic_cast<DxScriptPrimitiveObject*>(script->GetObjectPointer(id));
	if(obj != NULL)
		pos = obj->GetVertexPosition(index);

	std::vector<long double> listPos;
	listPos.resize(3);
	listPos[0] = pos.x;
	listPos[1] = pos.y;
	listPos[2] = pos.z;
	
	gstd::value res = script->CreateRealArrayValue(listPos);
	return res;
}

//Dx関数：オブジェクト操作(Sprite2D)
value DxScript::Func_ObjSprite2D_SetSourceRect(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject2D* obj = dynamic_cast<DxScriptSpriteObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcDest = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetSourceRect(rcDest);

	return value();
}
value DxScript::Func_ObjSprite2D_SetDestRect(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject2D* obj = dynamic_cast<DxScriptSpriteObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcDest = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetDestinationRect(rcDest);

	return value();
}
value DxScript::Func_ObjSprite2D_SetDestCenter(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject2D* obj = dynamic_cast<DxScriptSpriteObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->GetSpritePointer()->SetDestinationCenter();

	return value();
}

//Dx関数：オブジェクト操作(SpriteList2D)
gstd::value DxScript::Func_ObjSpriteList2D_SetSourceRect(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcDest = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetSourceRect(rcDest);

	return value();
}
gstd::value DxScript::Func_ObjSpriteList2D_SetDestRect(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcDest = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetDestinationRect(rcDest);

	return value();
}
gstd::value DxScript::Func_ObjSpriteList2D_SetDestCenter(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->GetSpritePointer()->SetDestinationCenter();

	return value();
}
gstd::value DxScript::Func_ObjSpriteList2D_AddVertex(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->AddVertex();

	return value();
}
gstd::value DxScript::Func_ObjSpriteList2D_CloseVertex(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->CloseVertex();

	return value();
}
gstd::value DxScript::Func_ObjSpriteList2D_ClearVertexCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteListObject2D* obj = dynamic_cast<DxScriptSpriteListObject2D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->GetSpritePointer()->ClearVertexCount();

	return value();
}

//Dx関数：オブジェクト操作(Sprite3D)
value DxScript::Func_ObjSprite3D_SetSourceRect(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject3D* obj = dynamic_cast<DxScriptSpriteObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcSrc = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetSourceRect(rcSrc);

	return value();
}
value DxScript::Func_ObjSprite3D_SetDestRect(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject3D* obj = dynamic_cast<DxScriptSpriteObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcDest = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetDestinationRect(rcDest);

	return value();
}
value DxScript::Func_ObjSprite3D_SetSourceDestRect(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject3D* obj = dynamic_cast<DxScriptSpriteObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	RECT_D rcSrc = {
		(double)argv[1].as_real(),
		(double)argv[2].as_real(),
		(double)argv[3].as_real(),
		(double)argv[4].as_real()
	};
	obj->GetSpritePointer()->SetSourceDestRect(rcSrc);
	return value();
}
value DxScript::Func_ObjSprite3D_SetBillboard(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptSpriteObject3D* obj = dynamic_cast<DxScriptSpriteObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	bool bEnable = argv[1].as_boolean();
	obj->GetSpritePointer()->SetBillboardEnable(bEnable);

	return value();
}
//Dx関数：オブジェクト操作(TrajectoryObject3D)
value DxScript::Func_ObjTrajectory3D_SetInitialPoint(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTrajectoryObject3D* obj = dynamic_cast<DxScriptTrajectoryObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	D3DXVECTOR3 pos1(argv[1].as_real(), argv[2].as_real(), argv[3].as_real());
	D3DXVECTOR3 pos2(argv[4].as_real(), argv[5].as_real(), argv[6].as_real());
	obj->GetObjectPointer()->SetInitialLine(pos1, pos2);

	return value();
}
value DxScript::Func_ObjTrajectory3D_SetAlphaVariation(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTrajectoryObject3D* obj = dynamic_cast<DxScriptTrajectoryObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->GetObjectPointer()->SetAlphaVariation(argv[1].as_real());

	return value();
}
value DxScript::Func_ObjTrajectory3D_SetComplementCount(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTrajectoryObject3D* obj = dynamic_cast<DxScriptTrajectoryObject3D*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->GetObjectPointer()->SetComplementCount((int)argv[1].as_real());

	return value();
}

//Dx関数：オブジェクト操作(DxMesh)
value DxScript::Func_ObjMesh_Create(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	ref_count_ptr<DxScriptMeshObject>::unsync obj = new DxScriptMeshObject();
	int id = ID_INVALID;
	if(obj != NULL)
		id = script->AddObject(obj);
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
value DxScript::Func_ObjMesh_Load(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	ref_count_ptr<DxMesh> mesh;
	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	std::wstring ext = PathProperty::GetFileExtension(path);

	bool res = false;
	if(ext == L".mqo")
	{
		mesh = new MetasequoiaMesh();
		res = mesh->CreateFromFile(path);
	}
	else if(ext == L".elem")
	{
		mesh = new ElfreinaMesh();
		res = mesh->CreateFromFile(path);
	}
	if(res)
	{
		obj->mesh_ = mesh;
		//script->AddMeshResource(path, mesh);
	}
	return value(machine->get_engine()->get_boolean_type(), res);
}
value DxScript::Func_ObjMesh_SetColor(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetColor((int)argv[1].as_real(), (int)argv[2].as_real(), (int)argv[3].as_real());
	return value();
}
value DxScript::Func_ObjMesh_SetAlpha(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	obj->SetAlpha((int)argv[1].as_real());
	return value();
}
value DxScript::Func_ObjMesh_SetAnimation(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	
	std::wstring anime = argv[1].as_string();
	obj->anime_ = anime;
	obj->time_ = (int)argv[2].as_real();

//	D3DXMATRIX mat = obj->mesh_->GetAnimationMatrix(anime, obj->time_, "悠久前部");
//	D3DXVECTOR3 pos;
//	D3DXVec3TransformCoord(&pos, &D3DXVECTOR3(0,0,0), &mat);
	return value();
}
gstd::value DxScript::Func_ObjMesh_SetCoordinate2D(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	
	bool bEnable = argv[1].as_boolean();
	obj->bCoordinate2D_ = bEnable;
	return value();
}
value DxScript::Func_ObjMesh_GetPath(script_machine* machine, int argc, value const * argv)
{
	std::wstring res;
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptMeshObject* obj = dynamic_cast<DxScriptMeshObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_string_type(), res);
	DxMesh* mesh = obj->mesh_.GetPointer();
	if(mesh == NULL)return value(machine->get_engine()->get_string_type(), res);
	res = mesh->GetPath();
	return value(machine->get_engine()->get_string_type(), res);
}

//Dx関数：オブジェクト操作(DxText)
value DxScript::Func_ObjText_Create(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	ref_count_ptr<DxScriptTextObject>::unsync obj = new DxScriptTextObject();
	int id = ID_INVALID;
	if(obj != NULL)
		id = script->AddObject(obj);
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
value DxScript::Func_ObjText_SetText(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	std::wstring wstr = argv[1].as_string();
	obj->SetText(wstr);
	return value();
}
value DxScript::Func_ObjText_SetFontType(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	std::wstring wstr = argv[1].as_string();
	obj->SetFontType(wstr);
	return value();
}
value DxScript::Func_ObjText_SetFontSize(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int size = (int)argv[1].as_real();
	obj->SetFontSize(size);
	return value();
}
value DxScript::Func_ObjText_SetFontBold(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	bool bBold = argv[1].as_boolean();
	obj->SetFontBold(bBold);
	return value();
}
value DxScript::Func_ObjText_SetFontColorTop(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int r = (int)argv[1].as_real();
	int g = (int)argv[2].as_real();
	int b = (int)argv[3].as_real();
	obj->SetFontColorTop(r, g, b);
	return value();
}
value DxScript::Func_ObjText_SetFontColorBottom(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int r = (int)argv[1].as_real();
	int g = (int)argv[2].as_real();
	int b = (int)argv[3].as_real();
	obj->SetFontColorBottom(r, g, b);
	return value();
}
value DxScript::Func_ObjText_SetFontBorderWidth(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int width = (int)argv[1].as_real();
	obj->SetFontBorderWidth(width);
	return value();
}
value DxScript::Func_ObjText_SetFontBorderType(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int type = (int)argv[1].as_real();
	obj->SetFontBorderType(type);
	return value();
}
value DxScript::Func_ObjText_SetFontBorderColor(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int r = (int)argv[1].as_real();
	int g = (int)argv[2].as_real();
	int b = (int)argv[3].as_real();
	obj->SetFontBorderColor(r, g, b);
	return value();
}
value DxScript::Func_ObjText_SetMaxWidth(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int width = (int)argv[1].as_real();
	obj->SetMaxWidth(width);
	return value();
}
value DxScript::Func_ObjText_SetMaxHeight(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int height = (int)argv[1].as_real();
	obj->SetMaxHeight(height);
	return value();
}
value DxScript::Func_ObjText_SetLinePitch(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int pitch = (int)argv[1].as_real();
	obj->SetLinePitch(pitch);
	return value();
}
value DxScript::Func_ObjText_SetSidePitch(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int pitch = (int)argv[1].as_real();
	obj->SetSidePitch(pitch);
	return value();
}
value DxScript::Func_ObjText_SetVertexColor(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int a = (int)argv[1].as_real();
	int r = (int)argv[2].as_real();
	int g = (int)argv[3].as_real();
	int b = (int)argv[4].as_real();
	obj->SetVertexColor(D3DCOLOR_ARGB(a, r, g, b));
	return value();
}
gstd::value DxScript::Func_ObjText_SetTransCenter(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	double centerX = argv[1].as_real();
	double centerY = argv[2].as_real();

	obj->center_ = D3DXVECTOR2(centerX, centerY);

	return value();
}
gstd::value DxScript::Func_ObjText_SetAutoTransCenter(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	bool bAutoCenter = argv[1].as_boolean();

	obj->bAutoCenter_ = bAutoCenter;

	return value();
}
gstd::value DxScript::Func_ObjText_SetHorizontalAlignment(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	int align = (int)argv[1].as_real();

	obj->SetHorizontalAlignment(align);

	return value();
}
gstd::value DxScript::Func_ObjText_SetSyntacticAnalysis(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	bool bEnable = argv[1].as_boolean();

	obj->SetSyntacticAnalysis(bEnable);

	return value();
}
value DxScript::Func_ObjText_GetTextLength(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	std::wstring text = obj->GetText();
	int res = StringUtility::CountAsciiSizeCharacter(text);
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value DxScript::Func_ObjText_GetTextLengthCU(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	std::vector<int> listCount = obj->GetTextCountCU();
	int res = 0;
	for(int iLine = 0 ; iLine < listCount.size() ; iLine++)
	{
		int count = listCount[iLine];
		res += count;
	}
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value DxScript::Func_ObjText_GetTextLengthCUL(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	std::vector<int> listCount = obj->GetTextCountCU();
	std::vector<long double> listCountD;
	for(int iLine = 0 ; iLine < listCount.size() ; iLine++)
	{
		int count = listCount[iLine];
		listCountD.push_back(count);
	}
	
	gstd::value res = script->CreateRealArrayValue(listCountD);
	return res;
}
value DxScript::Func_ObjText_GetTotalWidth(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetTotalWidth();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value DxScript::Func_ObjText_GetTotalHeight(script_machine* machine, int argc, value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxScriptTextObject* obj = dynamic_cast<DxScriptTextObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	int res = obj->GetTotalHeight();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}

//Dx関数：音声操作(DxSoundObject)
gstd::value DxScript::Func_ObjSound_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	DirectSoundManager* manager = DirectSoundManager::GetBase();

	ref_count_ptr<DxSoundObject>::unsync obj = new DxSoundObject();
	int id = script->AddObject(obj);
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value DxScript::Func_ObjSound_Load(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();;

	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	bool bLoad = obj->Load(path);

	return value(machine->get_engine()->get_boolean_type(), bLoad);
}
gstd::value DxScript::Func_ObjSound_Play(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	//obj->Play();
	script->GetObjectManager()->ReserveSound(player, obj->GetStyle());
	return value();
}
gstd::value DxScript::Func_ObjSound_Stop(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	player->Stop();
	script->GetObjectManager()->DeleteReservedSound(player);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetVolumeRate(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	double rate = argv[1].as_real();
	player->SetVolumeRate(rate);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetPanRate(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	double rate = argv[1].as_real();
	player->SetPanRate(rate);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetFade(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	double fade = argv[1].as_real();
	player->SetFade(fade);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetLoopEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	bool bLoop = (bool)argv[1].as_boolean();
	SoundPlayer::PlayStyle& style = obj->GetStyle();
	style.SetLoopEnable(bLoop);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetLoopTime(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	double startTime = argv[1].as_real();
	double endTime = argv[2].as_real();
	SoundPlayer::PlayStyle& style = obj->GetStyle();
	style.SetLoopStartTime(startTime);
	style.SetLoopEndTime(endTime);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetLoopSampleCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	double startSample = argv[1].as_real();
	double endSample = argv[2].as_real();

	WAVEFORMATEX fmt = obj->GetPlayer()->GetWaveFormat();
	double startTime = startSample / (double)fmt.nSamplesPerSec;
	double endTime = endSample / (double)fmt.nSamplesPerSec;;
	SoundPlayer::PlayStyle& style = obj->GetStyle();
	style.SetLoopStartTime(startTime);
	style.SetLoopEndTime(endTime);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetRestartEnable(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	bool bRestart = (bool)argv[1].as_boolean();
	SoundPlayer::PlayStyle& style = obj->GetStyle();
	style.SetRestart(bRestart);

	return value();
}
gstd::value DxScript::Func_ObjSound_SetSoundDivision(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)return value();

	int div = (int)argv[1].as_real();
	player->SetSoundDivision(div);

	return value();
}
gstd::value DxScript::Func_ObjSound_IsPlaying(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)
		return value(machine->get_engine()->get_boolean_type(), false);

	bool bPlay = player->IsPlaying();

	return value(machine->get_engine()->get_boolean_type(), bPlay);
}
gstd::value DxScript::Func_ObjSound_GetVolumeRate(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxSoundObject* obj = dynamic_cast<DxSoundObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();
	ref_count_ptr<SoundPlayer> player = obj->GetPlayer();
	if(player == NULL)
		return value(machine->get_engine()->get_real_type(),(long double)0);

	double rate = player->GetVolumeRate();

	return value(machine->get_engine()->get_real_type(),(long double)rate);
}

//Dx関数：ファイル操作(DxFileObject)
gstd::value DxScript::Func_ObjFile_Create(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
//	script->CheckRunInMainThread();
	int type = (int)argv[0].as_real();
	ref_count_ptr<DxFileObject>::unsync obj;
	if(type == OBJ_FILE_TEXT)
	{
		obj = new DxTextFileObject();
	}
	else if(type == OBJ_FILE_BINARY)
	{
		obj = new DxBinaryFileObject();
	}

	int id = ID_INVALID;
	if(obj != NULL)
	{
		obj->Initialize();
		obj->manager_ = script->objManager_.GetPointer();
		id = script->AddObject(obj);
	}
	return value(machine->get_engine()->get_real_type(),(long double)id);
}
gstd::value DxScript::Func_ObjFile_Open(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxFileObject* obj = dynamic_cast<DxFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	bool res = obj->OpenR(path);

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value DxScript::Func_ObjFile_OpenNW(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxFileObject* obj = dynamic_cast<DxFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_boolean_type(), false);

	std::wstring path = argv[1].as_string();
	path = PathProperty::GetUnique(path);
	bool res = obj->OpenW(path);

	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value DxScript::Func_ObjFile_Store(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxFileObject* obj = dynamic_cast<DxFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)	return value(machine->get_engine()->get_boolean_type(), false);

	bool res = obj->Store();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value DxScript::Func_ObjFile_GetSize(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxFileObject* obj = dynamic_cast<DxFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(),(long double)0);

	gstd::ref_count_ptr<File> file = obj->GetFile();
	int res = file != NULL ? file->GetSize() : 0;
	return value(machine->get_engine()->get_real_type(), (long double)res);
}

//Dx関数：ファイル操作(DxTextFileObject)
gstd::value DxScript::Func_ObjFileT_GetLineCount(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxTextFileObject* obj = dynamic_cast<DxTextFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(),(long double)0);

	int res = obj->GetLineCount();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
gstd::value DxScript::Func_ObjFileT_GetLineText(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxTextFileObject* obj = dynamic_cast<DxTextFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_string_type(), std::wstring());

	int pos = (int)argv[1].as_real();
	std::string line = obj->GetLine(pos);
	std::wstring res = to_wide(line);
	return value(machine->get_engine()->get_string_type(), res);
}
gstd::value DxScript::Func_ObjFileT_SplitLineText(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxTextFileObject* obj = dynamic_cast<DxTextFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_string_type(), std::wstring());

	int pos = (int)argv[1].as_real();
	std::string delim = to_mbcs(argv[2].as_string());
	std::string line = obj->GetLine(pos);
	std::vector<std::string> list = StringUtility::Split(line, delim);
	
	gstd::value res = script->CreateStringArrayValue(list);
	return res;
}
gstd::value DxScript::Func_ObjFileT_AddLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxTextFileObject* obj = dynamic_cast<DxTextFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	std::string str = to_mbcs(argv[1].as_string());
	obj->AddLine(str);
	return value();
}
gstd::value DxScript::Func_ObjFileT_ClearLine(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxTextFileObject* obj = dynamic_cast<DxTextFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value();

	obj->ClearLine();
	return value();
}

//Dx関数：ファイル操作(DxBinalyFileObject)
gstd::value DxScript::Func_ObjFileB_SetByteOrder(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return gstd::value();

	int order = (int)argv[1].as_real();
	obj->SetByteOrder(order);
	return gstd::value();
}
gstd::value DxScript::Func_ObjFileB_SetCharacterCode(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return gstd::value();

	unsigned int code = (unsigned int)argv[1].as_real();
	obj->SetCodePage(code);
	return gstd::value();
}
gstd::value DxScript::Func_ObjFileB_GetPointer(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	long double res = buffer->GetOffset();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_Seek(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return gstd::value();

	int pos = (int)argv[1].as_real();
	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	buffer->Seek(pos);
	return gstd::value();
}
gstd::value DxScript::Func_ObjFileB_ReadBoolean(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(1))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	bool res = buffer->ReadBoolean();
	return value(machine->get_engine()->get_boolean_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadByte(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(1))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	long double res = buffer->ReadCharacter();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadShort(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)0);
	if(!obj->IsReadableSize(2))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	short bv = buffer->ReadShort();
	if(obj->GetByteOrder() == ByteOrder::ENDIAN_BIG)
		ByteOrder::Reverse(&bv, sizeof(bv));

	long double res = bv;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadInteger(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(4))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	int bv = buffer->ReadInteger();
	if(obj->GetByteOrder() == ByteOrder::ENDIAN_BIG)
		ByteOrder::Reverse(&bv, sizeof(bv));

	long double res = bv;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadLong(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(8))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	_int64 bv = buffer->ReadInteger64();
	if(obj->GetByteOrder() == ByteOrder::ENDIAN_BIG)
		ByteOrder::Reverse(&bv, sizeof(bv));
	long double res = bv;
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadFloat(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(4))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	long double res = 0;
	if(obj->GetByteOrder() == ByteOrder::ENDIAN_BIG)
	{
		int bv = buffer->ReadInteger();
		ByteOrder::Reverse(&bv, sizeof(bv));
		res = (float&)bv;
	}
	else
		res = buffer->ReadFloat();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadDouble(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_real_type(), (long double)-1);
	if(!obj->IsReadableSize(8))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	long double res = 0;
	if(obj->GetByteOrder() == ByteOrder::ENDIAN_BIG)
	{
		_int64 bv = buffer->ReadInteger64();
		ByteOrder::Reverse(&bv, sizeof(bv));
		res = (double&)bv;
	}
	else
		res = buffer->ReadDouble();
	return value(machine->get_engine()->get_real_type(), res);
}
gstd::value DxScript::Func_ObjFileB_ReadString(gstd::script_machine* machine, int argc, gstd::value const * argv)
{
	DxScript* script = (DxScript*)machine->data;
	int id = (int)argv[0].as_real();
	DxBinaryFileObject* obj = dynamic_cast<DxBinaryFileObject*>(script->GetObjectPointer(id));
	if(obj == NULL)return value(machine->get_engine()->get_string_type(), std::wstring());

	int readSize = (int)argv[1].as_real();
	if(!obj->IsReadableSize(readSize))
		script->RaiseError(gstd::ErrorUtility::GetErrorMessage(ErrorUtility::ERROR_END_OF_FILE));

	ref_count_ptr<gstd::ByteBuffer> data = new gstd::ByteBuffer();
	data->SetSize(readSize);

	gstd::ref_count_ptr<gstd::ByteBuffer> buffer = obj->GetBuffer();
	buffer->Read(data->GetPointer(), readSize);

	std::wstring res;
	int code = obj->GetCodePage();
	if(code == CODE_ACP || code == CODE_UTF8)
	{
		std::string str;
		str.resize(readSize);
		memcpy(&str[0], data->GetPointer(), readSize);

		if(code == CODE_UTF8)
		{
			std::wstring wstr = StringUtility::ConvertMultiToWide(str, code);
			str = StringUtility::ConvertWideToMulti(wstr, CP_ACP);
		}
		res = to_wide(str);
	}
	else if(code == CODE_UTF16LE || code == CODE_UTF16BE)
	{
		int strSize = readSize / 2 * 2;
		int wsize = strSize / 2;
		if(code == CODE_UTF16BE)
		{
			char* pt = data->GetPointer();
			for(int iChar = 0 ; iChar < wsize ; iChar++)
			{
				int pos = iChar * 2;
				char temp = pt[pos];
				pt[pos] = pt[pos + 1];
				pt[pos + 1] = temp;
			}
		}
		res.resize(wsize);
		memcpy(&res[0], data->GetPointer(), readSize);

		std::string str = StringUtility::ConvertWideToMulti(res);
		res = to_wide(str);
	}

	return value(machine->get_engine()->get_string_type(), res);
}

