#include "StdAfx.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "InstanceBase.h"
#include "GameLib/RaceManager.h"

//////////////////////////////////////////////////////////////////////////
// RaceData 관련 시작
//////////////////////////////////////////////////////////////////////////

PyObject *chrmgrSetEmpireNameMode(PyObject *poSelf, PyObject *poArgs) {
  int iEnable;
  if (!PyTuple_GetInteger(poArgs, 0, &iEnable))
    return Py_BadArgument();

  CInstanceBase::SetEmpireNameMode(iEnable ? true : false);
  CPythonCharacterManager::Instance().RefreshAllPCTextTail();

  return Py_BuildNone();
}

PyObject *chrmgrRegisterTitleName(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BadArgument();
  char *szTitleName;
  if (!PyTuple_GetString(poArgs, 1, &szTitleName))
    return Py_BadArgument();

  CInstanceBase::RegisterTitleName(iIndex, szTitleName);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterNameColor(PyObject *poSelf, PyObject *poArgs) {
  int index;
  if (!PyTuple_GetInteger(poArgs, 0, &index))
    return Py_BadArgument();

  int ir;
  if (!PyTuple_GetInteger(poArgs, 1, &ir))
    return Py_BadArgument();
  int ig;
  if (!PyTuple_GetInteger(poArgs, 2, &ig))
    return Py_BadArgument();
  int ib;
  if (!PyTuple_GetInteger(poArgs, 3, &ib))
    return Py_BadArgument();

  CInstanceBase::RegisterNameColor(index, ir, ig, ib);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterTitleColor(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BadArgument();
  int ir;
  if (!PyTuple_GetInteger(poArgs, 1, &ir))
    return Py_BadArgument();
  int ig;
  if (!PyTuple_GetInteger(poArgs, 2, &ig))
    return Py_BadArgument();
  int ib;
  if (!PyTuple_GetInteger(poArgs, 3, &ib))
    return Py_BadArgument();

  CInstanceBase::RegisterTitleColor(iIndex, ir, ig, ib);
  return Py_BuildNone();
}

PyObject *chrmgrGetPickedVID(PyObject *poSelf, PyObject *poArgs) {
  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

  DWORD dwPickedActorID;
  if (rkChrMgr.OLD_GetPickedInstanceVID(&dwPickedActorID))
    return Py_BuildValue("i", dwPickedActorID);
  else
    return Py_BuildValue("i", -1);
}

PyObject *chrmgrGetVIDInfo(PyObject *poSelf, PyObject *poArgs) {
  int nVID;
  if (!PyTuple_GetInteger(poArgs, 0, &nVID))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

  char szDetail[256] = "";
  CInstanceBase *pkInstBase = rkChrMgr.GetInstancePtr(nVID);
  if (pkInstBase) {
    TPixelPosition kPPosInst;
    pkInstBase->NEW_GetPixelPosition(&kPPosInst);

    int32_t xInst = kPPosInst.x;
    int32_t yInst = kPPosInst.y;

    CPythonBackground &rkBG = CPythonBackground::Instance();
    rkBG.LocalPositionToGlobalPosition(xInst, yInst);
    sprintf(szDetail, "pos=(%d, %d)", xInst, yInst);
  }

  char szInfo[1024];
  sprintf(szInfo, "VID %d (isRegistered=%d, isAlive=%d, isDead=%d) %s", nVID,
          rkChrMgr.IsRegisteredVID(nVID), rkChrMgr.IsAliveVID(nVID),
          rkChrMgr.IsDeadVID(nVID), szDetail);

  return Py_BuildValue("s", szInfo);
}

PyObject *chrmgrSetPathName(PyObject *poSelf, PyObject *poArgs) {
  char *szPathName;
  if (!PyTuple_GetString(poArgs, 0, &szPathName))
    return Py_BadArgument();

  CRaceManager::Instance().SetPathName(szPathName);
  return Py_BuildNone();
}

PyObject *chrmgrCreateRace(PyObject *poSelf, PyObject *poArgs) {
  int iRace;
  if (!PyTuple_GetInteger(poArgs, 0, &iRace))
    return Py_BadArgument();

  CRaceManager::Instance().CreateRace(iRace);
  return Py_BuildNone();
}

PyObject *chrmgrSelectRace(PyObject *poSelf, PyObject *poArgs) {
  int iRace;
  if (!PyTuple_GetInteger(poArgs, 0, &iRace))
    return Py_BadArgument();

  CRaceManager::Instance().SelectRace(iRace);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterAttachingBoneName(PyObject *poSelf, PyObject *poArgs) {
  int iPartIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iPartIndex))
    return Py_BadArgument();
  char *szBoneName;
  if (!PyTuple_GetString(poArgs, 1, &szBoneName))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->RegisterAttachingBoneName(iPartIndex, szBoneName);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterMotionMode(PyObject *poSelf, PyObject *poArgs) {
  int iMotionIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iMotionIndex))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->RegisterMotionMode(iMotionIndex);
  return Py_BuildNone();
}

