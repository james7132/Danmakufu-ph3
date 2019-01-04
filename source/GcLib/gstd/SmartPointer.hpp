#ifndef __GSTD_SMART_POINTER__
#define __GSTD_SMART_POINTER__

#include"GstdConstant.hpp"

//下記を参考
//http://marupeke296.com/CPP_SmartPointer.html

namespace gstd
{
	//================================================================
	//スマートポインタ排他
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
	static ref_count_ptr_lock REF_COUNT_PTR_LOCK;//排他オブジェクト
	*/


	//================================================================
	//スマートポインタ情報
	template <class T>
	struct ref_count_ptr_info
	{
		long *countRef_; // 参照カウンタへのポインタ
		long *countWeak_; // 参照カウンタへのポインタ
		T* pPtr_; // T型のオブジェクトのポインタ

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
			typedef ref_count_ptr<T, false> unsync;//排他なし版

		private:
			ref_count_ptr_info<T> info_;

			// 参照カウンタ増加
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

			// 参照カウンタ減少
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
			// デフォルトコンストラクタ
			//explicit 必要?
			ref_count_ptr(T* src, long add = 0)
			{
				SetPointer(src, add);
			}

			// コピーコンストラクタ
			ref_count_ptr(const ref_count_ptr<T, SYNC> &src)
			{
				// 相手のポインタをコピー
				info_ = src.info_;

				// 自分自身の参照カウンタを増加
				_AddRef();
			}

			// コピーコンストラクタ（暗黙的アップキャスト付き）
			template<class T2> ref_count_ptr(ref_count_ptr<T2, SYNC> &src)
			{
				// 相手のポインタをコピー
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// 自分自身の参照カウンタを増加
				_AddRef();
			}

			// デストラクタ
			~ref_count_ptr()
			{
				_Release();
			}

			// =代入演算子
			ref_count_ptr<T, SYNC>& operator =(T *src)
			{
				if(src == info_.pPtr_)
					return (*this);
				SetPointer(src);
				return (*this);
			}

			// =代入演算子
			ref_count_ptr<T, SYNC>& operator =(const ref_count_ptr<T, SYNC> &src)
			{
				// 自分自身への代入は不正で意味が無いので
				// 行わない。
				if(src.info_.pPtr_ == info_.pPtr_)
					return (*this);

				// 自分は他人になってしまうので
				// 参照カウンタを1つ減少
				_Release();

				// 相手のポインタをコピー
				info_ = src.info_;

				// 新しい自分自身の参照カウンタを増加
				_AddRef();

				return (*this);
			}

			// =代入演算子（明示的アップキャスト付き）
			template<class T2> ref_count_ptr& operator =(ref_count_ptr<T2, SYNC> &src)
			{
				// 自分自身への代入は不正で意味が無いので
				// 行わない。
				if(src.GetPointer() == info_.pPtr_)
					return (*this);

				// 自分は他人になってしまうので
				// 参照カウンタを1つ減少
				_Release();

				// 相手のポインタをコピー
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// 新しい自分自身の参照カウンタを増加
				_AddRef();

				return (*this);
			}

			// *間接演算子
			T& operator *(){ return *info_.pPtr_;}

			// ->メンバ選択演算子
			T* operator ->(){ return info_.pPtr_;}

			// []配列参照演算子
			T& operator [](int n){ return info_.pPtr_[n];}

			// ==比較演算子
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

			// !=比較演算子
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

