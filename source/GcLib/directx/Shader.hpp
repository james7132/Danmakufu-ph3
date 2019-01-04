#ifndef __DIRECTX_SHADER__
#define __DIRECTX_SHADER__

#include"DxConstant.hpp"
#include"DirectGraphics.hpp"
#include"Texture.hpp"

namespace directx
{
	//http://msdn.microsoft.com/ja-jp/library/bb944006(v=vs.85).aspx
	//http://msdn.microsoft.com/ja-jp/library/bb509647(v=vs.85).aspx

	class ShaderManager;
	class Shader;
	class ShaderData;
	/**********************************************************
	//ShaderData
	**********************************************************/
	class ShaderData
	{
		friend Shader;
		friend ShaderManager;
		private:
			ShaderManager* manager_;
			ID3DXEffect* effect_;
			std::wstring name_;
			volatile bool bLoad_;
			volatile bool bText_;

		public:
			ShaderData();
			virtual ~ShaderData();
			std::wstring GetName(){return name_;}
	};

	/**********************************************************
	//ShaderManager
	**********************************************************/
	class ShaderManager : public DirectGraphicsListener
	{
		friend Shader;
		friend ShaderData;
		static ShaderManager* thisBase_;
		protected:
			gstd::CriticalSection lock_;
			std::map<std::wstring, gstd::ref_count_ptr<Shader> > mapShader_;
			std::map<std::wstring, gstd::ref_count_ptr<ShaderData> > mapShaderData_;

			std::list<Shader*> listExecuteShader_;
			std::wstring lastError_;

			void _ReleaseShaderData(std::wstring name);
			bool _CreateFromFile(std::wstring path);
			bool _CreateFromText(std::string& source);
			void _BeginShader(Shader* shader, int pass);
			void _EndShader(Shader* shader);
			static std::wstring _GetTextSourceID(std::string& source);
		public:
			ShaderManager();
			virtual ~ShaderManager();
			static ShaderManager* GetBase(){return thisBase_;}
			virtual bool Initialize();
			gstd::CriticalSection& GetLock(){return lock_;}
			void Clear();

			virtual void ReleaseDirectGraphics(){ReleaseDxResource();}
			virtual void RestoreDirectGraphics(){RestoreDxResource();}
			void ReleaseDxResource();
			void RestoreDxResource();

			virtual bool IsDataExists(std::wstring name);
			gstd::ref_count_ptr<ShaderData> GetShaderData(std::wstring name);
			gstd::ref_count_ptr<Shader> CreateFromFile(std::wstring path);//読み込みます。ShaderDataは保持しますが、Shaderは保持しません。
			gstd::ref_count_ptr<Shader> CreateFromText(std::string source);//読み込みます。ShaderDataは保持しますが、Shaderは保持しません。
			gstd::ref_count_ptr<Shader> CreateFromFileInLoadThread(std::wstring path);
			virtual void CallFromLoadThread(gstd::ref_count_ptr<gstd::FileManager::LoadThreadEvent> event);

			void AddShader(std::wstring name, gstd::ref_count_ptr<Shader> shader);
			void DeleteShader(std::wstring name);
			gstd::ref_count_ptr<Shader> GetShader(std::wstring name);
			gstd::ref_count_ptr<Shader> GetDefaultSkinnedMeshShader();

			void CheckExecutingShaderZero();
			std::wstring GetLastError();
	};

	/**********************************************************
	//ShaderParameter
	**********************************************************/
	class ShaderParameter
	{
		public:
			enum
			{
				TYPE_UNKNOWN,
				TYPE_MATRIX,
				TYPE_MATRIX_ARRAY,
				TYPE_VECTOR,
				TYPE_FLOAT,
				TYPE_FLOAT_ARRAY,
				TYPE_TEXTURE,
			};
		private:
			int type_;
			gstd::ref_count_ptr<gstd::ByteBuffer> value_;
			gstd::ref_count_ptr<Texture> texture_;


		public:
			ShaderParameter();
			virtual ~ShaderParameter();

			int GetType(){return type_;}
			void SetMatrix(D3DXMATRIX& matrix);
			D3DXMATRIX GetMatrix();
			void SetMatrixArray(std::vector<D3DXMATRIX>& matrix);
			std::vector<D3DXMATRIX> GetMatrixArray();
			void SetVector(D3DXVECTOR4& vector);
			D3DXVECTOR4 GetVector();
			void SetFloat(float value);	
			float GetFloat();
			void SetFloatArray(std::vector<float>& values);
			std::vector<float> GetFloatArray();
			void SetTexture(gstd::ref_count_ptr<Texture> texture);	
			gstd::ref_count_ptr<Texture> GetTexture();

	};

	/**********************************************************
	//Shader
	**********************************************************/
	class Shader
	{
		friend ShaderManager;
		protected:
			gstd::ref_count_ptr<ShaderData> data_;

			//bool bLoadShader_;
			//IDirect3DVertexShader9* pVertexShader_;
			//IDirect3DPixelShader9* pPixelShader_;

			std::string technique_;
			std::map<std::string, gstd::ref_count_ptr<ShaderParameter> > mapParam_;

			ShaderData* _GetShaderData(){return data_.GetPointer();}
			gstd::ref_count_ptr<ShaderParameter> _GetParameter(std::string name, bool bCreate);

			int _Begin();
			void _End();
			void _BeginPass(int pass = 0);
			void _EndPass();
			bool _SetupParameter();

		public:
			Shader();
			Shader(Shader* shader);
			virtual ~Shader();
			void Release();

			int Begin(int pass = 0);
			void End();

			ID3DXEffect* GetEffect();
			void ReleaseDxResource();
			void RestoreDxResource();

			bool CreateFromFile(std::wstring path);
			bool CreateFromText(std::string& source);
			bool IsLoad(){return data_ != NULL && data_->bLoad_;}

			bool SetTechnique(std::string name);
			bool SetMatrix(std::string name, D3DXMATRIX& matrix);
			bool SetMatrixArray(std::string name, std::vector<D3DXMATRIX>& matrix);
			bool SetVector(std::string name, D3DXVECTOR4& vector);
			bool SetFloat(std::string name, float value);
			bool SetFloatArray(std::string name, std::vector<float>& values);
			bool SetTexture(std::string name, gstd::ref_count_ptr<Texture> texture);

			
	};
}


#endif
