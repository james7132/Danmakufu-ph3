#ifndef __GSTD_FILE__
#define __GSTD_FILE__

#include"GstdConstant.hpp"
#include"GstdUtility.hpp"
#include"Thread.hpp"

namespace gstd
{
	const std::string HEADER_ARCHIVEFILE = "ArchiveFile";
	const std::string HEADER_RECORDFILE = "RecordBufferFile";

	class ByteBuffer;
	class FileManager;
	/**********************************************************
	//Writer
	**********************************************************/
	class Writer
	{
		public:
			virtual ~Writer(){};
			virtual DWORD Write(LPVOID buf,DWORD size) = 0;
			template <typename T> DWORD Write(T& data)
			{
				return Write(&data, sizeof(T));
			}
			void WriteBoolean(bool b){Write(b);}
			void WriteCharacter(char ch){Write(ch);}
			void WriteShort(short num){Write(num);}
			void WriteInteger(int num){Write(num);}
			void WriteInteger64(_int64 num){Write(num);}
			void WriteFloat(float num){Write(num);}
			void WriteDouble(double num){Write(num);}
	};

	/**********************************************************
	//Reader
	**********************************************************/
	class Reader
	{
		public:
			virtual ~Reader(){};
			virtual DWORD Read(LPVOID buf,DWORD size) = 0;
			template <typename T> DWORD Read(T& data)
			{
				return Read(&data, sizeof(T));
			}
			bool ReadBoolean(){bool res; Read(res);return res;}
			char ReadCharacter(){char res; Read(res);return res;}
			short ReadShort(){short res; Read(res);return res;}
			int ReadInteger(){int num; Read(num);return num;}
			_int64 ReadInteger64(){_int64 num; Read(num);return num;}
			float ReadFloat(){float num; Read(num);return num;}
			double ReadDouble(){double num; Read(num);return num;}

			std::string ReadString(int size)
			{
				std::string res = "";
				res.resize(size);
				Read(&res[0], size);
				return res;
			}
	};

	/**********************************************************
	//FileReader
	**********************************************************/
	class FileReader : public Reader
	{
		friend FileManager;
		protected:
			std::wstring pathOriginal_;
			void _SetOriginalPath(std::wstring path){pathOriginal_ = path;}

		public:
			virtual bool Open() = 0;
			virtual void Close() = 0;
			virtual int GetFileSize() = 0;
			virtual BOOL SetFilePointerBegin() = 0;
			virtual BOOL SetFilePointerEnd() = 0;
			virtual BOOL Seek(LONG offset) = 0;
			virtual LONG GetFilePointer() = 0;
			virtual bool IsArchived(){return false;}
			virtual bool IsCompressed(){return false;}

			std::wstring GetOriginalPath(){return pathOriginal_;}
			std::string ReadAllString()
			{
				SetFilePointerBegin();
				return ReadString(GetFileSize());
			}
	};

	/**********************************************************
	//ByteBuffer
	**********************************************************/
	class ByteBuffer : public Writer, public Reader
	{
		protected:
			int reserve_;
			int size_;
			int offset_;
			char* data_;

			int _GetReservedSize();
			void _Resize(int size);
		public:
			ByteBuffer();
			ByteBuffer(ByteBuffer& buffer);
			virtual ~ByteBuffer();
			void Copy(ByteBuffer& src);
			void Clear();

			void Seek(int pos);
			void SetSize(int size);		
			int GetSize(){return size_;}
			int GetOffset(){return offset_;}

			virtual DWORD Write(LPVOID buf,DWORD size);
			virtual DWORD Read(LPVOID buf,DWORD size);

			char* GetPointer(int offset = 0);
	};

	/**********************************************************
	//File
	//�t�@�C���́Ax:\fffff.xxx
	//�f�B���N�g����x:\ddddd\
	**********************************************************/
	class File : public Writer, public Reader
	{
		public:
			enum AccessType
			{
				READ,
				WRITE,
			};
		protected:
			HANDLE hFile_;
			std::wstring path_;

		public:
			File();
			File(std::wstring path);
			virtual ~File();
			bool CreateDirectory();
			void Delete();
			bool IsExists();
			static bool IsExists(std::wstring path);
			bool IsDirectory();

			int GetSize();
			std::wstring& GetPath(){return path_;}
			HANDLE GetHandle(){return hFile_;}

