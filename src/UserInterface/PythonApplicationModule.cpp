#include "StdAfx.h"
#include "Resource.h"
#include "PythonApplication.h"
#include "EterLib/Camera.h"
#include "PackLib/PackManager.h"
#include "EterBase/tea.h"

#include <stb_image.h>
#include <utf8.h>

extern D3DXCOLOR g_fSpecularColor;
extern BOOL bVisibleNotice = true;
extern BOOL bTestServerFlag = FALSE;
extern int TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = 0;

PyObject *appShowWebPage(PyObject *poSelf, PyObject *poArgs) {
  char *szWebPage;
  if (!PyTuple_GetString(poArgs, 0, &szWebPage))
    return Py_BuildException();

  PyObject *poRect = PyTuple_GetItem(poArgs, 1);
  if (!PyTuple_Check(poRect))
    return Py_BuildException();

  RECT rcWebPage;
  rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
  rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
  rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
  rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

  CPythonApplication::Instance().ShowWebPage(szWebPage, rcWebPage);
  return Py_BuildNone();
}

PyObject *appMoveWebPage(PyObject *poSelf, PyObject *poArgs) {
  PyObject *poRect = PyTuple_GetItem(poArgs, 0);
  if (!PyTuple_Check(poRect))
    return Py_BuildException();

  RECT rcWebPage;
  rcWebPage.left = PyInt_AsLong(PyTuple_GetItem(poRect, 0));
  rcWebPage.top = PyInt_AsLong(PyTuple_GetItem(poRect, 1));
  rcWebPage.right = PyInt_AsLong(PyTuple_GetItem(poRect, 2));
  rcWebPage.bottom = PyInt_AsLong(PyTuple_GetItem(poRect, 3));

  CPythonApplication::Instance().MoveWebPage(rcWebPage);
  return Py_BuildNone();
}

PyObject *appHideWebPage(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().HideWebPage();
  return Py_BuildNone();
}

PyObject *appIsWebPageMode(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().IsWebPageMode());
}

PyObject *appEnablePerformanceTime(PyObject *poSelf, PyObject *poArgs) {
  char *szMode;
  if (!PyTuple_GetString(poArgs, 0, &szMode))
    return Py_BuildException();

  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 1, &nEnable))
    return Py_BuildException();

  // TODO: remove this function

  return Py_BuildNone();
}

/////////////////////////////////////////////////////

extern BOOL HAIR_COLOR_ENABLE;
extern BOOL USE_ARMOR_SPECULAR;
extern BOOL USE_WEAPON_SPECULAR;
extern BOOL SKILL_EFFECT_UPGRADE_ENABLE;
extern BOOL RIDE_HORSE_ENABLE;
extern double g_specularSpd;

// TEXTTAIL_LIVINGTIME_CONTROL
extern void TextTail_SetLivingTime(long livingTime);

PyObject *appSetTextTailLivingTime(PyObject *poSelf, PyObject *poArgs) {
  float livingTime;
  if (!PyTuple_GetFloat(poArgs, 0, &livingTime))
    return Py_BuildException();

  TextTail_SetLivingTime(livingTime * 1000);

  return Py_BuildNone();
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

PyObject *appSetHairColorEnable(PyObject *poSelf, PyObject *poArgs) {
  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
    return Py_BuildException();

  HAIR_COLOR_ENABLE = nEnable;

  return Py_BuildNone();
}

PyObject *appSetArmorSpecularEnable(PyObject *poSelf, PyObject *poArgs) {
  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
    return Py_BuildException();

  USE_ARMOR_SPECULAR = nEnable;

  return Py_BuildNone();
}

PyObject *appSetWeaponSpecularEnable(PyObject *poSelf, PyObject *poArgs) {
  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
    return Py_BuildException();

  USE_WEAPON_SPECULAR = nEnable;

  return Py_BuildNone();
}

PyObject *appSetSkillEffectUpgradeEnable(PyObject *poSelf, PyObject *poArgs) {
  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
    return Py_BuildException();

  SKILL_EFFECT_UPGRADE_ENABLE = nEnable;

  return Py_BuildNone();
}

PyObject *SetTwoHandedWeaponAttSpeedDecreaseValue(PyObject *poSelf,
                                                  PyObject *poArgs) {
  int iValue;
  if (!PyTuple_GetInteger(poArgs, 0, &iValue))
    return Py_BuildException();

  TWOHANDED_WEWAPON_ATT_SPEED_DECREASE_VALUE = iValue;

  return Py_BuildNone();
}

PyObject *appSetRideHorseEnable(PyObject *poSelf, PyObject *poArgs) {
  int nEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
    return Py_BuildException();

  RIDE_HORSE_ENABLE = nEnable;

  return Py_BuildNone();
}

PyObject *appSetCameraMaxDistance(PyObject *poSelf, PyObject *poArgs) {
  float fMax;
  if (!PyTuple_GetFloat(poArgs, 0, &fMax))
    return Py_BuildException();

  CCamera::SetCameraMaxDistance(fMax);
  return Py_BuildNone();
}

PyObject *appSetControlFP(PyObject *poSelf, PyObject *poArgs) {
  _controlfp(_PC_24, _MCW_PC);
  return Py_BuildNone();
}

PyObject *appSetSpecularSpeed(PyObject *poSelf, PyObject *poArgs) {
  float fSpeed;
  if (!PyTuple_GetFloat(poArgs, 0, &fSpeed))
    return Py_BuildException();

  g_specularSpd = fSpeed;

  return Py_BuildNone();
}

PyObject *appSetMinFog(PyObject *poSelf, PyObject *poArgs) {
  float fMinFog;
  if (!PyTuple_GetFloat(poArgs, 0, &fMinFog))
    return Py_BuildException();

  CPythonApplication::Instance().SetMinFog(fMinFog);
  return Py_BuildNone();
}

PyObject *appSetFrameSkip(PyObject *poSelf, PyObject *poArgs) {
  int nFrameSkip;
  if (!PyTuple_GetInteger(poArgs, 0, &nFrameSkip))
    return Py_BuildException();

  CPythonApplication::Instance().SetFrameSkip(nFrameSkip ? true : false);
  return Py_BuildNone();
}

// LOCALE
bool LoadLocaleData(const char *localePath);

PyObject *appLoadLocaleData(PyObject *poSelf, PyObject *poArgs) {
  char *localePath;
  if (!PyTuple_GetString(poArgs, 0, &localePath))
    return Py_BuildException();

  return Py_BuildValue("i", LoadLocaleData(localePath));
}

PyObject *appGetLocaleName(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("s", GetLocaleName());
}

PyObject *appGetLocalePath(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("s", GetLocalePath());
}

PyObject *appGetLocalePathCommon(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("s", GetLocalePathCommon());
}
// END_OF_LOCALE

#ifdef __VTUNE__

PyObject *appGetImageInfo(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  return Py_BuildValue("iii", 0, 0, 0);
}

#else

PyObject *appGetImageInfo(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  int w = 0, h = 0, comp = 0;
  int canLoad = stbi_info(szFileName, &w, &h, &comp) ? 1 : 0;

  return Py_BuildValue("iii", canLoad, w, h);
}
#endif

PyObject *appIsExistFile(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  bool isExist = CPackManager::Instance().IsExist(szFileName);

  return Py_BuildValue("i", isExist);
}

PyObject *appGetFileList(PyObject *poSelf, PyObject *poArgs) {
  char *szFilter = nullptr;
  if (!PyTuple_GetString(poArgs, 0, &szFilter))
    return Py_BuildException();

  std::wstring wFilter = Utf8ToWide(szFilter ? szFilter : "");

  PyObject *poList = PyList_New(0);

  WIN32_FIND_DATAW wfd{};
  HANDLE hFind = FindFirstFileW(wFilter.c_str(), &wfd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      std::string filenameUtf8 = WideToUtf8(wfd.cFileName);
      PyObject *poFileName = PyString_FromString(filenameUtf8.c_str());
      PyList_Append(poList, poFileName);
      Py_DECREF(poFileName);
    } while (FindNextFileW(hFind, &wfd));

    FindClose(hFind);
  }

  return poList;
}

