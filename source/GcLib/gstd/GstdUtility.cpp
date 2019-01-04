#include"GstdUtility.hpp"
#include"Logger.hpp"
using namespace gstd;

//================================================================
//DebugUtility
void DebugUtility::DumpMemoryLeaksOnExit()
{
	#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	//if(!_CrtCheckMemory())
	#endif
}

//================================================================
//Encoding
const unsigned char Encoding::BOM_UTF16LE[] = {0xFF, 0xFE};
int Encoding::Detect(const void* data, int dataSize)
{
	return UNKNOWN;
}
bool Encoding::IsUtf16Le(const void* data, int dataSize)
{
	if(dataSize < 2)return false;
	if(memcmp(data, "\0", 1) == 0)return false;

	bool res = memcmp(data, BOM_UTF16LE, 2) == 0;
	if(!res && false)
	{
		int test = IS_TEXT_UNICODE_UNICODE_MASK;
		int resIsTextUnicode = IsTextUnicode((void*)data, dataSize, &test);
		res = resIsTextUnicode > 0;
	}
	return res;
}
int Encoding::GetBomSize(const void* data, int dataSize)
{
	if(dataSize < 2)return 0;

	int res = 0;
	if(memcmp(data, BOM_UTF16LE, 2) == 0)
		res = 2;
	return res;
}