			virtual bool Open();
			bool Open(AccessType typeAccess);
			bool Create();
			void Close();

			virtual DWORD Write(LPVOID buf,DWORD size);
			virtual DWORD Read(LPVOID buf,DWORD size);

			BOOL SetFilePointerBegin(){return (::SetFilePointer(hFile_,0,NULL,FILE_BEGIN)!=0xFFFFFFFF);}
			BOOL SetFilePointerEnd(){return (::SetFilePointer(hFile_,0,NULL,FILE_END)!=0xFFFFFFFF);}
			BOOL Seek(LONG offset, DWORD seek=FILE_BEGIN){return (::SetFilePointer(hFile_,offset,NULL,seek) != 0xFFFFFFFF);}
			LONG GetFilePointer(){return ::SetFilePointer(hFile_, 0, NULL, FILE_CURRENT);}

			static bool IsEqualsPath(std::wstring path1, std::wstring path2);
			static std::vector<std::wstring> GetFilePathList(std::wstring dir);
			static std::vector<std::wstring> GetDirectoryPathList(std::wstring dir);
	};

	/**********************************************************
	//ArchiveFileEntry
	**********************************************************/
	class FileArchiver;
	class ArchiveFile;

	class ArchiveFileEntry
	{
		friend FileArchiver;
		friend ArchiveFile;
		public:
			enum CompressType
			{
				CT_NON,
				CT_COMPRESS,
			};

		private:
			std::wstring dir_;
			std::wstring name_;
			CompressType typeCompress_;
			int sizeData_;
			int sizeCompressed_;
			int offset_;
			std::wstring pathArchive_;

			int _GetEntryRecordSize();
			void _WriteEntryRecord(ByteBuffer &buf);
			void _ReadEntryRecord(ByteBuffer &buf);

			void _SetOffset(int offset){offset_ = offset;}
			void _SetDataSize(int size){ sizeData_ = size;}
			void _SetCompressedDataSize(int size){sizeCompressed_ = size;}
			void _SetArchivePath(std::wstring path){pathArchive_ = path;}
			
		public:
			ArchiveFileEntry();
			virtual ~ArchiveFileEntry();

			void SetDirectory(std::wstring dir){dir_ = dir;}
			std::wstring& GetDirectory(){return dir_;}
			void SetName(std::wstring name){name_ = name;}
			std::wstring& GetName(){return name_;}
			void SetCompressType(CompressType type){typeCompress_ = type;}
			CompressType& GetCompressType(){return typeCompress_;}
			
			int GetOffset(){return offset_;}
			int GetDataSize(){return sizeData_;}
			int GetCompressedDataSize(){return sizeCompressed_;}

			std::wstring& GetArchivePath(){return pathArchive_;}
	};

	/**********************************************************
	//FileArchiver
	**********************************************************/
	class FileArchiver
	{
		private:
			std::list<ref_count_ptr<ArchiveFileEntry> > listEntry_;

		public:
			FileArchiver();
			virtual ~FileArchiver();

			void AddEntry(ref_count_ptr<ArchiveFileEntry> entry){listEntry_.push_back(entry);}
			bool CreateArchiveFile(std::wstring path);
	};

	/**********************************************************
	//ArchiveFile
	**********************************************************/
	class ArchiveFile
	{
		private:
			ref_count_ptr<File> file_;
			std::multimap<std::wstring, ref_count_ptr<ArchiveFileEntry> > mapEntry_;

		public:
			ArchiveFile(std::wstring path);
			virtual ~ArchiveFile();
			bool Open();
			void Close();

			std::set<std::wstring> GetKeyList();
			std::multimap<std::wstring, ref_count_ptr<ArchiveFileEntry> > GetEntryMap(){return mapEntry_;}
			std::vector<ref_count_ptr<ArchiveFileEntry> > GetEntryList(std::wstring name);
			bool IsExists(std::wstring name);
			static ref_count_ptr<ByteBuffer> CreateEntryBuffer(ref_count_ptr<ArchiveFileEntry> entry);
			//ref_count_ptr<ByteBuffer> GetBuffer(std::string name);
	};

	/**********************************************************
	//FileManager
	**********************************************************/
	class ManagedFileReader;
	class FileManager
	{
		public:
			class LoadObject;
			class LoadThread;
			class LoadThreadListener;
			class LoadThreadEvent;
			friend ManagedFileReader;
		private:
			static FileManager* thisBase_;
		protected:
			gstd::CriticalSection lock_;
			gstd::ref_count_ptr<LoadThread> threadLoad_;
			std::map<std::wstring, ref_count_ptr<ArchiveFile> > mapArchiveFile_;
			std::map<std::wstring, ref_count_ptr<ByteBuffer> > mapByteBuffer_;