PyObject *appUpdateGame(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().UpdateGame();
  return Py_BuildNone();
}

PyObject *appRenderGame(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().RenderGame();
  return Py_BuildNone();
}

PyObject *appSetMouseHandler(PyObject *poSelf, PyObject *poArgs) {
  PyObject *poHandler;
  if (!PyTuple_GetObject(poArgs, 0, &poHandler))
    return Py_BuildException();

  CPythonApplication::Instance().SetMouseHandler(poHandler);
  return Py_BuildNone();
}

PyObject *appCreate(PyObject *poSelf, PyObject *poArgs) {
  char *szName;
  if (!PyTuple_GetString(poArgs, 0, &szName))
    return Py_BuildException();

  int width;
  if (!PyTuple_GetInteger(poArgs, 1, &width))
    return Py_BuildException();

  int height;
  if (!PyTuple_GetInteger(poArgs, 2, &height))
    return Py_BuildException();

  int Windowed;
  if (!PyTuple_GetInteger(poArgs, 3, &Windowed))
    return Py_BuildException();

  CPythonApplication &rkApp = CPythonApplication::Instance();
  if (!rkApp.Create(poSelf, szName, width, height, Windowed)) {
    // return Py_BuildNone();
    return NULL;
  }

  return Py_BuildNone();
}

PyObject *appLoop(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().Loop();

  return Py_BuildNone();
}

PyObject *appGetInfo(PyObject *poSelf, PyObject *poArgs) {
  int nInfo;
  if (!PyTuple_GetInteger(poArgs, 0, &nInfo))
    return Py_BuildException();

  std::string stInfo;
  CPythonApplication::Instance().GetInfo(nInfo, &stInfo);
  return Py_BuildValue("s", stInfo.c_str());
}

PyObject *appProcess(PyObject *poSelf, PyObject *poArgs) {
  if (CPythonApplication::Instance().Process())
    return Py_BuildValue("i", 1);

  return Py_BuildValue("i", 0);
}

PyObject *appAbort(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().Abort();
  return Py_BuildNone();
}

PyObject *appExit(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().Exit();
  return Py_BuildNone();
}

PyObject *appSetCamera(PyObject *poSelf, PyObject *poArgs) {
  float Distance;
  if (!PyTuple_GetFloat(poArgs, 0, &Distance))
    return Py_BuildException();

  float Pitch;
  if (!PyTuple_GetFloat(poArgs, 1, &Pitch))
    return Py_BuildException();

  float Rotation;
  if (!PyTuple_GetFloat(poArgs, 2, &Rotation))
    return Py_BuildException();

  float fDestinationHeight;
  if (!PyTuple_GetFloat(poArgs, 3, &fDestinationHeight))
    return Py_BuildException();

  CPythonApplication::Instance().SetCamera(Distance, Pitch, Rotation,
                                           fDestinationHeight);
  return Py_BuildNone();
}

PyObject *appGetCamera(PyObject *poSelf, PyObject *poArgs) {
  float Distance, Pitch, Rotation, DestinationHeight;
  CPythonApplication::Instance().GetCamera(&Distance, &Pitch, &Rotation,
                                           &DestinationHeight);

  return Py_BuildValue("ffff", Distance, Pitch, Rotation, DestinationHeight);
}

PyObject *appGetCameraPitch(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("f", CPythonApplication::Instance().GetPitch());
}

PyObject *appGetCameraRotation(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("f", CPythonApplication::Instance().GetRotation());
}

PyObject *appGetTime(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("f", CPythonApplication::Instance().GetGlobalTime());
}

PyObject *appGetGlobalTime(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetServerTime());
}

PyObject *appGetGlobalTimeStamp(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i",
                       CPythonApplication::Instance().GetServerTimeStamp());
}

PyObject *appGetUpdateFPS(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetUpdateFPS());
}

PyObject *appGetRenderFPS(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetRenderFPS());
}

PyObject *appRotateCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().RotateCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appPitchCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().PitchCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appZoomCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().ZoomCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appMovieRotateCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().MovieRotateCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appMoviePitchCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().MoviePitchCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appMovieZoomCamera(PyObject *poSelf, PyObject *poArgs) {
  int iDirection;
  if (!PyTuple_GetInteger(poArgs, 0, &iDirection))
    return Py_BuildException();
  CPythonApplication::Instance().MovieZoomCamera(iDirection);
  return Py_BuildNone();
}

PyObject *appMovieResetCamera(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().MovieResetCamera();
  return Py_BuildNone();
}

PyObject *appGetFaceSpeed(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("f", CPythonApplication::Instance().GetFaceSpeed());
}

PyObject *appGetRenderTime(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("fi", CPythonApplication::Instance().GetAveRenderTime(),
                       CPythonApplication::Instance().GetCurRenderTime());
}

PyObject *appGetUpdateTime(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetCurUpdateTime());
}

PyObject *appGetLoad(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetLoad());
}
PyObject *appGetFaceCount(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetFaceCount());
}

PyObject *appGetAvaiableTextureMememory(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CGraphicBase::GetAvailableTextureMemory());
}

PyObject *appSetFPS(PyObject *poSelf, PyObject *poArgs) {
  int iFPS;
  if (!PyTuple_GetInteger(poArgs, 0, &iFPS))
    return Py_BuildException();

  CPythonApplication::Instance().SetFPS(iFPS);

  return Py_BuildNone();
}

