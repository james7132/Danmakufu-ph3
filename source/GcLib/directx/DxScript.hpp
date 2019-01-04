#ifndef __DIRECTX_DXSCRIPT__
#define __DIRECTX_DXSCRIPT__

#include"DxConstant.hpp"
#include"Texture.hpp"
#include"DxText.hpp"
#include"RenderObject.hpp"
#include"DirectSound.hpp"

namespace directx
{
	class DxScript;
	class DxScriptObjectManager;
	class DxScriptObjectBase;
	/**********************************************************
	//DxScriptObjectBase
	**********************************************************/
	class DxScriptObjectBase
	{
		friend DxScript;
		friend DxScriptObjectManager;
		protected:
			int idObject_;
			int typeObject_;
			_int64 idScript_;
			DxScriptObjectManager* manager_;
			double priRender_;
			bool bVisible_;
			bool bDeleted_;
			bool bActive_;

			std::map<std::wstring, gstd::value> mapObjectValue_;

		public:
			DxScriptObjectBase();
			virtual ~DxScriptObjectBase();
			void SetObjectManager(DxScriptObjectManager* manager){manager_ = manager;}
			virtual void Initialize(){}
			virtual void Work(){}
			virtual void Render() = 0;
			virtual void SetRenderState() = 0;

			int GetObjectID(){return idObject_;}
			int GetObjectType(){return typeObject_;}
			_int64 GetScriptID(){return idScript_;}
			bool IsDeleted(){return bDeleted_;}
			bool IsActive(){return bActive_;}
			void SetActive(bool bActive){bActive_ = bActive;}
			bool IsVisible(){return bVisible_;}

			double GetRenderPriority(){return priRender_;}
			int GetRenderPriorityI();
			void SetRenderPriority(double pri){priRender_ = pri;}
			void SetRenderPriorityI(int pri);

			bool IsObjectValueExists(std::wstring key){return mapObjectValue_.find(key) != mapObjectValue_.end();}
			gstd::value GetObjectValue(std::wstring key){return mapObjectValue_[key];}
			void SetObjectValue(std::wstring key, gstd::value val){mapObjectValue_[key] = val;}
			void DeleteObjectValue(std::wstring key){mapObjectValue_.erase(key);}
	};

	/**********************************************************
	//DxScriptRenderObject
	**********************************************************/
	class DxScriptRenderObject : public DxScriptObjectBase
	{
		friend DxScript;
		protected:
			bool bZWrite_;
			bool bZTest_;
			bool bFogEnable_;
			int modeCulling_;

			D3DXVECTOR3 position_;//移動先座標
			D3DXVECTOR3 angle_;//回転角度
			D3DXVECTOR3 scale_;//拡大率
			int typeBlend_;

			int idRelative_;
			std::wstring nameRelativeBone_;
		public:
			DxScriptRenderObject();
			virtual void SetX(double x){position_.x = x;}
			virtual void SetY(double y){position_.y = y;}
			virtual void SetZ(double z){position_.z = z;}
			virtual void SetAngleX(double x){angle_.x = x;}
			virtual void SetAngleY(double y){angle_.y = y;}
			virtual void SetAngleZ(double z){angle_.z = z;}
			virtual void SetScaleX(double x){scale_.x = x;}
			virtual void SetScaleY(double y){scale_.y = y;}
			virtual void SetScaleZ(double z){scale_.z = z;}
			virtual void SetColor(int r, int g, int b) = 0;
			virtual void SetAlpha(int alpha) = 0;

			D3DXVECTOR3 GetPosition(){return position_;}
			D3DXVECTOR3 GetAngle(){return angle_;}
			D3DXVECTOR3 GetScale(){return scale_;}
			void SetPosition(D3DXVECTOR3 pos){position_ = pos;}
			void SetAngle(D3DXVECTOR3 angle){angle_ = angle;}
			void SetScale(D3DXVECTOR3 scale){scale_ = scale;}

			int GetBlendType(){return typeBlend_;}
			void SetRelativeObject(int id, std::wstring bone){idRelative_ = id; nameRelativeBone_ = bone;}

			virtual gstd::ref_count_ptr<Shader> GetShader(){return NULL;}
			virtual void SetShader(gstd::ref_count_ptr<Shader> shader){}
			virtual void Render(){}
			virtual void SetRenderState(){}
	};

