#include "StdAfx.h"
#include "eterBase/Error.h"
#include "eterlib/Camera.h"
#include "eterlib/AttributeInstance.h"
#include "gamelib/AreaTerrain.h"
#include "EterGrnLib/Material.h"

#include "resource.h"
#include "PythonApplication.h"
#include "PythonCharacterManager.h"

#include "ProcessScanner.h"

#include <utf8.h>

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
m_bCursorVisible(TRUE),
m_bLiarCursorOn(false),
m_iCursorMode(CURSOR_MODE_HARDWARE),
m_isWindowed(false),
m_isFrameSkipDisable(false),
m_poMouseHandler(NULL),
m_dwUpdateFPS(0),
m_dwRenderFPS(0),
m_fAveRenderTime(0.0f),
m_dwFaceCount(0),
m_fGlobalTime(0.0f),
m_fGlobalElapsedTime(0.0f),
m_dwLButtonDownTime(0),
m_dwLastIdleTime(0),
m_IsMovingMainWindow(false)
{
#ifndef _DEBUG
	SetEterExceptionHandler();
#endif

	CTimer::Instance().UseCustomTime();
	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd=0.0f;

	m_dwFaceAccCount=0;
	m_dwFaceAccTime=0;

	m_dwFaceSpdSum=0;
	m_dwFaceSpdCount=0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);

	m_InitialMouseMovingPoint = {};
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);	
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);			
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

void CPythonApplication::RenderGame()
{
	float fAspect = m_kWndMgr.GetAspect();
	float fFarClip = m_pyBackground.GetFarClip();

	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);

	CCullingManager::Instance().Process();

	m_kChrMgr.Deform();

	m_pyBackground.RenderCharacterShadowToTexture();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	{
		long lx, ly;
		m_kWndMgr.GetMousePosition(lx, ly);
		m_pyGraphic.SetCursorPosition(lx, ly);
	}

	m_pyBackground.RenderSky();

	m_pyBackground.RenderBeforeLensFlare();

	m_pyBackground.RenderCloud();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	m_kChrMgr.Render();

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	m_pyBackground.RenderSnow();
	m_pyBackground.RenderEffect();

	m_pyBackground.EndEnvironment();

	m_kEftMgr.Render();
	m_pyItem.Render();
	m_FlyingManager.Render();

	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();

	m_pyBackground.RenderAfterLensFlare();
}

void CPythonApplication::UpdateGame()
{
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);


	//!@# Alt+Tab Áß SetTransfor ¿¡¼­ Æ¨±è Çö»ó ÇØ°áÀ» À§ÇØ - [levites]
	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		s.SetPerspective(30.0f,fAspect, 100.0f, fFarClip);
		s.BuildViewFrustum();
	}

	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();

	m_kChrMgr.Update();

	m_kEftMgr.Update();
	m_kEftMgr.UpdateSound();
	
	m_FlyingManager.Update();
	m_pyItem.Update(ptMouse);
	m_pyPlayer.Update();

	// NOTE : Update µ¿¾È À§Ä¡ °ªÀÌ ¹Ù²î¹Ç·Î ´Ù½Ã ¾ò¾î ¿É´Ï´Ù - [levites]
	//        ÀÌ ºÎºÐ ¶§¹®¿¡ ¸ÞÀÎ ÄÉ¸¯ÅÍÀÇ Sound°¡ ÀÌÀü À§Ä¡¿¡¼­ ÇÃ·¹ÀÌ µÇ´Â Çö»óÀÌ ÀÖ¾úÀ½.
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
}

bool CPythonApplication::Process()
{
	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000) [[unlikely]] {
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / std::max(1ul, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	// Update Time
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;	//17 - 1ÃÊ´ç 60fps±âÁØ.

	DWORD updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O	
	m_pyNetworkStream.Process();	
	//m_pyNetworkDatagram.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime4=ELTimer_GetMSec();
#endif
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point)) [[likely]] {
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);		
	}
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime5=ELTimer_GetMSec();
#endif
	//!@# Alt+Tab Áß SetTransfor ¿¡¼­ Æ¨±è Çö»ó ÇØ°áÀ» À§ÇØ - [levites]
	//if (m_isActivateWnd)
	__UpdateCamera();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime7=ELTimer_GetMSec();
#endif
	OnCameraUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime8=ELTimer_GetMSec();
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime9=ELTimer_GetMSec();
#endif
	OnUIUpdate();

