#ifndef __DIRECTX_DIRECTSOUND__
#define __DIRECTX_DIRECTSOUND__

#include"DxConstant.hpp"

namespace directx
{
	class DirectSoundManager;
	class SoundInfoPanel;
	class SoundInfo;
	class SoundDivision;
	class SoundPlayer;
	class SoundStreamingPlayer;

	class SoundPlayerWave;
	class SoundStreamingPlayerWave;
	class SoundStreamingPlayerMp3;
	class SoundStreamingPlayerOgg;

	class AcmBase;
	class AcmMp3;
	class AcmMp3Wave;


	struct WAVEFILEHEADER
	{//WAVE�\���t�H�[�}�b�g���A"fmt "�`�����N�f�[�^
		char cRIFF[4];
		int	iSizeRIFF;
		char cType[4];
		char cFmt[4];
		int	iSizeFmt;
		WAVEFORMATEX WaveFmt;
		char cData[4];
		int	iSizeData;
	};

	/**********************************************************
	//DirectSoundManager
	**********************************************************/
	class DirectSoundManager
	{
		public:
			class SoundManageThread;
			friend SoundManageThread;
			friend SoundInfoPanel;

		public:
			enum
			{
				SD_VOLUME_MIN=DSBVOLUME_MIN,
				SD_VOLUME_MAX=DSBVOLUME_MAX,
			};
			enum FileFormat
			{
				SD_MIDI,
				SD_WAVE,
				SD_MP3,
				SD_OGG,
				SD_AWAVE,//���kwave wave�w�b�_mp3
				SD_UNKNOWN,
			};

		private:
			static DirectSoundManager* thisBase_;

		protected:
			IDirectSound8* pDirectSound_;
			IDirectSoundBuffer8* pDirectSoundBuffer_;
			gstd::CriticalSection lock_;
			SoundManageThread* threadManage_;
			std::map<std::wstring, std::list<gstd::ref_count_ptr<SoundPlayer> > > mapPlayer_;
			std::map<int, gstd::ref_count_ptr<SoundDivision> > mapDivision_;
			std::map<std::wstring, gstd::ref_count_ptr<SoundInfo> > mapInfo_;
			gstd::ref_count_ptr<SoundInfoPanel> panelInfo_;

			gstd::ref_count_ptr<SoundPlayer> _GetPlayer(std::wstring path);
			gstd::ref_count_ptr<SoundPlayer> _CreatePlayer(std::wstring path);
		public:
			DirectSoundManager();
			virtual ~DirectSoundManager();
			static DirectSoundManager* GetBase(){return thisBase_;}
			virtual bool Initialize(HWND hWnd);
			void Clear();
			
			IDirectSound8* GetDirectSound(){return pDirectSound_;}
			gstd::CriticalSection& GetLock(){return lock_;}

			gstd::ref_count_ptr<SoundPlayer> GetPlayer(std::wstring path, bool bCreateAlways = false);
			gstd::ref_count_ptr<SoundDivision> CreateSoundDivision(int index);
			gstd::ref_count_ptr<SoundDivision> GetSoundDivision(int index);
			gstd::ref_count_ptr<SoundInfo> GetSoundInfo(std::wstring path);

			void SetInfoPanel(gstd::ref_count_ptr<SoundInfoPanel> panel){gstd::Lock lock(lock_);panelInfo_ = panel;}

			bool AddSoundInfoFromFile(std::wstring path);
			std::vector<gstd::ref_count_ptr<SoundInfo> > GetSoundInfoList();
			void SetFadeDeleteAll();
	};

	//�t�F�[�h�C���^�t�F�[�h�A�E�g����
	//�K�v�Ȃ��Ȃ����o�b�t�@�̊J���Ȃ�
	class DirectSoundManager::SoundManageThread : public gstd::Thread, public gstd::InnerClass<DirectSoundManager>
	{
		friend DirectSoundManager;
		protected:
			int timeCurrent_;
			int timePrevious_;

			SoundManageThread(DirectSoundManager* manager);
			void _Run();
			void _Arrange();//�K�v�Ȃ��Ȃ����f�[�^���폜
			void _Fade();//�t�F�[�h����
	};

