#pragma once

#pragma warning(disable : 4710) // not inlined
#pragma warning(disable : 4786) // character 255 넘어가는거 끄기
#pragma warning(disable : 4244) // type conversion possible lose of data

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#pragma warning(disable : 4201)
#include <mmsystem.h>
#pragma warning(default : 4201)
#include <imagehlp.h>
#include <time.h>

#pragma warning(push, 3)

#include <algorithm>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>

#pragma warning(pop)

#if _MSC_VER >= 1400
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strupt _strupr
#define strcmpi _strcmpi
#define fileno _fileno
// #define access _access_s
// #define _access _access_s
#define atoi _atoi64
#endif

// Armadillo nanomite protection
#ifndef NANOBEGIN
#ifdef __BORLANDC__
#define NANOBEGIN
#define NANOEND
#else
#define NANOBEGIN
#define NANOEND
#endif
#endif

#include "vk.h"
#include "filename.h"

#include "../UserInterface/Locale_inc.h"
