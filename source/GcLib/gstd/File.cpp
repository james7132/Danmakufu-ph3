#include"File.hpp"
#include"GstdUtility.hpp"
#include"Logger.hpp"

using namespace gstd;

/**********************************************************
//ByteBuffer
**********************************************************/
ByteBuffer::ByteBuffer()
{
	data_ = NULL;
	Clear();
}
ByteBuffer::ByteBuffer(ByteBuffer& buffer)
{
	data_ = NULL;
	Clear();
	Copy(buffer);
}
ByteBuffer::~ByteBuffer()
{
	Clear();
	if(data_ != NULL)
		delete[] data_;
}
int ByteBuffer::_GetReservedSize()
{
	return reserve_;
}
void ByteBuffer::_Resize(int size)
{
	char* oldData = data_;
	int oldSize = size_;

	data_ = new char[size];
	ZeroMemory(data_, size);

	//元のデータをコピー
	int sizeCopy = min(size, oldSize);
	memcpy(data_, oldData, sizeCopy);
	reserve_ = size;
	size_ = size;

	//古いデータを削除
	delete[] oldData;
}
void ByteBuffer::Copy(ByteBuffer& src)
{
	if(data_ != NULL && src.reserve_ != reserve_)
	{
		delete[] data_;
		data_ = new char[src.reserve_];
		ZeroMemory(data_, src.reserve_);
	}

	offset_ = src.offset_;
	reserve_ = src.reserve_;
	size_ = src.size_;

	memcpy(data_, src.data_, reserve_);
}
void ByteBuffer::Clear()
{
	if(data_ != NULL)
		delete[] data_;

	data_ = new char[0];
	offset_ = 0;
	reserve_ = 0;
	size_ = 0;
}
void ByteBuffer::Seek(int pos)
{
	offset_ = pos;
	if(offset_ < 0)offset_ = 0;
	else if(offset_ > size_)offset_ = size_;
}
void ByteBuffer::SetSize(int size)
{
	_Resize(size);
}
DWORD ByteBuffer::Write(LPVOID buf,DWORD size)
{
	if(offset_+size > reserve_)
	{
		int sizeNew = (offset_+size) * 2;
		_Resize(sizeNew);
		size_ = 0;//あとで再計算
	}

	memcpy(&data_[offset_], buf, size);
	offset_ += size;
	size_ = max(size_, offset_);
	return size;
}
DWORD ByteBuffer::Read(LPVOID buf,DWORD size)
{
	memcpy(buf, &data_[offset_], size);
	offset_ += size;
	return size;
}
char* ByteBuffer::GetPointer(int offset)
{
	if(offset > size_)
	{
		throw gstd::wexception(L"ByteBuffer:インデックスエラー");
	}
	return &data_[offset];
}

/**********************************************************
//File
**********************************************************/
File::File()
{
	hFile_=NULL;
	path_ = L"";
}
File::File(std::wstring path)
{
	hFile_ = NULL;
	path_ = path;
}
File::~File()
{
	Close();
}
bool File::CreateDirectory()
{
	std::wstring dir = PathProperty::GetFileDirectory(path_);
	if(File::IsExists(dir))return true;

	std::vector<std::wstring> str = StringUtility::Split(dir, L"\\/");
	std::wstring tPath = L"";
	for(int iDir=0; iDir < str.size(); iDir++)
	{
		tPath += str[iDir] + L"\\";
		WIN32_FIND_DATA fData;
		HANDLE hFile = ::FindFirstFile(tPath.c_str(), &fData);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			SECURITY_ATTRIBUTES attr;
            attr.nLength = sizeof(SECURITY_ATTRIBUTES);
            attr.lpSecurityDescriptor = NULL;
            attr.bInheritHandle = FALSE;
			::CreateDirectory(tPath.c_str(), &attr);
		}
		::FindClose(hFile);
	}
	return true;
}
void File::Delete()
{
	Close();
	::DeleteFile(path_.c_str());
}
bool File::IsExists()
{
	if(hFile_ != NULL)return true;

	bool res = IsExists(path_);
	return res;
}
bool File::IsExists(std::wstring path)
{
	bool res = PathFileExists(path.c_str()) == TRUE;
	return res;
}
bool File::IsDirectory()
{
	WIN32_FIND_DATA fData;
	HANDLE hFile = ::FindFirstFile(path_.c_str(), &fData);
	bool res = hFile != INVALID_HANDLE_VALUE ? true : false;
	if(res)res = (FILE_ATTRIBUTE_DIRECTORY & fData.dwFileAttributes) > 0;

	::FindClose(hFile);
	return res;
}
int File::GetSize()
{
	if(hFile_!=NULL)return ::GetFileSize(hFile_,NULL);

	int res = 0;
	WIN32_FIND_DATA fData;
	HANDLE hFile = ::FindFirstFile(path_.c_str(), &fData);
	res = hFile != INVALID_HANDLE_VALUE ? ::GetFileSize(hFile, NULL) : 0;
	::FindClose(hFile);
	return res;
}

bool File::Open()
{
	return this->Open(AccessType::READ);
}
bool File::Open(AccessType typeAccess)
{
	if(hFile_!=NULL)this->Close();

	DWORD access = typeAccess == AccessType::READ ? GENERIC_READ : GENERIC_READ|GENERIC_WRITE;
	hFile_=::CreateFile(path_.c_str(),access,
							FILE_SHARE_READ,NULL,OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile_==INVALID_HANDLE_VALUE)
	{
		hFile_=NULL;
		return false;
	}
	return true;
}
bool File::Create()
{
	if(hFile_!=NULL)this->Close();
	hFile_=CreateFile(path_.c_str(),GENERIC_READ|GENERIC_WRITE,
							FILE_SHARE_READ,NULL,CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile_==INVALID_HANDLE_VALUE)
	{
		hFile_=NULL;
		return false;
	}
	return true;
}
void File::Close()
{
	if(hFile_!=NULL)CloseHandle(hFile_);
	hFile_=NULL;
}

