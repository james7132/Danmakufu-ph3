#include"ScriptClient.hpp"
#include"File.hpp"
#include"Logger.hpp"

using namespace gstd;

/**********************************************************
//ScriptEngineData
**********************************************************/
ScriptEngineData::ScriptEngineData()
{
	encoding_ = Encoding::UNKNOWN;
	mapLine_ = new ScriptFileLineMap();
}
ScriptEngineData::~ScriptEngineData()
{
}
void ScriptEngineData::SetSource(std::vector<char>& source)
{
	encoding_ = Encoding::SHIFT_JIS;
	if(Encoding::IsUtf16Le(&source[0], source.size()))
	{
		encoding_ = Encoding::UTF16LE;
	}
	source_ = source;
}

/**********************************************************
//ScriptEngineCache
**********************************************************/
ScriptEngineCache::ScriptEngineCache()
{

}
ScriptEngineCache::~ScriptEngineCache()
{
}
void ScriptEngineCache::Clear()
{
	cache_.clear();
}
void ScriptEngineCache::AddCache(std::wstring name, ref_count_ptr<ScriptEngineData> data)
{
	cache_[name] = data;
}
ref_count_ptr<ScriptEngineData> ScriptEngineCache::GetCache(std::wstring name)
{
	if(!IsExists(name))return NULL;
	return cache_[name];
}
bool ScriptEngineCache::IsExists(std::wstring name)
{
	return cache_.find(name) != cache_.end();
}

/**********************************************************
//ScriptClientBase
**********************************************************/
script_type_manager ScriptClientBase::typeManagerDefault_;
function const commonFunction[] =  
{
	//共通関数：スクリプト引数結果
	{"GetScriptArgument", ScriptClientBase::Func_GetScriptArgument, 1},
	{"GetScriptArgumentCount", ScriptClientBase::Func_GetScriptArgumentCount, 0},
	{"SetScriptResult", ScriptClientBase::Func_SetScriptResult, 1},

	//共通関数：数学系
	{"min", ScriptClientBase::Func_Min, 2},
	{"max", ScriptClientBase::Func_Max, 2},
	{"log", ScriptClientBase::Func_Log, 1},
	{"log10", ScriptClientBase::Func_Log10, 1},
	{"cos", ScriptClientBase::Func_Cos, 1},
	{"sin", ScriptClientBase::Func_Sin, 1},
	{"tan", ScriptClientBase::Func_Tan, 1},
	{"acos", ScriptClientBase::Func_Acos, 1},
	{"asin", ScriptClientBase::Func_Asin, 1},
	{"atan", ScriptClientBase::Func_Atan, 1},
	{"atan2", ScriptClientBase::Func_Atan2, 2},
	{"rand", ScriptClientBase::Func_Rand, 2},

	//共通関数：文字列操作
	{"ToString", ScriptClientBase::Func_ToString, 1},
	{"IntToString", ScriptClientBase::Func_IntToString, 1},
	{"itoa", ScriptClientBase::Func_ItoA, 1},
	{"rtoa", ScriptClientBase::Func_RtoA, 1},
	{"rtos", ScriptClientBase::Func_RtoS, 2},
	{"vtos", ScriptClientBase::Func_VtoS, 2},
	{"atoi", ScriptClientBase::Func_AtoI, 1},
	{"ator", ScriptClientBase::Func_AtoR, 1},
	{"TrimString", ScriptClientBase::Func_TrimString, 1},
	{"SplitString", ScriptClientBase::Func_SplitString, 2},

	//共通関数：パス関連
	{"GetModuleDirectory", ScriptClientBase::Func_GetModuleDirectory, 0},
	{"GetMainScriptDirectory", ScriptClientBase::Func_GetMainScriptDirectory, 0},
	{"GetCurrentScriptDirectory", ScriptClientBase::Func_GetCurrentScriptDirectory, 0},
	{"GetFileDirectory", ScriptClientBase::Func_GetFileDirectory, 1},
	{"GetFilePathList", ScriptClientBase::Func_GetFilePathList, 1},
	{"GetDirectoryList", ScriptClientBase::Func_GetDirectoryList, 1},

	//共通関数：時刻関連
	{"GetCurrentDateTimeS", ScriptClientBase::Func_GetCurrentDateTimeS, 0},

	//共通関数：デバッグ関連
	{"WriteLog", ScriptClientBase::Func_WriteLog, 1},
	{"RaiseError", ScriptClientBase::Func_RaiseError, 1},

	//共通関数：共通データ
	{"SetDefaultCommonDataArea", ScriptClientBase::Func_SetDefaultCommonDataArea, 1},
	{"SetCommonData", ScriptClientBase::Func_SetCommonData, 2},
	{"GetCommonData", ScriptClientBase::Func_GetCommonData, 2},
	{"ClearCommonData", ScriptClientBase::Func_ClearCommonData, 0},
	{"DeleteCommonData", ScriptClientBase::Func_DeleteCommonData, 1},
	{"SetAreaCommonData", ScriptClientBase::Func_SetAreaCommonData, 3},
	{"GetAreaCommonData", ScriptClientBase::Func_GetAreaCommonData, 3},
	{"ClearAreaCommonData", ScriptClientBase::Func_ClearAreaCommonData, 1},
	{"DeleteAreaCommonData", ScriptClientBase::Func_DeleteAreaCommonData, 2},
	{"CreateCommonDataArea", ScriptClientBase::Func_CreateCommonDataArea, 1},
	{"CopyCommonDataArea", ScriptClientBase::Func_CopyCommonDataArea, 2},
	{"IsCommonDataAreaExists", ScriptClientBase::Func_IsCommonDataAreaExists, 1},
	{"GetCommonDataAreaKeyList", ScriptClientBase::Func_GetCommonDataAreaKeyList, 0},
	{"GetCommonDataValueKeyList", ScriptClientBase::Func_GetCommonDataValueKeyList, 1},

	//定数
	{"NULL",constant<0>::func, 0},
};

ScriptClientBase::ScriptClientBase()
{
	bError_ = false;
	engine_ = new ScriptEngineData();
	machine_ = NULL;
	mainThreadID_ = -1;
	idScript_ = ID_SCRIPT_FREE;
	valueRes_ = value();

	commonDataManager_ = new ScriptCommonDataManager();
	mt_ = new MersenneTwister();
	mt_->Initialize(timeGetTime());
	_AddFunction(commonFunction, sizeof(commonFunction)/sizeof(function));
}
ScriptClientBase::~ScriptClientBase()
{
	if(machine_ != NULL)delete machine_;
	machine_ = NULL;
	engine_ =NULL;
}

void ScriptClientBase::_AddFunction(char const * name, callback f, unsigned arguments)
{
	function tFunc;
	tFunc.name = name;
	tFunc.func = f;
	tFunc.arguments = arguments;
	func_.push_back(tFunc);
}
void ScriptClientBase::_AddFunction(const function* f, int count)
{
	int funcPos = func_.size();
	func_.resize(funcPos+count);
	memcpy(&func_[funcPos], f, sizeof(function)*count);
}