	/**********************************************************
	//SoundInfoPanel
	**********************************************************/
	class SoundInfoPanel : public gstd::WindowLogger::Panel
	{
		protected:
			struct Info
			{
				int address;
				std::wstring path;
				int countRef;
			};
			enum
			{
					ROW_ADDRESS,
					ROW_FILENAME,
					ROW_FULLPATH,
					ROW_COUNT_REFFRENCE,
			};
			gstd::WListView wndListView_;
			int timeLastUpdate_;
			int timeUpdateInterval_;

			virtual bool _AddedLogger(HWND hTab);
		public:
			SoundInfoPanel();
			void SetUpdateInterval(int time){timeUpdateInterval_=time;}
			virtual void LocateParts();
			virtual void Update(DirectSoundManager* soundManager);
	};

	/**********************************************************
	//SoundDivision
	//���ʂȂǂ����L���邽�߂̃N���X
	**********************************************************/
	class SoundDivision
	{
		public:
			enum
			{
				DIVISION_BGM = 0,
				DIVISION_SE,
				DIVISION_VOICE,
			};
		protected:
			double rateVolume_;//���ʊ���(0-100)
		public:
			SoundDivision();
			virtual ~SoundDivision();
			void SetVolumeRate(double rate){rateVolume_ = rate;}
			double GetVolumeRate(){return rateVolume_;}
	};

	/**********************************************************
	//SoundInfo
	**********************************************************/
	class SoundInfo
	{
		friend DirectSoundManager;
		private:
			std::wstring name_;
			std::wstring title_;
			double timeLoopStart_;
			double timeLoopEnd_;
		public:
			SoundInfo(){timeLoopStart_ = 0; timeLoopEnd_ = 0;}
			virtual ~SoundInfo(){};
			std::wstring GetName(){return name_;}
			std::wstring GetTitle(){return title_;}
			double GetLoopStartTime(){return timeLoopStart_;}
			double GetLoopEndTime(){return timeLoopEnd_;}
	};

	/**********************************************************
	//SoundPlayer
	**********************************************************/
	class SoundPlayer
	{
		friend DirectSoundManager;
		friend DirectSoundManager::SoundManageThread;
		public:
			class PlayStyle;
			enum
			{
				FADE_DEFAULT = 20,
			};
		protected:
			DirectSoundManager* manager_;
			std::wstring path_;
			gstd::CriticalSection lock_;
			IDirectSoundBuffer8* pDirectSoundBuffer_;
			gstd::ref_count_ptr<gstd::FileReader> reader_;
			gstd::ref_count_ptr<SoundDivision> division_;

			WAVEFORMATEX formatWave_;
			bool bLoop_;//���[�v�L��
			double timeLoopStart_;//���[�v�J�n����
			double timeLoopEnd_;//���[�v�I������
			bool bPause_;

			bool bDelete_;//�폜�t���O
			bool bFadeDelete_;//�t�F�[�h�A�E�g��ɍ폜
			bool bAutoDelete_;//�����폜
			double rateVolume_;//���ʊ���(0-100)
			double rateVolumeFadePerSec_;//�t�F�[�h���̉��ʒቺ����

			virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader) = 0;
			virtual void _SetSoundInfo();
			static int _GetValumeAsDirectSoundDecibel(float rate);
		public:
			SoundPlayer();
			virtual ~SoundPlayer();
			std::wstring GetPath(){return path_;}
			gstd::CriticalSection& GetLock(){return lock_;}
			virtual void Restore(){pDirectSoundBuffer_->Restore();}
			void SetSoundDivision(gstd::ref_count_ptr<SoundDivision> div);
			void SetSoundDivision(int index);