DWORD File::Read(LPVOID buf,DWORD size)
{
	DWORD res=0;
	::ReadFile(hFile_, buf, size, &res, NULL);
	return res;
}
DWORD File::Write(LPVOID buf,DWORD size)
{
	DWORD res=0;
	::WriteFile(hFile_, buf, size, &res, NULL);
	return res;
}
bool File::IsEqualsPath(std::wstring path1, std::wstring path2)
{
	path1 = PathProperty::GetUnique(path1);
	path2 = PathProperty::GetUnique(path2);
	bool res = (wcscmp(path1.c_str(), path2.c_str()) == 0);
	return res;
}
std::vector<std::wstring> File::GetFilePathList(std::wstring dir)
{
	std::vector<std::wstring> res;

	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::wstring findDir = dir + L"*.*";
	hFind = FindFirstFile(findDir.c_str(), &data);
	do 
	{
		std::wstring name = data.cFileName;
		if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			(name != L".." && name != L"."))
		{
			//ディレクトリ
			std::wstring tDir = dir + name;
			tDir += L"\\";

			continue;
		}
		if(wcscmp(data.cFileName, L"..")==0 || wcscmp(data.cFileName, L".")==0)
			continue;

		//ファイル
		std::wstring path = dir + name;

		res.push_back(path);

	}while(FindNextFile(hFind, &data));
	FindClose(hFind);

	return res;
}
std::vector<std::wstring> File::GetDirectoryPathList(std::wstring dir)
{
	std::vector<std::wstring> res;

	WIN32_FIND_DATA data;
	HANDLE hFind;
	std::wstring findDir = dir + L"*.*";
	hFind = FindFirstFile(findDir.c_str(), &data);
	do 
	{
		std::wstring name = data.cFileName;
		if((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && 
			(name != L".." && name != L"."))
		{
			//ディレクトリ
			std::wstring tDir = dir + name;
			tDir += L"\\";
			res.push_back(tDir);
			continue;
		}
		if(wcscmp(data.cFileName, L"..")==0 || wcscmp(data.cFileName, L".")==0)
			continue;

		//ファイル

	}while(FindNextFile(hFind, &data));
	FindClose(hFind);

	return res;
}

/**********************************************************
//ArchiveFileEntry
**********************************************************/
ArchiveFileEntry::ArchiveFileEntry()
{
	typeCompress_ = CT_NON;
	sizeData_ = 0;
	sizeCompressed_ = 0;
	offset_ = 0;
}
ArchiveFileEntry::~ArchiveFileEntry()
{

}
int ArchiveFileEntry::_GetEntryRecordSize()
{
	int res = 0;

	res += sizeof(int);
	res += StringUtility::GetByteSize(dir_);
	res += sizeof(int);
	res += StringUtility::GetByteSize(name_);
	res += sizeof(CompressType);
	res += sizeof(int);
	res += sizeof(int);
	res += sizeof(int);

	return res;
}
void ArchiveFileEntry::_WriteEntryRecord(ByteBuffer &buf)
{
	buf.WriteInteger(dir_.size());
	buf.Write(&dir_[0], StringUtility::GetByteSize(dir_));
	buf.WriteInteger(name_.size());
	buf.Write(&name_[0], StringUtility::GetByteSize(name_));
	buf.Write(&typeCompress_, sizeof(CompressType));
	buf.WriteInteger(sizeData_);
	buf.WriteInteger(sizeCompressed_);
	buf.WriteInteger(offset_);
}
void ArchiveFileEntry::_ReadEntryRecord(ByteBuffer &buf)
{
	dir_.resize(buf.ReadInteger());
	buf.Read(&dir_[0], StringUtility::GetByteSize(dir_));
	name_.resize(buf.ReadInteger());
	buf.Read(&name_[0], StringUtility::GetByteSize(name_));
	buf.Read(&typeCompress_, sizeof(CompressType));
	sizeData_ = buf.ReadInteger();
	sizeCompressed_ = buf.ReadInteger();
	offset_ = buf.ReadInteger();
}
/**********************************************************
//FileArchiver
**********************************************************/
FileArchiver::FileArchiver()
{

}
FileArchiver::~FileArchiver()
{

}
bool FileArchiver::CreateArchiveFile(std::wstring path)
{
	bool res = true;
	File fileArchive(path);
	if(!fileArchive.Create())
		throw gstd::wexception(StringUtility::Format(L"ファイル作成失敗[%s]", path.c_str()).c_str());

	fileArchive.Write((char*)&HEADER_ARCHIVEFILE[0], HEADER_ARCHIVEFILE.size());
	fileArchive.WriteInteger(listEntry_.size());

	int posArchiveEntryHeaderStart = fileArchive.GetFilePointer();
	fileArchive.WriteBoolean(false);
	fileArchive.WriteInteger(0);

	int posEntryStart = fileArchive.GetFilePointer();

	std::list<ref_count_ptr<ArchiveFileEntry> >::iterator itr;
	for(itr = listEntry_.begin(); itr != listEntry_.end(); itr++)
	{
		ref_count_ptr<ArchiveFileEntry> entry = *itr;

		std::wstring name = entry->GetName();
		entry->SetName(PathProperty::GetFileName(name));

		fileArchive.WriteInteger(entry->_GetEntryRecordSize());

		ByteBuffer buf;
		entry->_WriteEntryRecord(buf);
		fileArchive.Write(buf.GetPointer(), buf.GetSize());

		entry->SetName(name);
	}
	int posEntryEnd = fileArchive.GetFilePointer();

	for(itr = listEntry_.begin(); itr != listEntry_.end(); itr++)
	{
		ref_count_ptr<ArchiveFileEntry> entry = *itr;
		std::wstring path = entry->GetName();
		File file(path);
		if(!file.Open())
			throw gstd::wexception(StringUtility::Format(L"ファイルオープン失敗[%s]", path.c_str()).c_str());

		entry->_SetDataSize(file.GetSize());
		entry->_SetOffset(fileArchive.GetFilePointer());
		ByteBuffer buf;
		buf.SetSize(file.GetSize());
		file.Read(buf.GetPointer(), buf.GetSize());

		ArchiveFileEntry::CompressType typeCompress = entry->GetCompressType();
		if(typeCompress == ArchiveFileEntry::CT_NON)
		{
			fileArchive.Write(buf.GetPointer(), buf.GetSize());
		}
		else if(typeCompress == ArchiveFileEntry::CT_COMPRESS)
		{
			ByteBuffer bufComp;
			Compressor inflater;
			inflater.Compress(buf, bufComp);

			entry->_SetCompressedDataSize(bufComp.GetSize());
			fileArchive.Write(bufComp.GetPointer(), bufComp.GetSize());
		}
	}

	//とりあえず非圧縮で書き込む
	fileArchive.Seek(posArchiveEntryHeaderStart);
	fileArchive.WriteBoolean(false);
	fileArchive.WriteInteger(0);
	for(itr = listEntry_.begin(); itr != listEntry_.end(); itr++)
	{
		ref_count_ptr<ArchiveFileEntry> entry = *itr;

		std::wstring name = entry->GetName();
		entry->SetName(PathProperty::GetFileName(name));

		fileArchive.WriteInteger(entry->_GetEntryRecordSize());

		ByteBuffer buf;
		entry->_WriteEntryRecord(buf);
		fileArchive.Write(buf.GetPointer(), buf.GetSize());

		entry->SetName(name);
	}

	//圧縮可能か調べる
	fileArchive.Seek(posEntryStart);
	int sizeEntry = posEntryEnd - posEntryStart;
	ByteBuffer bufEntryIn;
	ByteBuffer bufEntryOut;
	bufEntryIn.SetSize(sizeEntry);
	fileArchive.Read(bufEntryIn.GetPointer(), sizeEntry);

	Compressor compEntry;
	compEntry.Compress(bufEntryIn, bufEntryOut);
	if(bufEntryOut.GetSize() < sizeEntry)
	{
		//エントリ圧縮
		fileArchive.Seek(posArchiveEntryHeaderStart);
		fileArchive.WriteBoolean(true);
		fileArchive.WriteInteger(bufEntryOut.GetSize());
		fileArchive.Write(bufEntryOut.GetPointer(), bufEntryOut.GetSize());
		
		int sizeGap = sizeEntry - bufEntryOut.GetSize();
		ByteBuffer bufGap;
		bufGap.SetSize(sizeGap);
		fileArchive.Write(bufGap.GetPointer(), sizeGap);
	}

	return res;
}

/**********************************************************
//ArchiveFile
**********************************************************/
ArchiveFile::ArchiveFile(std::wstring path)
{
	file_ = new File(path);
}
ArchiveFile::~ArchiveFile()
{
	Close();
}
bool ArchiveFile::Open()
{
	if(!file_->Open())return false;
	if(mapEntry_.size() != 0)return true;

	bool res = true;
	try
	{
		std::string header;
		header.resize(HEADER_ARCHIVEFILE.size());
		file_->Read(&header[0], header.size());
		if(header != HEADER_ARCHIVEFILE)throw gstd::wexception();

		int countEntry = file_->ReadInteger();
		bool bCompress = file_->ReadBoolean();
		int sizeArchiveHeader = file_->ReadInteger();

		ref_count_ptr<Reader> reader = NULL;
		if(!bCompress)reader = file_;
		else
		{
			ByteBuffer bufIn;
			ByteBuffer* bufOut = new ByteBuffer();
			bufIn.SetSize(sizeArchiveHeader);
			file_->Read(bufIn.GetPointer(), sizeArchiveHeader);
			DeCompressor decomp;
			decomp.DeCompress(bufIn, *bufOut);

			reader = bufOut;
		}

		for(int iEntry = 0 ; iEntry < countEntry ; iEntry++)
		{
			ref_count_ptr<ArchiveFileEntry> entry = new ArchiveFileEntry();
			int sizeEntry = reader->ReadInteger();
			ByteBuffer buf;
			buf.SetSize(sizeEntry);
			reader->Read(buf.GetPointer(), sizeEntry);
			entry->_ReadEntryRecord(buf);
			entry->_SetArchivePath(file_->GetPath());
			
			//std::string key = entry->GetDirectory() + entry->GetName();
			//mapEntry_[key] = entry;
			std::wstring key = entry->GetName();
			mapEntry_.insert(std::pair<std::wstring, ref_count_ptr<ArchiveFileEntry> >(key, entry));

		}

		res = true;
	}
	catch(...)
	{
		res = false;
	}
	file_->Close();
	return res;
}
void ArchiveFile::Close()
{
	file_->Close();
}
std::set<std::wstring> ArchiveFile::GetKeyList()
{
	std::set<std::wstring> res;
	std::multimap<std::wstring, ref_count_ptr<ArchiveFileEntry> >::iterator itr = mapEntry_.begin();
	for(; itr != mapEntry_.end() ; itr++)
	{
		ref_count_ptr<ArchiveFileEntry> entry = itr->second;
		//std::wstring key = entry->GetDirectory() + entry->GetName();
		std::wstring key = entry->GetName();
		res.insert(key);
	}
	return res;
}
std::vector<ref_count_ptr<ArchiveFileEntry> > ArchiveFile::GetEntryList(std::wstring name)
{
	std::vector<ref_count_ptr<ArchiveFileEntry> > res;
	if(!IsExists(name))return res;

	std::pair<std::multimap<std::wstring, ref_count_ptr<ArchiveFileEntry> >::iterator, 
		std::multimap<std::wstring, ref_count_ptr<ArchiveFileEntry> >::iterator> itrPair = 
		mapEntry_.equal_range(name);
	for( ; itrPair.first != itrPair.second ; itrPair.first++ )
	{
		ref_count_ptr<ArchiveFileEntry> entry = (itrPair.first)->second;
		res.push_back(entry);
	}

	return res;
}
bool ArchiveFile::IsExists(std::wstring name)
{
	return mapEntry_.find(name) != mapEntry_.end();
}
ref_count_ptr<ByteBuffer> ArchiveFile::CreateEntryBuffer(ref_count_ptr<ArchiveFileEntry> entry)
{
	ref_count_ptr<ByteBuffer> res;
	File file(entry->GetArchivePath());
	if(file.Open())
	{
		if(entry->GetCompressType() == ArchiveFileEntry::CT_NON)
		{
			file.Seek(entry->GetOffset());
			res = new ByteBuffer();
			int size = entry->GetDataSize();
			res->SetSize(size);
			file.Read(res->GetPointer(), size);
		}
		else if(entry->GetCompressType() == ArchiveFileEntry::CT_COMPRESS)
		{
			file.Seek(entry->GetOffset());
			res = new ByteBuffer();

			ByteBuffer bufComp;
			bufComp.SetSize(entry->GetCompressedDataSize());
			file.Read(bufComp.GetPointer(), bufComp.GetSize());

			gstd::DeCompressor deflater;
			deflater.DeCompress(bufComp, *res.GetPointer());
		}
	}
	return res;
}
/*
ref_count_ptr<ByteBuffer> ArchiveFile::GetBuffer(std::string name)
{
	if(!IsExists(name))return NULL;

	if(!file_->Open())return NULL;

	ref_count_ptr<ByteBuffer> res = new ByteBuffer();
	ref_count_ptr<ArchiveFileEntry> entry = mapEntry_[name];
	int offset = entry->GetOffset();
	int size = entry->GetDataSize();

	res->SetSize(size);
	file_->Seek(offset);
	file_->Read(res->GetPointer(), size);

	file_->Close();
	return res;
}
*/

/**********************************************************
//FileManager
**********************************************************/
FileManager* FileManager::thisBase_ = NULL;
FileManager::FileManager()
{
}
FileManager::~FileManager()
{
	EndLoadThread();
}
bool FileManager::Initialize()
{
	if(thisBase_ != NULL)return false;
	thisBase_ = this;
	threadLoad_ = new LoadThread();
	threadLoad_->Start();
	return true;
}
void FileManager::EndLoadThread()
{
	{
		Lock lock(lock_);
		if(threadLoad_ == NULL)return;
		threadLoad_->Stop();
		threadLoad_->Join();
		threadLoad_ = NULL;
	}
}
bool FileManager::AddArchiveFile(std::wstring path)
{
	if(mapArchiveFile_.find(path) != mapArchiveFile_.end())return true;

	ref_count_ptr<ArchiveFile> file = new ArchiveFile(path);
	if(!file->Open())return false;

	std::set<std::wstring> listKeyIn = file->GetKeyList();
	std::set<std::wstring> listKeyCurrent;
	std::map<std::wstring, ref_count_ptr<ArchiveFile> >::iterator itrFile;
	for(itrFile=mapArchiveFile_.begin() ; itrFile!=mapArchiveFile_.end() ; itrFile++)
	{
		ref_count_ptr<ArchiveFile> tFile = itrFile->second;
		std::set<std::wstring> tList = tFile->GetKeyList();
		std::set<std::wstring>::iterator itrList = tList.begin();
		for(; itrList != tList.end() ; itrList++)
		{
			listKeyCurrent.insert(*itrList);
		}
	}

	std::set<std::wstring>::iterator itrKey = listKeyIn.begin();
	for(; itrKey != listKeyIn.end() ; itrKey++)
	{
		std::wstring key = *itrKey;
		if(listKeyCurrent.find(key) == listKeyCurrent.end())continue;

		std::wstring log = StringUtility::Format(L"archive file entry already exists[%s]", key.c_str());
		Logger::WriteTop(log);
		throw wexception(log.c_str());
	}

	mapArchiveFile_[path] = file;
	return true;
}
bool FileManager::RemoveArchiveFile(std::wstring path)
{
	mapArchiveFile_.erase(path);
	return true;
}
ref_count_ptr<FileReader> FileManager::GetFileReader(std::wstring path)
{
	std::wstring orgPath = path;
	path = PathProperty::GetUnique(path);

	ref_count_ptr<FileReader> res = NULL;
	ref_count_ptr<File> fileRaw = new File(path);
	if(fileRaw->IsExists())
	{
		res = new ManagedFileReader(fileRaw, NULL);
	}
	else
	{
		std::vector<ref_count_ptr<ArchiveFileEntry> > listEntry;

		std::map<int, std::wstring> mapArchivePath;
		std::wstring key = PathProperty::GetFileName(path);
		std::map<std::wstring, ref_count_ptr<ArchiveFile> >::iterator itr;
		for(itr=mapArchiveFile_.begin() ; itr!=mapArchiveFile_.end() ; itr++)
		{
			std::wstring pathArchive = itr->first;
			ref_count_ptr<ArchiveFile> fileArchive = itr->second;
			if(!fileArchive->IsExists(key))continue;
			
			ref_count_ptr<File> file = new File(pathArchive);
			std::vector<ref_count_ptr<ArchiveFileEntry> > list = fileArchive->GetEntryList(key);
			listEntry.insert(listEntry.end(), list.begin(), list.end());
			for(int iEntry = 0 ; iEntry < list.size() ; iEntry++)
			{
				ref_count_ptr<ArchiveFileEntry> entry = list[iEntry];
				int addr = (int)entry.GetPointer();
				mapArchivePath[addr] = pathArchive;
			}
		}

		if(listEntry.size() == 1)
		{
			ref_count_ptr<ArchiveFileEntry> entry = listEntry[0];
			int addr = (int)entry.GetPointer();
			std::wstring pathArchive = mapArchivePath[addr];
			ref_count_ptr<File> file = new File(pathArchive);
			res = new ManagedFileReader(file, entry);
		}
		else
		{
			std::wstring module = PathProperty::GetModuleDirectory();
			module = PathProperty::GetUnique(module);

			std::wstring target = StringUtility::ReplaceAll(path, module, L"");
			for(int iEntry = 0 ; iEntry < listEntry.size() ; iEntry++)
			{
				ref_count_ptr<ArchiveFileEntry> entry = listEntry[iEntry];
				std::wstring dir = entry->GetDirectory();
				if(target.find(dir) == std::wstring::npos)continue;

				int addr = (int)entry.GetPointer();
				std::wstring pathArchive = mapArchivePath[addr];
				ref_count_ptr<File> file = new File(pathArchive);
				res = new ManagedFileReader(file, entry);
				break;
			}
		}

	}
	if(res != NULL)res->_SetOriginalPath(orgPath);
	return res;
}

ref_count_ptr<ByteBuffer> FileManager::_GetByteBuffer(ref_count_ptr<ArchiveFileEntry> entry)
{
	ref_count_ptr<ByteBuffer> res = NULL;
	try
	{
		Lock lock(lock_);
		std::wstring key = entry->GetDirectory() + entry->GetName();
		if(mapByteBuffer_.find(key) != mapByteBuffer_.end())
		{
			res = mapByteBuffer_[key];
		}
		else
		{
			res = ArchiveFile::CreateEntryBuffer(entry);
			if(res != NULL)mapByteBuffer_[key] = res;
		}
	}
	catch(...) { }

	return res;
}
void FileManager::_ReleaseByteBuffer(ref_count_ptr<ArchiveFileEntry> entry)
{
	{
		Lock lock(lock_);
		std::wstring key = entry->GetDirectory() + entry->GetName();
		if(mapByteBuffer_.find(key) == mapByteBuffer_.end())return;
		ref_count_ptr<ByteBuffer> buffer = mapByteBuffer_[key];
		if(buffer.GetReferenceCount() == 2)
		{
			mapByteBuffer_.erase(key);
		}
	}
}
void FileManager::AddLoadThreadEvent(ref_count_ptr<FileManager::LoadThreadEvent> event)
{
	{
		Lock lock(lock_);
		if(threadLoad_ == NULL)return;
		threadLoad_->AddEvent(event);
	}
}
void FileManager::AddLoadThreadListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lock_);
		if(threadLoad_ == NULL)return;
		threadLoad_->AddListener(listener);
	}
}
void FileManager::RemoveLoadThreadListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lock_);
		if(threadLoad_ == NULL)return;
		threadLoad_->RemoveListener(listener);
	}
}
void FileManager::WaitForThreadLoadComplete()
{
	while(!threadLoad_->IsThreadLoadComplete()) 
	{
		Sleep(1);
	}
}

