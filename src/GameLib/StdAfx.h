#ifndef __INC_YMIR_GAMELIB__
#define __INC_YMIR_GAMELIB__

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

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

#include "EterBase/Utils.h"
#include "EterBase/CRC32.h"
#include "EterBase/Random.h"

#include "EterLib/StdAfx.h"
#include "AudioLib/StdAfx.h"
#include "EffectLib/StdAfx.h"

#include "UserInterface/Locale_inc.h"

#include "GameType.h"
#include "GameUtil.h"
#include "MapType.h"
#include "MapUtil.h"
#include "Interface.h"

// #include "FlyingObjectManager.h"
// #include "FlyingData.h"
// #include "FlyingInstance.h"

// Octree
// #include "Octree.h"

// Item
// #include "ItemData.h"
// #include "ItemManager.h"

// Actor
// #include "WeaponTrace.h"
// #include "PhysicsObject.h"
// #include "RaceMotionData.h"
// #include "RaceData.h"
// #include "ActorInstance.h"
// #include "RaceManager.h"

// Property
// #include "Property.h"
// #include "PropertyManager.h"
// #include "PropertyLoader.h"

// Map
// #include "Area.h"

// Path Finder
// #include "PathFinder.h"

// Game Event Manager
// #include "GameEventManager.h"

#endif