void ScriptClientBase::_RaiseError(int line, std::wstring message)
{
	bError_ = true;
	std::wstring errorPos = _GetErrorLineSource(line);

	gstd::ref_count_ptr<ScriptFileLineMap> mapLine = engine_->GetScriptFileLineMap();
	ScriptFileLineMap::Entry entry = mapLine->GetEntry(line);
	int lineOriginal = entry.lineEndOriginal_ - (entry.lineEnd_ - line);

	std::wstring fileName = PathProperty::GetFileName(entry.path_);

	std::wstring str = StringUtility::Format(L"%s\r\n%s \r\n%s line(行)=%d\r\n\r\n↓\r\n%s\r\n～～～",
		message.c_str(),
		entry.path_.c_str(),
		fileName.c_str(),
		lineOriginal, 
		errorPos.c_str());
	throw ScriptException(str);
}
void ScriptClientBase::_RaiseErrorFromEngine()
{
	int line = engine_->GetEngine()->get_error_line();
	_RaiseError(line, engine_->GetEngine()->get_error_message());
}
void ScriptClientBase::_RaiseErrorFromMachine()
{
	int line = machine_->get_error_line();
	_RaiseError(line, machine_->get_error_message());
}
std::wstring ScriptClientBase::_GetErrorLineSource(int line)
{
	if(line == 0)return L"";
	int encoding = engine_->GetEncoding();
	std::vector<char>& source = engine_->GetSource();
	char* pbuf=(char*)&source[0];
	char* sbuf = pbuf;
	char* ebuf = sbuf + source.size();

	int tLine = 1;
	int rLine = line;
	while(pbuf < ebuf)
	{
		if(tLine==rLine)
			break;

		if(encoding == Encoding::UTF16LE)
		{
			wchar_t ch = (wchar_t&)*pbuf;
			if(ch == L'\n')
				tLine++;
			pbuf+=2;

		}
		else
		{
			if(IsDBCSLeadByte(*pbuf))pbuf+=2;
			else
			{
				if(*pbuf=='\n')
					tLine++;
				pbuf++;
			}
		}

	}

	const int countMax = 256;
	int count = 0;
	sbuf = pbuf;
	while(pbuf < ebuf && count < countMax)
	{
		pbuf++;
		count++;
	}

	int size = max(count-1, 0);
	std::wstring res;
	if(encoding == Encoding::UTF16LE)
	{
		wchar_t* wbufS = (wchar_t*)sbuf;
		wchar_t* wbufE = wbufS + size;
		res = std::wstring(wbufS, wbufE);
	}
	else
	{
		std::string sStr = std::string(sbuf, sbuf + size);
		res = StringUtility::ConvertMultiToWide(sStr);
	}

	return res;
}
std::vector<char> ScriptClientBase::_Include(std::vector<char>& source)
{
	//TODO とりあえず実装
	std::wstring pathSource = engine_->GetPath();
	std::vector<char> res = source;
	FileManager* fileManager = FileManager::GetBase();
	std::set<std::wstring> setReadPath;
	
	gstd::ref_count_ptr<ScriptFileLineMap> mapLine = new ScriptFileLineMap();
	engine_->SetScriptFileLineMap(mapLine);

	mapLine->AddEntry(pathSource,
		1, 
		StringUtility::CountCharacter(source, '\n')+1);

	int encoding = Encoding::SHIFT_JIS;
	bool bEnd = false;
	while(true)
	{
		if(bEnd)break;
		Scanner scanner(res);
		encoding = scanner.GetEncoding();
		int resSize = res.size();
	
		bEnd = true;
		while(scanner.HasNext())
		{
			Token& tok = scanner.Next();
			if( tok.GetType() == Token::TK_EOF )//Eofの識別子が来たらファイルの調査終了
			{
				break;
			}
			else if( tok.GetType() == Token::TK_SHARP)
			{
				int posInclude = scanner.GetCurrentPointer() - 1;
				if(encoding == Encoding::UTF16LE)posInclude--;

				tok = scanner.Next();
				if(tok.GetElement() != L"include" )continue;

				bEnd = false;
				int posCurrent = scanner.GetCurrentPointer();
				std::wstring wPath = scanner.Next().GetString();
				bool bNeedNewLine = false;
				if(scanner.HasNext())
				{
					int posBeforeNewLine = scanner.GetCurrentPointer();
					if(scanner.Next().GetType() != Token::TK_NEWLINE)
					{
						int line = scanner.GetCurrentLine();
						source = res;
						engine_->SetSource(source);

						std::wstring error;
						error += L"New line is not found after #include.\r\n";
						error += L"(#include後に改行がありません)";
						_RaiseError(line, error);
					}
					scanner.SetCurrentPointer(posBeforeNewLine);
				}
				else
				{
					//bNeedNewLine = true;
				}
				int posAfterInclude = scanner.GetCurrentPointer();
				scanner.SetCurrentPointer(posCurrent);

				// "../"から始まっていたら、前に"./"をつける。
				if(wPath.find(L"../") == 0 || wPath.find(L"..\\") == 0)
				{
					wPath = L"./" + wPath;
				}

				if(wPath.find(L".\\") != std::wstring::npos || wPath.find(L"./") != std::wstring::npos)
				{	//".\"展開
					int line = scanner.GetCurrentLine();
					std::wstring linePath = mapLine->GetPath(line);
					std::wstring tDir = PathProperty::GetFileDirectory(linePath);
					//std::string tDir = PathProperty::GetFileDirectory(pathSource);
					wPath = tDir.substr(PathProperty::GetModuleDirectory().size()) + wPath.substr(2);
				}
				wPath = PathProperty::GetModuleDirectory() + wPath;
				wPath = PathProperty::GetUnique(wPath);

				bool bReadPath = setReadPath.find(wPath) != setReadPath.end();
				if(bReadPath)
				{//すでに読み込まれていた
					std::vector<char> newSource;
					int size1 = posInclude;
					int size2 = res.size() - posAfterInclude;
					newSource.resize(size1 + size2);
					memcpy(&newSource[0], &res[0], size1);
					memcpy(&newSource[size1], &res[posAfterInclude], size2);
					
					res = newSource;
					break;
				}

				std::vector<char> placement;
				ref_count_ptr<FileReader> reader;
				reader = fileManager->GetFileReader(wPath);
				if(reader == NULL || !reader->Open())
				{
					int line = scanner.GetCurrentLine();
					source = res;
					engine_->SetSource(source);

					std::wstring error;
					error += StringUtility::Format(L"#Include file is not found[%s].\r\n", wPath.c_str());
					error += StringUtility::Format(L"(#includeで置換するファイル[%s]が見つかりません)", wPath.c_str());
					_RaiseError(line, error);
				}

				//ファイルを読み込み最後に改行を付加
				int targetBomSize = 0;
				int targetEncoding = Encoding::SHIFT_JIS;
				if(reader->GetFileSize() >= 2)
				{
					char data[2];
					reader->Read(&data[0], 2);
					if(Encoding::IsUtf16Le(&data[0], 2))
					{
						targetEncoding = Encoding::UTF16LE;
						targetBomSize = Encoding::GetBomSize(&data[0], 2);
					}
					//ファイルポインタを最初に戻す
					reader->SetFilePointerBegin();
				}

				if(targetEncoding == Encoding::UTF16LE)
				{
					//読み込み対象がUTF16LE
					int newLineSize = bNeedNewLine ? 4 : 0;
					reader->Seek(targetBomSize);
					placement.resize(reader->GetFileSize() - targetBomSize + newLineSize); //-BOM読み込み分+最後の改行
					int readSize = reader->GetFileSize() - targetBomSize;
					reader->Read(&placement[0], readSize);
					memcpy(&placement[readSize], L"\r\n", newLineSize);

					//結合先がShiftJISの場合、結合先をUTF16LEに変換する。
					if(encoding == Encoding::SHIFT_JIS)
					{
						encoding = Encoding::UTF16LE;
						int resSize = res.size();
						std::string sRes = std::string(&res[0], &res[resSize]);
						std::wstring wRes = StringUtility::ConvertMultiToWide(sRes);

						int dataSize = wRes.size() * sizeof(wchar_t);
						res.resize(dataSize + 2);
						memcpy(&res[0], Encoding::BOM_UTF16LE, 2);
						memcpy(&res[2], &wRes[0], dataSize);

						//再度scanしなおす
						break;
					}
				}
				else
				{
					//読み込み対象がShiftJis
					int newLineSize = bNeedNewLine ? 2 : 0;
					placement.resize(reader->GetFileSize() + newLineSize);
					reader->Read(&placement[0], reader->GetFileSize());
					memcpy(&placement[reader->GetFileSize()], "\r\n", newLineSize);

					//結合先がUTF16LEの場合、読み込んだデータをUTF16LEに変換する。
					if(encoding == Encoding::UTF16LE)
					{
						int placementSize = placement.size();
						std::string sPlacement = std::string(&placement[0], &placement[placementSize]);
						std::wstring wPlacement = StringUtility::ConvertMultiToWide(sPlacement);

						int dataSize = wPlacement.size() * sizeof(wchar_t);
						placement.resize(dataSize);
						memcpy(&placement[0], &wPlacement[0], dataSize);
					}

				}
				mapLine->AddEntry(wPath,
					scanner.GetCurrentLine(), 
					StringUtility::CountCharacter(placement, '\n') + 1);

				{//置換
					std::vector<char> newSource;
					int size1 = posInclude;
					int size2 = res.size() - posAfterInclude;
					int sizeP = placement.size();
					newSource.resize(size1 + sizeP +size2);
					memcpy(&newSource[0], &res[0], size1);
					memcpy(&newSource[size1], &placement[0], sizeP);
					memcpy(&newSource[size1 + sizeP], &res[posAfterInclude], size2);

					res = newSource;
				}
				setReadPath.insert(wPath);

				if(false)
				{
					static int countTest = 0;
					static std::wstring tPath = L"";
					if(tPath != pathSource)
					{
						countTest = 0;
						tPath = pathSource;
					}
					std::wstring pathTest = PathProperty::GetModuleDirectory() + StringUtility::Format(L"temp\\script_%s%03d.txt", PathProperty::GetFileName(pathSource).c_str(), countTest);
					File file(pathTest);
					file.CreateDirectory();
					file.Create();
					file.Write(&res[0], res.size());

					std::string strNewLine = "\r\n";
					std::wstring strNewLineW = L"\r\n";
					if(encoding == Encoding::UTF16LE)
					{
						file.Write(&strNewLineW[0] , strNewLine.size() * sizeof(wchar_t));
						file.Write(&strNewLineW[0] , strNewLine.size() * sizeof(wchar_t));
					}
					else
					{
						file.Write(&strNewLine[0] , strNewLine.size());
						file.Write(&strNewLine[0] , strNewLine.size());
					}

					std::list<ScriptFileLineMap::Entry> listEntry = mapLine->GetEntryList();
					std::list<ScriptFileLineMap::Entry>::iterator itr = listEntry.begin();
					for(; itr != listEntry.end() ; itr++)
					{
						if(encoding == Encoding::UTF16LE)
						{
							ScriptFileLineMap::Entry entry = (*itr);
							std::wstring strPath = entry.path_ + L"\r\n";
							std::wstring strLineStart = StringUtility::Format(   L"  lineStart   :%4d\r\n", entry.lineStart_);
							std::wstring strLineEnd = StringUtility::Format(     L"  lineEnd     :%4d\r\n", entry.lineEnd_);
							std::wstring strLineStartOrg = StringUtility::Format(L"  lineStartOrg:%4d\r\n", entry.lineStartOriginal_);
							std::wstring strLineEndOrg = StringUtility::Format(  L"  lineEndOrg  :%4d\r\n", entry.lineEndOriginal_);
						
							file.Write(&strPath[0] , strPath.size() * sizeof(wchar_t));
							file.Write(&strLineStart[0] , strLineStart.size() * sizeof(wchar_t));
							file.Write(&strLineEnd[0] , strLineEnd.size() * sizeof(wchar_t));
							file.Write(&strLineStartOrg[0] , strLineStartOrg.size() * sizeof(wchar_t));
							file.Write(&strLineEndOrg[0] , strLineEndOrg.size() * sizeof(wchar_t));
							file.Write(&strNewLineW[0] , strNewLineW.size() * sizeof(wchar_t));
						}
						else
						{
							ScriptFileLineMap::Entry entry = (*itr);
							std::string strPath = StringUtility::ConvertWideToMulti(entry.path_) + "\r\n";
							std::string strLineStart = StringUtility::Format(   "  lineStart   :%4d\r\n", entry.lineStart_);
							std::string strLineEnd = StringUtility::Format(     "  lineEnd     :%4d\r\n", entry.lineEnd_);
							std::string strLineStartOrg = StringUtility::Format("  lineStartOrg:%4d\r\n", entry.lineStartOriginal_);
							std::string strLineEndOrg = StringUtility::Format(  "  lineEndOrg  :%4d\r\n", entry.lineEndOriginal_);
						
							file.Write(&strPath[0] , strPath.size());
							file.Write(&strLineStart[0] , strLineStart.size());
							file.Write(&strLineEnd[0] , strLineEnd.size());
							file.Write(&strLineStartOrg[0] , strLineStartOrg.size());
							file.Write(&strLineEndOrg[0] , strLineEndOrg.size());
							file.Write(&strNewLine[0] , strNewLine.size());
						}

					}

					countTest++;
				}

				break;
			}
		}
	}

	res.push_back(0);
	if(encoding == Encoding::UTF16LE)
	{
		res.push_back(0);
	}

	return res;
}
bool ScriptClientBase::_CreateEngine()
{
	if(machine_ != NULL)delete machine_;
	machine_ = NULL;

	std::vector<char>& source = engine_->GetSource();

	ref_count_ptr<script_engine> engine = new script_engine(&typeManagerDefault_, source, func_.size(), &func_[0]);
	engine_->SetEngine(engine);
	return true;
}
bool ScriptClientBase::SetSourceFromFile(std::wstring path)
{
	path = PathProperty::GetUnique(path);
	if(cache_ != NULL && cache_->IsExists(path))
	{
		engine_ = cache_->GetCache(path);
		return true;
	}

	engine_->SetPath(path);
	ref_count_ptr<FileReader> reader;
	reader = FileManager::GetBase()->GetFileReader(path);
	if(reader == NULL) throw gstd::wexception(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());
	if(!reader->Open())throw gstd::wexception(ErrorUtility::GetFileNotFoundErrorMessage(path).c_str());

	int size = reader->GetFileSize();
	std::vector<char> source;
	source.resize(size);
	reader->Read(&source[0], size);
	this->SetSource(source);
	return true;
}
void ScriptClientBase::SetSource(std::string source)
{
	std::vector<char> vect;
	vect.resize(source.size());
	memcpy(&vect[0], &source[0], source.size());
	this->SetSource(vect);
}
void ScriptClientBase::SetSource(std::vector<char>& source)
{
	engine_->SetSource(source);
	gstd::ref_count_ptr<ScriptFileLineMap> mapLine = engine_->GetScriptFileLineMap();
	mapLine->AddEntry(engine_->GetPath(), 1, StringUtility::CountCharacter(source, '\n') + 1);
}
void ScriptClientBase::Compile()
{
	if(engine_->GetEngine() == NULL)
	{
		std::vector<char> source = _Include(engine_->GetSource());
		engine_->SetSource(source);

		_CreateEngine();
		if(engine_->GetEngine()->get_error())
		{			
			bError_ = true;
			_RaiseErrorFromEngine();
		}
		if(cache_ != NULL && engine_->GetPath().size() != 0)
		{
			cache_->AddCache(engine_->GetPath(), engine_);
		}

	}

	if(machine_ != NULL)delete machine_;
	machine_ = new script_machine(engine_->GetEngine().GetPointer());
	if(machine_->get_error())
	{	
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	machine_->data = this;
}

bool ScriptClientBase::Run()
{
	if(bError_)return false;
	machine_->run();
	if(machine_->get_error())
	{	
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	return true;
}

bool ScriptClientBase::Run(std::string target)
{
	if(bError_)return false;
	if(!machine_->has_event(target))
	{
		std::wstring error;
		error += StringUtility::FormatToWide("@ not found[%s]", target.c_str());
		error += StringUtility::FormatToWide("(イベントが存在しません[%s])", target.c_str());
		_RaiseError(0, error);
	}

	Run();
	machine_->call(target);
	if(machine_->get_error())
	{	
		bError_ = true;
		_RaiseErrorFromMachine();
	}
	return true;
}
bool ScriptClientBase::IsEventExists(std::string name)
{
	if(bError_)return false;
	return machine_->has_event(name);
}
int ScriptClientBase::GetThreadCount()
{
	if(machine_ == NULL)return 0;
	int res = machine_->get_thread_count();
	return res;
}
void ScriptClientBase::SetArgumentValue(value v, int index)
{
	if(listValueArg_.size() <= index)
	{
		listValueArg_.resize(index + 1);
	}
	listValueArg_[index] = v;
}
value ScriptClientBase::CreateRealValue(long double r)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_real_type(), (long double)r);
	return res;
}
value ScriptClientBase::CreateBooleanValue(bool b)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_boolean_type(), b);
	return res;
}
value ScriptClientBase::CreateStringValue(std::string s)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	std::wstring wstr = to_wide(s);
	value res(typeManager->get_string_type(), wstr);
	return res;
}
value ScriptClientBase::CreateStringValue(std::wstring s)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_string_type(), s);
	return res;
}
value ScriptClientBase::CreateRealArrayValue(std::vector<long double>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_string_type(), std::wstring());
	for(int iData = 0 ; iData < list.size() ; iData++)
	{
		value data = CreateRealValue(list[iData]);
		res.append(typeManager->get_array_type(typeManager->get_real_type()), data);
	}
	
	return res;
}
value ScriptClientBase::CreateStringArrayValue(std::vector<std::string>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_string_type(), std::wstring());
	for(int iData = 0 ; iData < list.size() ; iData++)
	{
		value data = CreateStringValue(list[iData]);
		res.append(typeManager->get_array_type(typeManager->get_string_type()), data);
	}
	
	return res;
}
value ScriptClientBase::CreateStringArrayValue(std::vector<std::wstring>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_string_type(), std::wstring());
	for(int iData = 0 ; iData < list.size() ; iData++)
	{
		value data = CreateStringValue(list[iData]);
		res.append(typeManager->get_array_type(typeManager->get_string_type()), data);
	}
	
	return res;
}
value ScriptClientBase::CreateValueArrayValue(std::vector<value>& list)
{
	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	value res(typeManager->get_string_type(), std::wstring());
	for(int iData = 0 ; iData < list.size() ; iData++)
	{
		value data = list[iData];
		res.append(typeManager->get_array_type(typeManager->get_real_type()), data);
	}
	return res;
}
bool ScriptClientBase::IsRealValue(value& v)
{
	if(bError_)return false;
	if(!v.has_data())return false;

	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	return v.get_type() == typeManager->get_real_type();
}
bool ScriptClientBase::IsBooleanValue(value& v)
{
	if(bError_)return false;
	if(!v.has_data())return false;

	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	return v.get_type() == typeManager->get_boolean_type();
}
bool ScriptClientBase::IsStringValue(value& v)
{
	if(bError_)return false;
	if(!v.has_data())return false;

	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	return v.get_type() == typeManager->get_string_type();
}
bool ScriptClientBase::IsRealArrayValue(value& v)
{
	if(bError_)return false;
	if(!v.has_data())return false;

	script_type_manager* typeManager = GetEngine()->GetEngine().GetPointer()->get_type_manager();
	return v.get_type() == typeManager->get_array_type(typeManager->get_real_type());
}
void ScriptClientBase::CheckRunInMainThread()
{
	if(mainThreadID_ < 0)return;
	if(mainThreadID_ != GetCurrentThreadId())
	{
		std::wstring error;
		error += L"This function can call in main thread only.\r\n";
		error += L"(メインスレッド以外では実行できません。)";
		machine_->raise_error(error);
	}
}
std::wstring ScriptClientBase::_ExtendPath(std::wstring path)
{
	int line = machine_->get_current_line();
	std::wstring pathScript = GetEngine()->GetScriptFileLineMap()->GetPath(line);

	path = StringUtility::ReplaceAll(path, L"\\", L"/");
	path = StringUtility::ReplaceAll(path, L"./", pathScript);

	return path;
}

