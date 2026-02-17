#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonBackground.h"
#include "EterLib/StateManager.h"
#include "GameLib/MapOutDoor.h"

PyObject *backgroundIsSoftwareTiling(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  return Py_BuildValue("i", rkBG.IsSoftwareTilingEnable());
}

PyObject *backgroundEnableSoftwareTiling(PyObject *poSelf, PyObject *poArgs) {
  int nIsEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nIsEnable))
    return Py_BadArgument();

  bool isEnable = nIsEnable ? true : false;

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.ReserveSoftwareTilingEnable(isEnable);

  CPythonSystem &rkSystem = CPythonSystem::Instance();
  rkSystem.SetSoftwareTiling(isEnable);
  return Py_BuildNone();
}

PyObject *backgroundEnableSnow(PyObject *poSelf, PyObject *poArgs) {
  int nIsEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &nIsEnable))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  if (nIsEnable)
    rkBG.EnableSnowEnvironment();
  else
    rkBG.DisableSnowEnvironment();

  return Py_BuildNone();
}

PyObject *backgroundLoadMap(PyObject *poSelf, PyObject *poArgs) {
  char *pszMapPathName;

  if (!PyTuple_GetString(poArgs, 0, &pszMapPathName))
    return Py_BadArgument();

  float x, y, z;

  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BadArgument();

  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BadArgument();

  if (!PyTuple_GetFloat(poArgs, 3, &z))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.LoadMap(pszMapPathName, x, y, z);

  // #ifdef _DEBUG
  //	CMapOutdoor& rkMap=rkBG.GetMapOutdoorRef();
  //	rkMap.EnablePortal(TRUE);
  //	rkBG.EnableTerrainOnlyForHeight();
  // #endif

  return Py_BuildNone();
}

PyObject *backgroundDestroy(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetShadowLevel(CPythonBackground::SHADOW_NONE);
  rkBG.Destroy();
  return Py_BuildNone();
}

PyObject *backgroundRegisterEnvironmentData(PyObject *poSelf,
                                            PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BadArgument();

  char *pszEnvironmentFileName;
  if (!PyTuple_GetString(poArgs, 1, &pszEnvironmentFileName))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  if (!rkBG.RegisterEnvironmentData(iIndex, pszEnvironmentFileName)) {
    TraceError("background.RegisterEnvironmentData(iIndex=%d, "
               "szEnvironmentFileName=%s)",
               iIndex, pszEnvironmentFileName);

    // TODO:
    // 디폴트 환경 설정 작업을 해주자
  }

  return Py_BuildNone();
}

PyObject *backgroundSetEnvironmentData(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BadArgument();

  const TEnvironmentData *c_pEnvironmenData;

  CPythonBackground &rkBG = CPythonBackground::Instance();
  if (rkBG.GetEnvironmentData(iIndex, &c_pEnvironmenData))
    rkBG.ResetEnvironmentDataPtr(c_pEnvironmenData);

  return Py_BuildNone();
}

PyObject *backgroundGetCurrentMapName(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  return Py_BuildValue("s", rkBG.GetWarpMapName());
}

PyObject *backgroundGetPickingPoint(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  TPixelPosition kPPosPicked(0.0f, 0.0f, 0.0f);
  if (rkBG.GetPickingPoint(&kPPosPicked)) {
    kPPosPicked.y = -kPPosPicked.y;
  }
  return Py_BuildValue("fff", kPPosPicked.x, kPPosPicked.y, kPPosPicked.z);
}

PyObject *backgroundBeginEnvironment(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.BeginEnvironment();
  return Py_BuildNone();
}

PyObject *backgroundEndEnvironemt(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.EndEnvironment();
  return Py_BuildNone();
}

PyObject *backgroundSetCharacterDirLight(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetCharacterDirLight();
  return Py_BuildNone();
}

PyObject *backgroundSetBackgroundDirLight(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetBackgroundDirLight();
  return Py_BuildNone();
}

PyObject *backgroundInitialize(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.Create();
  return Py_BuildNone();
}

PyObject *backgroundUpdate(PyObject *poSelf, PyObject *poArgs) {
  float fCameraX;
  if (!PyTuple_GetFloat(poArgs, 0, &fCameraX))
    return Py_BadArgument();

  float fCameraY;
  if (!PyTuple_GetFloat(poArgs, 1, &fCameraY))
    return Py_BadArgument();

  float fCameraZ;
  if (!PyTuple_GetFloat(poArgs, 2, &fCameraZ))
    return Py_BadArgument();

  CPythonBackground::Instance().Update(fCameraX, fCameraY, fCameraZ);
  return Py_BuildNone();
}