	/**********************************************************
	//DxScriptShaderObject
	**********************************************************/
	class DxScriptShaderObject : public DxScriptRenderObject
	{
		private:
			gstd::ref_count_ptr<Shader> shader_;

		public:
			DxScriptShaderObject();
			virtual gstd::ref_count_ptr<Shader> GetShader(){return shader_;}
			virtual void SetShader(gstd::ref_count_ptr<Shader> shader){shader_ = shader;}

			virtual void SetColor(int r, int g, int b){}
			virtual void SetAlpha(int alpha){}
	};

	/**********************************************************
	//DxScriptPrimitiveObject
	**********************************************************/
	class DxScriptPrimitiveObject : public DxScriptRenderObject
	{
		friend DxScript;
		protected:
			gstd::ref_count_ptr<RenderObject> objRender_;

		public:
			DxScriptPrimitiveObject();
			void SetPrimitiveType(D3DPRIMITIVETYPE type);
			void SetVertexCount(int count);
			int GetVertexCount();
			gstd::ref_count_ptr<Texture> GetTexture();
			virtual void SetTexture(gstd::ref_count_ptr<Texture> texture);

			virtual bool IsValidVertexIndex(int index) = 0;
			virtual void SetVertexPosition(int index, float x, float y, float z) = 0;
			virtual void SetVertexUV(int index, float u, float v) = 0;
			virtual void SetVertexAlpha(int index, int alpha) = 0;
			virtual void SetVertexColor(int index, int r, int g, int b) = 0;
			virtual D3DXVECTOR3 GetVertexPosition(int index) = 0;

			virtual gstd::ref_count_ptr<Shader> GetShader();
			virtual void SetShader(gstd::ref_count_ptr<Shader> shader);
	};

	/**********************************************************
	//DxScriptPrimitiveObject2D
	**********************************************************/
	class DxScriptPrimitiveObject2D : public DxScriptPrimitiveObject
	{
		public:
			DxScriptPrimitiveObject2D();
			virtual void Render();
			virtual void SetRenderState();
			RenderObjectTLX* GetObjectPointer(){return (RenderObjectTLX*)objRender_.GetPointer();}
			virtual bool IsValidVertexIndex(int index);
			virtual void SetColor(int r, int g, int b);
			virtual void SetAlpha(int alpha);
			virtual void SetVertexPosition(int index, float x, float y, float z);
			virtual void SetVertexUV(int index, float u, float v);
			virtual void SetVertexAlpha(int index, int alpha);
			virtual void SetVertexColor(int index, int r, int g, int b);
			void SetPermitCamera(bool bPermit);
			virtual D3DXVECTOR3 GetVertexPosition(int index);
	};

	/**********************************************************
	//DxScriptSpriteObject2D
	**********************************************************/
	class DxScriptSpriteObject2D : public DxScriptPrimitiveObject2D
	{
		public:
			DxScriptSpriteObject2D();
			void Copy(DxScriptSpriteObject2D* src);
			Sprite2D* GetSpritePointer(){return (Sprite2D*)objRender_.GetPointer();}
	};

	/**********************************************************
	//DxScriptSpriteListObject2D
	**********************************************************/
	class DxScriptSpriteListObject2D : public DxScriptPrimitiveObject2D
	{
		public:
			DxScriptSpriteListObject2D();
			virtual void SetColor(int r, int g, int b);
			virtual void SetAlpha(int alpha);
			void AddVertex();
			void CloseVertex();
			SpriteList2D* GetSpritePointer(){return (SpriteList2D*)objRender_.GetPointer();}
	};

	/**********************************************************
	//DxScriptPrimitiveObject3D
	**********************************************************/
	class DxScriptPrimitiveObject3D : public DxScriptPrimitiveObject
	{
			friend DxScript;
		public:
			DxScriptPrimitiveObject3D();
			virtual void Render();
			virtual void SetRenderState();
			RenderObjectLX* GetObjectPointer(){return (RenderObjectLX*)objRender_.GetPointer();}
			virtual bool IsValidVertexIndex(int index);
			virtual void SetColor(int r, int g, int b);
			virtual void SetAlpha(int alpha); 
			virtual void SetVertexPosition(int index, float x, float y, float z);
			virtual void SetVertexUV(int index, float u, float v);
			virtual void SetVertexAlpha(int index, int alpha);
			virtual void SetVertexColor(int index, int r, int g, int b);
			virtual D3DXVECTOR3 GetVertexPosition(int index);
	};
	/**********************************************************
	//DxScriptSpriteObject3D
	**********************************************************/
	class DxScriptSpriteObject3D : public DxScriptPrimitiveObject3D
	{
		public:
			DxScriptSpriteObject3D();
			Sprite3D* GetSpritePointer(){return (Sprite3D*)objRender_.GetPointer();}
	};