			ref_count_ptr<ByteBuffer> _GetByteBuffer(ref_count_ptr<ArchiveFileEntry> entry);
			void _ReleaseByteBuffer(ref_count_ptr<ArchiveFileEntry> entry);
		public:
			static FileManager* GetBase(){return thisBase_;}
			FileManager();
			virtual ~FileManager();
			virtual bool Initialize();
			void EndLoadThread();
			bool AddArchiveFile(std::wstring path);
			bool RemoveArchiveFile(std::wstring path);
			ref_count_ptr<FileReader> GetFileReader(std::wstring path);

			void AddLoadThreadEvent(ref_count_ptr<LoadThreadEvent> event);
			void AddLoadThreadListener(FileManager::LoadThreadListener* listener);
			void RemoveLoadThreadListener(FileManager::LoadThreadListener* listener);
			void WaitForThreadLoadComplete();
	};

	class FileManager::LoadObject
	{
		public:
			virtual ~LoadObject(){};
	};

	class FileManager::LoadThread : public Thread
	{
		gstd::CriticalSection lockEvent_;
		gstd::CriticalSection lockListener_;
		gstd::ThreadSignal signal_;
		protected:
			virtual void _Run();
			//std::set<std::string> listPath_;
			std::list<ref_count_ptr<FileManager::LoadThreadEvent> > listEvent_;
			std::list<FileManager::LoadThreadListener*> listListener_;
		public:
			LoadThread();
			virtual ~LoadThread();
			virtual void Stop();
			bool IsThreadLoadComplete();
			bool IsThreadLoadExists(std::wstring path);
			void AddEvent(ref_count_ptr<FileManager::LoadThreadEvent> event);
			void AddListener(FileManager::LoadThreadListener* listener);
			void RemoveListener(FileManager::LoadThreadListener* listener);
	};

	class FileManager::LoadThreadListener
	{
		public:
			virtual ~LoadThreadListener(){}
			virtual void CallFromLoadThread(ref_count_ptr<FileManager::LoadThreadEvent> event) = 0;
	};

	class FileManager::LoadThreadEvent
	{
		protected:
			FileManager::LoadThreadListener* listener_;
			std::wstring path_;
			ref_count_ptr<FileManager::LoadObject> source_;
		public:
			LoadThreadEvent(FileManager::LoadThreadListener* listener, std::wstring path, ref_count_ptr<FileManager::LoadObject> source)
			{
				listener_ = listener;
				path_ = path;
				source_ = source;
			};
			virtual ~LoadThreadEvent(){}

			FileManager::LoadThreadListener* GetListener(){return listener_;}
			std::wstring GetPath(){return path_;}
			ref_count_ptr<FileManager::LoadObject> GetSource(){return source_;}
	};

	/**********************************************************
	//ManagedFileReader
	**********************************************************/
	class ManagedFileReader : public FileReader
	{
		private:
			enum FILETYPE
			{
				TYPE_NORMAL,
				TYPE_ARCHIVED,
				TYPE_ARCHIVED_COMPRESSED,
			};

			FILETYPE type_;
			ref_count_ptr<File> file_;
			ref_count_ptr<ArchiveFileEntry> entry_;
			ref_count_ptr<ByteBuffer> buffer_;
			int offset_;

		public:
			ManagedFileReader(ref_count_ptr<File> file, ref_count_ptr<ArchiveFileEntry> entry);
			~ManagedFileReader();

			virtual bool Open();
			virtual void Close();
			virtual int GetFileSize();
			virtual DWORD Read(LPVOID buf,DWORD size);
			virtual BOOL SetFilePointerBegin();
			virtual BOOL SetFilePointerEnd();
			virtual BOOL Seek(LONG offset);
			virtual LONG GetFilePointer();

			virtual bool IsArchived();
			virtual bool IsCompressed();
	};


	/**********************************************************
	//Recordable
	**********************************************************/
	class Recordable;
	class RecordEntry;
	class RecordBuffer;
	class Recordable
	{
		public:
			virtual ~Recordable(){}
			virtual void Read(RecordBuffer& record) = 0;
			virtual void Write(RecordBuffer& record) = 0;
	};

