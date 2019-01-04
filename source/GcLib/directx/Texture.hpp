#ifndef __DIRECTX_TEXTURE__
#define __DIRECTX_TEXTURE__

#include"DxConstant.hpp"
#include"DirectGraphics.hpp"

namespace directx
{
	class TextureData;
	class Texture;
	class TextureManager;
	class TextureInfoPanel;

	/**********************************************************
	//Texture
	**********************************************************/
	class TextureData
	{
		friend Texture;
		friend TextureManager;
		friend TextureInfoPanel;
		public:
			enum
			{
				TYPE_TEXTURE,
				TYPE_RENDER_TARGET,
			};
		protected:
			int type_;
			TextureManager* manager_;
			IDirect3DTexture9 *pTexture_;
			D3DXIMAGE_INFO infoImage_;
			std::wstring name_;
			volatile bool bLoad_;

			IDirect3DSurface9* lpRenderSurface_;//バックバッファ実体(レンダリングターゲット用)
			IDirect3DSurface9* lpRenderZ_;//バックバッファのZバッファ実体(レンダリングターゲット用)
		public:
			TextureData();
			virtual ~TextureData();
			std::wstring GetName(){return name_;}
			D3DXIMAGE_INFO GetImageInfo(){return infoImage_;}
	};

	class Texture : public gstd::FileManager::LoadObject
	{
		friend TextureData;
		friend TextureManager;
		friend TextureInfoPanel;
		protected:
			gstd::ref_count_ptr<TextureData> data_;
			TextureData* _GetTextureData(){return data_.GetPointer();}
		public:
			Texture();
			Texture(Texture* texture);
			virtual ~Texture();
			void Release();

			std::wstring GetName();
			bool CreateFromFile(std::wstring path);
			bool CreateRenderTarget(std::wstring name);
			bool CreateFromFileInLoadThread(std::wstring path, bool bLoadImageInfo = false);

			void SetTexture(IDirect3DTexture9 *pTexture);
			IDirect3DTexture9* GetD3DTexture();
			IDirect3DSurface9* GetD3DSurface();
			IDirect3DSurface9* GetD3DZBuffer();

			int GetWidth();
			int GetHeight();
			bool IsLoad(){return data_ != NULL && data_->bLoad_;}
	};

	/**********************************************************
	//TextureManager
	**********************************************************/
	class TextureManager : public DirectGraphicsListener , public gstd::FileManager::LoadThreadListener
	{
		friend Texture;
		friend TextureData;
		friend TextureInfoPanel;
		static TextureManager* thisBase_;
		public:
			static const std::wstring TARGET_TRANSITION;
		protected:
			gstd::CriticalSection lock_;
			std::map<std::wstring, gstd::ref_count_ptr<Texture> > mapTexture_;
			std::map<std::wstring, gstd::ref_count_ptr<TextureData> > mapTextureData_;
			gstd::ref_count_ptr<TextureInfoPanel> panelInfo_;

			void _ReleaseTextureData(std::wstring name);
			bool _CreateFromFile(std::wstring path);
			bool _CreateRenderTarget(std::wstring name);
		public:
			TextureManager();
			virtual ~TextureManager();
			static TextureManager* GetBase(){return thisBase_;}
			virtual bool Initialize();
			gstd::CriticalSection& GetLock(){return lock_;}

			virtual void Clear();
			virtual void Add(std::wstring name, gstd::ref_count_ptr<Texture> texture);//テクスチャの参照を保持します
			virtual void Release(std::wstring name);//保持している参照を解放します
			virtual bool IsDataExists(std::wstring name);			
			
			virtual void ReleaseDirectGraphics(){ReleaseDxResource();}
			virtual void RestoreDirectGraphics(){RestoreDxResource();}
			void ReleaseDxResource();
			void RestoreDxResource();

			gstd::ref_count_ptr<TextureData> GetTextureData(std::wstring name);
			gstd::ref_count_ptr<Texture> CreateFromFile(std::wstring path);//テクスチャを読み込みます。TextureDataは保持しますが、Textureは保持しません。
			gstd::ref_count_ptr<Texture> CreateRenderTarget(std::wstring name);
			gstd::ref_count_ptr<Texture> GetTexture(std::wstring name);//作成済みのテクスチャを取得します
			gstd::ref_count_ptr<Texture> CreateFromFileInLoadThread(std::wstring path, bool bLoadImageInfo = false);
			virtual void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event);

			void SetInfoPanel(gstd::ref_count_ptr<TextureInfoPanel> panel){panelInfo_ = panel;}
	};

	/**********************************************************
	//TextureInfoPanel
	**********************************************************/
	class TextureInfoPanel : public gstd::WindowLogger::Panel , public gstd::Thread
	{
		protected:
			enum
			{
					ROW_ADDRESS,
					ROW_NAME,
					ROW_FULLNAME,
					ROW_COUNT_REFFRENCE,
					ROW_WIDTH_IMAGE,
					ROW_HEIGHT_IMAGE,
			};
			int timeUpdateInterval_;
			gstd::WListView wndListView_;
			virtual bool _AddedLogger(HWND hTab);
			void _Run();
		public:
			TextureInfoPanel();
			~TextureInfoPanel();
			virtual void LocateParts();
			virtual void Update(TextureManager* manager);
	};
}

#endif