//================================================================
//StringUtility
std::string StringUtility::ConvertWideToMulti(std::wstring const &wstr, int codeMulti)
{
	if(wstr == L"")return "";

	//�}���`�o�C�g�ϊ���̃T�C�Y�𒲂ׂ܂�
	//WideCharToMultiByte�̑�6������0��n���ƕϊ���̃T�C�Y���Ԃ�܂�
	int sizeMulti = ::WideCharToMultiByte(codeMulti, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	if(sizeMulti == 0)return "";//���s(�Ƃ肠�����󕶎��Ƃ��܂�)

	//�Ō��\0���܂܂�邽��
	sizeMulti = sizeMulti - 1;

	//�}���`�o�C�g�ɕϊ����܂�
	std::string str;
	str.resize(sizeMulti);
	::WideCharToMultiByte(codeMulti, 0, wstr.c_str(), -1, &str[0], 
	            sizeMulti, NULL, NULL);
	return str;
}

std::wstring StringUtility::ConvertMultiToWide(std::string const &str, int codeMulti)
{
	if(str == "")return L"";

	//UNICODE�ϊ���̃T�C�Y�𒲂ׂ܂�
	//MultiByteToWideChar�̑�6������0��n���ƕϊ���̃T�C�Y���Ԃ�܂�
	int sizeWide = ::MultiByteToWideChar(codeMulti, 0, str.c_str(), -1, NULL, 0 );
	if(sizeWide == 0)return L"";//���s(�Ƃ肠�����󕶎��Ƃ��܂�)

	//�Ō��\0���܂܂�邽��
	sizeWide = sizeWide - 1;

	//UNICODE�ɕϊ����܂�
	std::wstring wstr;
	wstr.resize(sizeWide);
	::MultiByteToWideChar(codeMulti, 0, str.c_str(), -1, &wstr[0], sizeWide );
	return wstr;
}

std::string StringUtility::ConvertUtf8ToMulti(std::vector<char>& text)
{
	std::wstring wstr = ConvertUtf8ToWide(text); //UTF16�ɕϊ�
	std::string strShiftJIS = ConvertWideToMulti(wstr); //ShiftJIS�ɕϊ�

	return strShiftJIS;
}
std::wstring StringUtility::ConvertUtf8ToWide(std::vector<char>& text)
{
	int posText = 0;
	if( (unsigned char)&text[0] == 0xef && 
		(unsigned char)&text[1] == 0xbb && 
		(unsigned char)&text[2] == 0xbf )
	{
		posText += 3;
	}

	std::string str = &text[posText];
	std::wstring wstr = ConvertMultiToWide(str, CP_UTF8); //UTF16�ɕϊ�
	return wstr;
}

//----------------------------------------------------------------
std::vector<std::string> StringUtility::Split(std::string str, std::string delim)
{
	std::vector<std::string> res;
	Split(str, delim, res);
	return res;
}
void StringUtility::Split(std::string str, std::string delim, std::vector<std::string>& res)
{
//wcstok
	std::wstring wstr = StringUtility::ConvertMultiToWide(str);
	wchar_t* wsource = new wchar_t[wstr.size() + sizeof(wchar_t)];
	memcpy(wsource, wstr.c_str(), wstr.size() * sizeof(wchar_t));
	wsource[wstr.size()] = 0;
	std::wstring wdelim = StringUtility::ConvertMultiToWide(delim);

	wchar_t* pStr = NULL;
	wchar_t* cDelim = const_cast<wchar_t*>(wdelim.c_str());
	while( (pStr = wcstok(pStr==NULL ? wsource : NULL, cDelim)) != NULL ) 
	{
		//�؂�o�����������ǉ�
		std::string s = StringUtility::ConvertWideToMulti(std::wstring(pStr));
		s = s.substr(0, s.size() - 1);//�Ō��\0���폜
		res.push_back(s);
	}
	delete[] wsource;

/*
	char* source = new char[str.size()+1];
	memcpy(source, str.c_str(), str.size());
	source[str.size()]='\0';

	char* pStr = NULL;
	char* cDelim = const_cast<char*>(delim.c_str());
	while( (pStr = strtok(pStr==NULL ? source : NULL, cDelim)) != NULL ) 
	{
		//�؂�o�����������ǉ�
		res.push_back(std::string(pStr));
	}
	delete[] source;
*/
}

std::string StringUtility::Format(char* str, ...)
{
	std::string res;
	char buf[256];
	va_list	vl;
	va_start(vl,str);
	if(_vsnprintf(buf, sizeof(buf), str, vl) < 0)
	{	//�o�b�t�@�𒴂��Ă����ꍇ�A���I�Ɋm�ۂ���
		int size = sizeof(buf);
		while(true)
		{
			size *= 2;
			char* nBuf = new char[size];
			if(_vsnprintf(nBuf, size, str, vl) >= 0)
			{
				res = nBuf;
				delete[] nBuf;
				break;
			}
			delete[] nBuf;
		}
	}
	else
	{
		res = buf;
	}
	va_end(vl);
	return res;
}

int StringUtility::CountCharacter(std::string& str, char c)
{
	int count = 0;
	char* pbuf = &str[0];
	char* ebuf = &str[str.size()-1];
	while(pbuf <= ebuf)
	{
		if(*pbuf==c)
			count++;

		if(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, *pbuf))pbuf+=2;
		else pbuf++;		
	}
	return count;
}
int StringUtility::CountCharacter(std::vector<char>& str, char c)
{
	if(str.size() == 0)return 0;

	int encoding = Encoding::SHIFT_JIS;
	if(Encoding::IsUtf16Le(&str[0], str.size()))
		encoding = Encoding::UTF16LE;

	int count = 0;
	char* pbuf = &str[0];
	char* ebuf = &str[str.size()-1];
	while(pbuf <= ebuf)
	{
		if(encoding == Encoding::UTF16LE)
		{
			wchar_t ch = (wchar_t&)*pbuf;
			if(ch == (wchar_t)c)
				count++;
			pbuf+=2;
		}
		else
		{
			if(*pbuf == c)
				count++;
			if(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, *pbuf))pbuf+=2;
			else pbuf++;	
		}
	
	}
	return count;

}
int StringUtility::ToInteger(std::string const & s)
{
	return atoi(s.c_str());
}
double StringUtility::ToDouble(std::string const & s)
{
	return atof(s.c_str());
}
std::string StringUtility::Replace(std::string& source, std::string pattern, std::string placement)
{
	std::string res = ReplaceAll(source, pattern, placement, 1);
	return res;
}
std::string StringUtility::ReplaceAll(std::string& source, std::string pattern, std::string placement, int replaceCount, int start, int end)
{
	bool bDBCSLeadByteCheck = (pattern.size() == 1);
	std::string result;
	if(end == 0) end = source.size();
	std::string::size_type pos_before = 0;
	std::string::size_type pos = start;
	std::string::size_type len = pattern.size();

	int count = 0;
	while( ( pos = source.find( pattern, pos ) ) != std::string::npos ) 
	{
		if(pos > 0)
		{
			char ch = source[pos - 1];
			if(bDBCSLeadByteCheck && IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, ch))
			{
				pos++;
				if(pos >= end) break;
				else continue;
			}
			if(pos >= end) break;
		}

		result.append(source, pos_before, pos - pos_before);
		result.append(placement);
		pos += len ;
		pos_before = pos ;

		count++;
		if(count >= replaceCount)break;
	}
	result.append(source, pos_before, source.size() - pos_before) ;
	return result;
}
std::string StringUtility::Slice(std::string const & s, int length)
{
	length = min(s.size()-1, length);
	return s.substr(0, length);
}
std::string StringUtility::Trim(const std::string& str) 
{
	if(str.size() == 0)return str;

	std::wstring wstr = StringUtility::ConvertMultiToWide(str);
	int left = 0;
	for(; left < wstr.size() ; left++)
	{
		wchar_t wch = wstr[left];
		if(wch != 0x20 && wch != 0x09)
			break;
	}

	int right = wstr.size() - 1;
	for(; right >= 0 ; right--)
	{
		wchar_t wch = wstr[right];
		if(wch != 0x20 && wch != 0x09 && wch != 0x0 && wch != '\r' && wch != '\n')
		{
			right++;
			break;
		}
	}

	std::wstring wres = wstr;
	if(left <= right)
	{
		wres = wstr.substr(left, right - left);
	}

	std::string res = StringUtility::ConvertWideToMulti(wres);
	return res;
}
//----------------------------------------------------------------
std::vector<std::wstring> StringUtility::Split(std::wstring str, std::wstring delim)
{
	std::vector<std::wstring> res;
	Split(str, delim, res);
	return res;
}
void StringUtility::Split(std::wstring str, std::wstring delim, std::vector<std::wstring>& res)
{
	wchar_t* wsource = (wchar_t*)str.c_str();
	wchar_t* pStr = NULL;
	wchar_t* cDelim = const_cast<wchar_t*>(delim.c_str());
	while( (pStr = wcstok(pStr==NULL ? wsource : NULL, cDelim)) != NULL ) 
	{
		//�؂�o�����������ǉ�
		std::wstring s = std::wstring(pStr);
		//s = s.substr(0, s.size() - 1);//�Ō��\0���폜
		res.push_back(s);
	}
}
std::wstring StringUtility::Format(wchar_t* str, ...)
{
	std::wstring res;
	wchar_t buf[256];
	va_list	vl;
	va_start(vl, str);
	if(_vsnwprintf(buf, sizeof(buf) / 2, str, vl) < 0)
	{	//�o�b�t�@�𒴂��Ă����ꍇ�A���I�Ɋm�ۂ���
		int size = sizeof(buf);
		while(true)
		{
			size *= 2;
			wchar_t* nBuf = new wchar_t[size];
			if(_vsnwprintf(nBuf, size / 2, str, vl) >= 0)
			{
				res = nBuf;
				delete[] nBuf;
				break;
			}
			delete[] nBuf;
		}
	}
	else
	{
		res = buf;
	}
	va_end(vl);
	return res;
}
std::wstring StringUtility::FormatToWide(char* str, ...)
{
	std::string res;
	char buf[256];
	va_list	vl;
	va_start(vl,str);
	if(_vsnprintf(buf, sizeof(buf), str, vl) < 0)
	{	//�o�b�t�@�𒴂��Ă����ꍇ�A���I�Ɋm�ۂ���
		int size = sizeof(buf);
		while(true)
		{
			size *= 2;
			char* nBuf = new char[size];
			if(_vsnprintf(nBuf, size, str, vl) >= 0)
			{
				res = nBuf;
				delete[] nBuf;
				break;
			}
			delete[] nBuf;
		}
	}
	else
	{
		res = buf;
	}
	va_end(vl);

	std::wstring wres = StringUtility::ConvertMultiToWide(res);
	return wres;
}