	/**********************************************************
	//DxScriptTrajectoryObject3D
	**********************************************************/
	class DxScriptTrajectoryObject3D : public DxScriptPrimitiveObject
	{
		public:
			DxScriptTrajectoryObject3D();
			virtual void Work();
			virtual void Render();
			virtual void SetRenderState();
			TrajectoryObject3D* GetObjectPointer(){return (TrajectoryObject3D*)objRender_.GetPointer();}

			virtual bool IsValidVertexIndex(int index){return false;}
			virtual void SetColor(int r, int g, int b);
			virtual void SetAlpha(int alpha){};
			virtual void SetVertexPosition(int index, float x, float y, float z){};
			virtual void SetVertexUV(int index, float u, float v){};
			virtual void SetVertexAlpha(int index, int alpha){};
			virtual void SetVertexColor(int index, int r, int g, int b){};
			virtual D3DXVECTOR3 GetVertexPosition(int index){return D3DXVECTOR3(0, 0, 0);}
	};

	/**********************************************************
	//DxScriptMeshObject
	**********************************************************/
	class DxScriptMeshObject : public DxScriptRenderObject
	{
		friend DxScript;
		protected:
			gstd::ref_count_ptr<DxMesh> mesh_;
			int time_;
			std::wstring anime_;
			D3DCOLOR color_;
			bool bCoordinate2D_;
			void _UpdateMeshState();
		public:
			DxScriptMeshObject();
			virtual void Render();
			virtual void SetRenderState();
			virtual void SetColor(int r, int g, int b);
			virtual void SetAlpha(int alpha);
			void SetMesh(gstd::ref_count_ptr<DxMesh> mesh){mesh_ = mesh;}
			gstd::ref_count_ptr<DxMesh> GetMesh(){return mesh_;}
			int GetAnimeFrame(){return time_;}
			std::wstring GetAnimeName(){return anime_;}
			
			virtual void SetX(float x){position_.x = x;_UpdateMeshState();}
			virtual void SetY(float y){position_.y = y;_UpdateMeshState();}
			virtual void SetZ(float z){position_.z = z;_UpdateMeshState();}
			virtual void SetAngleX(float x){angle_.x = x;_UpdateMeshState();}
			virtual void SetAngleY(float y){angle_.y = y;_UpdateMeshState();}
			virtual void SetAngleZ(float z){angle_.z = z;_UpdateMeshState();}
			virtual void SetScaleX(float x){scale_.x = x;_UpdateMeshState();}
			virtual void SetScaleY(float y){scale_.y = y;_UpdateMeshState();}
			virtual void SetScaleZ(float z){scale_.z = z;_UpdateMeshState();}
			virtual void SetShader(gstd::ref_count_ptr<Shader> shader);
	};

	/**********************************************************
	//DxScriptTextObject
	**********************************************************/
	class DxScriptTextObject : public DxScriptRenderObject
	{
			friend DxScript;
		protected:
			bool bChange_;
			DxText text_;
			gstd::ref_count_ptr<DxTextInfo> textInfo_;
			gstd::ref_count_ptr<DxTextRenderObject> objRender_;
			D3DXVECTOR2 center_;//座標変換の中心
			bool bAutoCenter_;

			void _UpdateRenderer();

		public:
			DxScriptTextObject();
			virtual void Render();
			virtual void SetRenderState();

			void SetText(std::wstring text){text_.SetText(text);bChange_=true;}
			std::wstring GetText(){return text_.GetText();}
			std::vector<int> GetTextCountCU();
			int GetTotalWidth();
			int GetTotalHeight();

