#ifndef __GSTD_SCRIPTCLIENT__
#define __GSTD_SCRIPTCLIENT__

#include"GstdConstant.hpp"
#include"GstdUtility.hpp"
#include"Script.hpp"
#include"MersenneTwister.hpp"
#include"Thread.hpp"
#include"File.hpp"
#include"Logger.hpp"

namespace gstd
{
	class ScriptFileLineMap;
	class ScriptCommonDataManager;
	/**********************************************************
	//ScriptException
	**********************************************************/
	class ScriptException : public gstd::wexception
	{
		public:
			ScriptException() : gstd::wexception(L""){};
			ScriptException(std::wstring str) : gstd::wexception(str.c_str()){}
	};

	/**********************************************************
	//ScriptEngineData
	**********************************************************/
	class ScriptEngineData
	{
		protected:
			std::wstring path_;
			int encoding_;
			std::vector<char> source_;
			gstd::ref_count_ptr<script_engine> engine_;
			gstd::ref_count_ptr<ScriptFileLineMap> mapLine_;
		public:
			ScriptEngineData();
			virtual ~ScriptEngineData();

			void SetPath(std::wstring path){path_ = path;}
			std::wstring GetPath(){return path_;}
			void SetSource(std::vector<char>& source);
			std::vector<char>& GetSource(){return source_;}
			int GetEncoding(){return encoding_;}
			void SetEngine(gstd::ref_count_ptr<script_engine> engine){engine_ = engine;}
			gstd::ref_count_ptr<script_engine> GetEngine(){return engine_;}
			gstd::ref_count_ptr<ScriptFileLineMap> GetScriptFileLineMap(){return mapLine_;}
			void SetScriptFileLineMap(gstd::ref_count_ptr<ScriptFileLineMap> mapLine){mapLine_ = mapLine;}
	};

	/**********************************************************
	//ScriptEngineCache
	**********************************************************/
	class ScriptEngineCache
	{
		protected:
			std::map<std::wstring, ref_count_ptr<ScriptEngineData> > cache_;
		public:
			ScriptEngineCache();
			virtual ~ScriptEngineCache();
			void Clear();

			void AddCache(std::wstring name, ref_count_ptr<ScriptEngineData> data);
			ref_count_ptr<ScriptEngineData> GetCache(std::wstring name);
			bool IsExists(std::wstring name);
	};

	/**********************************************************
	//ScriptBase
	**********************************************************/
	class ScriptClientBase
	{
		private:
			static script_type_manager typeManagerDefault_;
		public:
			enum
			{
				ID_SCRIPT_FREE = -1,
			};
			static script_type_manager* GetDefaultScriptTypeManager(){return &typeManagerDefault_;}

		protected:
			bool bError_;
			gstd::ref_count_ptr<ScriptEngineCache> cache_;
			gstd::ref_count_ptr<ScriptEngineData> engine_;
			script_machine* machine_;

			std::vector<function> func_;
			ref_count_ptr<MersenneTwister> mt_;
			gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;
			int mainThreadID_;
			_int64 idScript_;

			gstd::CriticalSection criticalSection_;

			std::vector<gstd::value> listValueArg_;
			gstd::value valueRes_;

			void _AddFunction(char const * name, callback f, unsigned arguments);
			void _AddFunction(const function* f,int count);
			void _RaiseErrorFromEngine();
			void _RaiseErrorFromMachine();
			void _RaiseError(int line, std::wstring message);
			std::wstring _GetErrorLineSource(int line);
			virtual std::vector<char> _Include(std::vector<char>& source);
			virtual bool _CreateEngine();
			std::wstring _ExtendPath(std::wstring path);

		public:
			ScriptClientBase();
			virtual ~ScriptClientBase();
			void SetScriptEngineCache(gstd::ref_count_ptr<ScriptEngineCache> cache){cache_ = cache;}
			gstd::ref_count_ptr<ScriptEngineData> GetEngine(){return engine_;}
			virtual bool SetSourceFromFile(std::wstring path);
			virtual void SetSource(std::vector<char>& source);
			virtual void SetSource(std::string source);

			std::wstring GetPath(){return engine_->GetPath();}
			void SetPath(std::wstring path){engine_->SetPath(path);}