int StringUtility::CountCharacter(std::wstring& str, wchar_t c)
{
	int count = 0;
	wchar_t* pbuf = &str[0];
	wchar_t* ebuf = &str[str.size()-1];
	while(pbuf <= ebuf)
	{
		if(*pbuf==c)
			count++;		
	}
	return count;
}
int StringUtility::ToInteger(std::wstring const & s)
{
	return _wtoi(s.c_str());
}
double StringUtility::ToDouble(std::wstring const & s)
{
	wchar_t *stopscan; 
    return wcstod(s.c_str(), &stopscan);
	//return _wtof(s.c_str());
}
std::wstring StringUtility::Replace(std::wstring& source, std::wstring pattern, std::wstring placement)
{
	std::wstring res = ReplaceAll(source, pattern, placement, 1);
	return res;
}
std::wstring StringUtility::ReplaceAll(std::wstring& source, std::wstring pattern, std::wstring placement, int replaceCount, int start , int end)
{
	std::wstring result;
	if(end == 0) end = source.size();
	std::wstring::size_type pos_before = 0;
	std::wstring::size_type pos = start;
	std::wstring::size_type len = pattern.size();

	int count = 0;
	while( ( pos = source.find( pattern, pos ) ) != std::wstring::npos ) 
	{
		result.append(source, pos_before, pos - pos_before);
		result.append(placement);
		pos += len ;
		pos_before = pos ;

		count++;
		if(count >= replaceCount)break;
	}
	result.append(source, pos_before, source.size() - pos_before) ;
	return result;
}
std::wstring StringUtility::Slice(std::wstring const & s, int length)
{
	length = min(s.size()-1, length);
	return s.substr(0, length);
}
std::wstring StringUtility::Trim(const std::wstring& str)
{
	if(str.size() == 0)return str;

	int left = 0;
	for(; left < str.size() ; left++)
	{
		wchar_t wch = str[left];
		if(wch != 0x20 && wch != 0x09)
			break;
	}

	int right = str.size() - 1;
	for(; right >= 0 ; right--)
	{
		wchar_t wch = str[right];
		if(wch != 0x20 && wch != 0x09 && wch != 0x0 && wch != '\r' && wch != '\n')
		{
			right++;
			break;
		}
	}

	std::wstring res = str;
	if(left <= right)
	{
		res = str.substr(left, right - left);
	}
	return res;
}
int StringUtility::CountAsciiSizeCharacter(std::wstring& str)
{
	if(str.size() == 0)return 0;

	int wcount = str.size();
	WORD *listType = new WORD[wcount];
	GetStringTypeEx(0, CT_CTYPE3, str.c_str(), wcount, listType);

	int res = 0;
	for(int iType = 0 ; iType < wcount ; iType++)
	{
		WORD type = listType[iType];
		if ((type& C3_HALFWIDTH) == C3_HALFWIDTH)
		{
			res++;
		}
		else
		{
			res += 2;
		}
	}

	delete[] listType;
	return res;
}
int StringUtility::GetByteSize(std::wstring& str)
{
	int res = str.size() * sizeof(wchar_t);
	return res;
}

//================================================================
//ErrorUtility
std::wstring ErrorUtility::GetLastErrorMessage(DWORD error)
{
	LPVOID lpMsgBuf;
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ����̌���
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
	std::wstring res = (wchar_t*)lpMsgBuf;
	::LocalFree(lpMsgBuf);
	return res;
}
std::wstring ErrorUtility::GetLastErrorMessage()
{
	return GetLastErrorMessage(GetLastError());
}
std::wstring ErrorUtility::GetErrorMessage(int type)
{
	std::wstring res = L"unknown error";
	if(type == ERROR_FILE_NOTFOUND)
		res = L"cannot file open";
	else if(type == ERROR_PARSE)
		res = L"parse failed";
	else if(type == ERROR_END_OF_FILE)
		res = L"end of file error";
	else if(type == ERROR_OUTOFRANGE_INDEX)
		res = L"invalid index";
	return res;
}
std::wstring ErrorUtility::GetFileNotFoundErrorMessage(std::wstring path)
{
	std::wstring res = GetErrorMessage(ERROR_FILE_NOTFOUND);
	res += StringUtility::Format(L" path[%s]", path.c_str());
	return res;
}
std::wstring ErrorUtility::GetParseErrorMessage(int line, std::wstring what)
{
	return GetParseErrorMessage(L"", line, what);
}
std::wstring ErrorUtility::GetParseErrorMessage(std::wstring path, int line, std::wstring what)
{
	std::wstring res = GetErrorMessage(ERROR_PARSE);
	res += StringUtility::Format(L" path[%s] line[%d] msg[%s]",path.c_str(), line, what.c_str());
	return res;
}