			void SetFontType(std::wstring type){text_.SetFontType(type.c_str());bChange_=true;}
			void SetFontSize(int size){text_.SetFontSize(size);bChange_=true;}
			void SetFontBold(bool bBold){text_.SetFontBold(bBold);}
			void SetFontItalic(bool bItalic){text_.SetFontItalic(bItalic);bChange_=true;}
			void SetFontUnderLine(bool bLine){text_.SetFontUnderLine(bLine);bChange_=true;}
			
			void SetFontColorTop(int r, int g, int b){text_.SetFontColorTop(D3DCOLOR_ARGB(255, r, g, b));bChange_=true;}
			void SetFontColorBottom(int r, int g, int b){text_.SetFontColorBottom(D3DCOLOR_ARGB(255, r, g, b));bChange_=true;}
			void SetFontBorderWidth(int width){text_.SetFontBorderWidth(width);bChange_=true;}
			void SetFontBorderType(int type){text_.SetFontBorderType(type);bChange_=true;}
			void SetFontBorderColor(int r, int g, int b){text_.SetFontBorderColor(D3DCOLOR_ARGB(255, r, g, b));bChange_=true;}

			void SetMaxWidth(int width){text_.SetMaxWidth(width);bChange_=true;}
			void SetMaxHeight(int height){text_.SetMaxHeight(height);bChange_=true;}
			void SetLinePitch(int pitch){text_.SetLinePitch(pitch);bChange_=true;}
			void SetSidePitch(int pitch){text_.SetSidePitch(pitch);bChange_=true;}
			void SetHorizontalAlignment(int value){text_.SetHorizontalAlignment(value);bChange_=true;}
			void SetVerticalAlignment(int value){text_.SetVerticalAlignment(value);bChange_=true;}
			void SetPermitCamera(bool bPermit){text_.SetPermitCamera(bPermit);}
			void SetSyntacticAnalysis(bool bEnable){text_.SetSyntacticAnalysis(bEnable);}

			virtual void SetAlpha(int alpha);
			virtual void SetColor(int r, int g, int b);
			void SetVertexColor(D3DCOLOR color){text_.SetVertexColor(color);}
			virtual void SetShader(gstd::ref_count_ptr<Shader> shader);
	};

	/**********************************************************
	//DxSoundObject
	**********************************************************/
	class DxSoundObject : public DxScriptObjectBase
	{
			friend DxScript;
		protected:
			gstd::ref_count_ptr<SoundPlayer> player_;
			SoundPlayer::PlayStyle style_;
		public:
			DxSoundObject();
			~DxSoundObject();
			virtual void Render(){}
			virtual void SetRenderState(){}

			bool Load(std::wstring path);
			void Play();

			gstd::ref_count_ptr<SoundPlayer> GetPlayer(){return player_;}
			SoundPlayer::PlayStyle& GetStyle(){return style_;}
	};

	/**********************************************************
	//DxFileObject
	**********************************************************/
	class DxFileObject : public DxScriptObjectBase
	{
			friend DxScript;
		protected:
			gstd::ref_count_ptr<gstd::File> file_;

		public:
			DxFileObject();
			~DxFileObject();
			gstd::ref_count_ptr<gstd::File> GetFile(){return file_;}

			virtual void Render(){}
			virtual void SetRenderState(){}

			virtual bool OpenR(std::wstring path);
			virtual bool OpenW(std::wstring path);
			virtual bool Store() = 0;
			virtual void Close();	
	};

	/**********************************************************
	//DxTextFileObject
	**********************************************************/
	class DxTextFileObject : public DxFileObject
	{
		protected:
			std::vector<std::string> listLine_;

		public:
			DxTextFileObject();
			virtual ~DxTextFileObject();
			virtual bool OpenR(std::wstring path);
			virtual bool OpenW(std::wstring path);
			virtual bool Store();
			int GetLineCount(){return listLine_.size();}
			std::string GetLine(int line);

			void AddLine(std::string line){listLine_.push_back(line);}
			void ClearLine(){listLine_.clear();}
	};

	/**********************************************************
	//DxBinaryFileObject
	**********************************************************/
	class DxBinaryFileObject : public DxFileObject
	{
		protected:
			int byteOrder_;
			unsigned int codePage_;
			gstd::ref_count_ptr<gstd::ByteBuffer> buffer_;

		public:
			DxBinaryFileObject();
			virtual ~DxBinaryFileObject();
			virtual bool OpenR(std::wstring path);
			virtual bool OpenW(std::wstring path);
			virtual bool Store();

