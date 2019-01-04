#ifndef __GSTD_UTILITIY__
#define __GSTD_UTILITIY__

#include"GstdConstant.hpp"
#include"SmartPointer.hpp"

namespace gstd
{
	//================================================================
	//wexception
	class wexception
	{
		protected:
			std::wstring message_;
		public:
			wexception(){}
			wexception(std::wstring msg){message_ = msg;}
			std::wstring GetMessage(){return message_;}
			const wchar_t* what(){return message_.c_str();}
	};

	//================================================================
	//DebugUtility
	class DebugUtility
	{
		public:
			static void DumpMemoryLeaksOnExit();
	};

	//================================================================
	//SystemUtility
	class SystemUtility
	{
		public:

	};

	//================================================================
	//Encoding
	class Encoding
	{
		//http://msdn.microsoft.com/ja-jp/library/system.text.encoding(v=vs.110).aspx
		//babel
		//http://d.hatena.ne.jp/A7M/20100801/1280629387
		public:
			enum
			{
				UNKNOWN = -1,
				SHIFT_JIS = 1,
				UTF16LE,
			};

			enum
			{
				CP_SHIFT_JIS = 932,
			};
		public:
			static int Detect(const void* data, int dataSize);
			static bool IsUtf16Le(const void* data, int dataSize);
			static int GetBomSize(const void* data, int dataSize);

			static const unsigned char BOM_UTF16LE[];
	};
	

	//================================================================
	//StringUtility
	class StringUtility
	{
		public:
			static std::string ConvertWideToMulti(std::wstring const &wstr, int codeMulti = 932);
			static std::wstring ConvertMultiToWide(std::string const &str, int codeMulti = 932);
			static std::string ConvertUtf8ToMulti(std::vector<char>& text);
			static std::wstring ConvertUtf8ToWide(std::vector<char>& text);

			//----------------------------------------------------------------
			static std::vector<std::string> Split(std::string str, std::string delim);
			static void Split(std::string str, std::string delim, std::vector<std::string>& res);
			static std::string Format(char* str, ...);

			static int CountCharacter(std::string& str, char c);
			static int CountCharacter(std::vector<char>& str, char c);
			static int ToInteger(std::string const & s);
			static double ToDouble(std::string const & s);
			static std::string Replace(std::string& source, std::string pattern, std::string placement);
			static std::string ReplaceAll(std::string& source, std::string pattern, std::string placement, int replaceCount = INT_MAX, int start = 0 , int end = 0);
			static std::string Slice(std::string const & s, int length);
			static std::string Trim(const std::string& str);

			//----------------------------------------------------------------
			//std::wstring.sizeは文字数を返す。バイト数ではない。
			static std::vector<std::wstring> Split(std::wstring str, std::wstring delim);
			static void Split(std::wstring str, std::wstring delim, std::vector<std::wstring>& res);
			static std::wstring Format(wchar_t* str, ...);
			static std::wstring FormatToWide(char* str, ...);

			static int CountCharacter(std::wstring& str, wchar_t c);
			static int ToInteger(std::wstring const & s);
			static double ToDouble(std::wstring const & s);
			static std::wstring Replace(std::wstring& source, std::wstring pattern, std::wstring placement);
			static std::wstring ReplaceAll(std::wstring& source, std::wstring pattern, std::wstring placement, int replaceCount = INT_MAX, int start = 0 , int end = 0);
			static std::wstring Slice(std::wstring const & s, int length);
			static std::wstring Trim(const std::wstring& str);
			static int CountAsciiSizeCharacter(std::wstring& str);
			static int GetByteSize(std::wstring& str);
	};

	//================================================================
	//ErrorUtility
	class ErrorUtility
	{
		public:
			enum
			{
				ERROR_FILE_NOTFOUND,
				ERROR_PARSE,
				ERROR_END_OF_FILE,
				ERROR_OUTOFRANGE_INDEX,
			};
		public:
			static std::wstring GetLastErrorMessage(DWORD error);
			static std::wstring GetLastErrorMessage();
			static std::wstring GetErrorMessage(int type);
			static std::wstring GetFileNotFoundErrorMessage(std::wstring path);
			static std::wstring GetParseErrorMessage(int line, std::wstring what);
			static std::wstring GetParseErrorMessage(std::wstring path, int line, std::wstring what);
	};