//FileManager::LoadThread
FileManager::LoadThread::LoadThread()
{
}
FileManager::LoadThread::~LoadThread()
{
}
void FileManager::LoadThread::_Run()
{
	while(this->GetStatus() == RUN)
	{
		signal_.Wait(10);

		while(this->GetStatus() == RUN)
		{
			//Logger::WriteTop(StringUtility::Format("ロードイベント取り出し開始"));
			ref_count_ptr<FileManager::LoadThreadEvent> event = NULL;
			{
				Lock lock(lockEvent_);
				if(listEvent_.size() == 0)break;
				event = listEvent_.front();
				//listPath_.erase(event->GetPath());
				listEvent_.pop_front();
			}
			//Logger::WriteTop(StringUtility::Format("ロードイベント取り出し完了：%s", event->GetPath().c_str()));

			//Logger::WriteTop(StringUtility::Format("ロード開始：%s", event->GetPath().c_str()));
			{
				Lock lock(lockListener_);
				std::list<FileManager::LoadThreadListener*>::iterator itr;
				for(itr = listListener_.begin(); itr != listListener_.end() ; itr++)
				{
					FileManager::LoadThreadListener* listener = (*itr);
					if(event->GetListener() == listener)
						listener->CallFromLoadThread(event);
				}	
			}
			//Logger::WriteTop(StringUtility::Format("ロード完了：%s", event->GetPath().c_str()));

		}
		Sleep(1);//TODO なぜか待機入れると落ちづらい？
	}

	{
		Lock lock(lockListener_);
		listListener_.clear();
	}
}
void FileManager::LoadThread::Stop()
{
	Thread::Stop();
	signal_.SetSignal();
}
bool FileManager::LoadThread::IsThreadLoadComplete()
{
	bool res = false;
	{
		Lock lock(lockEvent_);
		res = listEvent_.size() == 0;
	}
	return res;
}
bool FileManager::LoadThread::IsThreadLoadExists(std::wstring path)
{
	bool res = false;
	{
		Lock lock(lockEvent_);
		//res = listPath_.find(path) != listPath_.end();
	}
	return res;
}
void FileManager::LoadThread::AddEvent(ref_count_ptr<FileManager::LoadThreadEvent> event)
{
	{
		Lock lock(lockEvent_);
		std::wstring path = event->GetPath();
		if(IsThreadLoadExists(path))return;
		listEvent_.push_back(event);
		//listPath_.insert(path);
		signal_.SetSignal();
		signal_.SetSignal(false);
	}
}
void FileManager::LoadThread::AddListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lockListener_);
		std::list<FileManager::LoadThreadListener*>::iterator itr;
		for(itr = listListener_.begin(); itr != listListener_.end() ; itr++)
		{
			if(*itr == listener)return;
		}

		listListener_.push_back(listener);	
	}
}
void FileManager::LoadThread::RemoveListener(FileManager::LoadThreadListener* listener)
{
	{
		Lock lock(lockListener_);
		std::list<FileManager::LoadThreadListener*>::iterator itr;
		for(itr = listListener_.begin(); itr != listListener_.end() ; itr++)
		{
			if(*itr != listener)continue;
			listListener_.erase(itr);
			break;
		}
	}
}

