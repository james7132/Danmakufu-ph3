#ifndef __DIRECTX_DIRECTGRAPHICS__
#define __DIRECTX_DIRECTGRAPHICS__

#include"DxConstant.hpp"

namespace directx
{
	class DxCamera;
	class DxCamera2D;
	class Texture;
	/**********************************************************
	//DirectGraphicsConfig
	**********************************************************/
	class DirectGraphicsConfig
	{
		public:
			enum
			{
				COLOR_MODE_16BIT,
				COLOR_MODE_32BIT,
			};
		protected:
			bool bShowWindow_;
			int widthScreen_;
			int heightScreen_;
			bool bWindowed_;
			bool bUseRef_;
			int colorMode_;
			bool bUseTripleBuffer_;
			bool bUseWaitTimer_;
			bool bPseudoFullScreen_;
		public:
			DirectGraphicsConfig();
			virtual ~DirectGraphicsConfig();
			bool IsShowWindow(){return bShowWindow_;}
			void SetShowWindow(bool b){bShowWindow_ = b;}
			int GetScreenWidth(){return widthScreen_;}
			void SetScreenWidth(int width){widthScreen_ = width;}
			int GetScreenHeight(){return heightScreen_;}
			void SetScreenHeight(int height){heightScreen_ = height;}
			bool IsWindowed(){return bWindowed_;}
			void SetWindowd(bool bWindowed){bWindowed_ = bWindowed;}
			bool IsReferenceEnable(){return bUseRef_;}
			void SetReferenceEnable(bool bEnable){bUseRef_ = bEnable;}
			int GetColorMode(){return colorMode_;}
			void SetColorMode(int mode){colorMode_ = mode;}
			bool IsTripleBufferEnable(){return bUseTripleBuffer_;}
			void SetTripleBufferEnable(bool bEnable){bUseTripleBuffer_ = bEnable;}
			bool IsWaitTimerEnable(){return bUseWaitTimer_;}
			void SetWaitTimerEnable(bool bEnable){bUseWaitTimer_ = bEnable;}
			bool IsPseudoFullScreen(){return bPseudoFullScreen_;}
			void SetbPseudoFullScreen(bool b){bPseudoFullScreen_ = b;}
	};

	class DirectGraphicsListener
	{
		public:
			virtual ~DirectGraphicsListener(){}
			virtual void ReleaseDirectGraphics(){}
			virtual void RestoreDirectGraphics(){}
			virtual void StartChangeScreenMode(){ReleaseDirectGraphics();}
			virtual void EndChangeScreenMode(){RestoreDirectGraphics();}
	};

	class DirectGraphics
	{
			static DirectGraphics* thisBase_;
		public:
			enum
			{
				SCREENMODE_FULLSCREEN,
				SCREENMODE_WINDOW,
			};
			enum
			{
				MODE_BLEND_NONE,//�Ȃ�
				MODE_BLEND_ALPHA,//���Ŕ���������
				MODE_BLEND_ADD_RGB,//RGB�ŉ��Z����
				MODE_BLEND_ADD_ARGB,//���ŉ��Z����
				MODE_BLEND_MULTIPLY,//��Z����
				MODE_BLEND_SUBTRACT,//���Z����
				MODE_BLEND_SHADOW,//�e�`��p
				MODE_BLEND_INV_DESTRGB,//�`���F���]����

				MODE_TEXTURE_FILTER_NONE,//�t�B���^�Ȃ�
				MODE_TEXTURE_FILTER_POINT,//��ԂȂ�
				MODE_TEXTURE_FILTER_LINEAR,//���`���
			};
		protected:
			IDirect3D9* pDirect3D_;
			IDirect3DDevice9* pDevice_;
			D3DPRESENT_PARAMETERS d3dppFull_;
			D3DPRESENT_PARAMETERS d3dppWin_;
			IDirect3DSurface9* pBackSurf_;
			IDirect3DSurface9* pZBuffer_;

			DirectGraphicsConfig config_;
			HWND hAttachedWindow_;
			DWORD wndStyleFull_;
			DWORD wndStyleWin_;
			int modeScreen_;
			std::list<DirectGraphicsListener*> listListener_;