PyObject *chrmgrSetMotionRandomWeight(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iMotion;
  if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
    return Py_BadArgument();

  int iSubMotion;
  if (!PyTuple_GetInteger(poArgs, 2, &iSubMotion))
    return Py_BadArgument();

  int iPercentage;
  if (!PyTuple_GetInteger(poArgs, 3, &iPercentage))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  if (!pRaceData->SetMotionRandomWeight(iMode, iMotion, iSubMotion,
                                        iPercentage))
    Py_BuildException("Failed to SetMotionRandomWeight");

  return Py_BuildNone();
}

PyObject *chrmgrRegisterNormalAttack(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iMotion;
  if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->RegisterNormalAttack(iMode, iMotion);
  return Py_BuildNone();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 없어질 함수들
PyObject *chrmgrReserveComboAttack(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iCount;
  if (!PyTuple_GetInteger(poArgs, 1, &iCount))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->ReserveComboAttack(iMode, 0, iCount);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterComboAttack(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iComboIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iComboIndex))
    return Py_BadArgument();

  int iMotionIndex;
  if (!PyTuple_GetInteger(poArgs, 2, &iMotionIndex))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->RegisterComboAttack(iMode, 0, iComboIndex, iMotionIndex);
  return Py_BuildNone();
}
// 없어질 함수들
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *chrmgrReserveComboAttackNew(PyObject *poSelf, PyObject *poArgs) {
  int iMotionMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
    return Py_BadArgument();

  int iComboType;
  if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
    return Py_BadArgument();

  int iCount;
  if (!PyTuple_GetInteger(poArgs, 2, &iCount))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->ReserveComboAttack(iMotionMode, iComboType, iCount);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterComboAttackNew(PyObject *poSelf, PyObject *poArgs) {
  int iMotionMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMotionMode))
    return Py_BadArgument();

  int iComboType;
  if (!PyTuple_GetInteger(poArgs, 1, &iComboType))
    return Py_BadArgument();

  int iComboIndex;
  if (!PyTuple_GetInteger(poArgs, 2, &iComboIndex))
    return Py_BadArgument();

  int iMotionIndex;
  if (!PyTuple_GetInteger(poArgs, 3, &iMotionIndex))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->RegisterComboAttack(iMotionMode, iComboType, iComboIndex,
                                 iMotionIndex);
  return Py_BuildNone();
}

PyObject *chrmgrLoadRaceData(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  const char *c_szFullFileName =
      CRaceManager::Instance().GetFullPathFileName(szFileName);
  if (!pRaceData->LoadRaceData(c_szFullFileName)) {
    TraceError("Failed to load race data : %s\n", c_szFullFileName);
  }

  return Py_BuildNone();
}

PyObject *chrmgrLoadLocalRaceData(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  if (!pRaceData->LoadRaceData(szFileName)) {
    TraceError("Failed to load race data : %s\n", szFileName);
  }

  return Py_BuildNone();
}

/*
PyObject * chrmgrRegisterMotion(PyObject* poSelf, PyObject* poArgs)
{
        int iMode;
        if (!PyTuple_GetInteger(poArgs, 0, &iMode))
                return Py_BadArgument();

        int iMotion;
        if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
                return Py_BadArgument();

        char* szFileName;
        if (!PyTuple_GetString(poArgs, 2, &szFileName))
                return Py_BadArgument();

        int iWeight = 0;
        PyTuple_GetInteger(poArgs, 3, &iWeight);
        iWeight = MIN(100, iWeight);

        CRaceData * pRaceData =
CRaceManager::Instance().GetSelectedRaceDataPointer(); if (!pRaceData) return
Py_BuildException("RaceData has not selected!");

        pRaceData->NEW_RegisterMotion(iMode, iMotion,
CRaceManager::Instance().GetFullPathFileName(szFileName), iWeight); return
Py_BuildNone();
}
*/