PyObject *backgroundRender(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().Render();
  return Py_BuildNone();
}

PyObject *backgroundRenderPCBlocker(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderPCBlocker();
  return Py_BuildNone();
}

PyObject *backgroundRenderCollision(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderCollision();
  return Py_BuildNone();
}

PyObject *backgroundRenderSky(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderSky();
  return Py_BuildNone();
}

PyObject *backgroundRenderCloud(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderCloud();
  return Py_BuildNone();
}

PyObject *backgroundRenderWater(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderWater();
  return Py_BuildNone();
}

PyObject *backgroundRenderEffect(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderEffect();
  return Py_BuildNone();
}

PyObject *backgroundRenderBeforeLensFlare(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderBeforeLensFlare();
  return Py_BuildNone();
}

PyObject *backgroundRenderAfterLensFlare(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().RenderAfterLensFlare();
  return Py_BuildNone();
}

PyObject *backgroundRenderCharacterShadowToTexture(PyObject *poSelf,
                                                   PyObject *poArgs) {
  CPythonBackground::Instance().RenderCharacterShadowToTexture();
  return Py_BuildNone();
}

PyObject *backgroundRenderDungeon(PyObject *poSelf, PyObject *poArgs) {
  assert(!"background.RenderDungeon() - 사용하지 않는 함수입니다 - [levites]");
  return Py_BuildNone();
}

PyObject *backgroundGetHeight(PyObject *poSelf, PyObject *poArgs) {
  float fx;
  if (!PyTuple_GetFloat(poArgs, 0, &fx))
    return Py_BadArgument();

  float fy;
  if (!PyTuple_GetFloat(poArgs, 1, &fy))
    return Py_BadArgument();

  float fz = CPythonBackground::Instance().GetHeight(fx, fy);
  return Py_BuildValue("f", fz);
}

PyObject *backgroundGetRenderedSplatNum(PyObject *poSelf, PyObject *poArgs) {
  int iPatch;
  int iSplat;
  float fSplatRatio;

  std::vector<int> &aTextureNumVector =
      CPythonBackground::Instance().GetRenderedSplatNum(&iPatch, &iSplat,
                                                        &fSplatRatio);

  char szOutput[MAX_PATH] = "";
  int iOutput = 0;
  for (std::vector<int>::iterator it = aTextureNumVector.begin();
       it != aTextureNumVector.end(); it++) {
    iOutput +=
        snprintf(szOutput + iOutput, sizeof(szOutput) - iOutput, "%d ", *it);
  }
  // std::copy(aTextureNumVector.begin(),aTextureNumVector.end(),std::ostream_iterator<int>(ostr,"
  // "));

  return Py_BuildValue("iifs", iPatch, iSplat, fSplatRatio, szOutput);
}

PyObject *backgroundGetRenderedGTINum(PyObject *poSelf, PyObject *poArgs) {
  DWORD dwGraphicThingInstanceNum;
  DWORD dwCRCNum;

  /*CArea::TCRCWithNumberVector & rCRCWithNumberVector = */
  CPythonBackground::Instance().GetRenderedGraphicThingInstanceNum(
      &dwGraphicThingInstanceNum, &dwCRCNum);

  /*
          std::ostringstream ostr;

          std::for_each(rCRCWithNumberVector.begin(),rCRCWithNumberVector.end(),std::ostream_iterator<CArea::TCRCWithNumberVector>(ostr,"
     "));
  */

  return Py_BuildValue("ii", dwGraphicThingInstanceNum, dwCRCNum);
}

PyObject *backgroundGetRenderShadowTime(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground &rkBG = CPythonBackground::Instance();
  return Py_BuildValue("i", rkBG.GetRenderShadowTime());
}

PyObject *backgroundGetShadowMapcolor(PyObject *poSelf, PyObject *poArgs) {
  float fx;
  if (!PyTuple_GetFloat(poArgs, 0, &fx))
    return Py_BadArgument();

  float fy;
  if (!PyTuple_GetFloat(poArgs, 1, &fy))
    return Py_BadArgument();

  DWORD dwColor = CPythonBackground::Instance().GetShadowMapColor(fx, fy);
  return Py_BuildValue("i", dwColor);
}

PyObject *backgroundSetShadowLevel(PyObject *poSelf, PyObject *poArgs) {
  int iLevel;

  if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetShadowLevel(iLevel);
  return Py_BuildNone();
}