/**********************************************************
//ManagedFileReader
**********************************************************/
ManagedFileReader::ManagedFileReader(ref_count_ptr<File> file, ref_count_ptr<ArchiveFileEntry> entry)
{
	offset_ = 0;
	file_ = file;
	entry_ = entry;

	if(entry_ == NULL)
	{
		type_ = TYPE_NORMAL;
	}
	else if(entry_->GetCompressType() == ArchiveFileEntry::CT_NON && entry_ != NULL)
	{
		type_ = TYPE_ARCHIVED;
	}
	else if(entry_->GetCompressType() != ArchiveFileEntry::CT_NON && entry_ != NULL)
	{
		type_ = TYPE_ARCHIVED_COMPRESSED;
	}
}
ManagedFileReader::~ManagedFileReader()
{
	Close();
}
bool ManagedFileReader::Open()
{
	bool res = false;
	offset_ = 0;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->Open();
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		res = file_->Open();
		if(res)
		{
			file_->Seek(entry_->GetOffset());
		}
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{
		buffer_ = FileManager::GetBase()->_GetByteBuffer(entry_);
		res = buffer_ != NULL;
	}
	return res;
}
void ManagedFileReader::Close()
{
	if(file_ != NULL)file_->Close();
	if(buffer_ != NULL)
	{
		buffer_ = NULL;
		FileManager::GetBase()->_ReleaseByteBuffer(entry_);
	}
}
int ManagedFileReader::GetFileSize()
{
	int res = 0;
	if(type_ == TYPE_NORMAL)res = file_->GetSize();
	else if(type_ == TYPE_ARCHIVED )res = entry_->GetDataSize();
	else if(type_ == TYPE_ARCHIVED_COMPRESSED && buffer_ != NULL)res = buffer_->GetSize();
	return res;
}
DWORD ManagedFileReader::Read(LPVOID buf,DWORD size)
{
	DWORD res = 0;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->Read(buf, size);
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		int read = size;
		if(entry_->GetDataSize() < offset_ + size)
		{
			read = entry_->GetDataSize() - offset_;
		}
		res = file_->Read(buf, read);
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{
		int read = size;
		if(buffer_->GetSize() < offset_ + size)
		{
			read = buffer_->GetSize() - offset_;
		}
		memcpy(buf, &buffer_->GetPointer()[offset_], read);
		res = read;
	}
	offset_ += res;
	return res;
}
BOOL ManagedFileReader::SetFilePointerBegin()
{
	BOOL res = FALSE;
	offset_ = 0;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->SetFilePointerBegin();
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		res = file_->Seek(entry_->GetOffset());
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{

	}
	return res;
}
BOOL ManagedFileReader::SetFilePointerEnd()
{
	BOOL res = FALSE;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->SetFilePointerEnd();
		offset_ = file_->GetSize();
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		res = file_->Seek(entry_->GetOffset() + entry_->GetDataSize());
		offset_ = entry_->GetDataSize();
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{
		if(buffer_ != NULL)
		{
			offset_ = buffer_->GetSize();
			res = TRUE;
		}
	}
	return res;
}
BOOL ManagedFileReader::Seek(LONG offset)
{
	BOOL res = FALSE;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->Seek(offset);
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		res = file_->Seek(entry_->GetOffset() + offset);
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{
		if(buffer_ != NULL)
		{
			res = TRUE;
		}
	}
	if(res == TRUE)
		offset_ = offset;
	return res;
}
LONG ManagedFileReader::GetFilePointer()
{
	LONG res = 0;
	if(type_ == TYPE_NORMAL)
	{
		res = file_->GetFilePointer();
	}
	else if(type_ == TYPE_ARCHIVED)
	{
		res = file_->GetFilePointer() - entry_->GetOffset();
	}
	else if(type_ == TYPE_ARCHIVED_COMPRESSED)
	{
		if(buffer_ != NULL)
		{
			res = offset_;
		}
	}
	return res;
}
bool ManagedFileReader::IsArchived()
{
	return type_ != TYPE_NORMAL;
}
bool ManagedFileReader::IsCompressed()
{
	return type_ == TYPE_ARCHIVED_COMPRESSED;
}