//共通関数：スクリプト引数結果
value ScriptClientBase::Func_GetScriptArgument(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	int index = (int)argv[0].as_real();
	if(index < 0 || index >= script->listValueArg_.size())
	{
		std::wstring error;
		error += L"Invalid script argument index.\r\n";
		error += L"(スクリプト引数のインデックスが不正です。)";
		throw gstd::wexception(error);
	}
	return script->listValueArg_[index];
}
value ScriptClientBase::Func_GetScriptArgumentCount(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	int res = script->listValueArg_.size();
	return value(machine->get_engine()->get_real_type(), (long double)res);
}
value ScriptClientBase::Func_SetScriptResult(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	script->valueRes_ = argv[0];
	return value();
}

//組み込み関数：数学系
value ScriptClientBase::Func_Min(script_machine* machine, int argc, value const * argv)
{
	long double v1 = argv[0].as_real();
	long double v2 = argv[1].as_real();
	long double res = v1 <= v2 ? v1 : v2;
	return value(machine->get_engine()->get_real_type(), res);
}
value ScriptClientBase::Func_Max(script_machine* machine, int argc, value const * argv)
{
	long double v1 = argv[0].as_real();
	long double v2 = argv[1].as_real();
	long double res = v1 >= v2 ? v1 : v2;
	return value(machine->get_engine()->get_real_type(), res);
}
value ScriptClientBase::Func_Log(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)log(argv[0].as_real()));
}
value ScriptClientBase::Func_Log10(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)log10(argv[0].as_real()));
}
value ScriptClientBase::Func_Cos(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)cos(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Sin(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)sin(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Tan(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)tan(Math::DegreeToRadian(argv[0].as_real())));
}
value ScriptClientBase::Func_Acos(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)Math::RadianToDegree(acos(argv[0].as_real())));
}
value ScriptClientBase::Func_Asin(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)Math::RadianToDegree(asin(argv[0].as_real())));
}
value ScriptClientBase::Func_Atan(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)Math::RadianToDegree(atan(argv[0].as_real())));
}
value ScriptClientBase::Func_Atan2(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_real_type(),(long double)Math::RadianToDegree(atan2(argv[0].as_real(),argv[1].as_real())));
}
value ScriptClientBase::Func_Rand(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	script->CheckRunInMainThread();

	double min=argv[0].as_real();
	double max=argv[1].as_real();
	long double res = script->mt_->GetReal(min, max);
	return value(machine->get_engine()->get_real_type(),res);
}