PyObject *chrmgrRegisterCacheMotionData(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iMotion;
  if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
    return Py_BadArgument();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 2, &szFileName))
    return Py_BadArgument();

  int iWeight = 0;
  PyTuple_GetInteger(poArgs, 3, &iWeight);
  iWeight = MIN(100, iWeight);

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  const char *c_szFullFileName =
      CRaceManager::Instance().GetFullPathFileName(szFileName);
  CGraphicThing *pkMotionThing =
      pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

  if (pkMotionThing)
    CResourceManager::Instance().LoadStaticCache(pkMotionThing->GetFileName());

  return Py_BuildNone();
}

PyObject *chrmgrRegisterMotionData(PyObject *poSelf, PyObject *poArgs) {
  int iMode;
  if (!PyTuple_GetInteger(poArgs, 0, &iMode))
    return Py_BadArgument();

  int iMotion;
  if (!PyTuple_GetInteger(poArgs, 1, &iMotion))
    return Py_BadArgument();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 2, &szFileName))
    return Py_BadArgument();

  int iWeight = 0;
  PyTuple_GetInteger(poArgs, 3, &iWeight);
  iWeight = MIN(100, iWeight);

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  const char *c_szFullFileName =
      CRaceManager::Instance().GetFullPathFileName(szFileName);
  pRaceData->RegisterMotionData(iMode, iMotion, c_szFullFileName, iWeight);

  return Py_BuildNone();
}

PyObject *chrmgrRegisterRaceSrcName(PyObject *poSelf, PyObject *poArgs) {
  char *szName;
  if (!PyTuple_GetString(poArgs, 0, &szName))
    return Py_BadArgument();

  char *szSrcName;
  if (!PyTuple_GetString(poArgs, 1, &szSrcName))
    return Py_BadArgument();

  CRaceManager::Instance().RegisterRaceSrcName(szName, szSrcName);

  return Py_BuildNone();
}

PyObject *chrmgrRegisterRaceName(PyObject *poSelf, PyObject *poArgs) {
  int iRaceIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iRaceIndex))
    return Py_BadArgument();

  char *szName;
  if (!PyTuple_GetString(poArgs, 1, &szName))
    return Py_BadArgument();

  CRaceManager::Instance().RegisterRaceName(iRaceIndex, szName);

  return Py_BuildNone();
}

PyObject *chrmgrSetShapeModel(PyObject *poSelf, PyObject *poArgs) {
  int eShape;
  if (!PyTuple_GetInteger(poArgs, 0, &eShape))
    return Py_BadArgument();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 1, &szFileName))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->SetShapeModel(eShape, szFileName);
  return Py_BuildNone();
}

PyObject *chrmgrAppendShapeSkin(PyObject *poSelf, PyObject *poArgs) {
  int eShape;
  if (!PyTuple_GetInteger(poArgs, 0, &eShape))
    return Py_BadArgument();

  int ePart;
  if (!PyTuple_GetInteger(poArgs, 1, &ePart))
    return Py_BadArgument();

  char *szSrcFileName;
  if (!PyTuple_GetString(poArgs, 2, &szSrcFileName))
    return Py_BadArgument();

  char *szDstFileName;
  if (!PyTuple_GetString(poArgs, 3, &szDstFileName))
    return Py_BadArgument();

  CRaceData *pRaceData = CRaceManager::Instance().GetSelectedRaceDataPointer();
  if (!pRaceData)
    return Py_BuildException("RaceData has not selected!");

  pRaceData->AppendShapeSkin(eShape, ePart, szSrcFileName, szDstFileName);
  return Py_BuildNone();
}
//////////////////////////////////////////////////////////////////////////
// RaceData 관련 끝
//////////////////////////////////////////////////////////////////////////

PyObject *chrmgrSetMovingSpeed(PyObject *poSelf, PyObject *poArgs) {
#ifndef _DISTRIBUTE
  int nMovSpd;
  if (!PyTuple_GetInteger(poArgs, 0, &nMovSpd))
    return Py_BadArgument();

  if (nMovSpd < 0)
    return Py_BuildException("MovingSpeed < 0");

  CInstanceBase *pkInst =
      CPythonCharacterManager::Instance().GetSelectedInstancePtr();

  if (!pkInst)
    return Py_BuildException("MainCharacter has not selected!");

  pkInst->SetMoveSpeed(nMovSpd);
#endif
  return Py_BuildNone();
}

