#ifndef __GSTD_MERSENNETWISTER__
#define __GSTD_MERSENNETWISTER__

#include"GstdConstant.hpp"

namespace gstd
{
	/**********************************************************
	//MersenneTwister
	//Mersenne Twisterは、松本眞 ・西村拓士（アルファベット順）
	//により1996年から1997年に渡って開発された
	//疑似乱数生成アルゴリズムです。
	// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/what-is-mt.html
	**********************************************************/
	class MersenneTwister
	{
			struct mt_struct;
			friend struct mt_struct;
			enum
			{
				MT_N = 624,
				MT_M = 397,
			};

			typedef struct mt_struct
			{
				unsigned long mt[MT_N]; 
				int mti;
				virtual ~mt_struct(){}
			} mt_struct;
			mt_struct mts;
			int seed_;
			unsigned long _GenrandInt32();

		public:
			MersenneTwister();
			MersenneTwister(unsigned long s);
			virtual ~MersenneTwister(){}
			void Initialize(unsigned long s);
			void Initialize(unsigned long *init_key, int key_length);
			
			int GetSeed(){return seed_;}
			long GetInt();
			long GetInt(long min, long max);
			_int64 GetInt64();
			_int64 GetInt64(_int64 min, _int64 max);
			long double GetReal();
			long double GetReal(long double min, long double max);
	};
}

#endif