//組み込み関数：文字列操作
value ScriptClientBase::Func_ToString(script_machine* machine, int argc, value const * argv)
{
	return value(machine->get_engine()->get_string_type(), argv[0].as_string());
}
value ScriptClientBase::Func_IntToString(script_machine* machine, int argc, value const * argv)
{
	std::wstring res = StringUtility::Format(L"%d", (int)argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res); 
}
value ScriptClientBase::Func_ItoA(script_machine* machine, int argc, value const * argv)
{
	std::wstring res = StringUtility::Format(L"%d", (int)argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res); 
}
value ScriptClientBase::Func_RtoA(script_machine* machine, int argc, value const * argv)
{
	std::wstring res = StringUtility::Format(L"%f", argv[0].as_real());
	return value(machine->get_engine()->get_string_type(), res); 
}
value ScriptClientBase::Func_RtoS(script_machine* machine, int argc, value const * argv)
{
	std::string res = "";
	std::string fmtV = to_mbcs(argv[0].as_string());
	double num = argv[1].as_real();

	try
	{
		bool bF = false;
		int countIS = 0;
		int countI0 = 0;
		int countF = 0;
		
		for(int iCh = 0 ; iCh < fmtV.size() ; iCh++)
		{
			char ch = fmtV[iCh];
			if(IsDBCSLeadByte(ch))throw false;

			if(ch == '#')countIS++;
			else if(ch == '.' && bF)throw false;
			else if(ch == '.')bF = true;
			else if(ch == '0')
			{
				if(bF)countF++;
				else countI0++;
			}
		}

		std::string fmt = "";
		if(countI0 > 0 && countF >= 0)
		{
			fmt += "%0";
			fmt += StringUtility::Format("%d", countI0);
			fmt += ".";
			fmt += StringUtility::Format("%d", countF);
			fmt += "f";
		}
		else if(countIS > 0 && countF >= 0)
		{
			fmt += "%";
			fmt += StringUtility::Format("%d", countIS);
			fmt += ".";
			fmt += StringUtility::Format("%d", countF);
			fmt += "f";
		}

		if(fmt.size() > 0)
		{
			res = StringUtility::Format((char*)fmt.c_str(), num);
		}
	}
	catch(...)
	{
		res = "error format";
	}

	return value(machine->get_engine()->get_string_type(), to_wide(res)); 
}
value ScriptClientBase::Func_VtoS(script_machine* machine, int argc, value const * argv)
{
	std::string res = "";
	std::string fmtV = to_mbcs(argv[0].as_string());

	try
	{
		int countIS = 0;
		int countI0 = 0;
		int countF = 0;
		
		int advance = 0;//0:-, 1:0, 2:num, 3:[d,s,f], 4:., 5:num
		for(int iCh = 0 ; iCh < fmtV.size() ; iCh++)
		{
			char ch = fmtV[iCh];
			if(IsDBCSLeadByte(ch))throw false;
			if(advance == 0 && ch == '-')advance = 1;
			else if((advance == 0 || advance == 1 || advance == 2) && (ch >= '0' && ch <= '9'))advance = 2;
			else if(advance == 2 && (ch == 'd' || ch == 's' || ch == 'f'))advance = 4;
			else if(advance == 2 && ('.'))advance = 5;
			else if(advance == 4 && ('.'))advance = 5;
			else if(advance == 5 && (ch >= '0' && ch <= '9'))advance = 5;
			else if(advance == 5 && (ch == 'd' || ch == 's' || ch == 'f'))advance = 6;
			else throw false;
		}

		fmtV = std::string("%") + fmtV;
		char* fmt = (char*)fmtV.c_str();
		if(strstr(fmt, "d") != NULL)
			res = StringUtility::Format(fmt, (int)argv[1].as_real());
		else if(strstr(fmt, "f") != NULL)
			res = StringUtility::Format(fmt, argv[1].as_real());
		else if(strstr(fmt, "s") != NULL)
			res = StringUtility::Format(fmt, to_mbcs(argv[1].as_string()).c_str());
	}
	catch(...)
	{
		res = "error format";
	}

	return value(machine->get_engine()->get_string_type(), to_wide(res)); 
}
value ScriptClientBase::Func_AtoI(script_machine* machine, int argc, value const * argv)
{
	std::wstring str = argv[0].as_string();
	int num = StringUtility::ToInteger(str);
	return value(machine->get_engine()->get_real_type(), (long double)num); 
}
value ScriptClientBase::Func_AtoR(script_machine* machine, int argc, value const * argv)
{
	std::wstring str = argv[0].as_string();
	double num = StringUtility::ToDouble(str);
	return value(machine->get_engine()->get_real_type(), (long double)num); 
}
value ScriptClientBase::Func_TrimString(script_machine* machine, int argc, value const * argv)
{
	std::wstring str = argv[0].as_string();
	std::wstring res = StringUtility::Trim(str);
	return value(machine->get_engine()->get_string_type(), res); 
}
value ScriptClientBase::Func_SplitString(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	std::wstring str = argv[0].as_string();
	std::wstring delim = argv[1].as_string();
	std::vector<std::wstring> list = StringUtility::Split(str, delim);
	
	gstd::value res = script->CreateStringArrayValue(list);
	return res;
}