#ifdef __PERFORMANCE_CHECK__		
	DWORD dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{			
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %d (Time %d)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %d\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %d\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %d\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %d\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %d\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %d\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %d\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %d\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %d\n", dwUpdateTime10-dwUpdateTime9);			
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}		
#endif

	//UpdateÇÏ´Âµ¥ °É¸°½Ã°£.delta°ª
	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = dwCurrentTime - s_uiNextFrameTime;
		int nAdjustTime = ((float)dt / (float)uiFrameTime) * uiFrameTime; 

		if ( dt >= 500 )
		{
			s_uiNextFrameTime += nAdjustTime; 
			printf("FrameSkip º¸Á¤ %d\n",nAdjustTime);
			CTimer::Instance().Adjust(nAdjustTime);
		}

		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	//s_bFrameSkip = false;

	//if (dwCurrentTime > s_uiNextFrameTime)
	//{
	//	int dt = dwCurrentTime - s_uiNextFrameTime;

	//	//³Ê¹« ´Ê¾úÀ» °æ¿ì µû¶óÀâ´Â´Ù.
	//	//±×¸®°í m_dwCurUpdateTime´Â deltaÀÎµ¥ delta¶û absolute timeÀÌ¶û ºñ±³ÇÏ¸é ¾îÂ¼ÀÚ´Â°Ü?
	//	//if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)

	//	//±âÁ¸ÄÚµå´ë·Î ÇÏ¸é 0.5ÃÊ ÀÌÇÏ Â÷ÀÌ³­ »óÅÂ·Î update°¡ Áö¼ÓµÇ¸é °è¼Ó rendering frame skip¹ß»ý
	//	if (dt >= 500 || m_dwCurUpdateTime > s_uiNextFrameTime)
	//	{
	//		s_uiNextFrameTime += dt / uiFrameTime * uiFrameTime; 
	//		printf("FrameSkip º¸Á¤ %d\n", dt / uiFrameTime * uiFrameTime);
	//		CTimer::Instance().Adjust((dt / uiFrameTime) * uiFrameTime);
	//		s_bFrameSkip = true;
	//	}
	//}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;

#ifdef __VTUNE__
	s_bFrameSkip = false;
