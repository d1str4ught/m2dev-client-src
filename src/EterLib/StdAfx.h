#pragma once

#define _WIN32_DCOM

#pragma warning(disable : 4710) // not inlined
#pragma warning(disable : 4786) // character 255 넘어가는거 끄기
#pragma warning(disable : 4244) // type conversion possible lose of data

#pragma warning(disable : 4018)
#pragma warning(disable : 4245)
#pragma warning(disable : 4512)
#pragma warning(disable : 4201)

#if _MSC_VER >= 1400
#pragma warning(disable : 4201 4512 4238 4239)
#endif

#include <d3d9.h>
#include <d3dx9.h>

// DX11 — Phase 1 migration (coexists with DX9 during transition)
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>


#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>

#pragma warning(disable : 4201)
#include <mmsystem.h>
#pragma warning(default : 4201)
#include <direct.h>
#include <malloc.h>
#include <math.h>
#include <process.h>
#include <stdio.h>
#include <time.h>


#include "EterBase/Debug.h"
#include "EterBase/StdAfx.h"
#include "EterLocale/CodePageId.h"


#include "UserInterface/Locale_inc.h"

#ifndef VC_EXTRALEAN
#include <winsock.h>
#endif