			virtual bool Play();
			virtual bool Play(PlayStyle& style);
			virtual bool Stop();
			virtual bool IsPlaying();
			virtual bool Seek(double time) = 0;
			virtual bool SetVolumeRate(double rateVolume);
			bool SetPanRate(double ratePan);
			double GetVolumeRate();
			void SetFade(double rateVolumeFadePerSec);
			void SetFadeDelete(double rateVolumeFadePerSec);
			void SetAutoDelete(bool bAuto = true){bAutoDelete_ = bAuto;}
			double GetFadeVolumeRate();
			void Delete(){bDelete_ = true;}
			WAVEFORMATEX GetWaveFormat(){return formatWave_;}
	};
	class SoundPlayer::PlayStyle
	{
			bool bLoop_;
			double timeLoopStart_;
			double timeLoopEnd_;
			double timeStart_;
			bool bRestart_;
		public:
			PlayStyle();
			virtual ~PlayStyle();
			void SetLoopEnable(bool bLoop){bLoop_ = bLoop;}
			bool IsLoopEnable(){return bLoop_;}
			void SetLoopStartTime(double time){timeLoopStart_ = time;}
			double GetLoopStartTime(){return timeLoopStart_;}
			void SetLoopEndTime(double time){timeLoopEnd_ = time;}
			double GetLoopEndTime(){return timeLoopEnd_;}
			void SetStartTime(double time){timeStart_ = time;}
			double GetStartTime(){return timeStart_;}
			bool IsRestart(){return bRestart_;}
			void SetRestart(bool b){bRestart_ = b;}
	};

	/**********************************************************
	//SoundStreamPlayer
	**********************************************************/
	class SoundStreamingPlayer : public SoundPlayer
	{
		class StreamingThread;
		friend StreamingThread;
		protected:
			HANDLE hEvent_[3];
			IDirectSoundNotify* pDirectSoundNotify_;//�C�x���g
			int sizeCopy_;
			StreamingThread* thread_;
			bool bStreaming_;
			bool bRequestStop_;//���[�v�������̃t���O�B������~����ƍŌ�̃o�b�t�@���Đ�����Ȃ����߁B

			void _CreateSoundEvent(WAVEFORMATEX& formatWave);
			virtual void _CopyStream(int indexCopy);
			virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize) = 0;
			void _RequestStop(){bRequestStop_ = true;}

		public:
			SoundStreamingPlayer();
			virtual ~SoundStreamingPlayer();

			virtual bool Play(PlayStyle& style);
			virtual bool Stop();
			virtual bool IsPlaying();
	};
	class SoundStreamingPlayer::StreamingThread : public gstd::Thread , public gstd::InnerClass<SoundStreamingPlayer>
	{
		protected:
			virtual void _Run();
		public:
			StreamingThread(SoundStreamingPlayer* player){_SetOuter(player);}
	};

	/**********************************************************
	//SoundPlayerWave
	**********************************************************/
	class SoundPlayerWave : public SoundPlayer
	{
		protected:
			virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
		public:
			SoundPlayerWave();
			virtual ~SoundPlayerWave();

			virtual bool Play(PlayStyle& style);
			virtual bool Stop();
			virtual bool IsPlaying();
			virtual bool Seek(double time);
	};

	/**********************************************************
	//SoundStreamingPlayerWave
	**********************************************************/
	class SoundStreamingPlayerWave : public SoundStreamingPlayer
	{
		protected:
			int posWaveStart_;
			int posWaveEnd_;
			virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
			virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);
		public:
			SoundStreamingPlayerWave();
			virtual bool Seek(double time);
	};

	/**********************************************************
	//SoundStreamingPlayerOgg
	**********************************************************/
	class SoundStreamingPlayerOgg : public SoundStreamingPlayer
	{
		protected:
			OggVorbis_File fileOgg_;
			ov_callbacks oggCallBacks_;

			virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
			virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);

			static size_t _ReadOgg(void* ptr, size_t size, size_t nmemb, void* source);
			static int _SeekOgg( void* source, ogg_int64_t offset, int whence);
			static int _CloseOgg(void* source);
			static long _TellOgg(void* source);

		public:
			SoundStreamingPlayerOgg();
			~SoundStreamingPlayerOgg();
			virtual bool Seek(double time);
	};

	/**********************************************************
	//SoundStreamingPlayerMp3
	**********************************************************/
	class SoundStreamingPlayerMp3 : public SoundStreamingPlayer
	{
		protected:
			MPEGLAYER3WAVEFORMAT formatMp3_;
			WAVEFORMATEX formatWave_;
			HACMSTREAM hAcmStream_;
			ACMSTREAMHEADER acmStreamHeader_;
			int posMp3DataStart_;
			int posMp3DataEnd_;
			DWORD waveDataSize_;
			double timeCurrent_;
			gstd::ref_count_ptr<gstd::ByteBuffer> bufDecode_;

			virtual bool _CreateBuffer(gstd::ref_count_ptr<gstd::FileReader> reader);
			virtual void _CopyBuffer(LPVOID pMem, DWORD dwSize);
			int _ReadAcmStream(char* pBuffer, int size);
		public:
			SoundStreamingPlayerMp3();
			~SoundStreamingPlayerMp3();
			virtual bool Seek(double time);
	};
}

#endif