//共通関数：パス関連
value ScriptClientBase::Func_GetModuleDirectory(script_machine* machine, int argc, value const * argv)
{
	std::wstring res = PathProperty::GetModuleDirectory();
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetMainScriptDirectory(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	std::wstring path = script->GetEngine()->GetPath();
	std::wstring res = PathProperty::GetFileDirectory(path);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetCurrentScriptDirectory(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	int line = machine->get_current_line();
	std::wstring path = script->GetEngine()->GetScriptFileLineMap()->GetPath(line);
	std::wstring res = PathProperty::GetFileDirectory(path);
	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetFileDirectory(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	std::wstring path = argv[0].as_string();
	//path = StringUtility::ReplaceAll(path, "/", "\\");
	std::wstring res = PathProperty::GetFileDirectory(path);

	return value(machine->get_engine()->get_string_type(), res);
}
value ScriptClientBase::Func_GetFilePathList(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	std::wstring path = argv[0].as_string();
	std::wstring dir = PathProperty::GetFileDirectory(path);
	std::vector<std::wstring> listDir = File::GetFilePathList(dir);
	gstd::value res = script->CreateStringArrayValue(listDir);
	return res;
}
value ScriptClientBase::Func_GetDirectoryList(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script=(ScriptClientBase*)machine->data;
	std::wstring path = argv[0].as_string();
	std::wstring dir = PathProperty::GetFileDirectory(path);
	std::vector<std::wstring> listDir = File::GetDirectoryPathList(dir);
	gstd::value res = script->CreateStringArrayValue(listDir);
	return res;
}

//共通関数：時刻関連
value ScriptClientBase::Func_GetCurrentDateTimeS(script_machine* machine, int argc, value const * argv)
{
	SYSTEMTIME date;
	GetLocalTime(&date);

	std::wstring strDateTime = StringUtility::Format(
		L"%04d%02d%02d%02d%02d%02d",
		date.wYear, date.wMonth, date.wDay,
		date.wHour, date.wMinute, date.wSecond
	);
	std::wstring res = strDateTime;
	return value(machine->get_engine()->get_string_type(), res);
}

//共通関数：デバッグ関連
value ScriptClientBase::Func_WriteLog(script_machine* machine, int argc, value const * argv)
{
	std::wstring msg = argv[0].as_string();
	Logger::WriteTop(msg);
	return value();
}
value ScriptClientBase::Func_RaiseError(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	std::wstring msg = argv[0].as_string();
	script->RaiseError(msg);

	return value();
}

//共通関数：共通データ
value ScriptClientBase::Func_SetDefaultCommonDataArea(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string name = to_mbcs(argv[0].as_string());
	commonDataManager->SetDefaultAreaName(name);

	return value();
}
value ScriptClientBase::Func_SetCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = to_mbcs(argv[0].as_string());
	value val = argv[1];
	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->SetValue(key, val);
	return value();
}
value ScriptClientBase::Func_GetCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = to_mbcs(argv[0].as_string());
	value dv = argv[1];
	if(!commonDataManager->IsExists(area))return dv;
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	if(!data->IsExists(key))return dv;
	return data->GetValue(key);
}
value ScriptClientBase::Func_ClearCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->Clear();
	return value();
}
value ScriptClientBase::Func_DeleteCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();
	std::string area = commonDataManager->GetDefaultAreaName();

	std::string key = to_mbcs(argv[0].as_string());
	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->DeleteValue(key);
	return value();
}
value ScriptClientBase::Func_SetAreaCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	std::string key = to_mbcs(argv[1].as_string());
	value val = argv[2];

	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->SetValue(key, val);

	return value();
}
value ScriptClientBase::Func_GetAreaCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	std::string key = to_mbcs(argv[1].as_string());
	value dv = argv[2];
	if(!commonDataManager->IsExists(area))return dv;
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	if(!data->IsExists(key))return dv;
	return data->GetValue(key);
}
value ScriptClientBase::Func_ClearAreaCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->Clear();
	return value();
}
value ScriptClientBase::Func_DeleteAreaCommonData(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	std::string key = to_mbcs(argv[1].as_string());
	if(!commonDataManager->IsExists(area))return value();
	gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
	data->DeleteValue(key);
	return value();
}
value ScriptClientBase::Func_CreateCommonDataArea(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	commonDataManager->CreateArea(area);

	return value();
}
value ScriptClientBase::Func_CopyCommonDataArea(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string areaDest = to_mbcs(argv[0].as_string());
	std::string areaSrc = to_mbcs(argv[1].as_string());
	if(commonDataManager->IsExists(areaSrc))
		commonDataManager->CopyArea(areaDest, areaSrc);

	return value();
}
value ScriptClientBase::Func_IsCommonDataAreaExists(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::string area = to_mbcs(argv[0].as_string());
	bool res = commonDataManager->IsExists(area);
	return value(machine->get_engine()->get_boolean_type(), res);
}
value ScriptClientBase::Func_GetCommonDataAreaKeyList(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::vector<std::string> listKey = commonDataManager->GetKeyList();
	gstd::value res = script->CreateStringArrayValue(listKey);
	return res;
}
value ScriptClientBase::Func_GetCommonDataValueKeyList(script_machine* machine, int argc, value const * argv)
{
	ScriptClientBase* script = (ScriptClientBase*)machine->data;
	ScriptCommonDataManager* commonDataManager = script->GetCommonDataManager();

	std::vector<std::string> listKey;
	std::string area = to_mbcs(argv[0].as_string());
	if(commonDataManager->IsExists(area))
	{
		gstd::ref_count_ptr<ScriptCommonData> data = commonDataManager->GetData(area);
		listKey = data->GetKeyList();
	}
	gstd::value res = script->CreateStringArrayValue(listKey);
	return res;
}


