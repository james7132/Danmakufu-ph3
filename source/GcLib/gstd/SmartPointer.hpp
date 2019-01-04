#ifndef __GSTD_SMART_POINTER__
#define __GSTD_SMART_POINTER__

#include"GstdConstant.hpp"

//���L���Q�l
//http://marupeke296.com/CPP_SmartPointer.html

namespace gstd
{
	//================================================================
	//�X�}�[�g�|�C���^�r��
	/*
	class ref_count_ptr_lock
	{
			CRITICAL_SECTION cs_;
		public:
			ref_count_ptr_lock()
			{
				::InitializeCriticalSection(&cs_);
			}
			~ref_count_ptr_lock()
			{
				::DeleteCriticalSection(&cs_);
			}
			inline void Enter()
			{
				::EnterCriticalSection(&cs_);
			}
			inline void Leave()
			{
				::LeaveCriticalSection(&cs_);
			}
	};
	static ref_count_ptr_lock REF_COUNT_PTR_LOCK;//�r���I�u�W�F�N�g
	*/


	//================================================================
	//�X�}�[�g�|�C���^���
	template <class T>
	struct ref_count_ptr_info
	{
		long *countRef_; // �Q�ƃJ�E���^�ւ̃|�C���^
		long *countWeak_; // �Q�ƃJ�E���^�ւ̃|�C���^
		T* pPtr_; // T�^�̃I�u�W�F�N�g�̃|�C���^

		ref_count_ptr_info()
		{
			countRef_ = NULL;
			countWeak_ = NULL;
			pPtr_ = NULL;
		}
	};


	//================================================================
	//ref_count_ptr
	//inline void operator delete(volatile void *p)
	//{ 
	//	operator delete((volatile void*)(p)); 
	//}
	template <class T, bool SYNC>
	class ref_count_weak_ptr;

	template <class T, bool SYNC = true>
	class ref_count_ptr
	{
		friend ref_count_weak_ptr<T, SYNC>;
		public:
			typedef ref_count_ptr<T, false> unsync;//�r���Ȃ���

		private:
			ref_count_ptr_info<T> info_;

			// �Q�ƃJ�E���^����
			void _AddRef()
			{
				if(info_.countRef_ == NULL)return;

				if(SYNC)
				{
					InterlockedIncrement(info_.countRef_);
					InterlockedIncrement(info_.countWeak_);
				}
				else
				{
					++(*info_.countRef_);
					++(*info_.countWeak_);
				}
			}

			// �Q�ƃJ�E���^����
			void _Release()
			{
				if(info_.countRef_ == NULL)return;

				if(SYNC)
				{
					if(InterlockedDecrement(info_.countRef_) == 0)
					{
						delete[] info_.pPtr_;info_.pPtr_=NULL;
					}
					if(InterlockedDecrement(info_.countWeak_) == 0)
					{
						delete info_.countRef_;info_.countRef_=NULL;
						delete info_.countWeak_;info_.countWeak_=NULL;
					}
				}
				else
				{
					if(--(*info_.countRef_) == 0)
					{
						delete[] info_.pPtr_;info_.pPtr_=NULL;
					}
					if(--(*info_.countWeak_) == 0)
					{
						delete info_.countRef_;info_.countRef_=NULL;
						delete info_.countWeak_;info_.countWeak_=NULL;
					}
				}
			}

		public:
			ref_count_ptr()
			{
				SetPointer(NULL);
			}
			// �f�t�H���g�R���X�g���N�^
			//explicit �K�v?
			ref_count_ptr(T* src, long add = 0)
			{
				SetPointer(src, add);
			}

			// �R�s�[�R���X�g���N�^
			ref_count_ptr(const ref_count_ptr<T, SYNC> &src)
			{
				// ����̃|�C���^���R�s�[
				info_ = src.info_;

				// �������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();
			}

			// �R�s�[�R���X�g���N�^�i�ÖٓI�A�b�v�L���X�g�t���j
			template<class T2> ref_count_ptr(ref_count_ptr<T2, SYNC> &src)
			{
				// ����̃|�C���^���R�s�[
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// �������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();
			}