PyObject *chrmgrRegisterEffect(PyObject *poSelf, PyObject *poArgs) {
  int eEftType;
  if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
    return Py_BadArgument();

  char *szBoneName;
  if (!PyTuple_GetString(poArgs, 1, &szBoneName))
    return Py_BadArgument();

  char *szPathName;
  if (!PyTuple_GetString(poArgs, 2, &szPathName))
    return Py_BadArgument();

  CInstanceBase::RegisterEffect(eEftType, szBoneName, szPathName, false);
  return Py_BuildNone();
}

PyObject *chrmgrRegisterCacheEffect(PyObject *poSelf, PyObject *poArgs) {
  int eEftType;
  if (!PyTuple_GetInteger(poArgs, 0, &eEftType))
    return Py_BadArgument();

  char *szBoneName;
  if (!PyTuple_GetString(poArgs, 1, &szBoneName))
    return Py_BadArgument();

  char *szPathName;
  if (!PyTuple_GetString(poArgs, 2, &szPathName))
    return Py_BadArgument();

  CInstanceBase::RegisterEffect(eEftType, szBoneName, szPathName, true);

  return Py_BuildNone();
}

PyObject *chrmgrSetDustGap(PyObject *poSelf, PyObject *poArgs) {
  int nGap;
  if (!PyTuple_GetInteger(poArgs, 0, &nGap))
    return Py_BadArgument();

  CInstanceBase::SetDustGap(nGap);
  return Py_BuildNone();
}

PyObject *chrmgrSetHorseDustGap(PyObject *poSelf, PyObject *poArgs) {
  int nGap;
  if (!PyTuple_GetInteger(poArgs, 0, &nGap))
    return Py_BadArgument();

  CInstanceBase::SetHorseDustGap(nGap);
  return Py_BuildNone();
}

PyObject *chrmgrToggleDirectionLine(PyObject *poSelf, PyObject *poArgs) {
  static bool s_isVisible = true;
  CActorInstance::ShowDirectionLine(s_isVisible);

  s_isVisible = !s_isVisible;
  return Py_BuildNone();
}

PyObject *chrmgrRegisterPointEffect(PyObject *poSelf, PyObject *poArgs) {
  int iEft;
  if (!PyTuple_GetInteger(poArgs, 0, &iEft))
    return Py_BadArgument();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 1, &szFileName))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  rkChrMgr.RegisterPointEffect(iEft, szFileName);
  return Py_BuildNone();
}

PyObject *chrmgrShowPointEffect(PyObject *poSelf, PyObject *poArgs) {
  int nVID;
  if (!PyTuple_GetInteger(poArgs, 0, &nVID))
    return Py_BadArgument();

  int nEft;
  if (!PyTuple_GetInteger(poArgs, 1, &nEft))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  rkChrMgr.ShowPointEffect(nEft, nVID >= 0 ? nVID : 0xffffffff);
  return Py_BuildNone();
}