//================================================================
//Math
void Math::InitializeFPU()
{
	__asm
	{
		finit
	};
}

//================================================================
//ByteOrder
void ByteOrder::Reverse(LPVOID buf, DWORD size)
{
	unsigned char* pStart = (unsigned char*)buf;
	unsigned char* pEnd = (unsigned char*)buf + size - 1;

	for(;pStart < pEnd;)
	{
		unsigned char temp = *pStart;
		*pStart = *pEnd;
		*pEnd = temp;

		pStart++;
		pEnd--;
	}
}

//================================================================
//Scanner
Scanner::Scanner(char* str,int size)
{
	std::vector<char> buf;
	buf.resize(size);
	memcpy(&buf[0], str, size);
	buf.push_back('\0');
	this->Scanner::Scanner(buf);
}
Scanner::Scanner(std::string str)
{
	std::vector<char> buf;
	buf.resize(str.size()+1);
	memcpy(&buf[0], str.c_str(), str.size()+1);
	this->Scanner::Scanner(buf);
}
Scanner::Scanner(std::wstring wstr)
{
	std::vector<char> buf;
	int textSize = wstr.size() * sizeof(wchar_t);
	buf.resize(textSize + 4);
	memcpy(&buf[0], &Encoding::BOM_UTF16LE[0], 2);
	memcpy(&buf[2], wstr.c_str(), textSize + 2);
	this->Scanner::Scanner(buf);
}
Scanner::Scanner(std::vector<char>& buf)
{
	bPermitSignNumber_ = true;
	buffer_ = buf;
	pointer_ = 0;
	textStartPointer_ = 0;

	typeEncoding_ = Encoding::SHIFT_JIS;
	if(Encoding::IsUtf16Le(&buf[0], buf.size()))
	{
		typeEncoding_ = Encoding::UTF16LE;
		textStartPointer_ = Encoding::GetBomSize(&buf[0], buf.size());
	}

	buffer_.push_back(0);
	if(typeEncoding_ == Encoding::UTF16LE)
	{
		buffer_.push_back(0);
	}

	SetPointerBegin();
}
Scanner::~Scanner()
{

}
wchar_t Scanner::_CurrentChar()
{
	wchar_t res = L'\0';
	if(typeEncoding_ == Encoding::UTF16LE)
	{
		if(pointer_ + 1 < buffer_.size())
			res = (wchar_t&)buffer_[pointer_];
	}
	else
	{
		if(pointer_ < buffer_.size())
		{
			char ch = buffer_[pointer_];
			res = ch;
		}
	}
	return res;
}

wchar_t Scanner::_NextChar()
{
	if(HasNext() == false)
	{
		Logger::WriteTop(L"�I�[�ُ픭��->");

		int size = buffer_.size() - textStartPointer_;
		std::wstring source = GetString(textStartPointer_, size);
		std::wstring target = StringUtility::Format(L"�����͑Ώ� -> \r\n%s...", source.c_str());
		Logger::WriteTop(target);

		int index = 1;
		std::list<Token>::iterator itr;
		for(itr = listDebugToken_.begin(); itr != listDebugToken_.end();itr++)
		{
			Token token = *itr;
			std::wstring log = StringUtility::Format(L"  %2d token -> type=%2d, element=%s, start=%d, end=%d",
				index, token.GetType(), token.GetElement().c_str(), token.GetStartPointer(), token.GetEndPointer());
			Logger::WriteTop(log);
			index++;
		}

		_RaiseError(L"_NextChar:���łɕ�����I�[�ł�");
	}

	if(typeEncoding_ == Encoding::UTF16LE)
	{
		pointer_ += 2;
	}
	else
	{
		if(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, buffer_[pointer_]))
		{
			while(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, buffer_[pointer_]))
				pointer_+=2;
		}
		else
		{
			pointer_++;
		}
	}

	wchar_t res = _CurrentChar();
	return res;
}
void Scanner::_SkipComment()
{
	while(true)
	{
		int posStart = pointer_;
		_SkipSpace();

		wchar_t ch = _CurrentChar();

		if(ch == L'/')
		{//�R�����g�A�E�g����
			int tPos = pointer_;
			ch = _NextChar();
			if(ch == L'/')
			{// "//"
				while(ch != L'\r' && ch != L'\n' && HasNext())
					ch = _NextChar();
			}
			else if(ch == L'*')
			{// "/*"-"*/"
				while(true)
				{
					ch = _NextChar();
					if(ch == L'*')
					{
						ch = _NextChar();
						if(ch == L'/')
							break;
					}
				}
				ch = _NextChar();
			}
			else
			{
				pointer_ = tPos;
				ch = L'/';
			}
		}

		//�X�L�b�v���󔒔�΂��������ꍇ�A�I��
		if(posStart == pointer_)break;
	}
}
void Scanner::_SkipSpace()
{
	wchar_t ch = _CurrentChar();
	while(true)
	{
		if(ch != L' ' && ch != L'\t')break;
		ch = _NextChar();
	}
}
void Scanner::_RaiseError(std::wstring str)
{
	throw gstd::wexception(str);
}