/**********************************************************
//ScriptFileLineMap
**********************************************************/
ScriptFileLineMap::ScriptFileLineMap()
{

}
ScriptFileLineMap::~ScriptFileLineMap()
{

}
void ScriptFileLineMap::AddEntry(std::wstring path, int lineAdd, int lineCount)
{
	Entry entryNew;
	entryNew.path_ = path;
	entryNew.lineStartOriginal_ = 1;
	entryNew.lineEndOriginal_ = lineCount;
	entryNew.lineStart_ = lineAdd;
	entryNew.lineEnd_ = entryNew.lineStart_ + lineCount - 1;
	if(listEntry_.size() == 0)
	{
		listEntry_.push_back(entryNew);
		return;
	}

	Entry* pEntryDivide = NULL;
	std::list<Entry>::iterator itrInsert;
	for(itrInsert = listEntry_.begin(); itrInsert!=listEntry_.end();itrInsert++)
	{
		pEntryDivide = (Entry*)&*itrInsert;
		if(lineAdd >=pEntryDivide->lineStart_ && lineAdd <= pEntryDivide->lineEnd_)break;
	}
	
	Entry& entryDivide = *pEntryDivide;
	if(entryDivide.lineStart_ == lineAdd)
	{
		entryDivide.lineStartOriginal_++;
		listEntry_.insert(itrInsert, entryNew);
	}
	else if(entryDivide.lineEnd_ == lineAdd)
	{
		entryDivide.lineEnd_--;
		entryDivide.lineEndOriginal_--;

		listEntry_.insert(itrInsert, entryNew);
		itrInsert++;
	}
	else
	{
		Entry entryNew2 = entryDivide;
		entryDivide.lineEnd_ = lineAdd - 1;
		entryDivide.lineEndOriginal_ = lineAdd - entryDivide.lineStart_;

		entryNew2.lineStartOriginal_ = entryDivide.lineEndOriginal_ + 2;
		entryNew2.lineStart_ = entryNew.lineEnd_ + 1;
		entryNew2.lineEnd_ += lineCount - 1;
		
		if(itrInsert!=listEntry_.end())
			itrInsert++;
		listEntry_.insert(itrInsert, entryNew);
		listEntry_.insert(itrInsert, entryNew2);
	}

	for(; itrInsert!=listEntry_.end();itrInsert++)
	{
		Entry& entry = *itrInsert;
		entry.lineStart_ += lineCount-1;
		entry.lineEnd_ += lineCount-1;
	}
}
ScriptFileLineMap::Entry ScriptFileLineMap::GetEntry(int line)
{
	Entry res;
	std::list<Entry>::iterator itrInsert;
	for(itrInsert = listEntry_.begin(); itrInsert!=listEntry_.end();itrInsert++)
	{
		res = *itrInsert;
		if(line >=res.lineStart_ && line <= res.lineEnd_)break;
	}
	return res;
}
std::wstring ScriptFileLineMap::GetPath(int line)
{
	Entry entry = GetEntry(line);
	return entry.path_;
}