void CPythonCharacterManager::SCRIPT_SetAffect(DWORD dwVID, DWORD eState,
                                               BOOL isVisible) {
  CInstanceBase *pkInstSel =
      (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
  if (!pkInstSel)
    return;

  pkInstSel->SCRIPT_SetAffect(eState, isVisible ? true : false);
}

PyObject *chrmgrSetAffect(PyObject *poSelf, PyObject *poArgs) {
  int nVID;
  if (!PyTuple_GetInteger(poArgs, 0, &nVID))
    return Py_BadArgument();

  int nEft;
  if (!PyTuple_GetInteger(poArgs, 1, &nEft))
    return Py_BadArgument();

  int nVisible;
  if (!PyTuple_GetInteger(poArgs, 2, &nVisible))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  rkChrMgr.SCRIPT_SetAffect(nVID >= 0 ? nVID : 0xffffffff, nEft, nVisible);
  return Py_BuildNone();
}

void CPythonCharacterManager::SetEmoticon(DWORD dwVID, DWORD eState) {
  CInstanceBase *pkInstSel =
      (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
  if (!pkInstSel)
    return;

  pkInstSel->SetEmoticon(eState);
}

bool CPythonCharacterManager::IsPossibleEmoticon(DWORD dwVID) {
  CInstanceBase *pkInstSel =
      (dwVID == 0xffffffff) ? GetSelectedInstancePtr() : GetInstancePtr(dwVID);
  if (!pkInstSel)
    return false;

  return pkInstSel->IsPossibleEmoticon();
}

PyObject *chrmgrSetEmoticon(PyObject *poSelf, PyObject *poArgs) {
  int nVID;
  if (!PyTuple_GetInteger(poArgs, 0, &nVID))
    return Py_BadArgument();

  int nEft;
  if (!PyTuple_GetInteger(poArgs, 1, &nEft))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  rkChrMgr.SetEmoticon(nVID >= 0 ? nVID : 0xffffffff, nEft);
  return Py_BuildNone();
}

PyObject *chrmgrIsPossibleEmoticon(PyObject *poSelf, PyObject *poArgs) {
  int nVID;
  if (!PyTuple_GetInteger(poArgs, 0, &nVID))
    return Py_BadArgument();

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  int result = rkChrMgr.IsPossibleEmoticon(nVID >= 0 ? nVID : 0xffffffff);

  return Py_BuildValue("i", result);
}

void initchrmgr() {
  static PyMethodDef s_methods[] = {
      // RaceData 관련
      {"SetEmpireNameMode", chrmgrSetEmpireNameMode, METH_VARARGS},
      {"GetVIDInfo", chrmgrGetVIDInfo, METH_VARARGS},
      {"GetPickedVID", chrmgrGetPickedVID, METH_VARARGS},
      {"SetShapeModel", chrmgrSetShapeModel, METH_VARARGS},
      {"AppendShapeSkin", chrmgrAppendShapeSkin, METH_VARARGS},
      {"SetPathName", chrmgrSetPathName, METH_VARARGS},
      {"LoadRaceData", chrmgrLoadRaceData, METH_VARARGS},
      {"LoadLocalRaceData", chrmgrLoadLocalRaceData, METH_VARARGS},
      {"CreateRace", chrmgrCreateRace, METH_VARARGS},
      {"SelectRace", chrmgrSelectRace, METH_VARARGS},
      {"RegisterAttachingBoneName", chrmgrRegisterAttachingBoneName,
       METH_VARARGS},
      {"RegisterMotionMode", chrmgrRegisterMotionMode, METH_VARARGS},
      //{ "RegisterMotion",				chrmgrRegisterMotion,
      // METH_VARARGS },
      {"SetMotionRandomWeight", chrmgrSetMotionRandomWeight, METH_VARARGS},
      {"RegisterNormalAttack", chrmgrRegisterNormalAttack, METH_VARARGS},
      {"ReserveComboAttack", chrmgrReserveComboAttack, METH_VARARGS},
      {"RegisterComboAttack", chrmgrRegisterComboAttack, METH_VARARGS},
      {"ReserveComboAttackNew", chrmgrReserveComboAttackNew, METH_VARARGS},
      {"RegisterComboAttackNew", chrmgrRegisterComboAttackNew, METH_VARARGS},
      {"RegisterMotionData", chrmgrRegisterMotionData, METH_VARARGS},
      {"RegisterRaceName", chrmgrRegisterRaceName, METH_VARARGS},
      {"RegisterRaceSrcName", chrmgrRegisterRaceSrcName, METH_VARARGS},
      {"RegisterCacheMotionData", chrmgrRegisterCacheMotionData, METH_VARARGS},

      // ETC
      {"SetAffect", chrmgrSetAffect, METH_VARARGS},
      {"SetEmoticon", chrmgrSetEmoticon, METH_VARARGS},
      {"IsPossibleEmoticon", chrmgrIsPossibleEmoticon, METH_VARARGS},
      {"RegisterEffect", chrmgrRegisterEffect, METH_VARARGS},
      {"RegisterCacheEffect", chrmgrRegisterCacheEffect, METH_VARARGS},
      {"RegisterPointEffect", chrmgrRegisterPointEffect, METH_VARARGS},
      {"ShowPointEffect", chrmgrShowPointEffect, METH_VARARGS},
      {"ToggleDirectionLine", chrmgrToggleDirectionLine, METH_VARARGS},
      {"SetMovingSpeed", chrmgrSetMovingSpeed, METH_VARARGS},
      {"SetDustGap", chrmgrSetDustGap, METH_VARARGS},
      {"SetHorseDustGap", chrmgrSetHorseDustGap, METH_VARARGS},

      {"RegisterTitleName", chrmgrRegisterTitleName, METH_VARARGS},
      {"RegisterNameColor", chrmgrRegisterNameColor, METH_VARARGS},
      {"RegisterTitleColor", chrmgrRegisterTitleColor, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("chrmgr", s_methods);

  PyModule_AddIntConstant(poModule, "NAMECOLOR_MOB",
                          CInstanceBase::NAMECOLOR_NORMAL_MOB);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_NPC",
                          CInstanceBase::NAMECOLOR_NORMAL_NPC);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_PC",
                          CInstanceBase::NAMECOLOR_NORMAL_PC);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_MOB",
                          CInstanceBase::NAMECOLOR_EMPIRE_MOB);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_NPC",
                          CInstanceBase::NAMECOLOR_EMPIRE_NPC);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_EMPIRE_PC",
                          CInstanceBase::NAMECOLOR_EMPIRE_PC);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_FUNC",
                          CInstanceBase::NAMECOLOR_FUNC);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_PK",
                          CInstanceBase::NAMECOLOR_PK);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_PVP",
                          CInstanceBase::NAMECOLOR_PVP);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_PARTY",
                          CInstanceBase::NAMECOLOR_PARTY);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_WARP",
                          CInstanceBase::NAMECOLOR_WARP);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_WAYPOINT",
                          CInstanceBase::NAMECOLOR_WAYPOINT);
  PyModule_AddIntConstant(poModule, "NAMECOLOR_EXTRA",
                          CInstanceBase::NAMECOLOR_EXTRA);

  PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_DISAPPEAR",
                          CInstanceBase::EFFECT_SPAWN_DISAPPEAR);
  PyModule_AddIntConstant(poModule, "EFFECT_SPAWN_APPEAR",
                          CInstanceBase::EFFECT_SPAWN_APPEAR);
  PyModule_AddIntConstant(poModule, "EFFECT_DUST", CInstanceBase::EFFECT_DUST);
  PyModule_AddIntConstant(poModule, "EFFECT_HORSE_DUST",
                          CInstanceBase::EFFECT_HORSE_DUST);
  PyModule_AddIntConstant(poModule, "EFFECT_STUN", CInstanceBase::EFFECT_STUN);
  PyModule_AddIntConstant(poModule, "EFFECT_HIT", CInstanceBase::EFFECT_HIT);
  PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACK",
                          CInstanceBase::EFFECT_FLAME_ATTACK);
  PyModule_AddIntConstant(poModule, "EFFECT_FLAME_HIT",
                          CInstanceBase::EFFECT_FLAME_HIT);
  PyModule_AddIntConstant(poModule, "EFFECT_FLAME_ATTACH",
                          CInstanceBase::EFFECT_FLAME_ATTACH);
  PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACK",
                          CInstanceBase::EFFECT_ELECTRIC_ATTACK);
  PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_HIT",
                          CInstanceBase::EFFECT_ELECTRIC_HIT);
  PyModule_AddIntConstant(poModule, "EFFECT_ELECTRIC_ATTACH",
                          CInstanceBase::EFFECT_ELECTRIC_ATTACH);
  PyModule_AddIntConstant(poModule, "EFFECT_SELECT",
                          CInstanceBase::EFFECT_SELECT);
  PyModule_AddIntConstant(poModule, "EFFECT_TARGET",
                          CInstanceBase::EFFECT_TARGET);
  PyModule_AddIntConstant(poModule, "EFFECT_CRITICAL",
                          CInstanceBase::EFFECT_CRITICAL);

  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGET",
                          CInstanceBase::EFFECT_DAMAGE_TARGET);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_NOT_TARGET",
                          CInstanceBase::EFFECT_DAMAGE_NOT_TARGET);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE",
                          CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_SELFDAMAGE2",
                          CInstanceBase::EFFECT_DAMAGE_SELFDAMAGE2);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_POISON",
                          CInstanceBase::EFFECT_DAMAGE_POISON);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_MISS",
                          CInstanceBase::EFFECT_DAMAGE_MISS);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_TARGETMISS",
                          CInstanceBase::EFFECT_DAMAGE_TARGETMISS);
  PyModule_AddIntConstant(poModule, "EFFECT_DAMAGE_CRITICAL",
                          CInstanceBase::EFFECT_DAMAGE_CRITICAL);

  PyModule_AddIntConstant(poModule, "EFFECT_LEVELUP",
                          CInstanceBase::EFFECT_LEVELUP);
  PyModule_AddIntConstant(poModule, "EFFECT_SKILLUP",
                          CInstanceBase::EFFECT_SKILLUP);
  PyModule_AddIntConstant(poModule, "EFFECT_HPUP_RED",
                          CInstanceBase::EFFECT_HPUP_RED);
  PyModule_AddIntConstant(poModule, "EFFECT_SPUP_BLUE",
                          CInstanceBase::EFFECT_SPUP_BLUE);
  PyModule_AddIntConstant(poModule, "EFFECT_SPEEDUP_GREEN",
                          CInstanceBase::EFFECT_SPEEDUP_GREEN);
  PyModule_AddIntConstant(poModule, "EFFECT_DXUP_PURPLE",
                          CInstanceBase::EFFECT_DXUP_PURPLE);
  PyModule_AddIntConstant(poModule, "EFFECT_PENETRATE",
                          CInstanceBase::EFFECT_PENETRATE);
  PyModule_AddIntConstant(poModule, "EFFECT_BLOCK",
                          CInstanceBase::EFFECT_BLOCK);
  PyModule_AddIntConstant(poModule, "EFFECT_DODGE",
                          CInstanceBase::EFFECT_DODGE);
  PyModule_AddIntConstant(poModule, "EFFECT_FIRECRACKER",
                          CInstanceBase::EFFECT_FIRECRACKER);
  PyModule_AddIntConstant(poModule, "EFFECT_SPIN_TOP",
                          CInstanceBase::EFFECT_SPIN_TOP);

  PyModule_AddIntConstant(poModule, "EFFECT_WEAPON",
                          CInstanceBase::EFFECT_WEAPON);

  PyModule_AddIntConstant(poModule, "EFFECT_AFFECT",
                          CInstanceBase::EFFECT_AFFECT);
  PyModule_AddIntConstant(poModule, "EFFECT_EMOTICON",
                          CInstanceBase::EFFECT_EMOTICON);
  PyModule_AddIntConstant(poModule, "EFFECT_EMPIRE",
                          CInstanceBase::EFFECT_EMPIRE);

  PyModule_AddIntConstant(poModule, "EFFECT_REFINED",
                          CInstanceBase::EFFECT_REFINED);

  PyModule_AddIntConstant(poModule, "EFFECT_SUCCESS",
                          CInstanceBase::EFFECT_SUCCESS);
  PyModule_AddIntConstant(poModule, "EFFECT_FAIL", CInstanceBase::EFFECT_FAIL);
  PyModule_AddIntConstant(poModule, "EFFECT_FR_SUCCESS",
                          CInstanceBase::EFFECT_FR_SUCCESS);
  PyModule_AddIntConstant(
      poModule, "EFFECT_LEVELUP_ON_14_FOR_GERMANY",
      CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY); // 레벨업 14일때 (
                                                        // 독일전용 )
  PyModule_AddIntConstant(
      poModule, "EFFECT_LEVELUP_UNDER_15_FOR_GERMANY",
      CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY); // 레벨업 15일때 (
                                                           // 독일전용 )
  PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE1",
                          CInstanceBase::EFFECT_PERCENT_DAMAGE1);
  PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE2",
                          CInstanceBase::EFFECT_PERCENT_DAMAGE2);
  PyModule_AddIntConstant(poModule, "EFFECT_PERCENT_DAMAGE3",
                          CInstanceBase::EFFECT_PERCENT_DAMAGE3);

  // 자동물약 HP, SP
  PyModule_AddIntConstant(poModule, "EFFECT_AUTO_HPUP",
                          CInstanceBase::EFFECT_AUTO_HPUP);
  PyModule_AddIntConstant(poModule, "EFFECT_AUTO_SPUP",
                          CInstanceBase::EFFECT_AUTO_SPUP);

  PyModule_AddIntConstant(poModule, "EFFECT_RAMADAN_RING_EQUIP",
                          CInstanceBase::EFFECT_RAMADAN_RING_EQUIP);
  PyModule_AddIntConstant(poModule, "EFFECT_HALLOWEEN_CANDY_EQUIP",
                          CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP);
  PyModule_AddIntConstant(poModule, "EFFECT_HAPPINESS_RING_EQUIP",
                          CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP);
  PyModule_AddIntConstant(poModule, "EFFECT_LOVE_PENDANT_EQUIP",
                          CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP);

  PyModule_AddIntConstant(poModule, "EFFECT_AGGREGATE_MONSTER",
                          CInstanceBase::EFFECT_AGGREGATE_MONSTER);
}