#endif
	if (!s_bFrameSkip)
	{
		//		static double pos=0.0f;
		//		CGrannyMaterial::TranslateSpecularMatrix(fabs(sin(pos)*0.005), fabs(cos(pos)*0.005), 0.0f);
		//		pos+=0.01f;

		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();		

		bool canRender = true;

		if (m_isMinimizedWnd) [[unlikely]] {
			canRender = false;
		}
		else [[likely]] {
			if (m_pyGraphic.IsLostDevice()) [[unlikely]] {
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();

				if (m_pyGraphic.RestoreDevice())					
					rkBG.CreateCharacterShadowTexture();
				else
					canRender = false;				
			}
		}

		if (canRender) [[likely]]
		{
			// RestoreLostDevice
			CCullingManager::Instance().Update();
			if (m_pyGraphic.Begin()) [[likely]] {

				m_pyGraphic.ClearDepthBuffer();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

				OnUIRender();
				OnMouseRender();
				/////////////////////

				m_pyGraphic.End();

				//DWORD t1 = ELTimer_GetMSec();
				m_pyGraphic.Show();
				//DWORD t2 = ELTimer_GetMSec();

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;			
				s_dwRenderRangeTime += m_dwCurRenderTime;				
				++s_dwRenderRangeFrame;			

				if (dwRenderEndTime-s_dwRenderCheckTime>1000) [[unlikely]] {
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}										

				DWORD dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					m_fFaceSpd=(m_dwFaceAccCount/m_dwFaceAccTime);

					// °Å¸® ÀÚµ¿ Á¶Àý
					if (-1 == m_iForceSightRange)
					{
						static float s_fAveRenderTime = 16.0f;
						float fRatio=0.3f;
						s_fAveRenderTime=(s_fAveRenderTime*(100.0f-fRatio)+std::max(16.0f, (float)m_dwCurRenderTime)*fRatio)/100.0f;


						float fFar=25600.0f;
						float fNear=MIN_FOG;
						double dbAvePow=double(1000.0f/s_fAveRenderTime);
						double dbMaxPow=60.0;
						float fDistance=std::max((float)(fNear+(fFar-fNear)*(dbAvePow)/dbMaxPow), fNear);
						m_pyBackground.SetViewDistanceSet(0, fDistance);
					}
					// °Å¸® °­Á¦ ¼³Á¤½Ã
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					// 10000 Æú¸®°ï º¸´Ù ÀûÀ»¶§´Â °¡Àå ¸Ö¸® º¸ÀÌ°Ô ÇÑ´Ù
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	int rest = s_uiNextFrameTime - ELTimer_GetMSec();

	if (rest > 0 && !bCurrentLateUpdate )
	{
		s_uiLoad -= rest;	// ½® ½Ã°£Àº ·Îµå¿¡¼­ »«´Ù..
		Sleep(rest);
	}	

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	//m_Profiler.ProfileByScreen();	
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{	
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
		// µð¹ÙÀÌ½º°¡ ¾øÀ¸¸é ÇÁ·Î±×·¥ÀÌ Á¾·á µÇ¾î¾ß ÇÑ´Ù.
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

		// DEVICESTATE_BROKENÀÏ ¶§´Â ´ÙÀ½ ·çÇÁ¿¡¼­ º¹±¸ µÉ ¼ö ÀÖµµ·Ï ¸®ÅÏ ÇÑ´Ù.
		// ±×³É ÁøÇàÇÒ °æ¿ì DrawPrimitive °°Àº °ÍÀ» ÇÏ¸é ÇÁ·Î±×·¥ÀÌ ÅÍÁø´Ù.
	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		if (!m_grpDevice.Reset())
			return DEVICE_STATE_SKIP;

		break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit, int frequency)
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit, frequency);

	switch (iRet)
	{
		case CGraphicDevice::CREATE_OK:
			return true;

		case CGraphicDevice::CREATE_REFRESHRATE:
			return true;

		case CGraphicDevice::CREATE_ENUM:
		case CGraphicDevice::CREATE_DETECT:
			SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
			TraceError("CreateDevice: Enum & Detect failed");
			return false;

		case CGraphicDevice::CREATE_NO_DIRECTX:
			SET_EXCEPTION(CREATE_NO_DIRECTX);
			TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
			return false;

		case CGraphicDevice::CREATE_DEVICE:
			SET_EXCEPTION(CREATE_DEVICE);
			TraceError("CreateDevice: GraphicDevice create failed");
			return false;

		case CGraphicDevice::CREATE_FORMAT:
			SET_EXCEPTION(CREATE_FORMAT);
			TraceError("CreateDevice: Change the screen format");
			return false;

		case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
			PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
			TraceError("CreateDevice: GetDevCaps failed");
			return false;

		case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
			PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
			TraceError("CreateDevice: GetDevCaps2 failed");
			return false;

		default:
			if (iRet & CGraphicDevice::CREATE_OK)
			{
				if (iRet & CGraphicDevice::CREATE_NO_TNL)
				{
					CGrannyLODController::SetMinLODMode(true);
				}
				return true;
			}

			SET_EXCEPTION(UNKNOWN_ERROR);
			TraceError("CreateDevice: Unknown Error!");
			return false;
	}
}

void CPythonApplication::SetUserMovingMainWindow(bool flag)
{
	if (flag && !GetCursorPos(&m_InitialMouseMovingPoint))
		return;

	m_IsMovingMainWindow = flag;
}

bool CPythonApplication::IsUserMovingMainWindow() const
{
	return m_IsMovingMainWindow;
}

void CPythonApplication::UpdateMainWindowPosition()
{
	POINT finalPoint{};
	if (GetCursorPos(&finalPoint))
	{
		LONG xDiff = finalPoint.x - m_InitialMouseMovingPoint.x;
		LONG yDiff = finalPoint.y - m_InitialMouseMovingPoint.y;

		RECT r{};
		GetWindowRect(&r);

		SetPosition(r.left + xDiff, r.top + yDiff);
		m_InitialMouseMovingPoint = finalPoint;
	}
}

void CPythonApplication::Loop()
{	
	while (1)
	{	
		if (IsUserMovingMainWindow())
			UpdateMainWindowPosition();
		
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime=ELTimer_GetMSec();
		}
	}
}

bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();
	CItemManager&		rkItemMgr	= CItemManager::Instance();	
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];

	snprintf (szItemList,	sizeof (szItemList),	"%s/item_list.txt", GetLocalePathCommon());
	snprintf (szItemProto,	sizeof (szItemProto),	"%s/item_proto",	localePath);
	snprintf (szItemDesc,	sizeof (szItemDesc),	"%s/itemdesc.txt",	localePath);
	snprintf (szMobProto,	sizeof (szMobProto),	"%s/mob_proto",		localePath);
	snprintf (szSkillDescFileName, sizeof (szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	snprintf (szSkillTableFileName, sizeof (szSkillTableFileName),	"%s/SkillTable.txt", GetLocalePathCommon());
	snprintf (szInsultList,	sizeof (szInsultList),	"%s/insult.txt", localePath);

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);
	}

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szMobProto);
		return false;
	}

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);
	}

	return true;
}

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
		return WS_OVERLAPPED | WS_CAPTION |   WS_SYSMENU | WS_MINIMIZEBOX;

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed)
{
	// Initialize Game Thread Pool first - required by other systems
	CGameThreadPool* pThreadPool = CGameThreadPool::InstancePtr();
	if (pThreadPool)
	{
		pThreadPool->Initialize();
	}

	NANOBEGIN
		Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	std::wstring wWindowName = Utf8ToWide(c_szName ? c_szName : "");

	if (FindWindowW(nullptr, wWindowName.c_str()))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon( GetInstance(), MAKEINTRESOURCE( IDI_METIN2 ) ), IDC_CURSOR_NORMAL))
	{
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
	{
		CPythonIME::Instance().UseDefaultIME();
	}

#if defined(ENABLE_DISCORD_RPC)
	m_pyNetworkStream.Discord_Start();
#endif

	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;

			if (bAnotherWindow)
			{
				RECT rc;

				GetClientRect(&rc);

				int windowWidth = rc.right - rc.left;
				int windowHeight = (rc.bottom - rc.top);

				CMSApplication::SetPosition(GetScreenWidth() - windowWidth, GetScreenHeight() - 60 - windowHeight);
			}
			SetPosition(-8, 0); //Fix
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	NANOEND
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// Cursor
		if (!CreateCursors())
		{
			TraceError("CMSWindow::Cursors Create Error");
			SET_EXCEPTION("CREATE_CURSOR");
			return false;
		}

		if (!m_pySystem.IsNoSoundCard())
		{
			// Sound
			if (!m_SoundEngine.Initialize())
			{
				TraceError("Failed to initialize sound manager!");
				return false; // Is this important enough to stop the client?
			}
		}

		extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

		if (!m_pySystem.IsAutoTiling())
			GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

		// Device
		if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
			return false;

		GrannyCreateSharedDeformBuffer();

		if (m_pySystem.IsAutoTiling())
		{
			if (m_grpDevice.IsFastTNL())
			{
				m_pyBackground.ReserveSoftwareTilingEnable(false);
			}
			else
			{
				m_pyBackground.ReserveSoftwareTilingEnable(true);
			}
		}
		else
		{
			m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());
		}

		SetVisibleMode(true);

		if (m_isWindowFullScreenEnable)
		{
			SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
		}

		if (!InitializeKeyboard(GetWindowHandle()))
			return false;

		m_pySystem.GetDisplaySettings();

		// Mouse
		if (m_pySystem.IsSoftwareCursor())
			SetCursorMode(CURSOR_MODE_SOFTWARE);
		else
			SetCursorMode(CURSOR_MODE_HARDWARE);

		// Network
		if (!m_netDevice.Create())
		{
			TraceError("NetDevice::Create failed");
			SET_EXCEPTION("CREATE_NETWORK");
			return false;
		}

		if (!m_grpDevice.IsFastTNL())
			CGrannyLODController::SetMinLODMode(true);

		m_pyItem.Create();

		// Other Modules
		DefaultFont_Startup();

		CPythonIME::Instance().Create(GetWindowHandle());
		CPythonIME::Instance().SetText("", 0);
		CPythonTextTail::Instance().Initialize();

		// Light Manager
		m_LightManager.Initialize();

		CGraphicImageInstance::CreateSystem(32);

		// ¹é¾÷
		STICKYKEYS sStickKeys;
		memset(&sStickKeys, 0, sizeof(sStickKeys));
		sStickKeys.cbSize = sizeof(sStickKeys);
		SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
		m_dwStickyKeysFlag = sStickKeys.dwFlags;

		// ¼³Á¤
		sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
		SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );

		// SphereMap
		CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
		CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");
		return true;
}

void CPythonApplication::SetGlobalCenterPosition(int32_t x, int32_t y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}


void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

// 2005.03.28 - MALL ¾ÆÀÌÅÛ¿¡ µé¾îÀÖ´Â ½Ã°£ÀÇ ´ÜÀ§°¡ ¼­¹ö¿¡¼­ time(0) À¸·Î ¸¸µé¾îÁö´Â
//              °ªÀÌ±â ¶§¹®¿¡ ´ÜÀ§¸¦ ¸ÂÃß±â À§ÇØ ½Ã°£ °ü·Ã Ã³¸®¸¦ º°µµ·Î Ãß°¡
time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::Destroy()
{
	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();	
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();	
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();

	m_kEftMgr.Destroy();
	m_LightManager.Destroy();

	// Game Thread Pool
	CGameThreadPool::Instance().Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();
	
#if defined(ENABLE_DISCORD_RPC)
	m_pyNetworkStream.Discord_Close();
#endif	
	
	//m_pyNetworkDatagram.Destroy();	

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();

	m_grpDevice.Destroy();

	//CSpeedTreeForestDirectX::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}