Token& Scanner::GetToken()
{
	return token_;
}
Token& Scanner::Next()
{
	if(!HasNext())
	{
		_RaiseError(L"Next:���łɏI�[�ł�");
	}

	_SkipComment();//�R�����g���Ƃ΂��܂�

	wchar_t ch = _CurrentChar();

	Token::Type type = Token::TK_UNKNOWN;
	int posStart = pointer_;//�擪��ۑ�

	switch(ch)
	{
		case L'\0': type = Token::TK_EOF; break;//�I�[
		case L',': _NextChar(); type = Token::TK_COMMA;  break;
		case L'.': _NextChar(); type = Token::TK_PERIOD;  break;
		case L'=': _NextChar(); type = Token::TK_EQUAL;  break;
		case L'(': _NextChar(); type = Token::TK_OPENP; break;
		case L')': _NextChar(); type = Token::TK_CLOSEP; break;
		case L'[': _NextChar(); type = Token::TK_OPENB; break;
		case L']': _NextChar(); type = Token::TK_CLOSEB; break;
		case L'{': _NextChar(); type = Token::TK_OPENC; break;
		case L'}': _NextChar(); type = Token::TK_CLOSEC; break;
		case L'*': _NextChar(); type = Token::TK_ASTERISK; break;
		case L'/': _NextChar(); type = Token::TK_SLASH; break;
		case L':': _NextChar(); type = Token::TK_COLON; break;
		case L';': _NextChar(); type = Token::TK_SEMICOLON; break;
		case L'~': _NextChar(); type = Token::TK_TILDE; break;
		case L'!': _NextChar(); type = Token::TK_EXCLAMATION; break;
		case L'#': _NextChar(); type = Token::TK_SHARP; break;
		case L'|': _NextChar(); type = Token::TK_PIPE; break;
		case L'&': _NextChar(); type = Token::TK_AMPERSAND; break;
		case L'<': _NextChar(); type = Token::TK_LESS; break;
		case L'>': _NextChar(); type = Token::TK_GREATER; break;
		
		case L'"':
		{
			ch = _NextChar();//1�i�߂�
			//while( ch != '"' )ch = _NextChar();//���̃_�u���N�I�[�e�[�V�����܂Ői�߂�
			wchar_t pre = ch;
			while(true)
			{
				if(ch == L'"' && pre != L'\\')break;
				pre = ch;
				ch = _NextChar();//���̃_�u���N�I�[�e�[�V�����܂Ői�߂�
			}
			if(ch == L'"')
				_NextChar();//�_�u���N�I�[�e�[�V������������1�i�߂�
			else 
			{
				std::wstring error = GetString(posStart, pointer_);
				std::wstring log = StringUtility::Format(L"Next:���łɕ�����I�[�ł�(String������) -> %s", error.c_str());
				_RaiseError(log);
			}
			type = Token::TK_STRING;
			break;
		}

		case L'\r':case L'\n'://���s
			//���s�����܂ł������悤�Ȃ̂�1�̉��s�Ƃ��Ĉ���
			while(ch == L'\r' || ch == L'\n') ch = _NextChar();
			type = Token::TK_NEWLINE;
			break;

		case L'+':case L'-':
		{
			if(ch == L'+')
			{
				ch = _NextChar(); type = Token::TK_PLUS;
				
			}
			else if(ch == L'-')
			{
				ch = _NextChar(); type = Token::TK_MINUS;
			}

			if(!bPermitSignNumber_ || !iswdigit(ch))break;//���������łȂ��Ȃ甲����
		}

		default:
		{
			if(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, ch))
			{
				//Shift-JIS��s�o�C�g
				//���Ԃ񎯕ʎq
				while(iswalpha(ch) || iswdigit(ch) || ch == L'_' || IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, ch))
					ch = _NextChar();//���Ԃ񎯕ʎq�Ȋԃ|�C���^��i�߂�
				type = Token::TK_ID;
			}
			else if(iswdigit(ch))
			{
				//����������
				while(iswdigit(ch))ch = _NextChar();//���������̊ԃ|�C���^��i�߂�
				type = Token::TK_INT;
				if( ch == L'.' )
				{
					//�������������𒲂ׂ�B�����_�������������
					ch = _NextChar();
					while(iswdigit(ch))ch = _NextChar();//���������̊ԃ|�C���^��i�߂�
					type = Token::TK_REAL;					
				}
				
				if( ch == L'E' || ch == L'e')
				{
					//1E-5�݂����ȃP�[�X
					ch = _NextChar();
					while(iswdigit(ch) || ch == L'-')ch = _NextChar();//���������̊ԃ|�C���^��i�߂�
					type = Token::TK_REAL;	
				}
			
			}
			else if(iswalpha(ch) || ch == L'_')
			{
				//���Ԃ񎯕ʎq
				while(iswalpha(ch) || iswdigit(ch) || ch == L'_')
					ch = _NextChar();//���Ԃ񎯕ʎq�Ȋԃ|�C���^��i�߂�
				type = Token::TK_ID;
			}
			else
			{
				_NextChar();
				type = Token::TK_UNKNOWN;
			}

			break;
		}	
	}

	if(type == Token::TK_STRING)
	{
		std::wstring wstr;
		if(typeEncoding_ == Encoding::UTF16LE)
		{
			wchar_t* pPosStart = (wchar_t*)&buffer_[posStart];
			wchar_t* pPosEnd = (wchar_t*)&buffer_[pointer_];
			wstr = std::wstring(pPosStart, pPosEnd);
			wstr = StringUtility::ReplaceAll(wstr, L"\\\"", L"\"");
		}
		else
		{
			char* pPosStart = &buffer_[posStart];
			char* pPosEnd = &buffer_[pointer_];
			std::string str = std::string(pPosStart, pPosEnd);
			str = StringUtility::ReplaceAll(str, "\\\"", "\"");
			wstr = StringUtility::ConvertMultiToWide(str);
		}

		token_ = Token(type, wstr, posStart, pointer_);
	}
	else
	{
		std::wstring wstr;
		if(typeEncoding_ == Encoding::UTF16LE)
		{
			wchar_t* pPosStart = (wchar_t*)&buffer_[posStart];
			wchar_t* pPosEnd = (wchar_t*)&buffer_[pointer_];
			wstr = std::wstring(pPosStart, pPosEnd);
		}
		else
		{
			char* pPosStart = &buffer_[posStart];
			char* pPosEnd = &buffer_[pointer_];
			std::string str = std::string(pPosStart, pPosEnd);
			wstr = StringUtility::ConvertMultiToWide(str);
		}
		token_ = Token(type, wstr, posStart, pointer_);
	}

	listDebugToken_.push_back(token_);

	return token_;
}
bool Scanner::HasNext()
{
//	bool res = true;
//	res &= pointer_ < buffer_.size();
//	res &= _CurrentChar() != L'\0';
//	res &= token_.GetType() != Token::TK_EOF;
	return pointer_ < buffer_.size() && _CurrentChar() != L'\0' && token_.GetType() != Token::TK_EOF;
}
void Scanner::CheckType(Token& tok, Token::Type type)
{
	if(tok.type_ != type)
	{
		std::wstring str = StringUtility::Format(L"CheckType error[%s]:",tok.element_.c_str());
		_RaiseError(str);
	}
}
void Scanner::CheckIdentifer(Token& tok, std::wstring id)
{
	if(tok.type_ != Token::TK_ID || tok.GetIdentifier() != id)
	{
		std::wstring str = StringUtility::Format(L"CheckID error[%s]:",tok.element_.c_str());
		_RaiseError(str);
	}	
}
int Scanner::GetCurrentLine()
{
	if(buffer_.size() == 0)return 0;

	int line=1;
	char* pbuf = &buffer_[0];
	char* ebuf = &buffer_[pointer_];
	while(true)
	{
		if(typeEncoding_ == Encoding::UTF16LE)
		{
			if(pbuf + 1 >= ebuf)break;
			wchar_t wch = (wchar_t&)*pbuf;
			if(wch == L'\n')line++;
			pbuf+=2;
		}
		else
		{
			if(pbuf >= ebuf)break;
			if(IsDBCSLeadByteEx(Encoding::CP_SHIFT_JIS, *pbuf))
			{
				pbuf+=2;	
			}
			else
			{
				if(*pbuf == '\n')line++;
				pbuf++;
			}
		}
	}
	return line;
}
int Scanner::GetCurrentPointer()
{
	return pointer_;
}
void Scanner::SetCurrentPointer(int pos)
{
	pointer_ = pos;
}
void Scanner::SetPointerBegin()
{
	pointer_ = textStartPointer_;
}
std::wstring Scanner::GetString(int start, int end)
{
	std::wstring res;
	if(typeEncoding_ == Encoding::UTF16LE)
	{
		wchar_t* pPosStart = (wchar_t*)&buffer_[start];
		wchar_t* pPosEnd = (wchar_t*)&buffer_[end];
		res = std::wstring(pPosStart, pPosEnd);
	}
	else
	{
		char* pPosStart = &buffer_[start];
		char* pPosEnd = &buffer_[end];
		std::string str = std::string(pPosStart, pPosEnd);
		res = StringUtility::ConvertMultiToWide(str);
	}
	return res;
}
bool Scanner::CompareMemory(int start, int end, const char* data)
{
	if(end >= buffer_.size())return false;

	int bufSize = end - start;
	bool res = memcmp(&buffer_[start], data, bufSize) == 0;
	return res;
}