			virtual void Compile();
			virtual bool Run();
			virtual bool Run(std::string target);
			bool IsEventExists(std::string name);
			void RaiseError(std::wstring error){_RaiseError(machine_->get_error_line(), error);}
			void Terminate(std::wstring error){machine_->terminate(error);}
			_int64 GetScriptID(){return idScript_;}
			int GetThreadCount();

			void AddArgumentValue(value v){listValueArg_.push_back(v);}
			void SetArgumentValue(value v, int index = 0);
			value GetResultValue(){return valueRes_;}

			value CreateRealValue(long double r);
			value CreateBooleanValue(bool b);
			value CreateStringValue(std::string s);
			value CreateStringValue(std::wstring s);
			value CreateRealArrayValue(std::vector<long double>& list);
			value CreateStringArrayValue(std::vector<std::string>& list);
			value CreateStringArrayValue(std::vector<std::wstring>& list);
			value CreateValueArrayValue(std::vector<value>& list);
			bool IsRealValue(value& v);
			bool IsBooleanValue(value& v);
			bool IsStringValue(value& v);
			bool IsRealArrayValue(value& v);

			void CheckRunInMainThread();
			ScriptCommonDataManager* GetCommonDataManager(){return commonDataManager_.GetPointer();}

			//共通関数：スクリプト引数結果
			static value Func_GetScriptArgument(script_machine* machine, int argc, value const * argv);
			static value Func_GetScriptArgumentCount(script_machine* machine, int argc, value const * argv);
			static value Func_SetScriptResult(script_machine* machine, int argc, value const * argv);

			//共通関数：数学系
			static value Func_Min(script_machine* machine, int argc, value const * argv);
			static value Func_Max(script_machine* machine, int argc, value const * argv);
			static value Func_Log(script_machine* machine, int argc, value const * argv);
			static value Func_Log10(script_machine* machine, int argc, value const * argv);
			static value Func_Cos(script_machine* machine, int argc, value const * argv);
			static value Func_Sin(script_machine* machine, int argc, value const * argv);
			static value Func_Tan(script_machine* machine, int argc, value const * argv);
			static value Func_Acos(script_machine* machine, int argc, value const * argv);
			static value Func_Asin(script_machine* machine, int argc, value const * argv);
			static value Func_Atan(script_machine* machine, int argc, value const * argv);
			static value Func_Atan2(script_machine* machine, int argc, value const * argv);
			static value Func_Rand(script_machine* machine, int argc, value const * argv);

			//共通関数：文字列操作
			static value Func_ToString(script_machine* machine, int argc, value const * argv);
			static value Func_IntToString(script_machine* machine, int argc, value const * argv);
			static value Func_ItoA(script_machine* machine, int argc, value const * argv);
			static value Func_RtoA(script_machine* machine, int argc, value const * argv);
			static value Func_RtoS(script_machine* machine, int argc, value const * argv);
			static value Func_VtoS(script_machine* machine, int argc, value const * argv);
			static value Func_AtoI(script_machine* machine, int argc, value const * argv);
			static value Func_AtoR(script_machine* machine, int argc, value const * argv);
			static value Func_TrimString(script_machine* machine, int argc, value const * argv);
			static value Func_SplitString(script_machine* machine, int argc, value const * argv);

			//共通関数：パス関連
			static value Func_GetModuleDirectory(script_machine* machine, int argc, value const * argv);
			static value Func_GetMainScriptDirectory(script_machine* machine, int argc, value const * argv);
			static value Func_GetCurrentScriptDirectory(script_machine* machine, int argc, value const * argv);
			static value Func_GetFileDirectory(script_machine* machine, int argc, value const * argv);
			static value Func_GetFilePathList(script_machine* machine, int argc, value const * argv);
			static value Func_GetDirectoryList(script_machine* machine, int argc, value const * argv);

			//共通関数：時刻関連
			static value Func_GetCurrentDateTimeS(script_machine* machine, int argc, value const * argv);

			//共通関数：デバッグ関連
			static value Func_WriteLog(script_machine* machine, int argc, value const * argv);
			static value Func_RaiseError(script_machine* machine, int argc, value const * argv);