/**********************************************************
//ScriptCommonDataManager
**********************************************************/
ScriptCommonDataManager::ScriptCommonDataManager()
{
	nameAreaDefailt_ = "";
	CreateArea("");
}
ScriptCommonDataManager::~ScriptCommonDataManager()
{
}
void ScriptCommonDataManager::Clear()
{
	mapData_.clear();
}
bool ScriptCommonDataManager::IsExists(std::string name)
{
	return mapData_.find(name) != mapData_.end();
}
void ScriptCommonDataManager::CreateArea(std::string name)
{
	if(IsExists(name))return;
	mapData_[name] = new ScriptCommonData();
}
void ScriptCommonDataManager::CopyArea(std::string nameDest, std::string nameSrc)
{
	gstd::ref_count_ptr<ScriptCommonData> dataSrc = mapData_[nameSrc];
	gstd::ref_count_ptr<ScriptCommonData> dataDest = new ScriptCommonData();
	dataDest->Copy(dataSrc);
	mapData_[nameDest] = dataDest;
}
gstd::ref_count_ptr<ScriptCommonData> ScriptCommonDataManager::GetData(std::string name)
{
	if(!IsExists(name))return NULL;
	return mapData_[name];
}
void ScriptCommonDataManager::SetData(std::string name, gstd::ref_count_ptr<ScriptCommonData> commonData)
{
	mapData_[name] = commonData;
}
std::vector<std::string> ScriptCommonDataManager::GetKeyList()
{
	std::vector<std::string> res;
	std::map<std::string, gstd::ref_count_ptr<ScriptCommonData> >::iterator itrValue;
	for(itrValue = mapData_.begin() ; itrValue != mapData_.end() ; itrValue++)
	{
		std::string key = itrValue->first;
		res.push_back(key);
	}
	return res;
}
/**********************************************************
//ScriptCommonData
**********************************************************/
ScriptCommonData::ScriptCommonData()
{
}
ScriptCommonData::~ScriptCommonData()
{
}
void ScriptCommonData::Clear()
{
	mapValue_.clear();
}
bool ScriptCommonData::IsExists(std::string name)
{
	return mapValue_.find(name) != mapValue_.end();
}
gstd::value ScriptCommonData::GetValue(std::string name)
{
	if(!IsExists(name))return value();
	return mapValue_[name];
}
void ScriptCommonData::SetValue(std::string name, gstd::value v)
{
	mapValue_[name] = v;
}
void ScriptCommonData::DeleteValue(std::string name)
{
	mapValue_.erase(name);
}
void ScriptCommonData::Copy(gstd::ref_count_ptr<ScriptCommonData> dataSrc)
{
	mapValue_.clear();
	std::vector<std::string> listSrcKey = dataSrc->GetKeyList();
	for(int iKey = 0 ; iKey < listSrcKey.size() ; iKey++)
	{
		std::string key = listSrcKey[iKey];
		gstd::value vSrc = dataSrc->GetValue(key);
		gstd::value vDest = vSrc;
		vDest.unique();
		mapValue_[key] = vDest;
	}
}
std::vector<std::string> ScriptCommonData::GetKeyList()
{
	std::vector<std::string> res;
	std::map<std::string, gstd::value>::iterator itrValue;
	for(itrValue = mapValue_.begin() ; itrValue != mapValue_.end() ; itrValue++)
	{
		std::string key = itrValue->first;
		res.push_back(key);
	}
	return res;
}
void ScriptCommonData::ReadRecord(gstd::RecordBuffer& record)
{
	mapValue_.clear();

	std::vector<std::string> listKey = record.GetKeyList();
	for(int iKey = 0 ; iKey < listKey.size() ; iKey++)
	{
		std::string key = listKey[iKey];
		std::string keyValSize = StringUtility::Format("%s_size", key.c_str());
		if(!record.IsExists(keyValSize))continue;//サイズ自身がキー登録されている
		int valSize = record.GetRecordAsInteger(keyValSize);

		gstd::ByteBuffer buffer;
		buffer.SetSize(valSize);
		record.GetRecord(key, buffer.GetPointer(), valSize);
		gstd::value comVal = _ReadRecord(buffer);
		mapValue_[key] = comVal;
	}
}
gstd::value ScriptCommonData::_ReadRecord(gstd::ByteBuffer &buffer)
{
	script_type_manager* scriptTypeManager = ScriptClientBase::GetDefaultScriptTypeManager();
	gstd::value res;
	type_data::type_kind kind = (type_data::type_kind)buffer.ReadInteger();

	if(kind == type_data::tk_real)
	{
		long double data = buffer.ReadDouble();
		res = gstd::value(scriptTypeManager->get_real_type(), data);
	}
	else if(kind == type_data::tk_char)
	{
		wchar_t data;
		buffer.Read(&data, sizeof(wchar_t));
		res = gstd::value(scriptTypeManager->get_char_type(), data);
	}
	else if(kind == type_data::tk_boolean)
	{
		bool data = buffer.ReadBoolean();
		res = gstd::value(scriptTypeManager->get_boolean_type(), data);
	}
	else if(kind == type_data::tk_array)
	{
		int arrayLength = buffer.ReadInteger();
		value v;
		for(int iArray = 0 ; iArray < arrayLength ; iArray++)
		{
			value& arrayValue = _ReadRecord(buffer);
			v.append(scriptTypeManager->get_array_type(arrayValue.get_type()),
				arrayValue);
		}
		res = v;
	}

	return res;
}
void ScriptCommonData::WriteRecord(gstd::RecordBuffer& record)
{
	std::map<std::string, gstd::value>::iterator itrValue;
	for(itrValue = mapValue_.begin() ; itrValue != mapValue_.end() ; itrValue++)
	{
		std::string key = itrValue->first;
		gstd::value comVal = itrValue->second;

		gstd::ByteBuffer buffer;
		_WriteRecord(buffer, comVal);
		std::string keyValSize = StringUtility::Format("%s_size", key.c_str());
		int valSize = buffer.GetSize();
		record.SetRecordAsInteger(keyValSize, valSize);
		record.SetRecord(key, buffer.GetPointer(), valSize);
	}
}
void ScriptCommonData::_WriteRecord(gstd::ByteBuffer &buffer, gstd::value& comValue)
{
	type_data::type_kind kind = comValue.get_type()->get_kind();
	buffer.WriteInteger(kind);
	if(kind==type_data::tk_real)
	{
		buffer.WriteDouble(comValue.as_real());
	}
	else if(kind==type_data::tk_char)
	{
		wchar_t wch = comValue.as_char();
		buffer.Write(&wch, sizeof(wchar_t));
	}
	else if(kind==type_data::tk_boolean)
	{
		buffer.WriteBoolean(comValue.as_boolean());
	}
	else if(kind==type_data::tk_array)
	{
		int arrayLength = comValue.length_as_array();
		buffer.WriteInteger(arrayLength);

		for(int iArray = 0 ; iArray < arrayLength ; iArray++)
		{
			value& arrayValue = comValue.index_as_array(iArray);
			_WriteRecord(buffer, arrayValue);
		}
	}
}