PyObject *backgroundSetVisiblePart(PyObject *poSelf, PyObject *poArgs) {
  int ePart;
  if (!PyTuple_GetInteger(poArgs, 0, &ePart))
    return Py_BadArgument();

  int isVisible;
  if (!PyTuple_GetInteger(poArgs, 1, &isVisible))
    return Py_BadArgument();

  if (ePart >= CMapOutdoor::PART_NUM)
    return Py_BuildException("ePart(%d)<background.PART_NUM(%d)", ePart,
                             CMapOutdoor::PART_NUM);

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetVisiblePart(ePart, isVisible ? true : false);

  return Py_BuildNone();
}

PyObject *backgroundSetSpaltLimit(PyObject *poSelf, PyObject *poArgs) {
  int iSplatNum;
  if (!PyTuple_GetInteger(poArgs, 0, &iSplatNum))
    return Py_BadArgument();

  if (iSplatNum < 0)
    return Py_BuildException("background.SetSplatLimit(iSplatNum(%d)>=0)",
                             iSplatNum);

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetSplatLimit(iSplatNum);

  return Py_BuildNone();
}

PyObject *backgroundSelectViewDistanceNum(PyObject *poSelf, PyObject *poArgs) {
  int iNum;

  if (!PyTuple_GetInteger(poArgs, 0, &iNum))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SelectViewDistanceNum(iNum);

  return Py_BuildNone();
}

PyObject *backgroundSetViewDistanceSet(PyObject *poSelf, PyObject *poArgs) {
  int iNum;
  if (!PyTuple_GetInteger(poArgs, 0, &iNum))
    return Py_BadArgument();

  float fFarClip;
  if (!PyTuple_GetFloat(poArgs, 1, &fFarClip))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.SetViewDistanceSet(iNum, fFarClip);
  return Py_BuildNone();
}

PyObject *backgroundGetFarClip(PyObject *poSelf, PyObject *poArgs) {
  float fFarClip = CPythonBackground::Instance().GetFarClip();
  return Py_BuildValue("f", fFarClip);
}

PyObject *backgroundGetDistanceSetInfo(PyObject *poSelf, PyObject *poArgs) {
  int iNum;
  float fStart, fEnd, fFarClip;
  CPythonBackground::Instance().GetDistanceSetInfo(&iNum, &fStart, &fEnd,
                                                   &fFarClip);
  return Py_BuildValue("ifff", iNum, fStart, fEnd, fFarClip);
}

PyObject *backgroundSetBGLoading(PyObject *poSelf, PyObject *poArgs) {
  bool bBGLoading;
  if (!PyTuple_GetBoolean(poArgs, 0, &bBGLoading))
    return Py_BadArgument();

  // CPythonBackground::Instance().BGLoadingEnable(bBGLoading);
  return Py_BuildNone();
}

PyObject *backgroundSetRenderSort(PyObject *poSelf, PyObject *poArgs) {
  int eSort;
  if (!PyTuple_GetInteger(poArgs, 0, &eSort))
    return Py_BadArgument();

  CPythonBackground::Instance().SetTerrainRenderSort(
      (CMapOutdoor::ETerrainRenderSort)eSort);
  return Py_BuildNone();
}

PyObject *backgroundSetTransparentTree(PyObject *poSelf, PyObject *poArgs) {
  int bTransparent;
  if (!PyTuple_GetInteger(poArgs, 0, &bTransparent))
    return Py_BadArgument();

  CPythonBackground::Instance().SetTransparentTree(bTransparent ? true : false);
  return Py_BuildNone();
}

PyObject *backgroundGlobalPositionToLocalPosition(PyObject *poSelf,
                                                  PyObject *poArgs) {
  int iX;
  if (!PyTuple_GetInteger(poArgs, 0, &iX))
    return Py_BadArgument();

  int iY;
  if (!PyTuple_GetInteger(poArgs, 1, &iY))
    return Py_BadArgument();

  int32_t lX = iX;
  int32_t lY = iY;
  CPythonBackground &rkBG = CPythonBackground::Instance();
  rkBG.GlobalPositionToLocalPosition(lX, lY);

  return Py_BuildValue("ii", lX, lY);
}

PyObject *backgroundGlobalPositionToMapInfo(PyObject *poSelf,
                                            PyObject *poArgs) {
  int iX;
  if (!PyTuple_GetInteger(poArgs, 0, &iX))
    return Py_BadArgument();

  int iY;
  if (!PyTuple_GetInteger(poArgs, 1, &iY))
    return Py_BadArgument();

  CPythonBackground &rkBG = CPythonBackground::Instance();
  CPythonBackground::TMapInfo *pkMapInfo = rkBG.GlobalPositionToMapInfo(iX, iY);

  if (pkMapInfo)
    return Py_BuildValue("sii", pkMapInfo->m_strName.c_str(),
                         pkMapInfo->m_dwBaseX, pkMapInfo->m_dwBaseY);
  else
    return Py_BuildValue("sii", "", 0, 0);
}