PyObject *appSetGlobalCenterPosition(PyObject *poSelf, PyObject *poArgs) {
  int x;
  if (!PyTuple_GetInteger(poArgs, 0, &x))
    return Py_BuildException();

  int y;
  if (!PyTuple_GetInteger(poArgs, 1, &y))
    return Py_BuildException();

  CPythonApplication::Instance().SetGlobalCenterPosition(x, y);
  return Py_BuildNone();
}

PyObject *appSetCenterPosition(PyObject *poSelf, PyObject *poArgs) {
  float fx;
  if (!PyTuple_GetFloat(poArgs, 0, &fx))
    return Py_BuildException();

  float fy;
  if (!PyTuple_GetFloat(poArgs, 1, &fy))
    return Py_BuildException();

  float fz;
  if (!PyTuple_GetFloat(poArgs, 2, &fz))
    return Py_BuildException();

  CPythonApplication::Instance().SetCenterPosition(fx, -fy, fz);
  return Py_BuildNone();
}

PyObject *appGetCursorPosition(PyObject *poSelf, PyObject *poArgs) {
  long lx, ly;
  UI::CWindowManager &rkWndMgr = UI::CWindowManager::Instance();
  rkWndMgr.GetMousePosition(lx, ly);

  return Py_BuildValue("ii", lx, ly);
}

PyObject *appRunPythonFile(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  bool ret = CPythonLauncher::Instance().RunFile(szFileName);
  return Py_BuildValue("i", ret);
}

PyObject *appIsPressed(PyObject *poSelf, PyObject *poArgs) {
  int iKey;
  if (!PyTuple_GetInteger(poArgs, 0, &iKey))
    return Py_BuildException();

  return Py_BuildValue("i", CPythonApplication::Instance().IsPressed(iKey));
}

PyObject *appSetCursor(PyObject *poSelf, PyObject *poArgs) {
  /*
          char * szName;
          if (!PyTuple_GetString(poArgs, 0, &szName))
                  return Py_BuildException();

          if (!CPythonApplication::Instance().SetHardwareCursor(szName))
                  return Py_BuildException("Wrong Cursor Name [%s]", szName);
  */
  int iCursorNum;
  if (!PyTuple_GetInteger(poArgs, 0, &iCursorNum))
    return Py_BuildException();

  if (!CPythonApplication::Instance().SetCursorNum(iCursorNum))
    return Py_BuildException("Wrong Cursor Name [%d]", iCursorNum);

  return Py_BuildNone();
}

PyObject *appGetCursor(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonApplication::Instance().GetCursorNum());
}

PyObject *appShowCursor(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().SetCursorVisible(TRUE);

  return Py_BuildNone();
}

PyObject *appHideCursor(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().SetCursorVisible(FALSE);

  return Py_BuildNone();
}

PyObject *appIsShowCursor(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue(
      "i", TRUE == CPythonApplication::Instance().GetCursorVisible());
}

PyObject *appIsLiarCursorOn(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue(
      "i", TRUE == CPythonApplication::Instance().GetLiarCursorOn());
}

PyObject *appSetSoftwareCursor(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().SetCursorMode(
      CPythonApplication::CURSOR_MODE_SOFTWARE);
  return Py_BuildNone();
}

PyObject *appSetHardwareCursor(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().SetCursorMode(
      CPythonApplication::CURSOR_MODE_HARDWARE);
  return Py_BuildNone();
}

PyObject *appSetConnectData(PyObject *poSelf, PyObject *poArgs) {
  char *szIP;
  if (!PyTuple_GetString(poArgs, 0, &szIP))
    return Py_BuildException();

  int iPort;
  if (!PyTuple_GetInteger(poArgs, 1, &iPort))
    return Py_BuildException();

  CPythonApplication::Instance().SetConnectData(szIP, iPort);

  return Py_BuildNone();
}

PyObject *appGetConnectData(PyObject *poSelf, PyObject *poArgs) {
  std::string strIP;
  int iPort;

  CPythonApplication::Instance().GetConnectData(strIP, iPort);

  return Py_BuildValue("si", strIP.c_str(), iPort);
}

PyObject *appGetRandom(PyObject *poSelf, PyObject *poArgs) {
  int from;
  if (!PyTuple_GetInteger(poArgs, 0, &from))
    return Py_BuildException();

  int to;
  if (!PyTuple_GetInteger(poArgs, 1, &to))
    return Py_BuildException();

  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return Py_BuildValue("i", random_range(from, to));
}

PyObject *appGetRotatingDirection(PyObject *poSelf, PyObject *poArgs) {
  float fSource;
  if (!PyTuple_GetFloat(poArgs, 0, &fSource))
    return Py_BuildException();
  float fTarget;
  if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
    return Py_BuildException();

  return Py_BuildValue("i", GetRotatingDirection(fSource, fTarget));
}

PyObject *appGetDegreeDifference(PyObject *poSelf, PyObject *poArgs) {
  float fSource;
  if (!PyTuple_GetFloat(poArgs, 0, &fSource))
    return Py_BuildException();
  float fTarget;
  if (!PyTuple_GetFloat(poArgs, 1, &fTarget))
    return Py_BuildException();

  return Py_BuildValue("f", GetDegreeDifference(fSource, fTarget));
}

PyObject *appSleep(PyObject *poSelf, PyObject *poArgs) {
  int iTime;
  if (!PyTuple_GetInteger(poArgs, 0, &iTime))
    return Py_BuildException();

  Sleep(iTime);

  return Py_BuildNone();
}

PyObject *appSetDefaultFontName(PyObject *poSelf, PyObject *poArgs) {
  char *szFontName;
  if (!PyTuple_GetString(poArgs, 0, &szFontName))
    return Py_BuildException();

  // DEFAULT_FONT
  DefaultFont_SetName(szFontName);
  // END_OF_DEFAULT_FONT

  return Py_BuildNone();
}

PyObject *appSetGuildSymbolPath(PyObject *poSelf, PyObject *poArgs) {
  char *szPathName;
  if (!PyTuple_GetString(poArgs, 0, &szPathName))
    return Py_BuildException();

  SetGuildSymbolPath(szPathName);

  return Py_BuildNone();
}

PyObject *appEnableSpecialCameraMode(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().EnableSpecialCameraMode();
  return Py_BuildNone();
}

PyObject *appSetCameraSpeed(PyObject *poSelf, PyObject *poArgs) {
  int iPercentage;
  if (!PyTuple_GetInteger(poArgs, 0, &iPercentage))
    return Py_BuildException();

  CPythonApplication::Instance().SetCameraSpeed(iPercentage);

  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  if (pCamera)
    pCamera->SetResistance(float(iPercentage) / 100.0f);
  return Py_BuildNone();
}

PyObject *appIsFileExist(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  return Py_BuildValue("i", -1 != _access(szFileName, 0));
}

