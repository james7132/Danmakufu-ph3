#ifndef __DIRECTX_DXCONSTANT__
#define __DIRECTX_DXCONSTANT__

#include"../gstd/GstdLib.hpp"

//lib
#pragma comment(lib,"msacm32.lib") //for acm
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"d3dxof.lib")
#pragma comment(lib,"dxerr9.lib")

//define
#define D3D_OVERLOADS
#define DIRECTINPUT_VERSION 0x0800
#define DIRECTSOUND_VERSION 0x0900

#define DWORD_PTR DWORD*

#ifdef _DEBUG
#undef new
#endif

//include

#include<mmreg.h> //for acm
#include<msacm.h> //for acm

#include<basetsd.h>
#include<d3d9.h>
#include<d3dx9.h>
#include<dinput.h>
#include<dsound.h>
#include<dmusici.h>
#include<dxerr9.h>

#include "../ext/codec.h"
#include "../ext/vorbisfile.h"

#if defined(UNICODE) || defined(_UNICODE)
#pragma comment(linker, "/entry:\"wWinMainCRTStartup\"")
#endif


#ifdef _DEBUG
#include <cstdlib>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#endif


