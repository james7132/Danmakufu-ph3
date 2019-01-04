#include"MersenneTwister.hpp"

using namespace gstd;

/**********************************************************
//MersenneTwister
//Mersenne Twisterは、松本眞 ・西村拓士（アルファベット順）
//により1996年から1997年に渡って開発された
//疑似乱数生成アルゴリズムです。
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/what-is-mt.html
**********************************************************/
typedef unsigned long cardinal;
typedef long double real;
const cardinal MATRIX_A = 0x9908b0dfUL;
const cardinal UPPER_MASK = 0x80000000UL;
const cardinal LOWER_MASK = 0x7fffffffUL;

#define FIX32(value) value // 32bitの型が無い環境では value & 0xffffffffUL とか
static cardinal const mag01[2]={0x0UL, MATRIX_A};

MersenneTwister::MersenneTwister()
{
	this->Initialize(0);
}
MersenneTwister::MersenneTwister(unsigned long s)
{
	this->Initialize(s);
}
unsigned long MersenneTwister::_GenrandInt32()
{
	cardinal result;

	if (mts.mti >= MT_N) 
	{
		int kk;
		cardinal y;

#if 0
		if (mts.mti == MT_N+1) 
			init_genrand(mt, 5489UL);
#endif

		for(kk = 0; kk < MT_N - MT_M; ++kk) {
			y = (mts.mt[kk] & UPPER_MASK) | (mts.mt[kk + 1] & LOWER_MASK);
			mts.mt[kk] = mts.mt[kk + MT_M] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}
		for(; kk < MT_N - 1; ++kk) {
			y = (mts.mt[kk] & UPPER_MASK) | (mts.mt[kk + 1] & LOWER_MASK);
			mts.mt[kk] = mts.mt[kk + (MT_M - MT_N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
		}

		y = (mts.mt[MT_N - 1] & UPPER_MASK) | (mts.mt[0] & LOWER_MASK);
		mts.mt[MT_N-1] = mts.mt[MT_M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

		mts.mti = 0;
	}

	result = mts.mt[mts.mti];
	++(mts.mti);

	result ^= (result >> 11);
	result ^= (result << 7) & 0x9d2c5680UL;
	result ^= (result << 15) & 0xefc60000UL;
	result ^= (result >> 18);

	return result;
}

void MersenneTwister::Initialize(unsigned long s)
{
	int i;
	mts.mt[0] = FIX32(s);
	for(i = 1; i < MT_N; ++i) 
	{
		mts.mt[i] = FIX32(1812433253UL * (mts.mt[i - 1] ^ (mts.mt[i - 1] >> 30)) + i); 
	}
	mts.mti = i;
	seed_ = s;
}
void MersenneTwister::Initialize(unsigned long *init_key, int key_length)
{
	int i, j, k;
	Initialize(19650218UL);
	i = 1; 
	j = 0;
	for(k = (MT_N > key_length) ? MT_N : key_length; k; --k) 
	{
		mts.mt[i] = FIX32(
			(mts.mt[i] ^ ((mts.mt[i-1] ^ (mts.mt[i-1] >> 30)) * 1664525UL)) + init_key[j] + j /* non linear */
		); 
		++i; 
		if(i >= MT_N) 
		{ 
			mts.mt[0] = mts.mt[MT_N-1]; 
			i=1; 
		}
		++j;
		if(j >= key_length)
		{
			j=0;
		}
	}
	for(k = MT_N - 1; k; --k) 
	{
		mts.mt[i] = FIX32(
			(mts.mt[i] ^ ((mts.mt[i-1] ^ (mts.mt[i-1] >> 30)) * 1566083941UL)) - i 
		); 
		++i;
		if(i >= MT_N)
		{ 
			mts.mt[0] = mts.mt[MT_N - 1]; 
			i = 1; 
		}
	}
	mts.mt[0] = 0x80000000UL; 
}

long MersenneTwister::GetInt()
{
	return (long)(_GenrandInt32()>> 1);
}
long MersenneTwister::GetInt(long min, long max)
{
	return (int)this->GetReal(min, max);
}
_int64 MersenneTwister::GetInt64()
{
	return (_int64)this->GetReal();
}
_int64 MersenneTwister::GetInt64(_int64 min, _int64 max)
{
	return (_int64)this->GetReal(min, max);
}
long double MersenneTwister::GetReal()
{
	return _GenrandInt32() * (1.0 / 4294967295.0);
}
long double MersenneTwister::GetReal(long double min, long double max)
{
	if(min==max)return min;
	if(min > max)
	{
		long double t = min;
		min=max;
		max=t;
	}
	real r=GetReal();
	r=r*(max-min)+min;
	return r;
}