PyObject *appSetCameraSetting(PyObject *poSelf, PyObject *poArgs) {
  int ix;
  if (!PyTuple_GetInteger(poArgs, 0, &ix))
    return Py_BuildException();
  int iy;
  if (!PyTuple_GetInteger(poArgs, 1, &iy))
    return Py_BuildException();
  int iz;
  if (!PyTuple_GetInteger(poArgs, 2, &iz))
    return Py_BuildException();

  int iZoom;
  if (!PyTuple_GetInteger(poArgs, 3, &iZoom))
    return Py_BuildException();
  int iRotation;
  if (!PyTuple_GetInteger(poArgs, 4, &iRotation))
    return Py_BuildException();
  int iPitch;
  if (!PyTuple_GetInteger(poArgs, 5, &iPitch))
    return Py_BuildException();

  CPythonApplication::SCameraSetting CameraSetting;
  ZeroMemory(&CameraSetting, sizeof(CameraSetting));
  CameraSetting.v3CenterPosition.x = float(ix);
  CameraSetting.v3CenterPosition.y = float(iy);
  CameraSetting.v3CenterPosition.z = float(iz);
  CameraSetting.fZoom = float(iZoom);
  CameraSetting.fRotation = float(iRotation);
  CameraSetting.fPitch = float(iPitch);
  CPythonApplication::Instance().SetEventCamera(CameraSetting);
  return Py_BuildNone();
}

PyObject *appSaveCameraSetting(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  CPythonApplication::Instance().SaveCameraSetting(szFileName);
  return Py_BuildNone();
}

PyObject *appLoadCameraSetting(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  bool bResult = CPythonApplication::Instance().LoadCameraSetting(szFileName);
  return Py_BuildValue("i", bResult);
}

PyObject *appSetDefaultCamera(PyObject *poSelf, PyObject *poArgs) {
  CPythonApplication::Instance().SetDefaultCamera();
  return Py_BuildNone();
}

PyObject *appSetSightRange(PyObject *poSelf, PyObject *poArgs) {
  int iRange;
  if (!PyTuple_GetInteger(poArgs, 0, &iRange))
    return Py_BuildException();

  CPythonApplication::Instance().SetForceSightRange(iRange);
  return Py_BuildNone();
}

extern int g_iAccumulationTime;

PyObject *apptestGetAccumulationTime(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", g_iAccumulationTime);
}

PyObject *apptestResetAccumulationTime(PyObject *poSelf, PyObject *poArgs) {
  g_iAccumulationTime = 0;
  return Py_BuildNone();
}

PyObject *apptestSetSpecularColor(PyObject *poSelf, PyObject *poArgs) {
  float fr;
  if (!PyTuple_GetFloat(poArgs, 0, &fr))
    return Py_BuildException();
  float fg;
  if (!PyTuple_GetFloat(poArgs, 1, &fg))
    return Py_BuildException();
  float fb;
  if (!PyTuple_GetFloat(poArgs, 2, &fb))
    return Py_BuildException();
  g_fSpecularColor = D3DXCOLOR(fr, fg, fb, 1.0f);
  return Py_BuildNone();
}

PyObject *appSetVisibleNotice(PyObject *poSelf, PyObject *poArgs) {
  int iFlag;
  if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    return Py_BuildException();
  bVisibleNotice = iFlag;
  return Py_BuildNone();
}

PyObject *appIsVisibleNotice(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", bVisibleNotice);
}

PyObject *appEnableTestServerFlag(PyObject *poSelf, PyObject *poArgs) {
  bTestServerFlag = TRUE;
  return Py_BuildNone();
}

PyObject *appIsEnableTestServerFlag(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", bTestServerFlag);
}

class CTextLineLoader {
public:
  CTextLineLoader(const char *c_szFileName) {
    TPackFile kFile;
    if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
      return;

    m_kTextFileLoader.Bind(kFile.size(), kFile.data());
  }

  DWORD GetLineCount() { return m_kTextFileLoader.GetLineCount(); }

  const char *GetLine(DWORD dwIndex) {
    if (dwIndex >= GetLineCount())
      return "";

    return m_kTextFileLoader.GetLineString(dwIndex).c_str();
  }

protected:
  CMemoryTextFileLoader m_kTextFileLoader;
};

PyObject *appOpenTextFile(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  CTextLineLoader *pTextLineLoader = new CTextLineLoader(szFileName);

  return Py_BuildValue("K", pTextLineLoader);
}

PyObject *appCloseTextFile(PyObject *poSelf, PyObject *poArgs) {
  CTextLineLoader *pTextFileLoader;
  if (!PyTuple_GetPointer(poArgs, 0, &pTextFileLoader))
    return Py_BuildException();

  delete pTextFileLoader;

  return Py_BuildNone();
}

PyObject *appGetTextFileLineCount(PyObject *poSelf, PyObject *poArgs) {
  CTextLineLoader *pTextFileLoader;
  if (!PyTuple_GetPointer(poArgs, 0, &pTextFileLoader))
    return Py_BuildException();

  return Py_BuildValue("i", pTextFileLoader->GetLineCount());
}

PyObject *appGetTextFileLine(PyObject *poSelf, PyObject *poArgs) {
  CTextLineLoader *pTextFileLoader;
  if (!PyTuple_GetPointer(poArgs, 0, &pTextFileLoader))
    return Py_BuildException();
  int iLineIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iLineIndex))
    return Py_BuildException();

  return Py_BuildValue("s", pTextFileLoader->GetLine(iLineIndex));
}

PyObject *appSetGuildMarkPath(PyObject *poSelf, PyObject *poArgs) {
  char *path;
  if (!PyTuple_GetString(poArgs, 0, &path))
    return Py_BuildException();

  char newPath[256];
  char *ext = strstr(path, ".tga");

  if (ext) {
    int extPos = ext - path;
    strncpy(newPath, path, extPos);
    newPath[extPos] = '\0';
  } else
    strncpy(newPath, path, sizeof(newPath) - 1);

  CGuildMarkManager::Instance().SetMarkPathPrefix(newPath);
  return Py_BuildNone();
}

PyObject *appLogoOpen(PyObject *poSelf, PyObject *poArgs) {
  char *szName;
  if (!PyTuple_GetString(poArgs, 0, &szName))
    return Py_BuildException();

  int nIsSuccess = 1;
  CMovieMan::Instance().PlayLogo(szName);

  return Py_BuildValue("i", nIsSuccess);
}

PyObject *appLogoUpdate(PyObject *poSelf, PyObject *poArgs) {
  int nIsRun = 0;
  return Py_BuildValue("i", nIsRun);
}

PyObject *appLogoRender(PyObject *poSelf, PyObject *poArgs) {
  // CPythonApplication::Instance().OnLogoRender();
  return Py_BuildNone();
}

PyObject *appLogoClose(PyObject *poSelf, PyObject *poArgs) {
  // CPythonApplication::Instance().OnLogoClose();
  return Py_BuildNone();
}