	//================================================================
	//Math
	const double PAI = 3.14159265358979323846;
	class Math
	{
		public:
			inline static double DegreeToRadian(double angle){return angle*PAI/180;}
			inline static double RadianToDegree(double angle){return angle*180/PAI;}

			static void InitializeFPU();

			static double Round(double val){return floorl(val + 0.5);}
	};

	//================================================================
	//ByteOrder
	class ByteOrder
	{
		public:
			enum
			{
				ENDIAN_LITTLE,
				ENDIAN_BIG,
			};

		public:
			static void Reverse(LPVOID buf, DWORD size);

	};

	//================================================================
	//Sort
	class SortUtility
	{
		public:
			template <class BidirectionalIterator, class Predicate>
			static void CombSort(BidirectionalIterator first,
						BidirectionalIterator last,
						Predicate pr) 
			{
				int gap = static_cast<int>(std::distance(first, last));
				if ( gap < 1 )return;

				BidirectionalIterator first2 = last;
				bool swapped = false;
				do 
				{
					int newgap = (gap*10+3)/13;
					if ( newgap < 1 ) newgap = 1;
					if (newgap == 9 || newgap == 10) newgap = 11;
					std::advance(first2, newgap - gap);
					gap = newgap;
					swapped = false;
					for ( BidirectionalIterator target1 = first, target2 = first2;
						target2 != last;
						++target1, ++target2)
					{
						if ( pr(*target2, *target1) ) 
						{
							std::iter_swap(target1, target2);
							swapped = true;
						}
					}
				} while ( (gap > 1) || swapped );
			}		
	};

	//================================================================
	//PathProperty
	class PathProperty
	{
		public:
			static std::wstring GetFileDirectory(std::wstring path)
			{
				wchar_t pDrive[_MAX_PATH];
				wchar_t pDir[_MAX_PATH];
				_wsplitpath(path.c_str(), pDrive, pDir, NULL, NULL);
				return std::wstring(pDrive) + std::wstring(pDir);
			}

			static std::wstring GetDirectoryName(std::wstring path)
			{
				//ディレクトリ名を返す
				std::wstring dir = GetFileDirectory(path);
				dir = StringUtility::ReplaceAll(dir, L"\\", L"/");
				std::vector<std::wstring> strs = StringUtility::Split(dir, L"/");
				return strs[strs.size() - 1];
			}

			static std::wstring GetFileName(std::wstring path)
			{
				wchar_t pFileName[_MAX_PATH];
				wchar_t pExt[_MAX_PATH];
				_wsplitpath(path.c_str(), NULL, NULL, pFileName, pExt);
				return std::wstring(pFileName) + std::wstring(pExt);
			}

			static std::wstring GetDriveName(std::wstring path)
			{
				wchar_t pDrive[_MAX_PATH];
				_wsplitpath(path.c_str(), pDrive, NULL, NULL, NULL);
				return std::wstring(pDrive);
			}			

			static std::wstring GetFileNameWithoutExtension(std::wstring path)
			{
				wchar_t pFileName[_MAX_PATH];
				_wsplitpath(path.c_str(), NULL, NULL, pFileName, NULL);
				return std::wstring(pFileName);
			}

			static std::wstring GetFileExtension(std::wstring path)
			{
				wchar_t pExt[_MAX_PATH];
				_wsplitpath(path.c_str(), NULL, NULL, NULL, pExt);
				return std::wstring(pExt);
			}

			static std::wstring GetModuleName()
			{
				wchar_t modulePath[_MAX_PATH];
				ZeroMemory(modulePath ,sizeof(modulePath));
				GetModuleFileName(NULL, modulePath, sizeof(modulePath)-1);//実行ファイルパス取得
				return GetFileNameWithoutExtension(std::wstring(modulePath));
			}