//Token
std::wstring& Token::GetIdentifier()
{
	if(type_ != TK_ID)
	{
		throw gstd::wexception(L"Token::GetIdentifier:�f�[�^�̃^�C�v���Ⴂ�܂�");
	}
	return element_;
}
std::wstring Token::GetString()
{
	if(type_ != TK_STRING)
	{
		throw gstd::wexception(L"Token::GetString:�f�[�^�̃^�C�v���Ⴂ�܂�");
	}
	return element_.substr(1, element_.size()-2);
}
int Token::GetInteger()
{
	if(type_ != TK_INT)
	{
		throw gstd::wexception(L"Token::GetInterger:�f�[�^�̃^�C�v���Ⴂ�܂�");
	}
	return StringUtility::ToInteger(element_);
}
double Token::GetReal()
{
	if(type_ != TK_REAL && type_ != TK_INT)
	{
		throw gstd::wexception(L"Token::GetReal:�f�[�^�̃^�C�v���Ⴂ�܂�");
	}
	return StringUtility::ToDouble(element_);
}
bool Token::GetBoolean()
{
	bool res = false;
	if(type_ == TK_REAL && type_ == TK_INT)
	{
		res = GetReal() == 1;
	}
	else
	{
		res = element_ == L"true";
	}
	return res;
}