/**********************************************************
//RecordEntry
**********************************************************/
RecordEntry::RecordEntry()
{
	type_ = TYPE_UNKNOWN;
}
RecordEntry::~RecordEntry()
{

}
int RecordEntry::_GetEntryRecordSize()
{
	int res = 0;
	res += sizeof(char);
	res += sizeof(int);
	res += key_.size();
	res += sizeof(int);
	res += buffer_.GetSize();
	return res;
}
void RecordEntry::_WriteEntryRecord(Writer &writer)
{
	writer.WriteCharacter(type_);
	writer.WriteInteger(key_.size());
	writer.Write(&key_[0], key_.size());

	writer.WriteInteger(buffer_.GetSize());
	writer.Write(buffer_.GetPointer(), buffer_.GetSize());
}
void RecordEntry::_ReadEntryRecord(Reader &reader)
{
	type_ = reader.ReadCharacter();
	key_.resize(reader.ReadInteger());
	reader.Read(&key_[0], key_.size());

	buffer_.Clear();
	buffer_.SetSize(reader.ReadInteger());
	reader.Read(buffer_.GetPointer(), buffer_.GetSize());
}

/**********************************************************
//RecordBuffer
**********************************************************/
RecordBuffer::RecordBuffer()
{

}
RecordBuffer::~RecordBuffer()
{
	this->Clear();
}
void RecordBuffer::Clear()
{
	mapEntry_.clear();
}
ref_count_ptr<RecordEntry> RecordBuffer::GetEntry(std::string key)
{
	return IsExists(key) ? mapEntry_[key] : NULL;
}
bool RecordBuffer::IsExists(std::string key)
{
	return mapEntry_.find(key) != mapEntry_.end();
}
std::vector<std::string> RecordBuffer::GetKeyList()
{
	std::vector<std::string> res;
	std::map<std::string, ref_count_ptr<RecordEntry> >::iterator itr;
	for(itr=mapEntry_.begin() ; itr!=mapEntry_.end() ; itr++)
	{
		std::string key = itr->first;
		res.push_back(key);
	}
	return res;
}