			static std::wstring GetModuleDirectory()
			{
				wchar_t modulePath[_MAX_PATH];
				ZeroMemory(modulePath ,sizeof(modulePath));
				GetModuleFileName(NULL, modulePath, sizeof(modulePath)-1);//実行ファイルパス取得
				return GetFileDirectory(std::wstring(modulePath));
			}
			static std::wstring GetDirectoryWithoutModuleDirectory(std::wstring path)
			{	//パスから実行ファイルディレクトリを除いたディレクトリを返す
				std::wstring res = GetFileDirectory(path);
				std::wstring dirModule = GetModuleDirectory();
				if(res.find(dirModule) != std::wstring::npos)
				{
					res = res.substr(dirModule.size());
				}
				return res;
			}
			static std::wstring GetPathWithoutModuleDirectory(std::wstring path)
			{	//パスから実行ファイルディレクトリを取り除く
				std::wstring dirModule = GetModuleDirectory();
				dirModule = ReplaceYenToSlash(dirModule);
				path = Canonicalize(path);
				path = ReplaceYenToSlash(path);

				std::wstring res = path;
				if(res.find(dirModule) != std::wstring::npos)
				{
					res = res.substr(dirModule.size());
				}
				return res;
			}
			static std::wstring GetRelativeDirectory(std::wstring from, std::wstring to)
			{
				wchar_t path[_MAX_PATH];
				BOOL b = PathRelativePathTo(path, from.c_str(), FILE_ATTRIBUTE_DIRECTORY, to.c_str(), FILE_ATTRIBUTE_DIRECTORY);
				
				std::wstring res;
				if(b)
				{
					res = GetFileDirectory(path);
				}
				return res;
			}
			static std::wstring ReplaceYenToSlash(std::wstring path)
			{
				std::wstring res = StringUtility::ReplaceAll(path, L"\\", L"/");
				return res;
			}
			static std::wstring Canonicalize(std::wstring srcPath)
			{
				wchar_t destPath[_MAX_PATH];
				PathCanonicalize(destPath, srcPath.c_str());
				std::wstring res(destPath);
				return res;
			}
			static std::wstring GetUnique(std::wstring srcPath)
			{
				std::wstring res = StringUtility::ReplaceAll(srcPath, L"/", L"\\");
				res = Canonicalize(res);
				res = ReplaceYenToSlash(res);
				return res;
			}
	};

	//================================================================
	//BitAccess
	class BitAccess
	{
		public:
			template <typename T> static bool GetBit(T value, int bit)
			{
				T mask = (T)1 << bit;
				return (value & mask) != 0;
			}
			template <typename T> static T& SetBit(T& value, int bit, bool b)
			{
				T mask = (T)1 << bit;
				T write = (T)b << bit;
				value &= ~mask;
				value |= write;
				return value;
			}
			template <typename T> static unsigned char GetByte(T value, int bit)
			{
				return (unsigned char)(value >> bit);
			}
			template <typename T> static T& SetByte(T& value, int bit, unsigned char c)
			{
				T mask = (T)0xff << bit;
				T write = (T)c << bit;
				value &= ~mask;
				value |= write;
				return value;
			}
	};

	//================================================================
	//IStringInfo
	class IStringInfo
	{
		public:
			virtual ~IStringInfo(){}
			virtual std::wstring GetInfoAsString()
			{
				int address = (int)this;
				char* name = (char*)typeid(*this).name();
				std::string str = StringUtility::Format("%s[%08x]", name, address);
				std::wstring res = StringUtility::ConvertMultiToWide(str);
				return res;
			}
	};

	//================================================================
	//InnerClass
	//C++には内部クラスがないので、外部クラスアクセス用
	template <class T>
	class InnerClass 
	{
		T* outer_;
		protected:
			T* _GetOuter(){return outer_;}
			void _SetOuter(T* outer) { outer_ = outer;}
		public:
			InnerClass(T* outer=NULL) {outer_ = outer;}
	};

	//================================================================
	//Singleton
	template<class T> class Singleton
	{
		protected:
			Singleton(){};
			inline static T*& _This()
			{
				static T* s = NULL;
				return s;
			}
		public:
			virtual ~Singleton(){};
			static T* CreateInstance()
			{
				if(_This()==NULL)_This()=new T();
				return _This();
			}
			static T* GetInstance()
			{
				if(_This()==NULL)
				{
					throw std::exception("Singleton::GetInstance 未初期化");
				}
				return _This();
			}
			static void DeleteInstance()
			{
				if(_This()!=NULL)delete _This();
				_This()=NULL;	
			}
	};