PyObject *appIsRTL(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", IsRTL() ? 1 : 0);
}

PyObject *appReloadLocaleConfig(PyObject *poSelf, PyObject *poArgs) {
  LoadConfig("config/locale.cfg");
  return Py_BuildNone();
}

PyObject *appReloadLocale(PyObject *poSelf, PyObject *poArgs) {
  /**
   * Multi-language hot-reload system
   *
   * Reloads all locale-dependent data when the user changes language:
   * 1. Reloads config/locale.cfg to get new locale path
   * 2. Reloads C++ locale data (item_proto, mob_proto, etc.)
   * 3. Calls Python modules to reload locale text files
   *
   * Returns 1 on success, 0 on failure
   */

  // Step 1: Reload config to update locale path
  LoadConfig("config/locale.cfg");

  // Step 2: Reload C++ locale-dependent data (item_proto, mob_proto, etc.)
  if (!LoadLocaleData(GetLocalePath())) {
    // TraceError("app.ReloadLocale: Failed to load C++ locale data from %s",
    // GetLocalePath());
    return Py_BuildValue("i", 0);
  }

  // Step 3: Reload Python locale text files
  PyObject *modulesDict = PyImport_GetModuleDict();
  if (!modulesDict || !PyDict_Check(modulesDict)) {
    // TraceError("app.ReloadLocale: Failed to get Python modules dictionary");
    return Py_BuildValue("i", 0);
  }

  // Step 3a: Reload localeInfo module (locale_game.txt)
  PyObject *localeInfoModule = PyDict_GetItemString(modulesDict, "localeinfo");
  if (!localeInfoModule)
    localeInfoModule = PyDict_GetItemString(modulesDict, "localeInfo");

  if (localeInfoModule && PyModule_Check(localeInfoModule)) {
    if (PyObject_HasAttrString(localeInfoModule, "LoadLocaleData")) {
      PyObject *loadFunc =
          PyObject_GetAttrString(localeInfoModule, "LoadLocaleData");
      if (loadFunc && PyCallable_Check(loadFunc)) {
        PyObject *result = PyObject_CallObject(loadFunc, NULL);
        if (result)
          Py_DECREF(result);
        else {
          PyErr_Print();
          // TraceError("app.ReloadLocale: localeInfo.LoadLocaleData() failed");
        }
        Py_DECREF(loadFunc);
      }
    }
  }

  // Step 3b: Reload uiScriptLocale module (locale_interface.txt)
  PyObject *uiScriptLocaleModule = NULL;
  const char *uiScriptModuleNames[] = {"uiscriptlocale", "uiScriptLocale",
                                       NULL};

  for (int i = 0; uiScriptModuleNames[i] != NULL; i++) {
    uiScriptLocaleModule =
        PyDict_GetItemString(modulesDict, uiScriptModuleNames[i]);
    if (uiScriptLocaleModule)
      break;
  }

  if (uiScriptLocaleModule && PyModule_Check(uiScriptLocaleModule)) {
    if (PyObject_HasAttrString(uiScriptLocaleModule, "LoadLocaleData")) {
      PyObject *loadFunc =
          PyObject_GetAttrString(uiScriptLocaleModule, "LoadLocaleData");
      if (loadFunc && PyCallable_Check(loadFunc)) {
        PyObject *result = PyObject_CallObject(loadFunc, NULL);
        if (result)
          Py_DECREF(result);
        else {
          PyErr_Print();
          // TraceError("app.ReloadLocale: uiScriptLocale.LoadLocaleData()
          // failed");
        }
        Py_DECREF(loadFunc);
      }
    }
  }

  return Py_BuildValue("i", 1);
}