void RecordBuffer::Write(Writer& writer)
{
	int countEntry = mapEntry_.size();
	writer.WriteInteger(countEntry);

	std::map<std::string, ref_count_ptr<RecordEntry> >::iterator itr;
	for(itr=mapEntry_.begin() ; itr!=mapEntry_.end() ; itr++)
	{
		ref_count_ptr<RecordEntry> entry = itr->second;
		entry->_WriteEntryRecord(writer);
	}
}
void RecordBuffer::Read(Reader& reader)
{
	this->Clear();
	int countEntry = reader.ReadInteger();
	for(int iEntry = 0 ; iEntry < countEntry; iEntry++)
	{
		ref_count_ptr<RecordEntry> entry = new RecordEntry();
		entry->_ReadEntryRecord(reader);

		std::string key = entry->GetKey();
		mapEntry_[key] = entry;
	}
}
bool RecordBuffer::WriteToFile(std::wstring path, std::string header)
{
	File file(path);
	if(!file.Create())return false;

	file.Write((char*)&header[0], header.size());
	Write(file);
	file.Close();

	return true;
}
bool RecordBuffer::ReadFromFile(std::wstring path, std::string header)
{
	File file(path);
	if(!file.Open())return false;

	std::string fHead;
	fHead.resize(header.size());
	file.Read(&fHead[0], fHead.size());
	if(fHead != header)return false;

	Read(file);
	file.Close();

	return true;
}
int RecordBuffer::GetEntryType(std::string key)
{
	if(!IsExists(key))return RecordEntry::TYPE_NOENTRY;
	return mapEntry_[key]->GetType();
}
int RecordBuffer::GetEntrySize(std::string key)
{
	if(!IsExists(key))return 0;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	return buffer.GetSize();
}
bool RecordBuffer::GetRecord(std::string key, LPVOID buf, DWORD size)
{
	if(!IsExists(key))return false;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	buffer.Seek(0);
	buffer.Read(buf, size);
	return true;
}
bool RecordBuffer::GetRecordAsBoolean(std::string key)
{
	bool res = 0;
	GetRecord(key, res);
	return res;
}
int RecordBuffer::GetRecordAsInteger(std::string key)
{
	int res = 0;
	GetRecord(key, res);
	return res;
}
float RecordBuffer::GetRecordAsFloat(std::string key)
{
	float res = 0;
	GetRecord(key, res);
	return res;
}
double RecordBuffer::GetRecordAsDouble(std::string key)
{
	double res = 0;
	GetRecord(key, res);
	return res;
}
std::string RecordBuffer::GetRecordAsStringA(std::string key)
{
	if(!IsExists(key))return "";
	
	std::string res;
	ref_count_ptr<RecordEntry> entry = mapEntry_[key];
	int type = entry->GetType();
	ByteBuffer& buffer = entry->GetBufferRef();
	buffer.Seek(0);
	if(type == RecordEntry::TYPE_STRING_A)
	{
		res.resize(buffer.GetSize());
		buffer.Read(&res[0], buffer.GetSize());
	}
	else if(type == RecordEntry::TYPE_STRING_W)
	{
		std::wstring wstr;
		wstr.resize(buffer.GetSize() / sizeof(wchar_t));
		buffer.Read(&wstr[0], buffer.GetSize());
		res = StringUtility::ConvertWideToMulti(wstr);
	}

	return res;
}
std::wstring RecordBuffer::GetRecordAsStringW(std::string key)
{
	if(!IsExists(key))return L"";
	
	std::wstring res;
	ref_count_ptr<RecordEntry> entry = mapEntry_[key];
	int type = entry->GetType();
	ByteBuffer& buffer = entry->GetBufferRef();
	buffer.Seek(0);
	if(type == RecordEntry::TYPE_STRING_A)
	{
		std::string str;
		str.resize(buffer.GetSize());
		buffer.Read(&str[0], buffer.GetSize());

		res = StringUtility::ConvertMultiToWide(str);
	}
	else if(type == RecordEntry::TYPE_STRING_W)
	{
		res.resize(buffer.GetSize() / sizeof(wchar_t));
		buffer.Read(&res[0], buffer.GetSize());
	}

	return res;
}
bool RecordBuffer::GetRecordAsRecordBuffer(std::string key, RecordBuffer& record)
{
	if(!IsExists(key))return false;
	ByteBuffer& buffer = mapEntry_[key]->GetBufferRef();
	buffer.Seek(0);
	record.Read(buffer);
	return true;
}
void RecordBuffer::SetRecord(int type, std::string key, LPVOID buf, DWORD size)
{
	ref_count_ptr<RecordEntry> entry = new RecordEntry();
	entry->SetType((char)type);
	entry->SetKey(key);
	ByteBuffer& buffer = entry->GetBufferRef();
	buffer.SetSize(size);
	buffer.Write(buf, size);
	mapEntry_[key] = entry;
}
void RecordBuffer::SetRecordAsRecordBuffer(std::string key, RecordBuffer& record)
{
	ref_count_ptr<RecordEntry> entry = new RecordEntry();
	entry->SetType((char)RecordEntry::TYPE_RECORD);
	entry->SetKey(key);
	ByteBuffer& buffer = entry->GetBufferRef();
	record.Write(buffer);
	mapEntry_[key] = entry;
}

