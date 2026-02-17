#pragma once

#include "eterLib/MSApplication.h"
#include "eterLib/Input.h"
#include "eterLib/Profiler.h"
#include "eterLib/GrpDevice.h"
#include "eterLib/NetDevice.h"
#include "eterLib/GrpLightManager.h"
#include "eterLib/GameThreadPool.h"
#include "EffectLib/EffectManager.h"
#include "gamelib/RaceManager.h"
#include "gamelib/ItemManager.h"
#include "gamelib/FlyingObjectManager.h"
#include "gamelib/GameEventManager.h"
#include "AudioLib/SoundEngine.h"

#include "PythonEventManager.h"
#include "PythonPlayer.h"
#include "PythonNonPlayer.h"
#include "PythonMiniMap.h"
#include "PythonIME.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonChat.h"
#include "PythonTextTail.h"
#include "PythonSkill.h"
#include "PythonSystem.h"
#include "PythonNetworkStream.h"
#include "PythonCharacterManager.h"
#include "PythonQuest.h"
#include "PythonMessenger.h"
#include "PythonSafeBox.h"
#include "PythonGuild.h"

#include "GuildMarkDownloader.h"
#include "GuildMarkUploader.h"

#include "AccountConnector.h"

#include "ServerStateChecker.h"
#include "AbstractApplication.h"
#include "MovieMan.h"

#include <qedit.h>