			//共通関数：共通データ
			static value Func_SetDefaultCommonDataArea(script_machine* machine, int argc, value const * argv);
			static value Func_SetCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_GetCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_ClearCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_DeleteCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_SetAreaCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_GetAreaCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_ClearAreaCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_DeleteAreaCommonData(script_machine* machine, int argc, value const * argv);
			static value Func_CreateCommonDataArea(script_machine* machine, int argc, value const * argv);
			static value Func_CopyCommonDataArea(script_machine* machine, int argc, value const * argv);
			static value Func_IsCommonDataAreaExists(script_machine* machine, int argc, value const * argv);
			static value Func_GetCommonDataAreaKeyList(script_machine* machine, int argc, value const * argv);
			static value Func_GetCommonDataValueKeyList(script_machine* machine, int argc, value const * argv);

	};

	/**********************************************************
	//ScriptFileLineMap
	**********************************************************/
	class ScriptFileLineMap
	{
		public:
			struct Entry
			{
				int lineStart_;
				int lineEnd_;
				int lineStartOriginal_;
				int lineEndOriginal_;
				std::wstring path_;
			};
		protected:
			std::list<Entry> listEntry_;
		public:
			ScriptFileLineMap();
			virtual ~ScriptFileLineMap();
			void AddEntry(std::wstring path, int lineAdd, int lineCount);
			Entry GetEntry(int line);
			std::wstring GetPath(int line);
			std::list<Entry> GetEntryList(){return listEntry_;}
	};

	/**********************************************************
	//ScriptCommonDataManager
	**********************************************************/
	class ScriptCommonData;
	class ScriptCommonDataManager
	{
		protected:
			gstd::CriticalSection lock_;
			std::string nameAreaDefailt_;
			std::map<std::string, gstd::ref_count_ptr<ScriptCommonData> > mapData_;
		public:
			ScriptCommonDataManager();
			virtual ~ScriptCommonDataManager();
			void Clear();

			std::string GetDefaultAreaName(){return nameAreaDefailt_;}
			void SetDefaultAreaName(std::string name){nameAreaDefailt_ = name;}
			bool IsExists(std::string name);
			void CreateArea(std::string name);
			void CopyArea(std::string nameDest, std::string nameSrc);
			gstd::ref_count_ptr<ScriptCommonData> GetData(std::string name);
			void SetData(std::string name, gstd::ref_count_ptr<ScriptCommonData> commonData);
			std::vector<std::string> GetKeyList();

			gstd::CriticalSection& GetLock(){return lock_;}
	};

	/**********************************************************
	//ScriptCommonData
	**********************************************************/
	class ScriptCommonData
	{
		protected:
			std::map<std::string, gstd::value> mapValue_;

			gstd::value _ReadRecord(gstd::ByteBuffer &buffer);
			void _WriteRecord(gstd::ByteBuffer &buffer, gstd::value& comValue);
		public:
			ScriptCommonData();
			virtual ~ScriptCommonData();
			void Clear();
			bool IsExists(std::string name);
			gstd::value GetValue(std::string name);
			void SetValue(std::string name, gstd::value v);
			void DeleteValue(std::string name);
			void Copy(gstd::ref_count_ptr<ScriptCommonData> dataSrc);
			std::vector<std::string> GetKeyList();

			void ReadRecord(gstd::RecordBuffer& record);
			void WriteRecord(gstd::RecordBuffer& record);
	};
	
	/**********************************************************
	//ScriptCommonDataInfoPanel
	**********************************************************/
	class ScriptCommonDataInfoPanel : public WindowLogger::Panel
	{
		protected:
			enum
			{
				COL_AREA = 0,
				COL_KEY = 0,
				COL_VALUE,
			};

			gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager_;
			gstd::CriticalSection lock_;

			WSplitter wndSplitter_;
			WListView wndListViewArea_;
			WListView wndListViewValue_;
			int timeLastUpdate_;
			int timeUpdateInterval_;

			virtual bool _AddedLogger(HWND hTab);
			void _UpdateListViewKey(WListView* listView, std::vector<std::string> listKey);
			void _UpdateAreaView();
			void _UpdateValueView();
		public:
			ScriptCommonDataInfoPanel();
			void SetUpdateInterval(int time){timeUpdateInterval_=time;}
			virtual void LocateParts();
			virtual void Update(gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager);
	};

}


#endif