void RecordBuffer::Read(RecordBuffer& record)
{
}
void RecordBuffer::Write(RecordBuffer& record)
{
}

/**********************************************************
//PropertyFile
**********************************************************/
PropertyFile::PropertyFile()
{
}
PropertyFile::~PropertyFile()
{
}
bool PropertyFile::Load(std::wstring path)
{
	mapEntry_.clear();

	std::vector<char> text;
	FileManager* fileManager = FileManager::GetBase();
	if(fileManager != NULL)
	{
		ref_count_ptr<FileReader> reader = fileManager->GetFileReader(path);

		if(reader == NULL || !reader->Open())
		{
			Logger::WriteTop(ErrorUtility::GetFileNotFoundErrorMessage(path));
			return false;
		}

		int size = reader->GetFileSize();
		text.resize(size+1);
		reader->Read(&text[0], size);
		text[size] = '\0';
	}
	else
	{
		File file(path);
		if(!file.Open())return false;

		int size = file.GetSize();
		text.resize(size+1);
		file.Read(&text[0], size);
		text[size] = '\0';
	}

	bool res = false;
	gstd::Scanner scanner(text);
	try
	{
		while(scanner.HasNext())
		{
			gstd::Token& tok = scanner.Next();
			if(tok.GetType() != Token::TK_ID)continue;
			std::wstring key = tok.GetElement();
			while(true)
			{
				tok = scanner.Next();
				if(tok.GetType() == Token::TK_EQUAL)break;
				key += tok.GetElement();
			}

			std::wstring value;
			try 
			{
				int posS = scanner.GetCurrentPointer();
				int posE = posS;
				while(true)
				{
					bool bEndLine = false;
					if(!scanner.HasNext())
					{
						bEndLine = true;
					}
					else
					{
						tok = scanner.Next();
						bEndLine = tok.GetType() == Token::TK_NEWLINE;
					}

					if(bEndLine)
					{
						posE = scanner.GetCurrentPointer();
						value = scanner.GetString(posS, posE);
						value = StringUtility::Trim(value);
						break;
					}
				}
			} 
			catch(...){}

			mapEntry_[key] = value;
		}

		res = true;
	}
	catch(gstd::wexception& e)
	{
		mapEntry_.clear();
		Logger::WriteTop(
			ErrorUtility::GetParseErrorMessage(path, scanner.GetCurrentLine(), e.what()));
		res = false;
	}

	return res;
}
bool PropertyFile::HasProperty(std::wstring key)
{
	return mapEntry_.find(key) != mapEntry_.end();
}
std::wstring PropertyFile::GetString(std::wstring key, std::wstring def)
{
	if(!HasProperty(key))return def;
	return mapEntry_[key];
}
int PropertyFile::GetInteger(std::wstring key, int def)
{
	if(!HasProperty(key))return def;
	std::wstring strValue = mapEntry_[key];
	return StringUtility::ToInteger(strValue);
}
double PropertyFile::GetReal(std::wstring key, double def)
{
	if(!HasProperty(key))return def;
	std::wstring strValue = mapEntry_[key];
	return StringUtility::ToDouble(strValue);
}