	/**********************************************************
	//RecordEntry
	**********************************************************/
	class RecordEntry
	{
		friend RecordBuffer;
		public:
			enum
			{
				TYPE_NOENTRY = -2,
				TYPE_UNKNOWN = -1,
				TYPE_BOOLEAN = 1,
				TYPE_INTEGER = 2,
				TYPE_FLOAT = 3,
				TYPE_DOUBLE = 4,
				TYPE_STRING_A = 5,
				TYPE_RECORD = 6,
				TYPE_STRING_W = 7,
			};

		private:
			char type_;
			std::string key_;
			ByteBuffer buffer_;

			int _GetEntryRecordSize();
			void _WriteEntryRecord(Writer &writer);
			void _ReadEntryRecord(Reader &reader);
		public:
			RecordEntry();
			virtual ~RecordEntry();
			char GetType(){return type_;}

			void SetKey(std::string key){key_ = key;}
			void SetType(char type){type_ = type;}
			std::string& GetKey(){return key_;}
			ByteBuffer& GetBufferRef(){return buffer_;}
	};

	/**********************************************************
	//RecordBuffer
	**********************************************************/
	class RecordBuffer : public Recordable
	{
		private:
			std::map<std::string, ref_count_ptr<RecordEntry> > mapEntry_;

		public:
			RecordBuffer();
			virtual ~RecordBuffer();
			void Clear();//�ێ��f�[�^�N���A
			int GetEntryCount(){return mapEntry_.size();}
			bool IsExists(std::string key);
			std::vector<std::string> GetKeyList();
			ref_count_ptr<RecordEntry> GetEntry(std::string key);

			void Write(Writer& writer);
			void Read(Reader& reader);
			bool WriteToFile(std::wstring path, std::string header = HEADER_RECORDFILE);
			bool ReadFromFile(std::wstring path, std::string header = HEADER_RECORDFILE);

			//�G���g��
			int GetEntryType(std::string key);
			int GetEntrySize(std::string key);

			//�G���g���擾(������L�[)
			bool GetRecord(std::string key, LPVOID buf, DWORD size);
			template <typename T> bool GetRecord(std::string key, T& data)
			{
				return GetRecord(key, &data, sizeof(T));
			}
			bool GetRecordAsBoolean(std::string key);
			int GetRecordAsInteger(std::string key);
			float GetRecordAsFloat(std::string key);
			double GetRecordAsDouble(std::string key);
			std::string GetRecordAsStringA(std::string key);
			std::wstring GetRecordAsStringW(std::string key);
			bool GetRecordAsRecordBuffer(std::string key, RecordBuffer& record);

			//�G���g���擾(���l�L�[)
			bool GetRecord(int key, LPVOID buf, DWORD size){return GetRecord(StringUtility::Format("%d",key), buf, size);}
			template <typename T> bool GetRecord(int key, T& data){return GetRecord(StringUtility::Format("%d",key), data);}
			bool GetRecordAsBoolean(int key){return GetRecordAsBoolean(StringUtility::Format("%d",key));};
			int GetRecordAsInteger(int key){return GetRecordAsInteger(StringUtility::Format("%d",key));}
			float GetRecordAsFloat(int key){return GetRecordAsFloat(StringUtility::Format("%d",key));}
			double GetRecordAsDouble(int key){return GetRecordAsDouble(StringUtility::Format("%d",key));}
			std::string GetRecordAsStringA(int key){return GetRecordAsStringA(StringUtility::Format("%d",key));}
			std::wstring GetRecordAsStringW(int key){return GetRecordAsStringW(StringUtility::Format("%d",key));}
			bool GetRecordAsRecordBuffer(int key, RecordBuffer& record){return GetRecordAsRecordBuffer(StringUtility::Format("%d",key), record);}


