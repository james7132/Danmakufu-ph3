#ifndef __GSTD_GAMESTDCONSTANT__
#define __GSTD_GAMESTDCONSTANT__

//Unicode
#ifdef _MBCS
#undef _MBCS
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif


//�W���֐��Ή��\
//http://www1.kokusaika.jp/advisory/org/ja/win32_unicode.html

//Win2000�ȍ~
#define _WIN32_WINNT 0x0500

//lib
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"pdh.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"psapi.lib")

//pragma
#pragma warning (disable:4786)//STL Warning�}�~
#pragma warning (disable:4018)//signed �� unsigned �̐��l���r
#pragma warning (disable:4244)//double' ���� 'float' �ɕϊ�
#pragma warning (disable:4503)//

#pragma warning (disable:4302)// �؂�l�߂܂��B
#pragma warning (disable:4305)// 'double' ���� 'FLOAT' �֐؂�l�߂܂��B
#pragma warning (disable:4819)//�t�@�C���́A���݂̃R�[�h �y�[�W (932) �ŕ\���ł��Ȃ��������܂�ł��܂��B�f�[�^�̑�����h�����߂ɁA�t�@�C���� Unicode �`���ŕۑ����Ă��������B
#pragma warning (disable:4996)//This function or variable may be unsafe. 


//define
#ifndef STRICT
#define STRICT 1
#endif

//std
#include<cwchar>
#include<cstdlib>

#include<exception>
#include<cstdlib>
#include<cmath>
#include<string>
#include<list>
#include<vector>
#include<set>
#include<map>
#include<bitset>
#include<memory>
#include<algorithm>

//Windows
#include<windows.h>
#include<windowsx.h>
#include<mmsystem.h>
#include<commctrl.h>
#include<pdh.h>
#include<process.h>
#include<wingdi.h>
#include<shlwapi.h>

#include<mlang.h>
#include<psapi.h>



#ifdef _DEBUG
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


#endif
