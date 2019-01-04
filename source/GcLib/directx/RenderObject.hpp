#ifndef __DIRECTX_RENDEROBJECT__
#define __DIRECTX_RENDEROBJECT__

#include"DxConstant.hpp"
#include"DxUtility.hpp"
#include"Texture.hpp"
#include"Shader.hpp"

namespace directx
{
	class RenderObjectBase;
	class RenderManager;
	/**********************************************************
	//FVF頂点フォーマット
	//http://msdn.microsoft.com/ja-jp/library/cc324487.aspx
	**********************************************************/
	struct VERTEX_TL
	{
		//座標3D変換済み、ライティング済み
		VERTEX_TL(){}
		VERTEX_TL(D3DXVECTOR4 pos, D3DCOLOR dcol):position(pos),diffuse_color(dcol){}
		D3DXVECTOR4 position;
		D3DCOLOR diffuse_color;
		enum{ fvf = (D3DFVF_XYZRHW|D3DFVF_DIFFUSE) };
	};

	struct VERTEX_TLX
	{
		//座標3D変換済み、ライティング済み、テクスチャ有り
		VERTEX_TLX(){}
		VERTEX_TLX(D3DXVECTOR4 pos,D3DCOLOR diffcol, D3DXVECTOR2 tex):position(pos),diffuse_color(diffcol),texcoord(tex){}
		D3DXVECTOR4 position;
		D3DCOLOR diffuse_color;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1) };
	};

	struct VERTEX_L
	{
		//ライティング済み
		VERTEX_L(){}
		VERTEX_L(const D3DXVECTOR3 &pos, const D3DCOLOR& col):position(pos),diffuse_color(col){}
		D3DXVECTOR3 position;
		D3DCOLOR diffuse_color;
		enum{ fvf = (D3DFVF_XYZ|D3DFVF_DIFFUSE) };
	};

	struct VERTEX_LX
	{
		//ライティング済み、テクスチャ有り
		VERTEX_LX(){}
		VERTEX_LX(D3DXVECTOR3 &pos, D3DCOLOR &diffcol,D3DXVECTOR2 tex):position(pos),diffuse_color(diffcol),texcoord(tex){}
		D3DXVECTOR3 position;
		D3DCOLOR diffuse_color;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1) };
	};

	struct VERTEX_N
	{
		//未ライティング
		VERTEX_N(){}
		VERTEX_N(D3DXVECTOR3 pos, D3DXVECTOR3 n):position(pos),normal(n){}
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		enum{ fvf = (D3DFVF_XYZ|D3DFVF_NORMAL) };
	};

	struct VERTEX_NX
	{
		//未ライティング、テクスチャ有り
		VERTEX_NX(){}
		VERTEX_NX(D3DXVECTOR3 &pos, D3DXVECTOR3 &n, D3DXVECTOR2 &tc):position(pos),normal(n),texcoord(tc){}
		D3DXVECTOR3 position;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) };
	};

	struct VERTEX_NXG
	{
		VERTEX_NXG(){}
		VERTEX_NXG(D3DXVECTOR3 &pos, D3DXVECTOR3 &n, D3DXVECTOR2 &tc):position(pos),normal(n),texcoord(tc){}
		D3DXVECTOR3 position;
		float blend[3];
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZB3|D3DFVF_NORMAL|D3DFVF_TEX1) };
	};

	struct VERTEX_B1NX
	{
		//未ライティング、テクスチャ有り、頂点ブレンド1
		VERTEX_B1NX(){}
		VERTEX_B1NX(D3DXVECTOR3 &pos,DWORD bi, D3DXVECTOR3 &n, D3DXVECTOR2 &tc):position(pos),normal(n),texcoord(tc)
		{
			blendIndex=bi;
		}
		D3DXVECTOR3 position;
		DWORD blendIndex;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZB1|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_TEX1) };
	};

	struct VERTEX_B2NX
	{
		//未ライティング、テクスチャ有り、頂点ブレンド2
		VERTEX_B2NX(){}
		VERTEX_B2NX(D3DXVECTOR3 &pos,float rate, BYTE index1, BYTE index2, D3DXVECTOR3 &n, D3DXVECTOR2 &tc):position(pos),normal(n),texcoord(tc)
		{
			blendRate = rate;
			gstd::BitAccess::SetByte(blendIndex, 0, index1);
			gstd::BitAccess::SetByte(blendIndex, 8, index2);
		}
		D3DXVECTOR3 position;
		float blendRate;
		DWORD blendIndex;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZB2|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_TEX1) };
	};

	struct VERTEX_B4NX
	{
		//未ライティング、テクスチャ有り、頂点ブレンド4
		VERTEX_B4NX(){}
		VERTEX_B4NX(D3DXVECTOR3 &pos,float rate[3], BYTE index[4], D3DXVECTOR3 &n, D3DXVECTOR2 &tc):position(pos),normal(n),texcoord(tc)
		{
			for(int iRate = 0 ; iRate < 3 ; iRate++)
				blendRate[iRate] = rate[iRate];
			for(int iIndex = 0 ; iIndex < 4 ; iIndex++)
				gstd::BitAccess::SetByte(blendIndex, 8 * iIndex, index[iIndex]);
		}
		D3DXVECTOR3 position;
		float blendRate[3];
		DWORD blendIndex;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 texcoord;
		enum{ fvf = (D3DFVF_XYZB4|D3DFVF_LASTBETA_UBYTE4|D3DFVF_NORMAL|D3DFVF_TEX1) };
	};

	class RenderStateFunction;
	class RenderBlock;
	class RenderObject;

	/**********************************************************
	//RenderBlock
	**********************************************************/
	class RenderBlock
	{
		protected:
			float posSortKey_;
			gstd::ref_count_ptr<RenderStateFunction> func_;
			gstd::ref_count_ptr<RenderObject> obj_;

			D3DXVECTOR3 position_;//移動先座標
			D3DXVECTOR3 angle_;//回転角度
			D3DXVECTOR3 scale_;//拡大率

		public:
			RenderBlock();
			virtual ~RenderBlock();
			void SetRenderFunction(gstd::ref_count_ptr<RenderStateFunction> func){func_ = func;}
			virtual void Render();

			virtual void CalculateZValue() = 0;
			float GetZValue(){return posSortKey_;}
			void SetZValue(float pos){posSortKey_ = pos;}
			virtual bool IsTranslucent() = 0;//Zソート対象に使用

			void SetRenderObject(gstd::ref_count_ptr<RenderObject> obj){obj_ = obj;}
			gstd::ref_count_ptr<RenderObject> GetRenderObject(){return obj_;}
			void SetPosition(D3DXVECTOR3& pos){position_ = pos;}
			void SetAngle(D3DXVECTOR3& angle){angle_ = angle;}
			void SetScale(D3DXVECTOR3& scale){scale_ = scale;}
	};

	class RenderBlocks
	{
		protected:
			std::list<gstd::ref_count_ptr<RenderBlock> > listBlock_;
		public:
			RenderBlocks(){};
			virtual ~RenderBlocks(){};
			void Add(gstd::ref_count_ptr<RenderBlock> block){listBlock_.push_back(block);}
			std::list<gstd::ref_count_ptr<RenderBlock> >& GetList(){return listBlock_;}

	};

	/**********************************************************
	//RenderManager
	//レンダリング管理
	//3D不透明オブジェクト
	//3D半透明オブジェクトZソート順
	//2Dオブジェクト
	//順に描画する
	**********************************************************/
	class RenderManager
	{
		class ComparatorRenderBlockTranslucent;
		protected:
			std::list<gstd::ref_count_ptr<RenderBlock> > listBlockOpaque_;
			std::list<gstd::ref_count_ptr<RenderBlock> > listBlockTranslucent_;
		public:
			RenderManager();
			virtual ~RenderManager();
			virtual void Render();
			void AddBlock(gstd::ref_count_ptr<RenderBlock> block);
			void AddBlock(gstd::ref_count_ptr<RenderBlocks> blocks);
	};

	class RenderManager::ComparatorRenderBlockTranslucent
	{
		public:
			bool operator()(gstd::ref_count_ptr<RenderBlock> l, gstd::ref_count_ptr<RenderBlock> r)
			{
				return l->GetZValue() > r->GetZValue();
			}
	};

	/**********************************************************
	//RenderStateFunction
	**********************************************************/
	class RenderStateFunction
	{
		friend RenderObjectBase;
			enum FUNC_TYPE
			{
				FUNC_LIGHTING,
				FUNC_CULLING,
				FUNC_ZBUFFER_ENABLE,
				FUNC_ZBUFFER_WRITE_ENABLE,
				FUNC_BLEND,
				FUNC_TEXTURE_FILTER,
			};

			std::map<FUNC_TYPE, gstd::ref_count_ptr<gstd::ByteBuffer> > mapFuncRenderState_;
		public:
			RenderStateFunction();
			virtual ~RenderStateFunction();
			void CallRenderStateFunction();

			//レンダリングステート設定(RenderManager用)
			void SetLightingEnable(bool bEnable);//ライティング
			void SetCullingMode(DWORD mode);//カリング
			void SetZBufferEnable(bool bEnable);//Zバッファ参照
			void SetZWriteEnalbe(bool bEnable);//Zバッファ書き込み
			void SetBlendMode(DWORD mode, int stage = 0);
			void SetTextureFilter(DWORD mode, int stage = 0);
	};

	class Matrices
	{
		std::vector<D3DXMATRIX> matrix_;
		public:
			Matrices(){};
			virtual ~Matrices(){};
			void SetSize(int size){matrix_.resize(size);for(int iMat=0;iMat<size;iMat++){D3DXMatrixIdentity(&matrix_[iMat]);}}
			int GetSize(){return matrix_.size();}
			void SetMatrix(int index, D3DXMATRIX& mat){matrix_[index] = mat;}
			D3DXMATRIX& GetMatrix(int index){return matrix_[index];}
	};

	/**********************************************************
	//RenderObject
	//レンダリングオブジェクト
	//描画の最小単位
	//RenderManagerに登録して描画してもらう
	//(直接描画も可能)
	**********************************************************/
	class RenderObject
	{
		protected:
			D3DPRIMITIVETYPE typePrimitive_;//
			int strideVertexStreamZero_;//1頂点のサイズ
			gstd::ByteBuffer vertex_;//頂点
			std::vector<short> vertexIndices_;
			std::vector<gstd::ref_count_ptr<Texture> > texture_;//テクスチャ
			D3DXVECTOR3 posWeightCenter_;//重心

			//シェーダ用
			IDirect3DVertexDeclaration9* pVertexDecl_;
			IDirect3DVertexBuffer9* pVertexBuffer_;
			IDirect3DIndexBuffer9* pIndexBuffer_;
			
			D3DXVECTOR3 position_;//移動先座標
			D3DXVECTOR3 angle_;//回転角度
			D3DXVECTOR3 scale_;//拡大率
			D3DXMATRIX matRelative_;//関係行列
			bool bCoordinate2D_;//2D座標指定
			gstd::ref_count_ptr<Shader> shader_;

			virtual void _ReleaseVertexBuffer();
			virtual void _RestoreVertexBuffer();
			virtual void _CreateVertexDeclaration(){}

			int _GetPrimitiveCount();
			void _SetTextureStageCount(int count){texture_.resize(count);for(int i=0;i<count;i++)texture_[i]=NULL;}
			virtual D3DXMATRIX _CreateWorldTransformMaxtrix();//position_,angle_,scale_から作成
			void _SetCoordinate2dDeviceMatrix();
		public:
			RenderObject();
			virtual ~RenderObject();
			virtual void Render() = 0;
			virtual void InitializeVertexBuffer(){}
			virtual void CalculateWeightCenter(){}
			D3DXVECTOR3 GetWeightCenter(){return posWeightCenter_;}
			gstd::ref_count_ptr<Texture> GetTexture(int pos = 0){return texture_[pos];}

			void SetRalativeMatrix(D3DXMATRIX mat){matRelative_ = mat;}

			//頂点設定
			void SetPrimitiveType(D3DPRIMITIVETYPE type){typePrimitive_ = type;}
			virtual void SetVertexCount(int count){vertex_.SetSize(count * strideVertexStreamZero_); ZeroMemory(vertex_.GetPointer(), vertex_.GetSize());}
			virtual int GetVertexCount(){return vertex_.GetSize() / strideVertexStreamZero_;}
			void SetVertexIndicies(std::vector<short>& indecies){vertexIndices_ = indecies;}
			gstd::ByteBuffer* GetVertexPointer(){return &vertex_;}

			//描画用設定
			void SetPosition(D3DXVECTOR3& pos){position_ = pos;}
			void SetPosition(float x, float y, float z){position_.x=x; position_.y=y; position_.z=z;}
			void SetX(float x){position_.x=x;}
			void SetY(float y){position_.y=y;}
			void SetZ(float z){position_.z=z;}
			void SetAngle(D3DXVECTOR3& angle){angle_ = angle;}
			void SetAngleXYZ(float angx=0.0f, float angy=0.0f, float angz=0.0f){angle_.x=angx; angle_.y=angy; angle_.z=angz;}
			void SetScale(D3DXVECTOR3& scale){scale_ = scale;}
			void SetScaleXYZ(float sx=1.0f, float sy=1.0f, float sz=1.0f){scale_.x=sx;scale_.y=sy;scale_.z=sz;}
			void SetTexture(Texture* texture, int stage = 0);//テクスチャ設定
			void SetTexture(gstd::ref_count_ptr<Texture> texture, int stage = 0);//テクスチャ設定

			bool IsCoordinate2D(){return bCoordinate2D_;}
			void SetCoordinate2D(bool b){bCoordinate2D_ = b;}

			gstd::ref_count_ptr<Shader> GetShader(){return shader_;}
			void SetShader(gstd::ref_count_ptr<Shader> shader){shader_ = shader;}
			void BeginShader();
			void EndShader();
	};

	/**********************************************************
	//RenderObjectTLX
	//座標3D変換済み、ライティング済み、テクスチャ有り
	//2D自由変形スプライト用
	**********************************************************/
	class RenderObjectTLX : public RenderObject
	{
		protected:
			bool bPermitCamera_;
			gstd::ByteBuffer vertCopy_;

			virtual void _CreateVertexDeclaration();
		public:
			RenderObjectTLX();
			~RenderObjectTLX();
			virtual void Render();
			virtual void SetVertexCount(int count);

			//頂点設定
			VERTEX_TLX* GetVertex(int index);
			void SetVertex(int index, VERTEX_TLX& vertex);
			void SetVertexPosition(int index, float x, float y, float z = 1.0f, float w = 1.0f);
			void SetVertexUV(int index, float u, float v);
			void SetVertexColor(int index, D3DCOLOR color);
			void SetVertexColorARGB(int index, int a, int r, int g, int b);
			void SetVertexAlpha(int index, int alpha);
			void SetVertexColorRGB(int index, int r, int g, int b);
			void SetColorRGB(D3DCOLOR color);
			void SetAlpha(int alpha);

			//カメラ
			bool IsPermitCamera(){return bPermitCamera_;}
			void SetPermitCamera(bool bPermit){bPermitCamera_ = bPermit;}
	};

	/**********************************************************
	//RenderObjectLX
	//ライティング済み、テクスチャ有り
	//3Dエフェクト用
	**********************************************************/
	class RenderObjectLX : public RenderObject
	{
		protected:
			virtual void _CreateVertexDeclaration();
		public:
			RenderObjectLX();
			~RenderObjectLX();
			virtual void Render();
			virtual void SetVertexCount(int count);

			//頂点設定
			VERTEX_LX* GetVertex(int index);
			void SetVertex(int index, VERTEX_LX& vertex);
			void SetVertexPosition(int index, float x, float y, float z);
			void SetVertexUV(int index, float u, float v);
			void SetVertexColor(int index, D3DCOLOR color);
			void SetVertexColorARGB(int index, int a, int r, int g, int b);
			void SetVertexAlpha(int index, int alpha);
			void SetVertexColorRGB(int index, int r, int g, int b);
			void SetColorRGB(D3DCOLOR color);
			void SetAlpha(int alpha);
	};

	/**********************************************************
	//RenderObjectNX
	//法線有り、テクスチャ有り
	**********************************************************/
	class RenderObjectNX : public RenderObject
	{
		protected:
			D3DCOLOR color_;
			virtual void _CreateVertexDeclaration();
		public:
			RenderObjectNX();
			~RenderObjectNX();
			virtual void Render();

			//頂点設定
			VERTEX_NX* GetVertex(int index);
			void SetVertex(int index, VERTEX_NX& vertex);
			void SetVertexPosition(int index, float x, float y, float z);
			void SetVertexUV(int index, float u, float v);
			void SetVertexNormal(int index, float x, float y, float z);
			void SetColor(D3DCOLOR color){color_ = color;}
	};

	/**********************************************************
	//RenderObjectBNX
	//頂点ブレンド
	//法線有り
	//テクスチャ有り
	**********************************************************/
	class RenderObjectBNX : public RenderObject
	{
		public:
			struct Vertex
			{
				D3DXVECTOR3 position;
				D3DXVECTOR4 blendRate;
				D3DXVECTOR4 blendIndex;
				D3DXVECTOR3 normal;
				D3DXVECTOR2 texcoord;
			};
		protected:
			gstd::ref_count_ptr<Matrices> matrix_;
			D3DCOLOR color_;
			D3DMATERIAL9 materialBNX_;
			virtual void _CreateVertexDeclaration();
			virtual void _CopyVertexBufferOnInitialize() = 0;
		public:
			RenderObjectBNX();
			~RenderObjectBNX();
			virtual void InitializeVertexBuffer();
			virtual void Render();

			//描画用設定
			void SetMatrix(gstd::ref_count_ptr<Matrices> matrix){matrix_ = matrix;}
			void SetColor(D3DCOLOR color){color_ = color;}
	};

	class RenderObjectBNXBlock : public RenderBlock
	{
		protected:
			gstd::ref_count_ptr<Matrices> matrix_;
			D3DCOLOR color_;

		public:
			void SetMatrix(gstd::ref_count_ptr<Matrices> matrix){matrix_ = matrix;}
			void SetColor(D3DCOLOR color){color_ = color;}
			bool IsTranslucent() {return ColorAccess::GetColorA(color_) != 255;}
	};

	/**********************************************************
	//RenderObjectB2NX
	//頂点ブレンド2
	//法線有り
	//テクスチャ有り
	**********************************************************/
	class RenderObjectB2NX : public RenderObjectBNX
	{
		protected:
			virtual void _CopyVertexBufferOnInitialize();
		public:
			RenderObjectB2NX();
			~RenderObjectB2NX();

			virtual void CalculateWeightCenter();

			//頂点設定
			VERTEX_B2NX* GetVertex(int index);
			void SetVertex(int index, VERTEX_B2NX& vertex);
			void SetVertexPosition(int index, float x, float y, float z);
			void SetVertexUV(int index, float u, float v);
			void SetVertexBlend(int index, int pos, BYTE indexBlend, float rate);
			void SetVertexNormal(int index, float x, float y, float z);
	};

	class RenderObjectB2NXBlock : public RenderObjectBNXBlock
	{
		public:
			RenderObjectB2NXBlock();
			virtual ~RenderObjectB2NXBlock();
			virtual void Render();
	};

	/**********************************************************
	//RenderObjectB4NX
	//頂点ブレンド4
	//法線有り
	//テクスチャ有り
	**********************************************************/
	class RenderObjectB4NX : public RenderObjectBNX
	{
		protected:
			virtual void _CopyVertexBufferOnInitialize();
		public:
			RenderObjectB4NX();
			~RenderObjectB4NX();

			virtual void CalculateWeightCenter();

			//頂点設定
			VERTEX_B4NX* GetVertex(int index);
			void SetVertex(int index, VERTEX_B4NX& vertex);
			void SetVertexPosition(int index, float x, float y, float z);
			void SetVertexUV(int index, float u, float v);
			void SetVertexBlend(int index, int pos, BYTE indexBlend, float rate);
			void SetVertexNormal(int index, float x, float y, float z);
	};

	class RenderObjectB4NXBlock : public RenderObjectBNXBlock
	{
		public:
			RenderObjectB4NXBlock();
			virtual ~RenderObjectB4NXBlock();
			virtual void Render();
	};

	/**********************************************************
	//Sprite2D
	//矩形スプライト
	**********************************************************/
	class Sprite2D : public RenderObjectTLX
	{
		public:
			Sprite2D();
			~Sprite2D();
			void Copy(Sprite2D* src);
			void SetSourceRect(RECT_D &rcSrc);
			void SetDestinationRect(RECT_D &rcDest);
			void SetDestinationCenter();
			void SetVertex(RECT_D &rcSrc, RECT_D &rcDest, D3DCOLOR color=D3DCOLOR_ARGB(255,255,255,255));

			RECT_D GetDestinationRect();
	};

	/**********************************************************
	//SpriteList2D
	//矩形スプライトリスト
	**********************************************************/
	class SpriteList2D : public RenderObjectTLX
	{
			int countRenderVertex_;
			RECT_D rcSrc_;
			RECT_D rcDest_;
			D3DCOLOR color_;
			bool bCloseVertexList_;
			void _AddVertex(VERTEX_TLX& vertex);
		public:
			SpriteList2D();
			virtual int GetVertexCount();
			virtual void Render();
			void ClearVertexCount(){countRenderVertex_ = 0; bCloseVertexList_=false;}
			void AddVertex();
			void SetSourceRect(RECT_D &rcSrc){rcSrc_ = rcSrc;}
			void SetDestinationRect(RECT_D &rcDest){rcDest_ = rcDest;}
			void SetDestinationCenter();
			D3DCOLOR GetColor(){return color_;}
			void SetColor(D3DCOLOR color){color_ = color;}
			void CloseVertex();
	};

	/**********************************************************
	//Sprite3D
	//矩形スプライト
	**********************************************************/
	class Sprite3D : public RenderObjectLX
	{
		protected:
			bool bBillboard_;
			virtual D3DXMATRIX _CreateWorldTransformMaxtrix();
		public:
			Sprite3D();
			~Sprite3D();
			void SetSourceRect(RECT_D &rcSrc);
			void SetDestinationRect(RECT_D &rcDest);
			void SetVertex(RECT_D &rcSrc, RECT_D &rcDest, D3DCOLOR color=D3DCOLOR_ARGB(255,255,255,255));
			void SetSourceDestRect(RECT_D &rcSrc);
			void SetVertex(RECT_D &rcSrc, D3DCOLOR color=D3DCOLOR_ARGB(255,255,255,255));
			void SetBillboardEnable(bool bEnable){bBillboard_ = bEnable;}
	};

	/**********************************************************
	//TrajectoryObject3D
	//3D軌跡
	**********************************************************/
	class TrajectoryObject3D : public RenderObjectLX
	{
		struct Data
		{
			int alpha;
			D3DXVECTOR3 pos1;
			D3DXVECTOR3 pos2;
		};
		protected:
			D3DCOLOR color_;
			int diffAlpha_;
			int countComplement_;
			Data dataInit_;
			Data dataLast1_;
			Data dataLast2_;
			std::list<Data> listData_;
			virtual D3DXMATRIX _CreateWorldTransformMaxtrix();
		public:
			TrajectoryObject3D();
			~TrajectoryObject3D();
			virtual void Work();
			virtual void Render();
			void SetInitialLine(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2);
			void AddPoint(D3DXMATRIX mat);
			void SetAlphaVariation(int diff){diffAlpha_ = diff;}
			void SetComplementCount(int count){countComplement_ = count;}
			void SetColor(D3DCOLOR color){color_ = color;}
	};

	/**********************************************************
	//DxMesh
	**********************************************************/
	enum
	{
		MESH_ELFREINA,
		MESH_METASEQUOIA,
	};
	class DxMeshManager;
	class DxMeshData
	{
		public:
			friend DxMeshManager;
		protected:
			std::wstring name_;
			DxMeshManager* manager_;
			volatile bool bLoad_;
		public:
			DxMeshData();
			virtual ~DxMeshData();
			void SetName(std::wstring name){name_= name;}
			std::wstring& GetName(){return name_;}
			virtual bool CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;
	};
	class DxMesh : public gstd::FileManager::LoadObject
	{
		public:
			friend DxMeshManager;
		protected:
			D3DXVECTOR3 position_;//移動先座標
			D3DXVECTOR3 angle_;//回転角度
			D3DXVECTOR3 scale_;//拡大率
			D3DCOLOR color_;
			bool bCoordinate2D_;//2D座標指定
			gstd::ref_count_ptr<Shader> shader_;

			gstd::ref_count_ptr<DxMeshData> data_;
			gstd::ref_count_ptr<DxMeshData> _GetFromManager(std::wstring name);
			void _AddManager(std::wstring name, gstd::ref_count_ptr<DxMeshData> data);
		public:
			DxMesh();
			virtual ~DxMesh();
			virtual void Release();
			bool CreateFromFile(std::wstring path);
			virtual bool CreateFromFileReader(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;
			virtual bool CreateFromFileInLoadThread(std::wstring path, int type);
			virtual bool CreateFromFileInLoadThread(std::wstring path) = 0;
			virtual std::wstring GetPath() = 0;

			virtual void Render() = 0;
			virtual void Render(std::wstring nameAnime, int time){Render();}
			void SetPosition(D3DXVECTOR3 pos){position_ = pos;}
			void SetPosition(float x, float y, float z){position_.x=x; position_.y=y; position_.z=z;}
			void SetX(float x){position_.x=x;}
			void SetY(float y){position_.y=y;}
			void SetZ(float z){position_.z=z;}
			void SetAngle(D3DXVECTOR3 angle){angle_ = angle;}
			void SetAngleXYZ(float angx=0.0f, float angy=0.0f, float angz=0.0f){angle_.x=angx; angle_.y=angy; angle_.z=angz;}
			void SetScale(D3DXVECTOR3 scale){scale_ = scale;}
			void SetScaleXYZ(float sx=1.0f, float sy=1.0f, float sz=1.0f){scale_.x=sx;scale_.y=sy;scale_.z=sz;}

			void SetColor(D3DCOLOR color){color_ = color;}
			void SetColorRGB(D3DCOLOR color);
			void SetAlpha(int alpha);

			bool IsCoordinate2D(){return bCoordinate2D_;}
			void SetCoordinate2D(bool b){bCoordinate2D_ = b;}

			gstd::ref_count_ptr<RenderBlocks> CreateRenderBlocks(){return NULL;}
			virtual D3DXMATRIX GetAnimationMatrix(std::wstring nameAnime, double time, std::wstring nameBone){D3DXMATRIX mat; D3DXMatrixIdentity(&mat);return mat;}
			gstd::ref_count_ptr<Shader> GetShader(){return shader_;}
			void SetShader(gstd::ref_count_ptr<Shader> shader){shader_ = shader;}
	};

	/**********************************************************
	//DxMeshManager
	**********************************************************/
	class DxMeshInfoPanel;
	class DxMeshManager : public gstd::FileManager::LoadThreadListener
	{
		friend DxMeshData;
		friend DxMesh;
		friend DxMeshInfoPanel;
		static DxMeshManager* thisBase_;
		protected:
			gstd::CriticalSection lock_;
			std::map<std::wstring, gstd::ref_count_ptr<DxMesh> > mapMesh_;
			std::map<std::wstring, gstd::ref_count_ptr<DxMeshData> > mapMeshData_;
			gstd::ref_count_ptr<DxMeshInfoPanel> panelInfo_;

			void _AddMeshData(std::wstring name, gstd::ref_count_ptr<DxMeshData> data);
			gstd::ref_count_ptr<DxMeshData> _GetMeshData(std::wstring name);
			void _ReleaseMeshData(std::wstring name);
		public:
			DxMeshManager();
			virtual ~DxMeshManager();
			static DxMeshManager* GetBase(){return thisBase_;}
			bool Initialize();
			gstd::CriticalSection& GetLock(){return lock_;}

			virtual void Clear();
			virtual void Add(std::wstring name, gstd::ref_count_ptr<DxMesh> mesh);//参照を保持します
			virtual void Release(std::wstring name);//保持している参照を解放します
			virtual bool IsDataExists(std::wstring name);

			gstd::ref_count_ptr<DxMesh> CreateFromFileInLoadThread(std::wstring path, int type);
			virtual void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event);

			void SetInfoPanel(gstd::ref_count_ptr<DxMeshInfoPanel> panel){panelInfo_ = panel;}
	};

	class DxMeshInfoPanel : public gstd::WindowLogger::Panel , public gstd::Thread
	{
		protected:
			enum
			{
					ROW_ADDRESS,
					ROW_NAME,
					ROW_FULLNAME,
					ROW_COUNT_REFFRENCE,
			};
			int timeUpdateInterval_;
			gstd::WListView wndListView_;
			virtual bool _AddedLogger(HWND hTab);
			void _Run();
		public:
			DxMeshInfoPanel();
			~DxMeshInfoPanel();
			virtual void LocateParts();
			virtual void Update(DxMeshManager* manager);
	};
}

#endif