			//�G���g���ݒ�(������L�[)
			void SetRecord(std::string key, LPVOID buf, DWORD size){SetRecord(RecordEntry::TYPE_UNKNOWN, key, buf, size);}
			template <typename T> void SetRecord(std::string key, T& data)
			{
				SetRecord(RecordEntry::TYPE_UNKNOWN, key, &data, sizeof(T));
			}
			void SetRecord(int type, std::string key, LPVOID buf, DWORD size);
			template <typename T> void SetRecord(int type, std::string key, T& data)
			{
				SetRecord(type, key, &data, sizeof(T));
			}
			void SetRecordAsBoolean(std::string key, bool data){SetRecord(RecordEntry::TYPE_BOOLEAN, key, data);}
			void SetRecordAsInteger(std::string key, int data){SetRecord(RecordEntry::TYPE_INTEGER, key, data);}
			void SetRecordAsFloat(std::string key, float data){SetRecord(RecordEntry::TYPE_FLOAT, key, data);}
			void SetRecordAsDouble(std::string key, double data){SetRecord(RecordEntry::TYPE_DOUBLE, key, data);}
			void SetRecordAsStringA(std::string key, std::string data){SetRecord(RecordEntry::TYPE_STRING_A, key, &data[0], data.size());}
			void SetRecordAsStringW(std::string key, std::wstring data){SetRecord(RecordEntry::TYPE_STRING_W, key, &data[0], data.size() * sizeof(wchar_t));}
			void SetRecordAsRecordBuffer(std::string key, RecordBuffer& record);

			//�G���g���ݒ�(���l�L�[)
			void SetRecord(int key, LPVOID buf, DWORD size){SetRecord(StringUtility::Format("%d",key), buf, size);}
			template <typename T> void SetRecord(int key, T& data){SetRecord(StringUtility::Format("%d",key), data);}
			void SetRecordAsBoolean(int key, bool data){SetRecordAsInteger(StringUtility::Format("%d",key), data);}
			void SetRecordAsInteger(int key, int data){SetRecordAsInteger(StringUtility::Format("%d",key), data);}
			void SetRecordAsFloat(int key, float data){SetRecordAsFloat(StringUtility::Format("%d",key), data);}
			void SetRecordAsDouble(int key, double data){SetRecordAsDouble(StringUtility::Format("%d",key), data);}
			void SetRecordAsStringA(int key, std::string data){SetRecordAsStringA(StringUtility::Format("%d",key), data);}
			void SetRecordAsStringW(int key, std::wstring data){SetRecordAsStringW(StringUtility::Format("%d",key), data);}
			void SetRecordAsRecordBuffer(int key, RecordBuffer& record){SetRecordAsRecordBuffer(StringUtility::Format("%d",key), record);}

			//Recoedable
			virtual void Read(RecordBuffer& record);
			virtual void Write(RecordBuffer& record);
	};

	/**********************************************************
	//PropertyFile
	**********************************************************/
	class PropertyFile
	{
		protected:
			std::map<std::wstring, std::wstring> mapEntry_;
		public:
			PropertyFile();
			virtual ~PropertyFile();

			bool Load(std::wstring path);

			bool HasProperty(std::wstring key);
			std::wstring GetString(std::wstring key){return GetString(key, L"");}
			std::wstring GetString(std::wstring key, std::wstring def);
			int GetInteger(std::wstring key){return GetInteger(key, 0);}
			int GetInteger(std::wstring key, int def);
			double GetReal(std::wstring key){return GetReal(key, 0.0);}
			double GetReal(std::wstring key, double def);
	};

	/**********************************************************
	//Compressor
	**********************************************************/
	class Compressor
	{
		protected:

		public:
			Compressor();
			virtual ~Compressor();
			bool Compress(ByteBuffer& bufIn, ByteBuffer& bufOut);
	};

	/**********************************************************
	//DeCompressor
	**********************************************************/
	class DeCompressor
	{
		protected:

		public:
			DeCompressor();
			virtual ~DeCompressor();
			bool DeCompress(ByteBuffer& bufIn, ByteBuffer& bufOut);
	};

	/**********************************************************
	//SystemValueManager
	**********************************************************/
	class SystemValueManager
	{
		public:
			const static std::string RECORD_SYSTEM;
			const static std::string RECORD_SYSTEM_GLOBAL;
		private:
			static SystemValueManager* thisBase_;

		protected:
			std::map<std::string, gstd::ref_count_ptr<RecordBuffer> > mapRecord_;

		public:
			SystemValueManager();
			virtual ~SystemValueManager();
			static SystemValueManager* GetBase(){return thisBase_;}
			virtual bool Initialize();

			virtual void ClearRecordBuffer(std::string key);
			bool IsExists(std::string key);
			bool IsExists(std::string keyRecord, std::string keyValue);
			gstd::ref_count_ptr<RecordBuffer> GetRecordBuffer(std::string key);
			
	};
}


#endif