std::string Token::GetElementA()
{
	std::wstring wstr = GetElement();
	std::string res = StringUtility::ConvertWideToMulti(wstr);
	return res;
}
std::string Token::GetStringA()
{
	std::wstring wstr = GetString();
	std::string res = StringUtility::ConvertWideToMulti(wstr);
	return res;
}
std::string Token::GetIdentifierA()
{
	std::wstring wstr = GetIdentifier();
	std::string res = StringUtility::ConvertWideToMulti(wstr);
	return res;
}

//================================================================
//TextParser
TextParser::TextParser()
{

}
TextParser::TextParser(std::string source)
{
	SetSource(source);
}
TextParser::~TextParser()
{
}
void TextParser::_RaiseError(std::wstring message)
{
	throw gstd::wexception(message);
}
TextParser::Result TextParser::_ParseComparison(int pos)
{
	Result res = _ParseSum(pos);
	while(scan_->HasNext())
	{
		scan_->SetCurrentPointer(res.pos_);

		Token& tok = scan_->Next();
		int type =tok.GetType();
		if(type == Token::TK_EXCLAMATION || type == Token::TK_EQUAL)
		{
			//�u==�v�u!=�v
			bool bNot = type == Token::TK_EXCLAMATION;
			tok = scan_->Next();
			type =tok.GetType();
			if(type != Token::TK_EQUAL)break;

			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if(res.type_ == TYPE_BOOLEAN && tRes.type_ == TYPE_BOOLEAN)
			{
				res.valueBoolean_ = bNot ? 
					res.valueBoolean_ != tRes.valueBoolean_ : res.valueBoolean_ == tRes.valueBoolean_ ;					
			}
			else if(res.type_ == TYPE_REAL && tRes.type_ == TYPE_REAL)
			{
				res.valueBoolean_ = bNot ? 
					res.valueReal_ != tRes.valueReal_ : res.valueReal_ == tRes.valueReal_ ;
			}
			else
			{
				_RaiseError(L"��r�ł��Ȃ��^");
			}
			res.type_ = TYPE_BOOLEAN;
		}
		else if(type == Token::TK_PIPE)
		{
			tok = scan_->Next();
			type =tok.GetType();
			if(type != Token::TK_PIPE)break;
			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if(res.type_ == TYPE_BOOLEAN && tRes.type_ == TYPE_BOOLEAN)
			{
				res.valueBoolean_ = res.valueBoolean_ || tRes.valueBoolean_;
			}
			else
			{
				_RaiseError(L"�^�U�l�ȊO�ł�||");
			}
		}
		else if(type == Token::TK_AMPERSAND)
		{
			tok = scan_->Next();
			type =tok.GetType();
			if(type != Token::TK_AMPERSAND)break;
			Result tRes = _ParseSum(scan_->GetCurrentPointer());
			res.pos_ = tRes.pos_;
			if(res.type_ == TYPE_BOOLEAN && tRes.type_ == TYPE_BOOLEAN)
			{
				res.valueBoolean_ = res.valueBoolean_ && tRes.valueBoolean_;
			}
			else
			{
				_RaiseError(L"�^�U�l�ȊO�ł�&&");
			}
		}
		else break;
	}
	return res;
}

TextParser::Result TextParser::_ParseSum(int pos)
{
	Result res = _ParseProduct(pos);
	while(scan_->HasNext())
	{
		scan_->SetCurrentPointer(res.pos_);

		Token& tok = scan_->Next();
		int type =tok.GetType();
		if(type != Token::TK_PLUS && type != Token::TK_MINUS)
			break;

		Result tRes = _ParseProduct(scan_->GetCurrentPointer());
		if(res.IsString() || tRes.IsString())
		{
			res.type_ = TYPE_STRING;
			res.valueString_ = res.GetString() + tRes.GetString();
		}
		else
		{
			if(tRes.type_ == TYPE_BOOLEAN)_RaiseError(L"�^�U�l�̉��Z���Z");
			res.pos_ = tRes.pos_;
			if(type == Token::TK_PLUS)
			{
				res.valueReal_ += tRes.valueReal_;
			}
			else if(type == Token::TK_MINUS)
			{
				res.valueReal_ -= tRes.valueReal_;
			}
		}

	}

	return res;
}
TextParser::Result TextParser::_ParseProduct(int pos)
{
	Result res = _ParseTerm(pos);
	while(scan_->HasNext())
	{
		scan_->SetCurrentPointer(res.pos_);
		Token& tok = scan_->Next();
		int type =tok.GetType();
		if(type != Token::TK_ASTERISK && type != Token::TK_SLASH)break;

		Result tRes = _ParseTerm(scan_->GetCurrentPointer());
		if(tRes.type_ == TYPE_BOOLEAN)_RaiseError(L"�^�U�l�̏�Z���Z");

		res.type_ = tRes.type_;
		res.pos_ = tRes.pos_;
		if(type == Token::TK_ASTERISK)
		{
			res.valueReal_ *= tRes.valueReal_;
		}
		else if(type == Token::TK_SLASH)
		{
			res.valueReal_ /= tRes.valueReal_;
		}
	}

	return res;
}