void initapp() {
  static PyMethodDef s_methods[] = {
      // TEXTTAIL_LIVINGTIME_CONTROL
      {"SetTextTailLivingTime", appSetTextTailLivingTime, METH_VARARGS},
      // END_OF_TEXTTAIL_LIVINGTIME_CONTROL

      {"EnablePerformanceTime", appEnablePerformanceTime, METH_VARARGS},
      {"SetHairColorEnable", appSetHairColorEnable, METH_VARARGS},

      {"SetArmorSpecularEnable", appSetArmorSpecularEnable, METH_VARARGS},
      {"SetWeaponSpecularEnable", appSetWeaponSpecularEnable, METH_VARARGS},
      {"SetSkillEffectUpgradeEnable", appSetSkillEffectUpgradeEnable,
       METH_VARARGS},
      {"SetTwoHandedWeaponAttSpeedDecreaseValue",
       SetTwoHandedWeaponAttSpeedDecreaseValue, METH_VARARGS},
      {"SetRideHorseEnable", appSetRideHorseEnable, METH_VARARGS},

      {"SetCameraMaxDistance", appSetCameraMaxDistance, METH_VARARGS},
      {"SetMinFog", appSetMinFog, METH_VARARGS},
      {"SetFrameSkip", appSetFrameSkip, METH_VARARGS},
      {"GetImageInfo", appGetImageInfo, METH_VARARGS},
      {"GetInfo", appGetInfo, METH_VARARGS},
      {"UpdateGame", appUpdateGame, METH_VARARGS},
      {"RenderGame", appRenderGame, METH_VARARGS},
      {"Loop", appLoop, METH_VARARGS},
      {"Create", appCreate, METH_VARARGS},
      {"Process", appProcess, METH_VARARGS},
      {"Exit", appExit, METH_VARARGS},
      {"Abort", appAbort, METH_VARARGS},
      {"SetMouseHandler", appSetMouseHandler, METH_VARARGS},
      {"IsExistFile", appIsExistFile, METH_VARARGS},
      {"GetFileList", appGetFileList, METH_VARARGS},

      {"SetCamera", appSetCamera, METH_VARARGS},
      {"GetCamera", appGetCamera, METH_VARARGS},
      {"GetCameraPitch", appGetCameraPitch, METH_VARARGS},
      {"GetCameraRotation", appGetCameraRotation, METH_VARARGS},
      {"GetTime", appGetTime, METH_VARARGS},
      {"GetGlobalTime", appGetGlobalTime, METH_VARARGS},
      {"GetGlobalTimeStamp", appGetGlobalTimeStamp, METH_VARARGS},
      {"GetUpdateFPS", appGetUpdateFPS, METH_VARARGS},
      {"GetRenderFPS", appGetRenderFPS, METH_VARARGS},
      {"RotateCamera", appRotateCamera, METH_VARARGS},
      {"PitchCamera", appPitchCamera, METH_VARARGS},
      {"ZoomCamera", appZoomCamera, METH_VARARGS},
      {"MovieRotateCamera", appMovieRotateCamera, METH_VARARGS},
      {"MoviePitchCamera", appMoviePitchCamera, METH_VARARGS},
      {"MovieZoomCamera", appMovieZoomCamera, METH_VARARGS},
      {"MovieResetCamera", appMovieResetCamera, METH_VARARGS},

      {"GetAvailableTextureMemory", appGetAvaiableTextureMememory,
       METH_VARARGS},
      {"GetRenderTime", appGetRenderTime, METH_VARARGS},
      {"GetUpdateTime", appGetUpdateTime, METH_VARARGS},
      {"GetLoad", appGetLoad, METH_VARARGS},
      {"GetFaceSpeed", appGetFaceSpeed, METH_VARARGS},
      {"GetFaceCount", appGetFaceCount, METH_VARARGS},
      {"SetFPS", appSetFPS, METH_VARARGS},
      {"SetGlobalCenterPosition", appSetGlobalCenterPosition, METH_VARARGS},
      {"SetCenterPosition", appSetCenterPosition, METH_VARARGS},
      {"GetCursorPosition", appGetCursorPosition, METH_VARARGS},

      {"GetRandom", appGetRandom, METH_VARARGS},
      {"RunPythonFile", appRunPythonFile, METH_VARARGS},
      {"IsWebPageMode", appIsWebPageMode, METH_VARARGS},
      {"ShowWebPage", appShowWebPage, METH_VARARGS},
      {"MoveWebPage", appMoveWebPage, METH_VARARGS},
      {"HideWebPage", appHideWebPage, METH_VARARGS},
      {"IsPressed", appIsPressed, METH_VARARGS},
      {"SetCursor", appSetCursor, METH_VARARGS},
      {"GetCursor", appGetCursor, METH_VARARGS},
      {"ShowCursor", appShowCursor, METH_VARARGS},
      {"HideCursor", appHideCursor, METH_VARARGS},
      {"IsShowCursor", appIsShowCursor, METH_VARARGS},
      {"IsLiarCursorOn", appIsLiarCursorOn, METH_VARARGS},
      {"SetSoftwareCursor", appSetSoftwareCursor, METH_VARARGS},
      {"SetHardwareCursor", appSetHardwareCursor, METH_VARARGS},

      {"SetConnectData", appSetConnectData, METH_VARARGS},
      {"GetConnectData", appGetConnectData, METH_VARARGS},

      {"GetRotatingDirection", appGetRotatingDirection, METH_VARARGS},
      {"GetDegreeDifference", appGetDegreeDifference, METH_VARARGS},
      {"Sleep", appSleep, METH_VARARGS},
      {"SetDefaultFontName", appSetDefaultFontName, METH_VARARGS},
      {"SetGuildSymbolPath", appSetGuildSymbolPath, METH_VARARGS},

      {"EnableSpecialCameraMode", appEnableSpecialCameraMode, METH_VARARGS},
      {"SetCameraSpeed", appSetCameraSpeed, METH_VARARGS},

      {"SaveCameraSetting", appSaveCameraSetting, METH_VARARGS},
      {"LoadCameraSetting", appLoadCameraSetting, METH_VARARGS},
      {"SetDefaultCamera", appSetDefaultCamera, METH_VARARGS},
      {"SetCameraSetting", appSetCameraSetting, METH_VARARGS},

      {"SetSightRange", appSetSightRange, METH_VARARGS},

      {"IsFileExist", appIsFileExist, METH_VARARGS},
      {"OpenTextFile", appOpenTextFile, METH_VARARGS},
      {"CloseTextFile", appCloseTextFile, METH_VARARGS},
      {"GetTextFileLineCount", appGetTextFileLineCount, METH_VARARGS},
      {"GetTextFileLine", appGetTextFileLine, METH_VARARGS},

      // LOCALE
      {"GetLocaleName", appGetLocaleName, METH_VARARGS},
      {"GetLocalePath", appGetLocalePath, METH_VARARGS},
      {"GetLocalePathCommon", appGetLocalePathCommon, METH_VARARGS},
      {"LoadLocaleData", appLoadLocaleData, METH_VARARGS},
      {"IsRTL", appIsRTL, METH_VARARGS},
      // END_OF_LOCALE

      {"SetControlFP", appSetControlFP, METH_VARARGS},
      {"SetSpecularSpeed", appSetSpecularSpeed, METH_VARARGS},

      {"testGetAccumulationTime", apptestGetAccumulationTime, METH_VARARGS},
      {"testResetAccumulationTime", apptestResetAccumulationTime, METH_VARARGS},
      {"testSetSpecularColor", apptestSetSpecularColor, METH_VARARGS},

      {"SetVisibleNotice", appSetVisibleNotice, METH_VARARGS},
      {"IsVisibleNotice", appIsVisibleNotice, METH_VARARGS},
      {"EnableTestServerFlag", appEnableTestServerFlag, METH_VARARGS},
      {"IsEnableTestServerFlag", appIsEnableTestServerFlag, METH_VARARGS},

      {"SetGuildMarkPath", appSetGuildMarkPath, METH_VARARGS},

      {"OnLogoUpdate", appLogoUpdate, METH_VARARGS},
      {"OnLogoRender", appLogoRender, METH_VARARGS},
      {"OnLogoOpen", appLogoOpen, METH_VARARGS},
      {"OnLogoClose", appLogoClose, METH_VARARGS},

      {"ReloadLocaleConfig", appReloadLocaleConfig, METH_VARARGS},
      {"ReloadLocale", appReloadLocale, METH_VARARGS},

      {NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("app", s_methods);

  PyModule_AddIntConstant(poModule, "INFO_ITEM", CPythonApplication::INFO_ITEM);
  PyModule_AddIntConstant(poModule, "INFO_ACTOR",
                          CPythonApplication::INFO_ACTOR);
  PyModule_AddIntConstant(poModule, "INFO_EFFECT",
                          CPythonApplication::INFO_EFFECT);
  PyModule_AddIntConstant(poModule, "INFO_TEXTTAIL",
                          CPythonApplication::INFO_TEXTTAIL);

  PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_SAME",
                          DEGREE_DIRECTION_SAME);
  PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_RIGHT",
                          DEGREE_DIRECTION_RIGHT);
  PyModule_AddIntConstant(poModule, "DEGREE_DIRECTION_LEFT",
                          DEGREE_DIRECTION_LEFT);

  PyModule_AddIntConstant(poModule, "VK_LEFT", VK_LEFT);
  PyModule_AddIntConstant(poModule, "VK_RIGHT", VK_RIGHT);
  PyModule_AddIntConstant(poModule, "VK_UP", VK_UP);
  PyModule_AddIntConstant(poModule, "VK_DOWN", VK_DOWN);
  PyModule_AddIntConstant(poModule, "VK_HOME", VK_HOME);
  PyModule_AddIntConstant(poModule, "VK_END", VK_END);
  PyModule_AddIntConstant(poModule, "VK_DELETE", VK_DELETE);

  PyModule_AddIntConstant(poModule, "DIK_ESCAPE", DIK_ESCAPE);
  PyModule_AddIntConstant(poModule, "DIK_ESC", DIK_ESCAPE); // 편의를 위해
  PyModule_AddIntConstant(poModule, "DIK_1", DIK_1);
  PyModule_AddIntConstant(poModule, "DIK_2", DIK_2);
  PyModule_AddIntConstant(poModule, "DIK_3", DIK_3);
  PyModule_AddIntConstant(poModule, "DIK_4", DIK_4);
  PyModule_AddIntConstant(poModule, "DIK_5", DIK_5);
  PyModule_AddIntConstant(poModule, "DIK_6", DIK_6);
  PyModule_AddIntConstant(poModule, "DIK_7", DIK_7);
  PyModule_AddIntConstant(poModule, "DIK_8", DIK_8);
  PyModule_AddIntConstant(poModule, "DIK_9", DIK_9);
  PyModule_AddIntConstant(poModule, "DIK_0", DIK_0);
  PyModule_AddIntConstant(poModule, "DIK_MINUS",
                          DIK_MINUS); /* - on main keyboard */
  PyModule_AddIntConstant(poModule, "DIK_EQUALS", DIK_EQUALS);
  PyModule_AddIntConstant(poModule, "DIK_BACK", DIK_BACK); /* backspace */
  PyModule_AddIntConstant(poModule, "DIK_TAB", DIK_TAB);
  PyModule_AddIntConstant(poModule, "DIK_Q", DIK_Q);
  PyModule_AddIntConstant(poModule, "DIK_W", DIK_W);
  PyModule_AddIntConstant(poModule, "DIK_E", DIK_E);
  PyModule_AddIntConstant(poModule, "DIK_R", DIK_R);
  PyModule_AddIntConstant(poModule, "DIK_T", DIK_T);
  PyModule_AddIntConstant(poModule, "DIK_Y", DIK_Y);
  PyModule_AddIntConstant(poModule, "DIK_U", DIK_U);
  PyModule_AddIntConstant(poModule, "DIK_I", DIK_I);
  PyModule_AddIntConstant(poModule, "DIK_O", DIK_O);
  PyModule_AddIntConstant(poModule, "DIK_P", DIK_P);
  PyModule_AddIntConstant(poModule, "DIK_LBRACKET", DIK_LBRACKET);
  PyModule_AddIntConstant(poModule, "DIK_RBRACKET", DIK_RBRACKET);
  PyModule_AddIntConstant(poModule, "DIK_RETURN",
                          DIK_RETURN); /* Enter on main keyboard */
  PyModule_AddIntConstant(poModule, "DIK_LCONTROL", DIK_LCONTROL);
  PyModule_AddIntConstant(poModule, "DIK_A", DIK_A);
  PyModule_AddIntConstant(poModule, "DIK_S", DIK_S);
  PyModule_AddIntConstant(poModule, "DIK_D", DIK_D);
  PyModule_AddIntConstant(poModule, "DIK_F", DIK_F);
  PyModule_AddIntConstant(poModule, "DIK_G", DIK_G);
  PyModule_AddIntConstant(poModule, "DIK_H", DIK_H);
  PyModule_AddIntConstant(poModule, "DIK_J", DIK_J);
  PyModule_AddIntConstant(poModule, "DIK_K", DIK_K);
  PyModule_AddIntConstant(poModule, "DIK_L", DIK_L);
  PyModule_AddIntConstant(poModule, "DIK_SEMICOLON", DIK_SEMICOLON);
  PyModule_AddIntConstant(poModule, "DIK_APOSTROPHE", DIK_APOSTROPHE);
  PyModule_AddIntConstant(poModule, "DIK_GRAVE", DIK_GRAVE); /* accent grave */
  PyModule_AddIntConstant(poModule, "DIK_LSHIFT", DIK_LSHIFT);
  PyModule_AddIntConstant(poModule, "DIK_BACKSLASH", DIK_BACKSLASH);
  PyModule_AddIntConstant(poModule, "DIK_Z", DIK_Z);
  PyModule_AddIntConstant(poModule, "DIK_X", DIK_X);
  PyModule_AddIntConstant(poModule, "DIK_C", DIK_C);
  PyModule_AddIntConstant(poModule, "DIK_V", DIK_V);
  PyModule_AddIntConstant(poModule, "DIK_B", DIK_B);
  PyModule_AddIntConstant(poModule, "DIK_N", DIK_N);
  PyModule_AddIntConstant(poModule, "DIK_M", DIK_M);
  PyModule_AddIntConstant(poModule, "DIK_COMMA", DIK_COMMA);
  PyModule_AddIntConstant(poModule, "DIK_PERIOD",
                          DIK_PERIOD); /* . on main keyboard */
  PyModule_AddIntConstant(poModule, "DIK_SLASH",
                          DIK_SLASH); /* / on main keyboard */
  PyModule_AddIntConstant(poModule, "DIK_RSHIFT", DIK_RSHIFT);
  PyModule_AddIntConstant(poModule, "DIK_MULTIPLY",
                          DIK_MULTIPLY); /* * on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_LALT", DIK_LMENU); /* left Alt */
  PyModule_AddIntConstant(poModule, "DIK_SPACE", DIK_SPACE);
  PyModule_AddIntConstant(poModule, "DIK_CAPITAL", DIK_CAPITAL);
  PyModule_AddIntConstant(poModule, "DIK_F1", DIK_F1);
  PyModule_AddIntConstant(poModule, "DIK_F2", DIK_F2);
  PyModule_AddIntConstant(poModule, "DIK_F3", DIK_F3);
  PyModule_AddIntConstant(poModule, "DIK_F4", DIK_F4);
  PyModule_AddIntConstant(poModule, "DIK_F5", DIK_F5);
  PyModule_AddIntConstant(poModule, "DIK_F6", DIK_F6);
  PyModule_AddIntConstant(poModule, "DIK_F7", DIK_F7);
  PyModule_AddIntConstant(poModule, "DIK_F8", DIK_F8);
  PyModule_AddIntConstant(poModule, "DIK_F9", DIK_F9);
  PyModule_AddIntConstant(poModule, "DIK_F10", DIK_F10);
  PyModule_AddIntConstant(poModule, "DIK_NUMLOCK", DIK_NUMLOCK);
  PyModule_AddIntConstant(poModule, "DIK_SCROLL", DIK_SCROLL); /* Scroll Lock */
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD7", DIK_NUMPAD7);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD8", DIK_NUMPAD8);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD9", DIK_NUMPAD9);
  PyModule_AddIntConstant(poModule, "DIK_SUBTRACT",
                          DIK_SUBTRACT); /* - on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD4", DIK_NUMPAD4);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD5", DIK_NUMPAD5);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD6", DIK_NUMPAD6);
  PyModule_AddIntConstant(poModule, "DIK_ADD",
                          DIK_ADD); /* + on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD1", DIK_NUMPAD1);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD2", DIK_NUMPAD2);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD3", DIK_NUMPAD3);
  PyModule_AddIntConstant(poModule, "DIK_NUMPAD0", DIK_NUMPAD0);
  PyModule_AddIntConstant(poModule, "DIK_DECIMAL",
                          DIK_DECIMAL); /* . on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_F11", DIK_F11);
  PyModule_AddIntConstant(poModule, "DIK_F12", DIK_F12);
  PyModule_AddIntConstant(poModule, "DIK_NEXTTRACK",
                          DIK_NEXTTRACK); /* Next Track */
  PyModule_AddIntConstant(poModule, "DIK_NUMPADENTER",
                          DIK_NUMPADENTER); /* Enter on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_RCONTROL", DIK_RCONTROL);
  PyModule_AddIntConstant(poModule, "DIK_MUTE", DIK_MUTE); /* Mute */
  PyModule_AddIntConstant(poModule, "DIK_CALCULATOR",
                          DIK_CALCULATOR); /* Calculator */
  PyModule_AddIntConstant(poModule, "DIK_PLAYPAUSE",
                          DIK_PLAYPAUSE); /* Play / Pause */
  PyModule_AddIntConstant(poModule, "DIK_MEDIASTOP",
                          DIK_MEDIASTOP); /* Media Stop */
  PyModule_AddIntConstant(poModule, "DIK_VOLUMEDOWN",
                          DIK_VOLUMEDOWN); /* Volume - */
  PyModule_AddIntConstant(poModule, "DIK_VOLUMEUP",
                          DIK_VOLUMEUP);                         /* Volume + */
  PyModule_AddIntConstant(poModule, "DIK_WEBHOME", DIK_WEBHOME); /* Web home */
  PyModule_AddIntConstant(poModule, "DIK_NUMPADCOMMA",
                          DIK_NUMPADCOMMA); /* , on numeric keypad (NEC PC98) */
  PyModule_AddIntConstant(poModule, "DIK_DIVIDE",
                          DIK_DIVIDE); /* / on numeric keypad */
  PyModule_AddIntConstant(poModule, "DIK_SYSRQ", DIK_SYSRQ);
  PyModule_AddIntConstant(poModule, "DIK_RALT", DIK_RMENU);  /* right Alt */
  PyModule_AddIntConstant(poModule, "DIK_PAUSE", DIK_PAUSE); /* Pause */
  PyModule_AddIntConstant(poModule, "DIK_HOME",
                          DIK_HOME); /* Home on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_UP",
                          DIK_UP); /* UpArrow on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_PGUP",
                          DIK_PRIOR); /* PgUp on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_LEFT",
                          DIK_LEFT); /* LeftArrow on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_RIGHT",
                          DIK_RIGHT); /* RightArrow on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_END",
                          DIK_END); /* End on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_DOWN",
                          DIK_DOWN); /* DownArrow on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_PGDN",
                          DIK_NEXT); /* PgDn on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_INSERT",
                          DIK_INSERT); /* Insert on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_DELETE",
                          DIK_DELETE); /* Delete on arrow keypad */
  PyModule_AddIntConstant(poModule, "DIK_LWIN",
                          DIK_LWIN); /* Left Windows key */
  PyModule_AddIntConstant(poModule, "DIK_RWIN",
                          DIK_RWIN); /* Right Windows key */
  PyModule_AddIntConstant(poModule, "DIK_APPS", DIK_APPS); /* AppMenu key */

  // Cursor
  PyModule_AddIntConstant(poModule, "NORMAL",
                          CPythonApplication::CURSOR_SHAPE_NORMAL);
  PyModule_AddIntConstant(poModule, "ATTACK",
                          CPythonApplication::CURSOR_SHAPE_ATTACK);
  PyModule_AddIntConstant(poModule, "TARGET",
                          CPythonApplication::CURSOR_SHAPE_TARGET);
  PyModule_AddIntConstant(poModule, "TALK",
                          CPythonApplication::CURSOR_SHAPE_TALK);
  PyModule_AddIntConstant(poModule, "CANT_GO",
                          CPythonApplication::CURSOR_SHAPE_CANT_GO);
  PyModule_AddIntConstant(poModule, "PICK",
                          CPythonApplication::CURSOR_SHAPE_PICK);

  PyModule_AddIntConstant(poModule, "DOOR",
                          CPythonApplication::CURSOR_SHAPE_DOOR);
  PyModule_AddIntConstant(poModule, "CHAIR",
                          CPythonApplication::CURSOR_SHAPE_CHAIR);
  PyModule_AddIntConstant(poModule, "MAGIC",
                          CPythonApplication::CURSOR_SHAPE_MAGIC);
  PyModule_AddIntConstant(poModule, "BUY",
                          CPythonApplication::CURSOR_SHAPE_BUY);
  PyModule_AddIntConstant(poModule, "SELL",
                          CPythonApplication::CURSOR_SHAPE_SELL);

  PyModule_AddIntConstant(poModule, "CAMERA_ROTATE",
                          CPythonApplication::CURSOR_SHAPE_CAMERA_ROTATE);
  PyModule_AddIntConstant(poModule, "HSIZE",
                          CPythonApplication::CURSOR_SHAPE_HSIZE);
  PyModule_AddIntConstant(poModule, "VSIZE",
                          CPythonApplication::CURSOR_SHAPE_VSIZE);
  PyModule_AddIntConstant(poModule, "HVSIZE",
                          CPythonApplication::CURSOR_SHAPE_HVSIZE);

  PyModule_AddIntConstant(poModule, "CAMERA_TO_POSITIVE",
                          CPythonApplication::CAMERA_TO_POSITIVE);
  PyModule_AddIntConstant(poModule, "CAMERA_TO_NEGATIVE",
                          CPythonApplication::CAMERA_TO_NEGITIVE);
  PyModule_AddIntConstant(poModule, "CAMERA_STOP",
                          CPythonApplication::CAMERA_STOP);

#ifdef ENABLE_COSTUME_SYSTEM
  PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", 1);
#else
  PyModule_AddIntConstant(poModule, "ENABLE_COSTUME_SYSTEM", 0);
#endif

#ifdef ENABLE_ENERGY_SYSTEM
  PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", 1);
#else
  PyModule_AddIntConstant(poModule, "ENABLE_ENERGY_SYSTEM", 0);
#endif

#ifdef ENABLE_DRAGON_SOUL_SYSTEM
  PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", 1);
#else
  PyModule_AddIntConstant(poModule, "ENABLE_DRAGON_SOUL_SYSTEM", 0);
#endif

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
  PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", 1);
#else
  PyModule_AddIntConstant(poModule, "ENABLE_NEW_EQUIPMENT_SYSTEM", 0);
#endif
}