			// ポインタの明示的な登録
			void SetPointer(T* src = NULL, long add = 0)
			{
				// 参照カウンタを減らした後に再初期化
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

			// ポインタの貸し出し
			inline T* GetPointer(){return info_.pPtr_;}

			// 参照カウンタへのポインタを取得
			inline long* _GetReferenceCountPointer(){return info_.countRef_;}//この関数は外部からしようしないこと
			inline long* _GetWeakCountPointer(){return info_.countWeak_;}//この関数は外部からしようしないこと
			int GetReferenceCount(){int res = info_.countRef_ != NULL ? (int)*info_.countRef_ : 0;return res;}

			template <class T2>
			static ref_count_ptr<T, SYNC> DownCast(ref_count_ptr<T2, SYNC> &src)
			{
				// 引数のスマートポインタが持つポインタが、
				// 自分の登録しているポインタに
				// ダウンキャスト可能な場合はオブジェクトを返す
				ref_count_ptr<T, SYNC> res;
				T* castPtr = dynamic_cast<T*>(src.GetPointer());
				if(castPtr != NULL)
				{
					// ダウンキャスト可能
					res._Release();//現在の参照を破棄する必要がある
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
			typedef ref_count_weak_ptr<T, false> unsync;//排他なし版

		private:
			ref_count_ptr_info<T> info_;

			// 参照カウンタ増加
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

			// 参照カウンタ減少
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
					throw std::exception("ref_count_weak_ptrコンストラクタに非NULLを代入しようとしました");
			}
			// コピーコンストラクタ
			ref_count_weak_ptr(const ref_count_weak_ptr<T, SYNC> &src)
			{
				// 相手のポインタをコピー
				info_ = src.info_;

				// 自分自身の参照カウンタを増加
				_AddRef();
			}

			// コピーコンストラクタ（暗黙的アップキャスト付き）
			template<class T2> ref_count_weak_ptr(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// 相手のポインタをコピー
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// 自分自身の参照カウンタを増加
				_AddRef();
			}

			template<class T2> ref_count_weak_ptr(ref_count_ptr<T2, SYNC> &src)
			{
				info_ = src.info_;
				_AddRef();
			}

			// デストラクタ
			~ref_count_weak_ptr()
			{
				_Release();
			}

			// =代入演算子
			ref_count_weak_ptr<T, SYNC>& operator =(T *src)
			{
				if(src != NULL)
					throw std::exception("ref_count_weak_ptr =に非NULLを代入しようとしました");
				_Release();
				info_.pPtr_ = src;
				info_.countRef_ = NULL;
				info_.countWeak_ = NULL;
				return (*this);
			}
			ref_count_weak_ptr<T, SYNC>& operator =(const ref_count_weak_ptr<T, SYNC> &src)
			{
				// 自分自身への代入は不正で意味が無いので
				// 行わない。
				if(src.info_.pPtr_ == info_.pPtr_)
					return (*this);

				// 自分は他人になってしまうので
				// 参照カウンタを1つ減少
				_Release();

				// 相手のポインタをコピー
				info_ = src.info_;

				// 新しい自分自身の参照カウンタを増加
				_AddRef();

				return (*this);
			}

			// =代入演算子
			ref_count_weak_ptr<T, SYNC>& operator =(const ref_count_ptr<T, SYNC> &src)
			{
				// 自分自身への代入は不正で意味が無いので
				// 行わない。
				if(src.info_.pPtr_== info_.pPtr_)
					return (*this);

				// 自分は他人になってしまうので
				// 参照カウンタを1つ減少
				_Release();

				// 相手のポインタをコピー
				info_.countRef_ = src.info_.countRef_;
				info_.countWeak_ = src.info_.countWeak_;
				info_.pPtr_ = src.info_.pPtr_;

				// 新しい自分自身の参照カウンタを増加
				_AddRef();

				return (*this);
			}

			// =代入演算子（明示的アップキャスト付き）
			template<class T2> ref_count_weak_ptr& operator =(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// 自分自身への代入は不正で意味が無いので
				// 行わない。
				if(src.GetPointer() == info_.pPtr_)
					return (*this);

				// 自分は他人になってしまうので
				// 参照カウンタを1つ減少
				_Release();

				// 相手のポインタをコピー
				info_.countRef_ = src._GetReferenceCountPointer();
				info_.countWeak_ = src._GetWeakCountPointer();
				info_.pPtr_ = src.GetPointer();

				// 新しい自分自身の参照カウンタを増加
				_AddRef();

				return (*this);
			}

			// *間接演算子
			T& operator *(){ return *info_.pPtr_;}

			// ->メンバ選択演算子
			T* operator ->(){ return info_.pPtr_;}

			// []配列参照演算子
			T& operator [](int n){ return info_.pPtr_[n];}

			// ==比較演算子
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

			// !=比較演算子
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

			// ポインタの貸し出し
			inline T* GetPointer(){return IsExists() ? info_.pPtr_ : NULL;}

			// 参照カウンタへのポインタを取得
			inline long* _GetReferenceCountPointer(){return info_.countRef_;}//この関数は外部からしようしないこと
			inline long* _GetWeakCountPointer(){return info_.countWeak_;}//この関数は外部からしようしないこと
			int GetReferenceCount(){int res = info_.countRef_ != NULL ? (int)*info_.countRef_ : 0;return res;}
			bool IsExists(){return info_.countRef_ != NULL ? (*info_.countRef_ > 0) : false;}

			template <class T2>
			static ref_count_weak_ptr<T, SYNC> DownCast(ref_count_weak_ptr<T2, SYNC> &src)
			{
				// 引数のスマートポインタが持つポインタが、
				// 自分の登録しているポインタに
				// ダウンキャスト可能な場合はオブジェクトを返す
				ref_count_weak_ptr<T, SYNC> res;
				T* castPtr = dynamic_cast<T*>(src.GetPointer());
				if(castPtr != NULL)
				{
					// ダウンキャスト可能
					res._Release();//現在の参照を破棄する必要がある
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