/**********************************************************
//ScriptCommonDataPanel
**********************************************************/
ScriptCommonDataInfoPanel::ScriptCommonDataInfoPanel()
{
	timeLastUpdate_=0;
	timeUpdateInterval_ = 500;
	commonDataManager_ = new ScriptCommonDataManager();
}
bool ScriptCommonDataInfoPanel::_AddedLogger(HWND hTab)
{
	Create(hTab);

	gstd::WListView::Style styleListViewArea;
	styleListViewArea.SetStyle(WS_CHILD | WS_VISIBLE | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL| LVS_NOCOLUMNHEADER);
	styleListViewArea.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListViewArea.SetListViewStyleEx(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	wndListViewArea_.Create(hWnd_, styleListViewArea);
	wndListViewArea_.AddColumn(256, 0, L"area");

	gstd::WListView::Style styleListViewValue;
	styleListViewValue.SetStyle(WS_CHILD | WS_VISIBLE | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL| LVS_NOSORTHEADER);
	styleListViewValue.SetStyleEx(WS_EX_CLIENTEDGE);
	styleListViewValue.SetListViewStyleEx(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
	wndListViewValue_.Create(hWnd_, styleListViewValue);
	wndListViewValue_.AddColumn(96, COL_KEY, L"Key");
	wndListViewValue_.AddColumn(256, COL_VALUE, L"Value");

	wndSplitter_.Create(hWnd_, WSplitter::TYPE_HORIZONTAL);
	wndSplitter_.SetRatioY(0.25f);

	return true;
}
void ScriptCommonDataInfoPanel::LocateParts()
{
	int wx = GetClientX();
	int wy = GetClientY();
	int wWidth = GetClientWidth();
	int wHeight = GetClientHeight();

	int ySplitter = (int)((float)wHeight*wndSplitter_.GetRatioY());
	int heightSplitter = 6;

	wndSplitter_.SetBounds(wx, ySplitter, wWidth, heightSplitter);
	wndListViewArea_.SetBounds(wx, wy, wWidth, ySplitter);
	wndListViewValue_.SetBounds(wx, ySplitter+heightSplitter, wWidth, wHeight-ySplitter-heightSplitter);
}
void ScriptCommonDataInfoPanel::Update(gstd::ref_count_ptr<ScriptCommonDataManager> commonDataManager)
{
	if(!IsWindowVisible())return;
	{
		Lock lock(lock_);
		commonDataManager_->Clear();

		if(commonDataManager != NULL)
		{
			std::vector<std::string> listKey = commonDataManager->GetKeyList();
			for(int iKey = 0 ; iKey < listKey.size() ; iKey++)
			{
				std::string area = listKey[iKey];
				gstd::ref_count_ptr<ScriptCommonData> dataSrc = commonDataManager->GetData(area);
				gstd::ref_count_ptr<ScriptCommonData> dataDest = new ScriptCommonData();
				dataDest->Copy(dataSrc);
				commonDataManager_->SetData(area, dataDest);
			}
		}

		_UpdateAreaView();
		_UpdateValueView();
	}
}
void ScriptCommonDataInfoPanel::_UpdateListViewKey(WListView* listView, std::vector<std::string> listKey)
{
	std::set<std::string> listManageData;
	std::set<std::string> listAdd;

	int iKey = 0;
	for(iKey = 0 ; iKey < listKey.size() ; iKey++)
	{
		std::string area = listKey[iKey];
		listManageData.insert(area);
		listAdd.insert(area);
	}

	int countView = listView->GetRowCount();
	int iRow = 0;
	for(iRow = 0 ; iRow < countView ; iRow++)
	{
		std::wstring wArea = listView->GetText(iRow, COL_KEY);
		std::string area = StringUtility::ConvertWideToMulti(wArea);
		listAdd.erase(area);
	}

	//削除
	for(iRow=0; iRow<listView->GetRowCount();)
	{
		std::wstring wKey = listView->GetText(iRow, COL_KEY);
		std::string key = StringUtility::ConvertWideToMulti(wKey);
		if(listManageData.find(key) != listManageData.end())iRow++;
		else listView->DeleteRow(iRow);
	}

	//追加
	iRow = 0;
	countView = listView->GetRowCount();
	std::set<std::string>::iterator itr = listAdd.begin();
	for(; itr != listAdd.end() ;)
	{
		std::string str1 = *itr;
		for(;iRow < listView->GetRowCount() ; iRow++)
		{
			std::wstring wstr2 = listView->GetText(iRow, COL_KEY);
			std::string str2 = StringUtility::ConvertWideToMulti(wstr2);
			int index = strcmp(str1.c_str(), str2.c_str());
			if(index < 0)
			{
				std::wstring wstr1 = StringUtility::ConvertMultiToWide(str1);
				listView->SetText(iRow, COL_KEY, wstr1);
				break;
			}
		}
		if(iRow >= listView->GetRowCount())
			break;
		itr = listAdd.erase(itr);
	}

	itr = listAdd.begin();
	for(; itr != listAdd.end() ; itr++)
	{
		std::string str1 = *itr;
		std::wstring wstr1 = StringUtility::ConvertMultiToWide(str1);
		listView->SetText(listView->GetRowCount(), COL_KEY, wstr1);
	}
}
void ScriptCommonDataInfoPanel::_UpdateAreaView()
{
	std::vector<std::string> listKey = commonDataManager_->GetKeyList();
	_UpdateListViewKey(&wndListViewArea_, listKey);
}
void ScriptCommonDataInfoPanel::_UpdateValueView()
{
	int indexArea = wndListViewArea_.GetSelectedRow();
	if(indexArea < 0) 
	{
		if(wndListViewValue_.GetRowCount() > 0)
			wndListViewValue_.Clear();
		return;
	}

	std::wstring wArea = wndListViewArea_.GetText(indexArea, COL_AREA);
	std::string area = StringUtility::ConvertWideToMulti(wArea);
	gstd::ref_count_ptr<ScriptCommonData> commonData = commonDataManager_->GetData(area);
	std::vector<std::string> listKey = commonData->GetKeyList();
	_UpdateListViewKey(&wndListViewValue_, listKey);

	int countView = wndListViewValue_.GetRowCount();
	for(int iRow = 0 ; iRow < countView ; iRow++)
	{
		std::wstring wKey = wndListViewValue_.GetText(iRow, COL_KEY);
		std::string key = StringUtility::ConvertWideToMulti(wKey);
		gstd::value val = commonData->GetValue(key);
		wndListViewValue_.SetText(iRow, COL_VALUE, val.as_string());
	}
}