	//================================================================
	//Scanner
	class Scanner;
	class Token
	{
		friend Scanner;
		public:
			enum Type
			{
				TK_UNKNOWN,TK_EOF,TK_NEWLINE,
				TK_ID,
				TK_INT,TK_REAL,TK_STRING,

				TK_OPENP,TK_CLOSEP,TK_OPENB,TK_CLOSEB,TK_OPENC,TK_CLOSEC,
				TK_SHARP,
				TK_PIPE,TK_AMPERSAND,

				TK_COMMA,TK_PERIOD,TK_EQUAL,
				TK_ASTERISK,TK_SLASH,TK_COLON,TK_SEMICOLON,TK_TILDE,TK_EXCLAMATION,
				TK_PLUS,TK_MINUS,
				TK_LESS,TK_GREATER,
			};
		protected:
			Type type_;
			std::wstring element_;
			int posStart_;
			int posEnd_;
		public:
			Token(){type_ = TK_UNKNOWN;posStart_=0;posEnd_=0;}
			Token(Type type, std::wstring& element, int start, int end){type_ = type; element_ = element;posStart_=start;posEnd_=end;}
			virtual ~Token(){};

			Type GetType(){return type_;}
			std::wstring& GetElement(){return element_;}
			std::string GetElementA();

			int GetStartPointer(){return posStart_;}
			int GetEndPointer(){return posEnd_;}

			int GetInteger();
			double GetReal();
			bool GetBoolean();
			std::wstring GetString();
			std::wstring& GetIdentifier();

			std::string GetStringA();
			std::string GetIdentifierA();
	};

	class Scanner
	{
		public:
			enum
			{

			};

		protected:
			int typeEncoding_;
			int textStartPointer_;
			std::vector<char> buffer_;
			int pointer_;//今の位置
			Token token_;//現在のトークン
			bool bPermitSignNumber_;
			std::list<Token> listDebugToken_;

			wchar_t _CurrentChar();
			wchar_t _NextChar();//ポインタを進めて次の文字を調べる
			
			virtual void _SkipComment();//コメントをとばす
			virtual void _SkipSpace();//空白をとばす
			virtual void _RaiseError(std::wstring str);//例外を投げます
		public:
			Scanner(char* str, int size);
			Scanner(std::string str);
			Scanner(std::wstring wstr);
			Scanner(std::vector<char>& buf);
			virtual ~Scanner();

			void SetPermitSignNumber(bool bEnable){bPermitSignNumber_ = bEnable;}
			int GetEncoding(){return typeEncoding_;}

			Token& GetToken();//現在のトークンを取得
			Token& Next();
			bool HasNext();
			void CheckType(Token& tok, Token::Type type);
			void CheckIdentifer(Token& tok, std::wstring id);
			int GetCurrentLine();

			int GetCurrentPointer();
			void SetCurrentPointer(int pos);
			void SetPointerBegin();
			std::wstring GetString(int start, int end);

			bool CompareMemory(int start, int end, const char* data);
	};

	//================================================================
	//TextParser
	class TextParser
	{
		public:
			enum
			{
				TYPE_REAL,
				TYPE_BOOLEAN,
				TYPE_STRING,
			};

			class Result
			{
				friend TextParser;
				protected:
					int type_;
					int pos_;
					std::wstring valueString_;
					union
					{
						double valueReal_;
						bool valueBoolean_;
					};
				public:
					virtual ~Result(){};
					int GetType(){return type_;}
					double GetReal()
					{
						double res = valueReal_;
						if(IsBoolean())res = valueBoolean_ ? 1.0 : 0.0;
						if(IsString())res = StringUtility::ToDouble(valueString_);
						return res;
					}
					void SetReal(double value)
					{
						type_ = TYPE_REAL;
						valueReal_ = value;
					}
					bool GetBoolean()
					{
						bool res = valueBoolean_;
						if(IsReal())res = (valueReal_ != 0.0 ? true : false);
						if(IsString())res = (valueString_ == L"true" ? true : false);
						return res;
					}
					void SetBoolean(bool value)
					{
						type_ = TYPE_BOOLEAN;
						valueBoolean_ = value;
					}
					std::wstring GetString()
					{
						std::wstring res = valueString_;
						if(IsReal())res = gstd::StringUtility::Format(L"%f", valueReal_);
						if(IsBoolean())res = (valueBoolean_ ? L"true" : L"false");
						return res;
					}
					void SetString(std::wstring value)
					{
						type_ = TYPE_STRING;
						valueString_ = value;
					}
					bool IsReal(){return type_ == TYPE_REAL;}
					bool IsBoolean(){return type_ == TYPE_BOOLEAN;}
					bool IsString(){return type_ == TYPE_STRING;}
			};