/**********************************************************
//Compressor
**********************************************************/
Compressor::Compressor()
{

}
Compressor::~Compressor()
{

	
}
bool Compressor::Compress(ByteBuffer& bufIn, ByteBuffer& bufOut)
{
	//TODO 要独自の圧縮を実装
	//公開ソースでは、受け渡されたデータをそのまま返す
	bool res = true;

	int inputSize = bufIn.GetSize();
	bufOut.SetSize(inputSize);
	memcpy(bufOut.GetPointer(0), bufIn.GetPointer(0), inputSize);

	return res;
}

/**********************************************************
//DeCompressor
**********************************************************/
DeCompressor::DeCompressor()
{

}
DeCompressor::~DeCompressor()
{

}

bool DeCompressor::DeCompress(ByteBuffer& bufIn, ByteBuffer& bufOut)
{
	//TODO 要独自の圧縮を実装
	//公開ソースでは、受け渡されたデータをそのまま返す
	bool res = true;

	int inputSize = bufIn.GetSize();
	bufOut.SetSize(inputSize);
	memcpy(bufOut.GetPointer(0), bufIn.GetPointer(0), inputSize);
	return res;
}

/**********************************************************
//SystemValueManager
**********************************************************/
const std::string SystemValueManager::RECORD_SYSTEM = "__RECORD_SYSTEM__";
const std::string SystemValueManager::RECORD_SYSTEM_GLOBAL = "__RECORD_SYSTEM_GLOBAL__";
SystemValueManager* SystemValueManager::thisBase_ = NULL;
SystemValueManager::SystemValueManager()
{
}
SystemValueManager::~SystemValueManager()
{
}
bool SystemValueManager::Initialize()
{
	if(thisBase_ != NULL)return false;

	mapRecord_[RECORD_SYSTEM] = new RecordBuffer();
	mapRecord_[RECORD_SYSTEM_GLOBAL] = new RecordBuffer();

	thisBase_ = this;
	return true;
}
void SystemValueManager::ClearRecordBuffer(std::string key)
{
	if(!IsExists(key))return;
	mapRecord_[key]->Clear();
}
bool SystemValueManager::IsExists(std::string key)
{
	return mapRecord_.find(key) != mapRecord_.end();
}
bool SystemValueManager::IsExists(std::string keyRecord, std::string keyValue)
{
	if(!IsExists(keyRecord))return false;
	gstd::ref_count_ptr<RecordBuffer> record = GetRecordBuffer(keyRecord);
	return record->IsExists(keyValue);
}
gstd::ref_count_ptr<RecordBuffer> SystemValueManager::GetRecordBuffer(std::string key)
{
	return IsExists(key) ? mapRecord_[key] : NULL;
}