PyObject *backgroundWarpTest(PyObject *poSelf, PyObject *poArgs) {
  int iX;
  if (!PyTuple_GetInteger(poArgs, 0, &iX))
    return Py_BadArgument();

  int iY;
  if (!PyTuple_GetInteger(poArgs, 1, &iY))
    return Py_BadArgument();

  CPythonBackground::Instance().Warp((DWORD)iX * 100, (DWORD)iY * 100);
  return Py_BuildNone();
}

PyObject *backgroundSetXMasTree(PyObject *poSelf, PyObject *poArgs) {
  int iGrade;
  if (!PyTuple_GetInteger(poArgs, 0, &iGrade))
    return Py_BadArgument();

  CPythonBackground::Instance().SetXMaxTree(iGrade);
  return Py_BuildNone();
}

PyObject *backgroundRegisterDungeonMapName(PyObject *poSelf, PyObject *poArgs) {
  char *szName;
  if (!PyTuple_GetString(poArgs, 0, &szName))
    return Py_BadArgument();

  CPythonBackground::Instance().RegisterDungeonMapName(szName);
  return Py_BuildNone();
}

PyObject *backgroundVisibleGuildArea(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().VisibleGuildArea();
  return Py_BuildNone();
}

PyObject *backgroundDisableGuildArea(PyObject *poSelf, PyObject *poArgs) {
  CPythonBackground::Instance().DisableGuildArea();
  return Py_BuildNone();
}