			gstd::ref_count_ptr<gstd::ByteBuffer> GetBuffer(){return buffer_;}
			bool IsReadableSize(int size);

			unsigned int GetCodePage(){return codePage_;}
			void SetCodePage(unsigned int page){codePage_ = page;}

			void SetByteOrder(int order){byteOrder_ = order;}
			int GetByteOrder(){return byteOrder_;}
	};

	/**********************************************************
	//DxScriptObjectManager
	**********************************************************/
	class DxScriptObjectManager
	{
		friend DxScriptObjectBase;
		public:
			struct SoundInfo 
			{
				gstd::ref_count_ptr<SoundPlayer> player_;
				SoundPlayer::PlayStyle style_;
				virtual ~SoundInfo(){}
			};
		protected:
			_int64 totalObjectCreateCount_;
			std::list<int> listUnusedIndex_;
			std::vector<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > obj_;//オブジェクト
			std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > listActiveObject_;
			std::map<std::wstring, gstd::ref_count_ptr<SoundInfo> > mapReservedSound_;

			//フォグ
			bool bFogEnable_;
			D3DCOLOR fogColor_;
			float fogStart_;
			float fogEnd_;

			std::vector<std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > > objRender_;//描画バケットソート
			std::vector<gstd::ref_count_ptr<Shader> > listShader_;