TextParser::Result TextParser::_ParseTerm(int pos)
{
	scan_->SetCurrentPointer(pos);
	Result res;
	Token& tok = scan_->Next();

	bool bMinus = false;
	bool bNot = false;
	int type =tok.GetType();
	if(type == Token::TK_PLUS || 
		type == Token::TK_MINUS || 
		type == Token::TK_EXCLAMATION)
	{
		if(type == Token::TK_MINUS)bMinus = true;
		if(type == Token::TK_EXCLAMATION)bNot = true;
		tok = scan_->Next();
	}

	if(tok.GetType() == Token::TK_OPENP)
	{
		res = _ParseComparison(scan_->GetCurrentPointer());
		scan_->SetCurrentPointer(res.pos_);
		tok = scan_->Next();
		if(tok.GetType() != Token::TK_CLOSEP)_RaiseError(L")������܂���");
	}
	else
	{
		int type =tok.GetType();
		if(type == Token::TK_INT || type == Token::TK_REAL)
		{
			res.valueReal_ = tok.GetReal();
			res.type_ = TYPE_REAL;
		}
		else if(type == Token::TK_ID)
		{
			Result tRes = _ParseIdentifer(scan_->GetCurrentPointer());
			res = tRes;
		}
		else if(type == Token::TK_STRING)
		{
			res.valueString_ = tok.GetString();
			res.type_ = TYPE_STRING;
		}
		else _RaiseError(StringUtility::Format(L"�s���ȃg�[�N��:%s", tok.GetElement().c_str()));
	}

	if(bMinus)
	{
		if(res.type_ != TYPE_REAL)_RaiseError(L"�����ȊO�ł̕��L��");
		res.valueReal_ = -res.valueReal_;
	}
	if(bNot)
	{
		if(res.type_ != TYPE_BOOLEAN)_RaiseError(L"�^�U�l�ȊO�ł̔ے�");
		res.valueBoolean_ = !res.valueBoolean_;
	}

	res.pos_ = scan_->GetCurrentPointer();
	
	return res;
}
TextParser::Result TextParser::_ParseIdentifer(int pos)
{
	Result res;
	res.pos_ = scan_->GetCurrentPointer();

	Token& tok = scan_->GetToken();
	std::wstring id = tok.GetElement();
	if(id == L"true")
	{
		res.type_ = TYPE_BOOLEAN;
		res.valueBoolean_ = true;
	}
	else if(id == L"false")
	{
		res.type_ = TYPE_BOOLEAN;
		res.valueBoolean_ = false;
	}
	else
	{
		_RaiseError(StringUtility::Format(L"�s���Ȏ��ʎq:%s", id.c_str()));
	}

	return res;
}

void TextParser::SetSource(std::string source)
{
	std::vector<char> buf;
	buf.resize(source.size()+1);
	memcpy(&buf[0], source.c_str(), source.size()+1);
	scan_ = new Scanner(buf);
	scan_->SetPermitSignNumber(false);
}
TextParser::Result TextParser::GetResult()
{
	if(scan_ == NULL)_RaiseError(L"�e�L�X�g���ݒ肳��Ă��܂���");
	scan_->SetPointerBegin();
	Result res = _ParseComparison(scan_->GetCurrentPointer());
	if(scan_->HasNext())_RaiseError(StringUtility::Format(L"�s���ȃg�[�N��:%s", scan_->GetToken().GetElement().c_str()));
	return res;
}
double TextParser::GetReal()
{
	if(scan_ == NULL)_RaiseError(L"�e�L�X�g���ݒ肳��Ă��܂���");
	scan_->SetPointerBegin();
	Result res = _ParseSum(scan_->GetCurrentPointer());
	if(scan_->HasNext())_RaiseError(StringUtility::Format(L"�s���ȃg�[�N��:%s", scan_->GetToken().GetElement().c_str()));
	return res.GetReal();
}

//================================================================
//Font
//const wchar_t* Font::GOTHIC  = L"�W���S�V�b�N";
//const wchar_t* Font::MINCHOH = L"�W������";
const wchar_t* Font::GOTHIC  = L"MS Gothic";
const wchar_t* Font::MINCHOH = L"MS Mincho";

Font::Font()
{
	hFont_ = NULL;
}
Font::~Font()
{
	this->Clear();
}
void Font::Clear()
{
	if(hFont_ != NULL)
	{
		::DeleteObject(hFont_);
		hFont_ = NULL;
		ZeroMemory(&info_, sizeof(LOGFONT));
	}
}
void Font::CreateFont(const wchar_t* type,int size, bool bBold, bool bItalic, bool bLine)
{
	LOGFONT fontInfo;

	lstrcpy(fontInfo.lfFaceName, type);
	fontInfo.lfWeight=(bBold==false)*FW_NORMAL+(bBold==TRUE)*FW_BOLD;
	fontInfo.lfEscapement = 0;
	fontInfo.lfWidth = 0;
	fontInfo.lfHeight = size;
	fontInfo.lfItalic = bItalic;
	fontInfo.lfUnderline = bLine;
	fontInfo.lfCharSet=ANSI_CHARSET;
	for (int i=0;i<(int)wcslen(type);i++)
	{
		if (!(IsCharAlphaNumeric(type[i]) || type[i] == L' ' || type[i] == L'-'))
		{
			fontInfo.lfCharSet = SHIFTJIS_CHARSET;
			break;
		}
	}

	this->CreateFontIndirect(fontInfo);
}
void Font::CreateFontIndirect(LOGFONT& fontInfo)
{
	if(hFont_ != NULL)this->Clear();
	hFont_ = ::CreateFontIndirect(&fontInfo);
	info_ = fontInfo;
}