			// �f�X�g���N�^
			~ref_count_ptr()
			{
				_Release();
			}

			// =������Z�q
			ref_count_ptr<T, SYNC>& operator =(T *src)
			{
				if(src == info_.pPtr_)
					return (*this);
				SetPointer(src);
				return (*this);
			}

			// =������Z�q
			ref_count_ptr<T, SYNC>& operator =(const ref_count_ptr<T, SYNC> &src)
			{
				// �������g�ւ̑���͕s���ňӖ��������̂�
				// �s��Ȃ��B
				if(src.info_.pPtr_ == info_.pPtr_)
					return (*this);

				// �����͑��l�ɂȂ��Ă��܂��̂�
				// �Q�ƃJ�E���^��1����
				_Release();

				// ����̃|�C���^���R�s�[
				info_ = src.info_;

				// �V�����������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();

				return (*this);
			}

			// =������Z�q�i�����I�A�b�v�L���X�g�t���j
			template<class T2> ref_count_ptr& operator =(ref_count_ptr<T2, SYNC> &src)
			{
				// �������g�ւ̑���͕s���ňӖ��������̂�
				// �s��Ȃ��B
				if(src.GetPointer() == info_.pPtr_)
					return (*this);

				// �����͑��l�ɂȂ��Ă��܂��̂�
				// �Q�ƃJ�E���^��1����
				_Release();

				// ����̃|�C���^���R�s�[
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// �V�����������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();

				return (*this);
			}

			// *�Ԑډ��Z�q
			T& operator *(){ return *info_.pPtr_;}

			// ->�����o�I�����Z�q
			T* operator ->(){ return info_.pPtr_;}

			// []�z��Q�Ɖ��Z�q
			T& operator [](int n){ return info_.pPtr_[n];}

			// ==��r���Z�q
			bool operator ==(const T* p)
			{
				return info_.pPtr_ == p;
			}
			bool operator ==(const ref_count_ptr<T, SYNC>& p)const
			{
				return info_.pPtr_ == p.info_.pPtr_;
			}
			template< class D >
			bool operator ==(ref_count_ptr<D, SYNC>& p)const
			{
				return info_.pPtr_ == p.GetPointer();
			}

			// !=��r���Z�q
			bool operator !=(const T* p)
			{
				return info_.pPtr_ != p;
			}
			bool operator !=(const ref_count_ptr<T, SYNC>& p)const
			{
				return info_.pPtr_ != p.info_.pPtr_;
			}
			template< class D >
			bool operator !=(ref_count_ptr<D, SYNC>& p)const
			{
				return info_.pPtr_ != p.info_.pPtr_;
			}

			// �|�C���^�̖����I�ȓo�^
			void SetPointer(T* src = NULL, long add = 0)
			{
				// �Q�ƃJ�E���^�����炵����ɍď�����
				_Release();
				if(src == NULL)
				{
					info_.countRef_ = NULL;
					info_.countWeak_ = NULL;
				}
				else 
				{
					info_.countRef_ = new long;
					*info_.countRef_ = add;
					info_.countWeak_ = new long;
					*info_.countWeak_ = add;
				}
				info_.pPtr_ = src;
				_AddRef();
			}

			// �|�C���^�݂̑��o��
			inline T* GetPointer(){return info_.pPtr_;}

			// �Q�ƃJ�E���^�ւ̃|�C���^���擾
			inline long* _GetReferenceCountPointer(){return info_.countRef_;}//���̊֐��͊O�����炵�悤���Ȃ�����
			inline long* _GetWeakCountPointer(){return info_.countWeak_;}//���̊֐��͊O�����炵�悤���Ȃ�����
			int GetReferenceCount(){int res = info_.countRef_ != NULL ? (int)*info_.countRef_ : 0;return res;}