void initBackground() {
  static PyMethodDef s_methods[] = {
      {"IsSoftwareTiling", backgroundIsSoftwareTiling, METH_VARARGS},
      {"EnableSoftwareTiling", backgroundEnableSoftwareTiling, METH_VARARGS},
      {"EnableSnow", backgroundEnableSnow, METH_VARARGS},
      {"GlobalPositionToLocalPosition", backgroundGlobalPositionToLocalPosition,
       METH_VARARGS},
      {"GlobalPositionToMapInfo", backgroundGlobalPositionToMapInfo,
       METH_VARARGS},
      {"GetRenderShadowTime", backgroundGetRenderShadowTime, METH_VARARGS},
      {"LoadMap", backgroundLoadMap, METH_VARARGS},
      {"Destroy", backgroundDestroy, METH_VARARGS},
      {"RegisterEnvironmentData", backgroundRegisterEnvironmentData,
       METH_VARARGS},
      {"SetEnvironmentData", backgroundSetEnvironmentData, METH_VARARGS},
      {"GetCurrentMapName", backgroundGetCurrentMapName, METH_VARARGS},
      {"GetPickingPoint", backgroundGetPickingPoint, METH_VARARGS},

      {"BeginEnvironment", backgroundBeginEnvironment, METH_VARARGS},
      {"EndEnvironment", backgroundEndEnvironemt, METH_VARARGS},
      {"SetCharacterDirLight", backgroundSetCharacterDirLight, METH_VARARGS},
      {"SetBackgroundDirLight", backgroundSetBackgroundDirLight, METH_VARARGS},

      {"Initialize", backgroundInitialize, METH_VARARGS},
      {"Update", backgroundUpdate, METH_VARARGS},
      {"Render", backgroundRender, METH_VARARGS},
      {"RenderPCBlocker", backgroundRenderPCBlocker, METH_VARARGS},
      {"RenderCollision", backgroundRenderCollision, METH_VARARGS},
      {"RenderSky", backgroundRenderSky, METH_VARARGS},
      {"RenderCloud", backgroundRenderCloud, METH_VARARGS},
      {"RenderWater", backgroundRenderWater, METH_VARARGS},
      {"RenderEffect", backgroundRenderEffect, METH_VARARGS},
      {"RenderBeforeLensFlare", backgroundRenderBeforeLensFlare, METH_VARARGS},
      {"RenderAfterLensFlare", backgroundRenderAfterLensFlare, METH_VARARGS},
      {"RenderCharacterShadowToTexture",
       backgroundRenderCharacterShadowToTexture, METH_VARARGS},
      {"RenderDungeon", backgroundRenderDungeon, METH_VARARGS},
      {"GetHeight", backgroundGetHeight, METH_VARARGS},

      {"SetShadowLevel", backgroundSetShadowLevel, METH_VARARGS},

      {"SetVisiblePart", backgroundSetVisiblePart, METH_VARARGS},
      {"GetShadowMapColor", backgroundGetShadowMapcolor, METH_VARARGS},
      {"SetSplatLimit", backgroundSetSpaltLimit, METH_VARARGS},
      {"GetRenderedSplatNum", backgroundGetRenderedSplatNum, METH_VARARGS},
      {"GetRenderedGraphicThingInstanceNum", backgroundGetRenderedGTINum,
       METH_VARARGS},
      {"SelectViewDistanceNum", backgroundSelectViewDistanceNum, METH_VARARGS},
      {"SetViewDistanceSet", backgroundSetViewDistanceSet, METH_VARARGS},
      {"GetFarClip", backgroundGetFarClip, METH_VARARGS},
      {"GetDistanceSetInfo", backgroundGetDistanceSetInfo, METH_VARARGS},
      {"SetBGLoading", backgroundSetBGLoading, METH_VARARGS},
      {"SetRenderSort", backgroundSetRenderSort, METH_VARARGS},
      {"SetTransparentTree", backgroundSetTransparentTree, METH_VARARGS},
      {"SetXMasTree", backgroundSetXMasTree, METH_VARARGS},
      {"RegisterDungeonMapName", backgroundRegisterDungeonMapName,
       METH_VARARGS},

      {"VisibleGuildArea", backgroundVisibleGuildArea, METH_VARARGS},
      {"DisableGuildArea", backgroundDisableGuildArea, METH_VARARGS},

      {"WarpTest", backgroundWarpTest, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("background", s_methods);

  PyModule_AddIntConstant(poModule, "PART_SKY", CMapOutdoor::PART_SKY);
  PyModule_AddIntConstant(poModule, "PART_TREE", CMapOutdoor::PART_TREE);
  PyModule_AddIntConstant(poModule, "PART_CLOUD", CMapOutdoor::PART_CLOUD);
  PyModule_AddIntConstant(poModule, "PART_WATER", CMapOutdoor::PART_WATER);
  PyModule_AddIntConstant(poModule, "PART_OBJECT", CMapOutdoor::PART_OBJECT);
  PyModule_AddIntConstant(poModule, "PART_TERRAIN", CMapOutdoor::PART_TERRAIN);

  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DEFAULT",
                          CSkyObject::SKY_RENDER_MODE_DEFAULT);
  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_DIFFUSE",
                          CSkyObject::SKY_RENDER_MODE_DIFFUSE);
  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_TEXTURE",
                          CSkyObject::SKY_RENDER_MODE_TEXTURE);
  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE",
                          CSkyObject::SKY_RENDER_MODE_MODULATE);
  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE2X",
                          CSkyObject::SKY_RENDER_MODE_MODULATE2X);
  PyModule_AddIntConstant(poModule, "SKY_RENDER_MODE_MODULATE4X",
                          CSkyObject::SKY_RENDER_MODE_MODULATE4X);

  PyModule_AddIntConstant(poModule, "SHADOW_NONE",
                          CPythonBackground::SHADOW_NONE);
  PyModule_AddIntConstant(poModule, "SHADOW_GROUND",
                          CPythonBackground::SHADOW_GROUND);
  PyModule_AddIntConstant(poModule, "SHADOW_GROUND_AND_SOLO",
                          CPythonBackground::SHADOW_GROUND_AND_SOLO);
  PyModule_AddIntConstant(poModule, "SHADOW_ALL",
                          CPythonBackground::SHADOW_ALL);
  PyModule_AddIntConstant(poModule, "SHADOW_ALL_HIGH",
                          CPythonBackground::SHADOW_ALL_HIGH);
  PyModule_AddIntConstant(poModule, "SHADOW_ALL_MAX",
                          CPythonBackground::SHADOW_ALL_MAX);

  PyModule_AddIntConstant(poModule, "DISTANCE0", CPythonBackground::DISTANCE0);
  PyModule_AddIntConstant(poModule, "DISTANCE1", CPythonBackground::DISTANCE1);
  PyModule_AddIntConstant(poModule, "DISTANCE2", CPythonBackground::DISTANCE2);
  PyModule_AddIntConstant(poModule, "DISTANCE3", CPythonBackground::DISTANCE3);
  PyModule_AddIntConstant(poModule, "DISTANCE4", CPythonBackground::DISTANCE4);

  PyModule_AddIntConstant(poModule, "DISTANCE_SORT",
                          CMapOutdoor::DISTANCE_SORT);
  PyModule_AddIntConstant(poModule, "TEXTURE_SORT", CMapOutdoor::TEXTURE_SORT);
}