class CPythonApplication : public CMSApplication,
                           public CInputKeyboard,
                           public IAbstractApplication {
public:
  enum EDeviceState {
    DEVICE_STATE_FALSE,
    DEVICE_STATE_SKIP,
    DEVICE_STATE_OK,
  };

  enum ECursorMode {
    CURSOR_MODE_HARDWARE,
    CURSOR_MODE_SOFTWARE,
  };

  enum ECursorShape {
    CURSOR_SHAPE_NORMAL,
    CURSOR_SHAPE_ATTACK,
    CURSOR_SHAPE_TARGET,
    CURSOR_SHAPE_TALK,
    CURSOR_SHAPE_CANT_GO,
    CURSOR_SHAPE_PICK,

    CURSOR_SHAPE_DOOR,
    CURSOR_SHAPE_CHAIR,
    CURSOR_SHAPE_MAGIC, // Magic
    CURSOR_SHAPE_BUY,   // Buy
    CURSOR_SHAPE_SELL,  // Sell

    CURSOR_SHAPE_CAMERA_ROTATE, // Camera Rotate
    CURSOR_SHAPE_HSIZE,         // Horizontal Size
    CURSOR_SHAPE_VSIZE,         // Vertical Size
    CURSOR_SHAPE_HVSIZE,        // Horizontal & Vertical Size

    CURSOR_SHAPE_COUNT,

    // 안정적인 네이밍 변환을 위한 임시 enumerate
    NORMAL = CURSOR_SHAPE_NORMAL,
    ATTACK = CURSOR_SHAPE_ATTACK,
    TARGET = CURSOR_SHAPE_TARGET,
    CAMERA_ROTATE = CURSOR_SHAPE_CAMERA_ROTATE,
    CURSOR_COUNT = CURSOR_SHAPE_COUNT,
  };

  enum EInfo {
    INFO_ACTOR,
    INFO_EFFECT,
    INFO_ITEM,
    INFO_TEXTTAIL,
  };

  enum ECameraControlDirection {
    CAMERA_TO_POSITIVE = 1,
    CAMERA_TO_NEGITIVE = -1,
    CAMERA_STOP = 0,
  };

  enum {
    CAMERA_MODE_NORMAL = 0,
    CAMERA_MODE_STAND = 1,
    CAMERA_MODE_BLEND = 2,

    EVENT_CAMERA_NUMBER = 101,
  };

  struct SCameraSpeed {
    float m_fUpDir;
    float m_fViewDir;
    float m_fCrossDir;

    SCameraSpeed() : m_fUpDir(0.0f), m_fViewDir(0.0f), m_fCrossDir(0.0f) {}
  };

public:
  CPythonApplication();
  virtual ~CPythonApplication();

public:
  void ShowWebPage(const char *c_szURL, const RECT &c_rcWebPage);
  void MoveWebPage(const RECT &c_rcWebPage);
  void HideWebPage();

  bool IsWebPageMode();

public:
  void NotifyHack(const char *c_szFormat, ...);
  void GetInfo(UINT eInfo, std::string *pstInfo);
  void GetMousePosition(POINT *ppt);

  static CPythonApplication &Instance() {
    assert(ms_pInstance != NULL);
    return *ms_pInstance;
  }

  void Loop();
  void Destroy();
  void Clear();
  void Exit();
  void Abort();

  bool IsUserMovingMainWindow() const;
  void SetUserMovingMainWindow(bool flag);
  void UpdateMainWindowPosition();

  void SetMinFog(float fMinFog);
  void SetFrameSkip(bool isEnable);

  bool Create(PyObject *poSelf, const char *c_szName, int width, int height,
              int Windowed);
  bool CreateDevice(int width, int height, int Windowed, int bit = 32,
                    int frequency = 0);

  void UpdateGame();
  void RenderGame();

  bool Process();

  void UpdateClientRect();

  bool CreateCursors();
  void DestroyCursors();

  void SafeSetCapture();
  void SafeReleaseCapture();

  BOOL SetCursorNum(int iCursorNum);
  void SetCursorVisible(BOOL bFlag, bool bLiarCursorOn = false);
  BOOL GetCursorVisible();
  bool GetLiarCursorOn();
  void SetCursorMode(int iMode);
  int GetCursorMode();
  int GetCursorNum() { return m_iCursorNum; }

  void SetMouseHandler(PyObject *poMouseHandler);

  int GetWidth();
  int GetHeight();

  void SetGlobalCenterPosition(int32_t x, int32_t y);
  void SetCenterPosition(float fx, float fy, float fz);
  void GetCenterPosition(TPixelPosition *pPixelPosition);
  void SetCamera(float Distance, float Pitch, float Rotation,
                 float fDestinationHeight);
  void GetCamera(float *Distance, float *Pitch, float *Rotation,
                 float *DestinationHeight);
  void RotateCamera(int iDirection);
  void PitchCamera(int iDirection);
  void ZoomCamera(int iDirection);
  void MovieRotateCamera(int iDirection);
  void MoviePitchCamera(int iDirection);
  void MovieZoomCamera(int iDirection);
  void MovieResetCamera();
  void SetViewDirCameraSpeed(float fSpeed);
  void SetCrossDirCameraSpeed(float fSpeed);
  void SetUpDirCameraSpeed(float fSpeed);
  float GetRotation();
  float GetPitch();

  void SetFPS(int iFPS);
  void SetServerTime(time_t tTime);
  time_t GetServerTime();
  time_t GetServerTimeStamp();
  float GetGlobalTime();
  float GetGlobalElapsedTime();

  float GetFaceSpeed() { return m_fFaceSpd; }
  float GetAveRenderTime() { return m_fAveRenderTime; }
  DWORD GetCurRenderTime() { return m_dwCurRenderTime; }
  DWORD GetCurUpdateTime() { return m_dwCurUpdateTime; }
  DWORD GetUpdateFPS() { return m_dwUpdateFPS; }
  DWORD GetRenderFPS() { return m_dwRenderFPS; }
  DWORD GetLoad() { return m_dwLoad; }
  DWORD GetFaceCount() { return m_dwFaceCount; }

  void SetConnectData(const char *c_szIP, int iPort);
  void GetConnectData(std::string &rstIP, int &riPort);

  void RunIMEUpdate();
  void RunIMETabEvent();
  void RunIMEReturnEvent();
  void RunPressExitKey();

  void RunIMEOpenCandidateListEvent();
  void RunIMECloseCandidateListEvent();
  void RunIMEOpenReadingWndEvent();
  void RunIMECloseReadingWndEvent();

  void EnableSpecialCameraMode();
  void SetCameraSpeed(int iPercentage);

  bool IsLockCurrentCamera();
  void SetEventCamera(const SCameraSetting &c_rCameraSetting);
  void BlendEventCamera(const SCameraSetting &c_rCameraSetting,
                        float fBlendTime);
  void SetDefaultCamera();

  void SetCameraSetting(const SCameraSetting &c_rCameraSetting);
  void GetCameraSetting(SCameraSetting *pCameraSetting);
  void SaveCameraSetting(const char *c_szFileName);
  bool LoadCameraSetting(const char *c_szFileName);

  void SetForceSightRange(int iRange);

public:
  int OnLogoOpen(char *szName);
  int OnLogoUpdate();
  void OnLogoRender();
  void OnLogoClose();

protected:
  IGraphBuilder *m_pGraphBuilder;   // Graph Builder
  IBaseFilter *m_pFilterSG;         // Sample Grabber 필터
  ISampleGrabber *m_pSampleGrabber; // 영상 이미지 캡처를 위한 샘플 그래버
  IMediaControl *m_pMediaCtrl;      // Media Control
  IMediaEventEx *m_pMediaEvent;     // Media Event
  IVideoWindow *m_pVideoWnd;        // Video Window
  IBasicVideo *m_pBasicVideo;
  uint8_t *m_pCaptureBuffer;        // 영상 이미지를 캡처한 버퍼
  LONG m_lBufferSize;               // Video 버퍼 크기 변수
  CGraphicImageTexture *m_pLogoTex; // 출력할 텍스쳐
  bool m_bLogoError;                // 영상 읽기 상태
  bool m_bLogoPlay;

  int m_nLeft, m_nRight, m_nTop, m_nBottom;

protected:
  LRESULT WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

  void OnCameraUpdate();

  void OnUIUpdate();
  void OnUIRender();

  void OnMouseUpdate();
  void OnMouseRender();

  void OnMouseWheel(int nLen);
  void OnMouseMove(int x, int y);
  void OnMouseMiddleButtonDown(int x, int y);
  void OnMouseMiddleButtonUp(int x, int y);
  void OnMouseLeftButtonDown(int x, int y);
  void OnMouseLeftButtonUp(int x, int y);
  void OnMouseLeftButtonDoubleClick(int x, int y);
  void OnMouseRightButtonDown(int x, int y);
  void OnMouseRightButtonUp(int x, int y);
  void OnSizeChange(int width, int height);
  void OnKeyDown(int iIndex);
  void OnKeyUp(int iIndex);
  void OnIMEKeyDown(int iIndex);

  int CheckDeviceState();

  BOOL __IsContinuousChangeTypeCursor(int iCursorNum);

  void __UpdateCamera();

  void __SetFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight,
                             DWORD dwBPP);
  void __MinimizeFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight);

