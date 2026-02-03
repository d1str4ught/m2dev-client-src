#include "StdAfx.h"
#include "PythonApplication.h"
#include "ProcessScanner.h"
#include "PythonExceptionSender.h"
#include "resource.h"
#include "Version.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "eterLib/Util.h"
#include "EterLib/GameThreadPool.h"
#include "EterBase/lzo.h"

#include "PackLib/PackManager.h"

#include <filesystem>
#include <format>
#include <thread>
#include <atomic>

#include <stdlib.h>
#include <utf8.h>
#include <sodium.h>
#include "EterLib/FontManager.h"

extern "C" {
	extern int _fltused;
	FILE __iob_func[3] = { *stdin, *stdout, *stderr };
	volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
};

#pragma comment(linker, "/NODEFAULTLIB:libci.lib")

int Setup(LPSTR lpCmdLine);
static const char * sc_apszPythonLibraryFilenames[] =
{
	"UserDict.pyc",
	"__future__.pyc",
	"copy_reg.pyc",
	"linecache.pyc",
	"ntpath.pyc",
	"os.pyc",
	"site.pyc",
	"stat.pyc",
	"string.pyc",
	"traceback.pyc",
	"types.pyc",
	"\n",
};

bool CheckPythonLibraryFilenames()
{
	for (int i = 0; *sc_apszPythonLibraryFilenames[i] != '\n'; ++i)
	{
		std::string stFilenameUtf8 = "lib\\";
		stFilenameUtf8 += sc_apszPythonLibraryFilenames[i];

		std::wstring stFilenameW = Utf8ToWide(stFilenameUtf8);

		// Check existence
		if (GetFileAttributesW(stFilenameW.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			return false;
		}

		// Keep original behavior (forces Windows path normalization)
		MoveFileW(stFilenameW.c_str(), stFilenameW.c_str());
	}

	return true;
}

bool PackInitialize(const char * c_pszFolder)
{
	if (_access(c_pszFolder, 0) != 0)
		return false;

	std::vector<std::string> packFiles = {
		"patch1",
		"season3_eu",
		"patch2",
		"metin2_patch_snow",
		"metin2_patch_snow_dungeon",
		"metin2_patch_etc_costume1",
		"metin2_patch_pet1",
		"metin2_patch_pet2",
		"metin2_patch_ramadan_costume",
		"metin2_patch_flame",
		"metin2_patch_flame_dungeon",
		"metin2_patch_w21_etc",
		"metin2_patch_w21_mobs",
		"metin2_patch_w21_mobs_m",
		"metin2_patch_dss_box",
		"metin2_patch_costume_soccer",
		"metin2_patch_easter1",
		"metin2_patch_mineral",
		"metin2_patch_w20_sound",
		"metin2_patch_ds",
		"metin2_patch_5th_armor",
		"metin2_patch_w20_etc",
		"metin2_patch_dragon_rock",
		"metin2_patch_dragon_rock_mobs",
		"metin2_patch_dragon_rock_texcache",
		"metin2_patch_dragon_rock_mobs_texcache",
		"metin2_patch_etc",
		"metin2_patch_xmas",
		"metin2_patch_eu3",
		"metin2_patch_eu4",
		"metin2_patch_mundi",
		"metin2_patch_sd",
		"metin2_patch_halloween",
		"metin2_patch_party",
		"metin2_patch_dance",
		"pc",
		"pc2",
		"monster",
		"monster2",
		"effect",
		"zone",
		"terrain",
		"npc",
		"npc2",
		"tree",
		"guild",
		"item",
		"textureset",
		"property",
		"icon",
		"season1",
		"season2",
		"outdoora1",
		"outdoora2",
		"outdoora3",
		"outdoorb1",
		"outdoorb3",
		"outdoorc1",
		"outdoorc3",
		"outdoorsnow1",
		"outdoordesert1",
		"outdoorflame1",
		"outdoorfielddungeon1",
		"outdoort1",
		"outdoort2",
		"outdoort3",
		"outdoort4",
		"outdoorwedding",
		"outdoormilgyo1",
		"indoorspiderdungeon1",
		"indoordeviltower1",
		"indoormonkeydungeon1",
		"indoormonkeydungeon2",
		"indoormonkeydungeon3",
		"outdoortrent",
		"outdoortrent02",
		"outdoorguild1",
		"outdoorguild2",
		"outdoorguild3",
		"outdoorduel",
		"outdoorgmguildbuild",
		"sound",
		"sound_m",
		"sound2",
		// "bgm", // BGM files are loaded directly from disk, not from pack
		"locale",
		"uiscript",
		"ETC",
		"uiloading",
	};

	Tracef("PackInitialize: Loading root.pck...");
	if (!CPackManager::instance().AddPack(std::format("{}/root.pck", c_pszFolder)))
	{
		TraceError("Failed to load root.pck");
		return false;
	}

	Tracef("PackInitialize: Loading %d pack files in parallel...", packFiles.size());
	const size_t numThreads = std::min<size_t>(std::thread::hardware_concurrency(), packFiles.size());
	const size_t packsPerThread = (packFiles.size() + numThreads - 1) / numThreads;

	std::vector<std::thread> threads;
	threads.reserve(numThreads);  // Pre-allocate to prevent reallocation
	std::atomic<size_t> failedCount(0);

	// Create all threads first (prevents vector reallocation during emplace_back)
	for (size_t t = 0; t < numThreads; ++t)
	{
		size_t start = t * packsPerThread;
		size_t end = std::min(start + packsPerThread, packFiles.size());

		threads.emplace_back([&failedCount, &packFiles, c_pszFolder, start, end]() {
			for (size_t i = start; i < end; ++i)
			{
				std::string packPath = std::format("{}/{}.pck", c_pszFolder, packFiles[i]);
				if (!CPackManager::instance().AddPack(packPath))
				{
					TraceError("Failed to load %s", packPath.c_str());
					failedCount++;
				}
			}
		});
	}

	// Wait for all threads to complete
	for (auto& thread : threads)
	{
		thread.join();
	}

	Tracef("PackInitialize: Completed! Failed: %d / %d", failedCount.load(), packFiles.size());
	return true;
}

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();

	initudp();
	initapp();
	initsystem();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
	std::string stRegisterDebugFlag;

	#ifdef _DISTRIBUTE
	stRegisterDebugFlag = "__DEBUG__ = 0";
	#else
	stRegisterDebugFlag = "__DEBUG__ = 1";
	#endif

	if (!pyLauncher.RunLine(stRegisterDebugFlag.c_str()))
	{
		TraceError("RegisterDebugFlag Error");
		return false;
	}

	if (!pyLauncher.RunFile("system.py"))
	{
		TraceError("RunMain Error");
		return false;
	}

	return true;
}

static bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
	DWORD dwRandSeed = (DWORD)time(NULL) ^ GetCurrentProcessId() ^ GetTickCount();
	srandom(dwRandSeed);
	srand(random());
	SetLogLevel(1);

	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#else
	OpenLogFile(false); // false == uses syserr.txt only
#endif

	if (sodium_init() < 0)
	{
		LogBox("sodium_init() failed");
		return false;
	}

	if (!CFontManager::Instance().Initialize())
	{
		LogBox("FreeType initialization failed");
		return false;
	}

	static CLZO lzo;
	CPackManager packMgr;

	if (!PackInitialize("pack"))
	{
		LogBox("Pack Initialization failed. Check log.txt file..");
		return false;
	}

	// Create game thread pool singleton before CPythonApplication
	static CGameThreadPool gameThreadPool;

	auto app = new CPythonApplication;
	app->Initialize (hInstance);
	CPythonLauncher pyLauncher;

	if (pyLauncher.Create())
	{
		RunMainScript (pyLauncher, lpCmdLine);
	}

	app->Clear();
	timeEndPeriod (1);
	pyLauncher.Clear();

	app->Destroy();
	delete app;

	CFontManager::Instance().Destroy();
	return 0;
}

void __ErrorPythonLibraryIsNotExist()
{
	LogBoxf("FATAL ERROR!! Python Library file not exist!");
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LoadConfig("config/locale.cfg");

	int nArgc = 0;
	auto szArgv = CommandLineToArgv (lpCmdLine, &nArgc);

	if (!CheckPythonLibraryFilenames())
	{
		__ErrorPythonLibraryIsNotExist();
		goto Clean;
	}

	Main (hInstance, lpCmdLine);
	::CoUninitialize();

Clean:
	SAFE_FREE_GLOBAL (szArgv);
	return 0;
}

static void GrannyError(granny_log_message_type Type, granny_log_message_origin Origin, char const* File, granny_int32x Line, char const* Message, void* UserData)
{
	TraceError("GRANNY: %s", Message);
}

int Setup(LPSTR lpCmdLine)
{
	TIMECAPS tc;
	UINT wTimerRes;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 
		return 0;

	wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax); 
	timeBeginPeriod(wTimerRes); 

	granny_log_callback Callback;
	Callback.Function = nullptr;
	Callback.UserData = 0;
	GrannySetLogCallback(&Callback);
	return 1;
}
