#include "StdAfx.h"
#include "PythonNonPlayer.h"

#include "InstanceBase.h"
#include "PythonCharacterManager.h"

PyObject *nonplayerGetEventType(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualNumber;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualNumber))
    return Py_BuildException();

  BYTE iType = CPythonNonPlayer::Instance().GetEventType(iVirtualNumber);

  return Py_BuildValue("i", iType);
}

PyObject *nonplayerGetEventTypeByVID(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
    return Py_BuildException();

  BYTE iType = CPythonNonPlayer::Instance().GetEventTypeByVID(iVirtualID);

  return Py_BuildValue("i", iType);
}

PyObject *nonplayerGetLevelByVID(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
    return Py_BuildException();

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

  if (!pInstance)
    return Py_BuildValue("i", -1);

  const CPythonNonPlayer::TMobTable *pMobTable =
      CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

  if (!pMobTable)
    return Py_BuildValue("i", -1);

  float fAverageLevel =
      pMobTable->bLevel; //(float(pMobTable->abLevelRange[0]) +
                         // float(pMobTable->abLevelRange[1])) / 2.0f;
  fAverageLevel = floor(fAverageLevel + 0.5f);
  return Py_BuildValue("i", int(fAverageLevel));
}

PyObject *nonplayerGetGradeByVID(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
    return Py_BuildException();

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(iVirtualID);

  if (!pInstance)
    return Py_BuildValue("i", -1);

  const CPythonNonPlayer::TMobTable *pMobTable =
      CPythonNonPlayer::Instance().GetTable(pInstance->GetVirtualNumber());

  if (!pMobTable)
    return Py_BuildValue("i", -1);

  return Py_BuildValue("i", pMobTable->bRank);
}

PyObject *nonplayerGetMonsterName(PyObject *poSelf, PyObject *poArgs) {
  int iVNum;
  if (!PyTuple_GetInteger(poArgs, 0, &iVNum))
    return Py_BuildException();

  CPythonNonPlayer &rkNonPlayer = CPythonNonPlayer::Instance();
  return Py_BuildValue("s", rkNonPlayer.GetMonsterName(iVNum));
}

PyObject *nonplayerLoadNonPlayerData(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  CPythonNonPlayer::Instance().LoadNonPlayerData(szFileName);
  return Py_BuildNone();
}

void initNonPlayer() {
  static PyMethodDef s_methods[] = {
      {"GetEventType", nonplayerGetEventType, METH_VARARGS},
      {"GetEventTypeByVID", nonplayerGetEventTypeByVID, METH_VARARGS},
      {"GetLevelByVID", nonplayerGetLevelByVID, METH_VARARGS},
      {"GetGradeByVID", nonplayerGetGradeByVID, METH_VARARGS},
      {"GetMonsterName", nonplayerGetMonsterName, METH_VARARGS},

      {"LoadNonPlayerData", nonplayerLoadNonPlayerData, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("nonplayer", s_methods);

  PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_NONE",
                          CPythonNonPlayer::ON_CLICK_EVENT_NONE);
  PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_BATTLE",
                          CPythonNonPlayer::ON_CLICK_EVENT_BATTLE);
  PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_SHOP",
                          CPythonNonPlayer::ON_CLICK_EVENT_SHOP);
  PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_TALK",
                          CPythonNonPlayer::ON_CLICK_EVENT_TALK);
  PyModule_AddIntConstant(poModule, "ON_CLICK_EVENT_VEHICLE",
                          CPythonNonPlayer::ON_CLICK_EVENT_VEHICLE);

  PyModule_AddIntConstant(poModule, "PAWN", 0);
  PyModule_AddIntConstant(poModule, "S_PAWN", 1);
  PyModule_AddIntConstant(poModule, "KNIGHT", 2);
  PyModule_AddIntConstant(poModule, "S_KNIGHT", 3);
  PyModule_AddIntConstant(poModule, "BOSS", 4);
  PyModule_AddIntConstant(poModule, "KING", 5);
}