protected:
  CTimer m_timer;

  CLightManager m_LightManager;
  SoundEngine m_SoundEngine;
  CFlyingManager m_FlyingManager;
  CRaceManager m_RaceManager;
  CGameEventManager m_GameEventManager;
  CItemManager m_kItemMgr;
  CMovieMan m_MovieManager;

  UI::CWindowManager m_kWndMgr;
  CEffectManager m_kEftMgr;
  CPythonCharacterManager m_kChrMgr;

  CServerStateChecker m_kServerStateChecker;
  CPythonGraphic m_pyGraphic;
  CPythonNetworkStream m_pyNetworkStream;
  CPythonPlayer m_pyPlayer;
  CPythonIME m_pyIme;
  CPythonItem m_pyItem;
  CPythonShop m_pyShop;
  CPythonExchange m_pyExchange;
  CPythonChat m_pyChat;
  CPythonTextTail m_pyTextTail;
  CPythonNonPlayer m_pyNonPlayer;
  CPythonMiniMap m_pyMiniMap;
  CPythonEventManager m_pyEventManager;
  CPythonBackground m_pyBackground;
  CPythonSkill m_pySkill;
  CPythonResource m_pyRes;
  CPythonQuest m_pyQuest;
  CPythonMessenger m_pyManager;
  CPythonSafeBox m_pySafeBox;
  CPythonGuild m_pyGuild;

  CGuildMarkManager m_kGuildMarkManager;
  CGuildMarkDownloader m_kGuildMarkDownloader;
  CGuildMarkUploader m_kGuildMarkUploader;
  CAccountConnector m_kAccountConnector;

  CGraphicDevice m_grpDevice;
  CNetworkDevice m_netDevice;

  CPythonSystem m_pySystem;

  PyObject *m_poMouseHandler;
  D3DXVECTOR3 m_v3CenterPosition;

  unsigned int m_iFPS;
  float m_fAveRenderTime;
  DWORD m_dwCurRenderTime;
  DWORD m_dwCurUpdateTime;
  DWORD m_dwLoad;
  DWORD m_dwWidth;
  DWORD m_dwHeight;

protected:
  // Time
  DWORD m_dwLastIdleTime;
  DWORD m_dwStartLocalTime;
  time_t m_tServerTime;
  time_t m_tLocalStartTime;
  float m_fGlobalTime;
  float m_fGlobalElapsedTime;

  /////////////////////////////////////////////////////////////
  // Camera
  SCameraSetting m_DefaultCameraSetting;
  SCameraSetting m_kEventCameraSetting;

  int m_iCameraMode;
  float m_fBlendCameraStartTime;
  float m_fBlendCameraBlendTime;
  SCameraSetting m_kEndBlendCameraSetting;

  float m_fRotationSpeed;
  float m_fPitchSpeed;
  float m_fZoomSpeed;
  float m_fCameraRotateSpeed;
  float m_fCameraPitchSpeed;
  float m_fCameraZoomSpeed;

  SCameraPos m_kCmrPos;
  SCameraSpeed m_kCmrSpd;

  BOOL m_isSpecialCameraMode;
  // Camera
  /////////////////////////////////////////////////////////////

  float m_fFaceSpd;
  DWORD m_dwFaceSpdSum;
  DWORD m_dwFaceSpdCount;

  DWORD m_dwFaceAccCount;
  DWORD m_dwFaceAccTime;

  DWORD m_dwUpdateFPS;
  DWORD m_dwRenderFPS;
  DWORD m_dwFaceCount;

  DWORD m_dwLButtonDownTime;
  DWORD m_dwLButtonUpTime;

  typedef std::map<int, HANDLE> TCursorHandleMap;
  TCursorHandleMap m_CursorHandleMap;
  HANDLE m_hCurrentCursor;

  BOOL m_bCursorVisible;
  bool m_bLiarCursorOn;
  int m_iCursorMode;
  bool m_isWindowed;
  bool m_isFrameSkipDisable;

  // Connect Data
  std::string m_strIP;
  int m_iPort;

  static CPythonApplication *ms_pInstance;

  bool m_isMinimizedWnd;
  bool m_isActivateWnd;
  BOOL m_isWindowFullScreenEnable;

  DWORD m_dwStickyKeysFlag;
  int m_iForceSightRange;

protected:
  bool m_IsMovingMainWindow;
  POINT m_InitialMouseMovingPoint;
  int m_iCursorNum;
  int m_iContinuousCursorNum;
};