			template <class T2>
			static ref_count_ptr<T, SYNC> DownCast(ref_count_ptr<T2, SYNC> &src)
			{
				// �����̃X�}�[�g�|�C���^�����|�C���^���A
				// �����̓o�^���Ă���|�C���^��
				// �_�E���L���X�g�\�ȏꍇ�̓I�u�W�F�N�g��Ԃ�
				ref_count_ptr<T, SYNC> res;
				T* castPtr = dynamic_cast<T*>(src.GetPointer());
				if(castPtr != NULL)
				{
					// �_�E���L���X�g�\
					res._Release();//���݂̎Q�Ƃ�j������K�v������
					res.info_.countRef_ = src._GetReferenceCountPointer();
					res.info_.countWeak_ = src._GetWeakCountPointer();
					res.info_.pPtr_ = castPtr;
					res._AddRef();
				}
				return res;
			}
	};

	//================================================================
	//ref_count_weak_ptr
	template <class T, bool SYNC = true>
	class ref_count_weak_ptr
	{
		public:
			typedef ref_count_weak_ptr<T, false> unsync;//�r���Ȃ���

		private:
			ref_count_ptr_info<T> info_;

			// �Q�ƃJ�E���^����
			void _AddRef()
			{
				if(info_.countRef_ == NULL)return;

				if(SYNC)
				{
					InterlockedIncrement(info_.countWeak_);
				}
				else
				{
					++(*info_.countWeak_);
				}
			}

			// �Q�ƃJ�E���^����
			void _Release()
			{
				if(info_.countRef_ == NULL)return;

				if(SYNC)
				{
					if(InterlockedDecrement(info_.countWeak_) == 0)
					{
						delete info_.countRef_;info_.countRef_=NULL;
						delete info_.countWeak_;info_.countWeak_=NULL;
					}
				}
				else
				{
					if(--(*info_.countWeak_) == 0)
					{
						delete info_.countRef_;info_.countRef_=NULL;
						delete info_.countWeak_;info_.countWeak_=NULL;
					}
				}
			}

		public:
			ref_count_weak_ptr()
			{
			}
			ref_count_weak_ptr(T* src)
			{
				if(src != NULL)
					throw std::exception("ref_count_weak_ptr�R���X�g���N�^�ɔ�NULL�������悤�Ƃ��܂���");
			}
			// �R�s�[�R���X�g���N�^
			ref_count_weak_ptr(const ref_count_weak_ptr<T, SYNC> &src)
			{
				// ����̃|�C���^���R�s�[
				info_ = src.info_;

				// �������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();
			}

			// �R�s�[�R���X�g���N�^�i�ÖٓI�A�b�v�L���X�g�t���j
			template<class T2> ref_count_weak_ptr(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// ����̃|�C���^���R�s�[
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// �������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();
			}

			template<class T2> ref_count_weak_ptr(ref_count_ptr<T2, SYNC> &src)
			{
				info_ = src.info_;
				_AddRef();
			}

			// �f�X�g���N�^
			~ref_count_weak_ptr()
			{
				_Release();
			}

			// =������Z�q
			ref_count_weak_ptr<T, SYNC>& operator =(T *src)
			{
				if(src != NULL)
					throw std::exception("ref_count_weak_ptr =�ɔ�NULL�������悤�Ƃ��܂���");
				_Release();
				info_.pPtr_ = src;
				info_.countRef_ = NULL;
				info_.countWeak_ = NULL;
				return (*this);
			}
			ref_count_weak_ptr<T, SYNC>& operator =(const ref_count_weak_ptr<T, SYNC> &src)
			{
				// �������g�ւ̑���͕s���ňӖ��������̂�
				// �s��Ȃ��B
				if(src.info_.pPtr_ == info_.pPtr_)
					return (*this);

				// �����͑��l�ɂȂ��Ă��܂��̂�
				// �Q�ƃJ�E���^��1����
				_Release();

				// ����̃|�C���^���R�s�[
				info_ = src.info_;

				// �V�����������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();

				return (*this);
			}

