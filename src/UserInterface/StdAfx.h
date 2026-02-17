#pragma once

#pragma warning(disable : 4702)
#pragma warning(disable : 4100)
#pragma warning(disable : 4201)
#pragma warning(disable : 4511)
#pragma warning(disable : 4663)
#pragma warning(disable : 4018)
#pragma warning(disable : 4245)
#pragma warning(disable : 4995)

#include "EterLib/StdAfx.h"
#include "EterPythonLib/StdAfx.h"
#include "GameLib/StdAfx.h"
#include "ScriptLib/StdAfx.h"
#include "AudioLib/StdAfx.h"
#include "EffectLib/StdAfx.h"
#include "PRTerrainLib/StdAfx.h"
#include "SpeedTreeLib/StdAfx.h"

#ifndef __D3DRM_H__
#define __D3DRM_H__
#endif

#include <dshow.h>
#include "Locale.h"
#include "GameType.h"

#define APP_NAME "Metin 2"

enum {
  POINT_MAX_NUM = 255,
  CHARACTER_NAME_MAX_LEN = 64,
  PLAYER_NAME_MAX_LEN = 12,
};

void initapp();
void initime();
void initsystem();
void initchr();
void initchrmgr();
void initChat();
void initTextTail();
void initime();
void initItem();
void initNonPlayer();
void initnet();
void initPlayer();
void initServerStateChecker();
void initTrade();
void initMiniMap();
void initProfiler();
void initEvent();
void initeffect();
void initsnd();
void initeventmgr();
void initBackground();
void initwndMgr();
void initshop();
void initpack();
void initskill();
void initfly();
void initquest();
void initsafebox();
void initguild();
void initMessenger();