			gstd::ref_count_ptr<DxCamera> camera_;
			gstd::ref_count_ptr<DxCamera2D> camera2D_;
			gstd::ref_count_ptr<Texture> textureTarget_;

			void _ReleaseDxResource();
			void _RestoreDxResource();
			void _Restore();
			void _InitializeDeviceState();
		public:
			DirectGraphics();
			virtual ~DirectGraphics();
			static DirectGraphics* GetBase(){return thisBase_;}
			HWND GetAttachedWindowHandle(){return hAttachedWindow_;}

			virtual bool Initialize(HWND hWnd);
			virtual bool Initialize(HWND hWnd, DirectGraphicsConfig& config);
			void AddDirectGraphicsListener(DirectGraphicsListener* listener);
			void RemoveDirectGraphicsListener(DirectGraphicsListener* listener);
			int GetScreenMode(){return modeScreen_;}
			D3DPRESENT_PARAMETERS GetFullScreenPresentParameter(){return d3dppFull_;}
			D3DPRESENT_PARAMETERS GetWindowPresentParameter(){return d3dppWin_;}

			IDirect3DDevice9* GetDevice(){return pDevice_;}
			DirectGraphicsConfig& GetConfigData(){return config_;}

			void BeginScene(bool bClear = true);//�`��J�n
			void EndScene();//�`��I��
			void ClearRenderTarget();
			void ClearRenderTarget(RECT rect);
			void SetRenderTarget(gstd::ref_count_ptr<Texture> texture);
			gstd::ref_count_ptr<Texture> GetRenderTarget(){return textureTarget_;}

			//�����_�����O�X�e�[�g���b�p
			void SetLightingEnable(bool bEnable);//���C�e�B���O
			void SetSpecularEnable(bool bEnable);//�X�y�L����
			void SetCullingMode(DWORD mode);//�J�����O
			void SetShadingMode(DWORD mode);//�V�F�[�f�B���O
			void SetZBufferEnable(bool bEnable);//Z�o�b�t�@�Q��
			void SetZWriteEnalbe(bool bEnable);//Z�o�b�t�@��������
			void SetAlphaTest(bool bEnable, DWORD ref = 0, D3DCMPFUNC func = D3DCMP_GREATER);
			void SetBlendMode(DWORD mode, int stage = 0);
			void SetFillMode(DWORD mode);
			void SetFogEnable(bool bEnable);
			bool IsFogEnable();
			void SetVertexFog(bool bEnable, D3DCOLOR color, float start, float end);
			void SetPixelFog(bool bEnable, D3DCOLOR color, float start, float end);
			void SetTextureFilter(DWORD mode, int stage = 0);
			DWORD GetTextureFilter(int stage = 0);
			

			void SetDirectionalLight(D3DVECTOR& dir);

			void SetViewPort(int x, int y, int width, int height);
			void ResetViewPort();

			int GetScreenWidth();
			int GetScreenHeight();
			double GetScreenWidthRatio();
			double GetScreenHeightRatio();
			POINT GetMousePosition();
			gstd::ref_count_ptr<DxCamera> GetCamera(){return camera_;}
			gstd::ref_count_ptr<DxCamera2D> GetCamera2D(){return camera2D_;}

			void SaveBackSurfaceToFile(std::wstring path);
			bool IsPixelShaderSupported(int major, int minor);
	};

	/**********************************************************
	//DirectGraphicsPrimaryWindow
	**********************************************************/
	class DirectGraphicsPrimaryWindow : public DirectGraphics , public gstd::WindowBase
	{
		protected:
			gstd::WindowBase wndGraphics_;
			virtual LRESULT _WindowProcedure(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);//�I�[�o�[���C�h�p�v���V�[�W��
			void _PauseDrawing();
			void _RestartDrawing();
		public:
			DirectGraphicsPrimaryWindow();
			~DirectGraphicsPrimaryWindow();
			virtual bool Initialize();
			virtual bool Initialize(DirectGraphicsConfig& config);
			void ChangeScreenMode();
	};

	/**********************************************************
	//DxCamera
	**********************************************************/
	class DxCamera
	{
			D3DXVECTOR3 pos_;//�œ_
			float radius_;
			float angleAzimuth_;
			float angleElevation_;
			D3DXMATRIX matProjection_;