			// =������Z�q
			ref_count_weak_ptr<T, SYNC>& operator =(const ref_count_ptr<T, SYNC> &src)
			{
				// �������g�ւ̑���͕s���ňӖ��������̂�
				// �s��Ȃ��B
				if(src.info_.pPtr_== info_.pPtr_)
					return (*this);

				// �����͑��l�ɂȂ��Ă��܂��̂�
				// �Q�ƃJ�E���^��1����
				_Release();

				// ����̃|�C���^���R�s�[
				info_.countRef_ = src.info_.countRef_;
				info_.countWeak_ = src.info_.countWeak_;
				info_.pPtr_ = src.info_.pPtr_;

				// �V�����������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();

				return (*this);
			}

			// =������Z�q�i�����I�A�b�v�L���X�g�t���j
			template<class T2> ref_count_weak_ptr& operator =(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// �������g�ւ̑���͕s���ňӖ��������̂�
				// �s��Ȃ��B
				if(src.GetPointer() == info_.pPtr_)
					return (*this);

				// �����͑��l�ɂȂ��Ă��܂��̂�
				// �Q�ƃJ�E���^��1����
				_Release();

				// ����̃|�C���^���R�s�[
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// �V�����������g�̎Q�ƃJ�E���^�𑝉�
				_AddRef();

				return (*this);
			}

			// *�Ԑډ��Z�q
			T& operator *(){ return *info_.pPtr_;}

			// ->�����o�I�����Z�q
			T* operator ->(){ return info_.pPtr_;}

			// []�z��Q�Ɖ��Z�q
			T& operator [](int n){ return info_.pPtr_[n];}

			// ==��r���Z�q
			bool operator ==(const T* p)
			{
				return IsExists() ? (info_.pPtr_ == p) : (NULL == p);
			}
			bool operator ==(const ref_count_weak_ptr<T, SYNC>& p)const
			{
				return IsExists() ? (info_.pPtr_ == p.info_.pPtr_) : (NULL == p.info_.pPtr_);
			}
			template< class D >
			bool operator ==(ref_count_weak_ptr<D, SYNC>& p)const
			{
				return IsExists() ? (info_.pPtr_ == p.GetPointer() ) : (NULL == p.GetPointer());
			}

			// !=��r���Z�q
			bool operator !=(const T* p)
			{
				return IsExists() ? (info_.pPtr_ != p) : (NULL != p);
			}
			bool operator !=(const ref_count_weak_ptr<T, SYNC>& p)const
			{
				return IsExists() ? (info_.pPtr_ != p.info_.pPtr_) : (NULL != p.info_.pPtr_);
			}
			template< class D >
			bool operator !=(ref_count_weak_ptr<D, SYNC>& p)const
			{
				return IsExists() ? (info_.pPtr_ != p.GetPointer() ) : (NULL != p.GetPointer());
			}

			// �|�C���^�݂̑��o��
			inline T* GetPointer(){return IsExists() ? info_.pPtr_ : NULL;}

			// �Q�ƃJ�E���^�ւ̃|�C���^���擾
			inline long* _GetReferenceCountPointer(){return info_.countRef_;}//���̊֐��͊O�����炵�悤���Ȃ�����
			inline long* _GetWeakCountPointer(){return info_.countWeak_;}//���̊֐��͊O�����炵�悤���Ȃ�����
			int GetReferenceCount(){int res = info_.countRef_ != NULL ? (int)*info_.countRef_ : 0;return res;}
			bool IsExists(){return info_.countRef_ != NULL ? (*info_.countRef_ > 0) : false;}

			template <class T2>
			static ref_count_weak_ptr<T, SYNC> DownCast(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// �����̃X�}�[�g�|�C���^�����|�C���^���A
				// �����̓o�^���Ă���|�C���^��
				// �_�E���L���X�g�\�ȏꍇ�̓I�u�W�F�N�g��Ԃ�
				ref_count_weak_ptr<T, SYNC> res;
				T* castPtr = dynamic_cast<T*>(src.GetPointer());
				if(castPtr != NULL)
				{
					// �_�E���L���X�g�\
					res._Release();//���݂̎Q�Ƃ�j������K�v������
					res.info_.countRef_ = src._GetReferenceCountPointer();
					res.info_.countWeak_ = src._GetWeakCountPointer();
					res.info_.pPtr_ = castPtr;
					res._AddRef();
				}
				return res;
			}
	};
}

#endif