		protected:
			gstd::ref_count_ptr<Scanner> scan_;

			void _RaiseError(std::wstring message);
			Result _ParseComparison(int pos);
			Result _ParseSum(int pos);
			Result _ParseProduct(int pos);
			Result _ParseTerm(int pos);
			virtual Result _ParseIdentifer(int pos);
		public:
			TextParser();
			TextParser(std::string source);
			virtual ~TextParser();

			void SetSource(std::string source);
			Result GetResult();
			double GetReal();
	};

	//================================================================
	//Font
	class Font
	{
		public:
			const static wchar_t* GOTHIC;
			const static wchar_t* MINCHOH;
		protected:
			HFONT hFont_;
			LOGFONT info_;
		public:
			Font();
			virtual ~Font();
			void CreateFont(const wchar_t* type,int size, bool bBold=false, bool bItalic=false, bool bLine=false);
			void CreateFontIndirect(LOGFONT& fontInfo);
			void Clear();
			HFONT GetHandle(){return hFont_;}
			LOGFONT GetInfo(){return info_;}
	};

	//================================================================
	//ObjectPool
	template <class T, bool SYNC>
	class ObjectPool
	{
		protected:
			std::vector<std::list<gstd::ref_count_ptr<T, SYNC> > >listUsedPool_;
			std::vector<std::vector<gstd::ref_count_ptr<T, SYNC> > >listCachePool_;

			virtual void _CreatePool(int countType)
			{
				listUsedPool_.resize(countType);
				listCachePool_.resize(countType);
			}
			virtual gstd::ref_count_ptr<T, SYNC> _CreatePoolObject(int type) = 0;
			virtual void _ResetPoolObject(gstd::ref_count_ptr<T, SYNC>& obj){}
			virtual void _ArrangePool()
			{
				int countType = listUsedPool_.size();
				for(int iType = 0 ; iType < countType ; iType++)
				{
					std::list<gstd::ref_count_ptr<T, SYNC> >* listUsed = &listUsedPool_[iType];
					std::vector<gstd::ref_count_ptr<T, SYNC> >* listCache = &listCachePool_[iType];
					
					std::list<gstd::ref_count_ptr<T, SYNC> >::iterator itr = listUsed->begin();
					for(; itr != listUsed->end() ; )
					{
						gstd::ref_count_ptr<T, SYNC> obj = (*itr);
						if(obj.GetReferenceCount() == 2)
						{
							itr = listUsed->erase(itr);
							_ResetPoolObject(obj);
							listCache->push_back(obj);
						}
						else
						{
							itr++;
						}
					}
				}
			}
		public:
			ObjectPool(){}
			virtual ~ObjectPool(){}
			virtual gstd::ref_count_ptr<T, SYNC> GetPoolObject(int type)
			{
				gstd::ref_count_ptr<T, SYNC> res = NULL;
				if(listCachePool_[type].size() > 0)
				{
					res = listCachePool_[type].back();
					listCachePool_[type].pop_back();
				}
				else
				{
					res = _CreatePoolObject(type);
				}
				listUsedPool_[type].push_back(res);
				return res;
			}

			int GetUsedPoolObjectCount()
			{
				int res = 0 ;
				for(int i = 0 ; i < listUsedPool_.size() ; i++)
				{
					res += listUsedPool_[i].size();
				}
				return res;
			}

			int GetCachePoolObjectCount()
			{
				int res = 0 ;
				for(int i = 0 ; i < listCachePool_.size() ; i++)
				{
					res += listCachePool_[i].size();
				}
				return res;
			}
	};

}

#endif