			void _SetObjectID(DxScriptObjectBase* obj, int index){obj->idObject_ = index;obj->manager_ = this;}
			void _ArrangeActiveObjectList();
		public:
			DxScriptObjectManager();
			virtual ~DxScriptObjectManager();
			int GetMaxObject(){return obj_.size();}
			void SetMaxObject(int max);
			int GetAliveObjectCount(){return obj_.size() - listUnusedIndex_.size();}
			int GetRenderBucketCapacity(){return objRender_.size();}
			void SetRenderBucketCapacity(int capacity);
			virtual int AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate = true);
			void AddObject(int id, gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate = true);
			void ActivateObject(int id, bool bActivate);
			gstd::ref_count_ptr<DxScriptObjectBase>::unsync GetObject(int id){return obj_[id];}
			std::vector<int> GetValidObjectIdentifier();
			DxScriptObjectBase* GetObjectPointer(int id);
			virtual void DeleteObject(int id);
			void ClearObject();
			void DeleteObjectByScriptID(_int64 idScript);
			void AddRenderObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj);//要フレームごとに登録
			void WorkObject();
			void RenderObject();

			void PrepareRenderObject();
			void ClearRenderObject();
			std::vector<std::list<gstd::ref_count_ptr<DxScriptObjectBase>::unsync > >* GetRenderObjectListPointer(){return &objRender_;}

			void SetShader(gstd::ref_count_ptr<Shader> shader, double min, double max);
			void ResetShader();
			void ResetShader(double min, double max);
			gstd::ref_count_ptr<Shader> GetShader(int index);

			void ReserveSound(gstd::ref_count_ptr<SoundPlayer> player, SoundPlayer::PlayStyle& style);
			void DeleteReservedSound(gstd::ref_count_ptr<SoundPlayer> player);
			void SetFogParam(bool bEnable, D3DCOLOR fogColor, float start, float end);
			_int64 GetTotalObjectCreateCount(){return totalObjectCreateCount_;}

			bool IsFogEneble(){return bFogEnable_;}
			D3DCOLOR GetFogColor(){return fogColor_;}
			float GetFogStart(){return fogStart_;}
			float GetFogEnd(){return fogEnd_;}
	};

	/**********************************************************
	//DxScript
	**********************************************************/
	class DxScript : public gstd::ScriptClientBase
	{
		public:
			enum
			{
				ID_INVALID = -1,
				OBJ_INVALID = -1,
				OBJ_PRIMITIVE_2D = 1,
				OBJ_SPRITE_2D,
				OBJ_SPRITE_LIST_2D,
				OBJ_PRIMITIVE_3D,
				OBJ_SPRITE_3D,
				OBJ_TRAJECTORY_3D,
				OBJ_SHADER,

				OBJ_MESH,
				OBJ_TEXT,
				OBJ_SOUND,

				OBJ_FILE_TEXT,
				OBJ_FILE_BINARY,

				CODE_ACP = CP_ACP,
				CODE_UTF8 = CP_UTF8,
				CODE_UTF16LE,
				CODE_UTF16BE,
			};

		protected:
			gstd::ref_count_ptr<DxScriptObjectManager> objManager_;

			//リソース
			std::map<std::wstring, gstd::ref_count_ptr<Texture> > mapTexture_;
			std::map<std::wstring, gstd::ref_count_ptr<SoundPlayer> > mapSoundPlayer_;
			std::map<std::wstring, gstd::ref_count_ptr<DxMesh> > mapMesh_;

			void _ClearResource();
			gstd::ref_count_ptr<Texture> _GetTexture(std::wstring name);

		public:
			DxScript();
			virtual ~DxScript();

			void SetObjectManager(gstd::ref_count_ptr<DxScriptObjectManager> manager){objManager_ = manager;}
			gstd::ref_count_ptr<DxScriptObjectManager> GetObjectManager(){return objManager_;}
			void SetMaxObject(int max){objManager_->SetMaxObject(max);}
			void SetRenderBucketCapacity(int capacity){objManager_->SetRenderBucketCapacity(capacity);}
			virtual int AddObject(gstd::ref_count_ptr<DxScriptObjectBase>::unsync obj, bool bActivate = true);
			virtual void ActivateObject(int id, bool bActivate){objManager_->ActivateObject(id, bActivate);}
			gstd::ref_count_ptr<DxScriptObjectBase>::unsync GetObject(int id){return objManager_->GetObject(id);}
			DxScriptObjectBase* GetObjectPointer(int id){return objManager_->GetObjectPointer(id);}
			virtual void DeleteObject(int id){objManager_->DeleteObject(id);}
			void ClearObject(){objManager_->ClearObject();}
			virtual void WorkObject(){objManager_->WorkObject();}
			virtual void RenderObject(){objManager_->RenderObject();}

			void AddMeshResource(std::wstring name, gstd::ref_count_ptr<DxMesh> mesh){mapMesh_[name] = mesh;}

			//Dx関数：システム系
			static gstd::value Func_InstallFont(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：音声系
			static gstd::value Func_LoadSound(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_RemoveSound(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_PlayBGM(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_PlaySE(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_StopSound(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：キー系
			static gstd::value Func_GetKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetMouseX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetMouseY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetMouseMoveZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetMouseState(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetVirtualKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetVirtualKeyState(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：描画系
			static gstd::value Func_GetScreenWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetScreenHeight(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_LoadTexture(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_LoadTextureInLoadThread(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_RemoveTexture(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetTextureWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetTextureHeight(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetFogEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetFogParam(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_CreateRenderTarget(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetRenderTarget(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetTransitionRenderTargetName(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SaveRenderedTextureA1(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SaveRenderedTextureA2(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetShader(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetShaderI(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ResetShader(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ResetShaderI(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_IsPixelShaderSupported(gstd::script_machine* machine, int argc, gstd::value const * argv);


			//Dx関数：カメラ3D
			static gstd::value Func_SetCameraFocusX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraFocusY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraFocusZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraFocusXYZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraRadius(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraAzimuthAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraElevationAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraYaw(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraPitch(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_SetCameraRoll(gstd::script_machine* machine, int argc, gstd::value const * argv);
										
			static gstd::value Func_GetCameraX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraFocusX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraFocusY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraFocusZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraRadius(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraAzimuthAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraElevationAngle(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraYaw(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraPitch(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetCameraRoll(gstd::script_machine* machine, int argc, gstd::value const * argv);
			
			static gstd::value Func_SetCameraPerspectiveClip(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：カメラ2D
			static gstd::value Func_Set2DCameraFocusX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Set2DCameraFocusY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Set2DCameraAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Set2DCameraRatio(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Set2DCameraRatioX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Set2DCameraRatioY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Reset2DCamera(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraRatio(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraRatioX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2DCameraRatioY(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：その他
			static gstd::value Func_GetObjectDistance(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_GetObject2dPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Get2dPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);


			//Dx関数：オブジェクト操作(共通)
			static gstd::value Func_Obj_Delete(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_IsDeleted(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_SetVisible(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_IsVisible(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_SetRenderPriority(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_SetRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_GetRenderPriority(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_GetRenderPriorityI(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_GetValue(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_GetValueD(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_SetValue(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_DeleteValue(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_IsValueExists(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_Obj_GetType(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(RenderObject)
			static gstd::value Func_ObjRender_SetX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetAngleX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetAngleY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetAngleXYZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetScaleX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetScaleY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetScaleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetScaleXYZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetColor(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetColorHSV(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetAlpha(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetBlendType(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetAngleX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetAngleY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetAngleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetScaleX(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetScaleY(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetScaleZ(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetZWrite(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetZTest(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetFogEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetCullingMode(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetRalativeObject(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_SetPermitCamera(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjRender_GetBlendType(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(ShaderObject)
			static gstd::value Func_ObjShader_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetShaderF(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetShaderO(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_ResetShader(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetTechnique(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetMatrix(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetMatrixArray(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetVector(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetFloat(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetFloatArray(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjShader_SetTexture(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(PrimitiveObject)
			static gstd::value Func_ObjPrimitive_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetPrimitiveType(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetTexture(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_GetVertexCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexUV(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexUVT(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexColor(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_SetVertexAlpha(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjPrimitive_GetVertexPosition(gstd::script_machine* machine, int argc, gstd::value const * argv);


			//Dx関数：オブジェクト操作(Sprite2D)
			static gstd::value Func_ObjSprite2D_SetSourceRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSprite2D_SetDestRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSprite2D_SetDestCenter(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(SpriteList2D)
			static gstd::value Func_ObjSpriteList2D_SetSourceRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSpriteList2D_SetDestRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSpriteList2D_SetDestCenter(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSpriteList2D_AddVertex(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSpriteList2D_CloseVertex(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSpriteList2D_ClearVertexCount(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(Sprite3D)
			static gstd::value Func_ObjSprite3D_SetSourceRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSprite3D_SetDestRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSprite3D_SetSourceDestRect(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSprite3D_SetBillboard(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(TrajectoryObject3D)
			static gstd::value Func_ObjTrajectory3D_SetInitialPoint(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjTrajectory3D_SetAlphaVariation(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjTrajectory3D_SetComplementCount(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：オブジェクト操作(DxMesh)
			static gstd::value Func_ObjMesh_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_Load(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_SetColor(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_SetAlpha(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_SetAnimation(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_SetCoordinate2D(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjMesh_GetPath(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：テキスト操作(DxText)
			static gstd::value Func_ObjText_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetText(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontType(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontSize(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontBold(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontColorTop(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontColorBottom(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontBorderWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontBorderType(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetFontBorderColor(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetMaxWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetMaxHeight(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetLinePitch(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetSidePitch(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetVertexColor(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetTransCenter(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetAutoTransCenter(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetHorizontalAlignment(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_SetSyntacticAnalysis(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_GetTextLength(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_GetTextLengthCU(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_GetTextLengthCUL(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_GetTotalWidth(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjText_GetTotalHeight(gstd::script_machine* machine, int argc, gstd::value const * argv);
						
			//Dx関数：音声操作(DxSoundObject)
			static gstd::value Func_ObjSound_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_Load(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_Play(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_Stop(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetVolumeRate(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetPanRate(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetFade(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetLoopEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetLoopTime(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetLoopSampleCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetRestartEnable(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_SetSoundDivision(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_IsPlaying(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjSound_GetVolumeRate(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：ファイル操作(DxFileObject)
			static gstd::value Func_ObjFile_Create(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFile_Open(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFile_OpenNW(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFile_Store(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFile_GetSize(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：ファイル操作(DxTextFileObject)
			static gstd::value Func_ObjFileT_GetLineCount(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileT_GetLineText(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileT_SplitLineText(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileT_AddLine(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileT_ClearLine(gstd::script_machine* machine, int argc, gstd::value const * argv);

			//Dx関数：ファイル操作(DxBinalyFileObject)
			static gstd::value Func_ObjFileB_SetByteOrder(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_SetCharacterCode(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_GetPointer(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_Seek(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadBoolean(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadByte(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadShort(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadInteger(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadLong(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadFloat(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadDouble(gstd::script_machine* machine, int argc, gstd::value const * argv);
			static gstd::value Func_ObjFileB_ReadString(gstd::script_machine* machine, int argc, gstd::value const * argv);
	};
}

#endif