			float yaw_;
			float pitch_;
			float roll_;

			double clipNear_;
			double clipFar_;

		public:
			DxCamera();
			virtual ~DxCamera();
			void Reset();
			D3DXVECTOR3 GetCameraPosition();
			D3DXVECTOR3 GetFocusPosition(){return pos_;}
			void SetFocus(float x, float y, float z){pos_.x=x;pos_.y=y;pos_.z=z;}
			void SetFocusX(float x){pos_.x = x;}
			void SetFocusY(float y){pos_.y = y;}
			void SetFocusZ(float z){pos_.z = z;}
			float GetRadius(){return radius_;}
			void SetRadius(float r){radius_ = r;}
			float GetAzimuthAngle(){return angleAzimuth_;}
			void SetAzimuthAngle(float angle){angleAzimuth_ = angle;}
			float GetElevationAngle(){return angleElevation_;}
			void SetElevationAngle(float angle){angleElevation_ = angle;}

			float GetYaw(){return yaw_;}
			void SetYaw(float yaw){yaw_ = yaw;}
			float GetPitch(){return pitch_;}
			void SetPitch(float pitch){pitch_ = pitch;}
			float GetRoll(){return roll_;}
			void SetRoll(float roll){roll_ = roll;}

			double GetNearClip(){return clipNear_;}
			double GetFarClip(){return clipFar_;}

			D3DXMATRIX GetMatrixLookAtLH();
			void UpdateDeviceWorldViewMatrix();
			void SetProjectionMatrix(float width, float height, float posNear, float posFar);
			void UpdateDeviceProjectionMatrix();

			D3DXVECTOR2 TransformCoordinateTo2D(D3DXVECTOR3 pos);
	};

	/**********************************************************
	//DxCamera2D
	**********************************************************/
	class DxCamera2D
	{
		public:

		private:
			bool bEnable_;
			D3DXVECTOR2 pos_;//�œ_
			double ratioX_;//�g�嗦
			double ratioY_;
			double angleZ_;
			RECT rcClip_;//����

			gstd::ref_count_ptr<D3DXVECTOR2> posReset_;
		public:
			DxCamera2D();
			virtual ~DxCamera2D();

			bool IsEnable(){return bEnable_;}
			void SetEnable(bool bEnable){bEnable_ = bEnable;}

			D3DXVECTOR2 GetFocusPosition(){return pos_;} 
			float GetFocusX(){return pos_.x;}
			float GetFocusY(){return pos_.y;}
			void SetFocus(float x, float y){pos_.x=x;pos_.y=y;}
			void SetFocus(D3DXVECTOR2 pos){pos_ = pos;}
			void SetFocusX(float x){pos_.x = x;}
			void SetFocusY(float y){pos_.y = y;}
			double GetRatio(){return max(ratioX_, ratioY_);}
			void SetRatio(double ratio){ratioX_ = ratio; ratioY_ = ratio;}
			double GetRatioX(){return ratioX_;}
			void SetRatioX(double ratio){ratioX_ = ratio;}
			double GetRatioY(){return ratioY_;}
			void SetRatioY(double ratio){ratioY_ = ratio;}
			double GetAngleZ(){return angleZ_;}
			void SetAngleZ(double angle){angleZ_ = angle;}

			RECT GetClip(){return rcClip_;}
			void SetClip(RECT rect){rcClip_ = rect;}

			void SetResetFocus(gstd::ref_count_ptr<D3DXVECTOR2> pos){posReset_ = pos;}
			void Reset();
			inline D3DXVECTOR2 GetLeftTopPosition();
			inline static D3DXVECTOR2 GetLeftTopPosition(D3DXVECTOR2 focus, double ratio);
			inline static D3DXVECTOR2 GetLeftTopPosition(D3DXVECTOR2 focus, double ratioX, double ratioY);
			inline static D3DXVECTOR2 GetLeftTopPosition(D3DXVECTOR2 focus, double ratioX, double ratioY, RECT rcClip);

			D3DXMATRIX GetMatrix();
	};
}

#endif
