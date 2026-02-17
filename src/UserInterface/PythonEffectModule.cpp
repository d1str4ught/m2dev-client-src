#include "StdAfx.h"
#include "EffectLib/EffectManager.h"
#include "PythonCharacterManager.h"

PyObject *effectRegisterEffect(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  CEffectManager::Instance().RegisterEffect(szFileName);
  return Py_BuildNone();
}

PyObject *effectUpdate(PyObject *poSelf, PyObject *poArgs) {
  CEffectManager::Instance().Update();
  return Py_BuildNone();
}

PyObject *effectRender(PyObject *poSelf, PyObject *poArgs) {
  CEffectManager::Instance().Render();
  return Py_BuildNone();
}

PyObject *effectCreateEffect(PyObject *poSelf, PyObject *poArgs) {
  char *szEffectName;
  if (!PyTuple_GetString(poArgs, 0, &szEffectName))
    return Py_BuildException();

  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetSelectedInstancePtr();
  TPixelPosition PixelPosition;
  pInstance->NEW_GetPixelPosition(&PixelPosition);
  int iIndex = CEffectManager::Instance().CreateEffect(
      szEffectName, PixelPosition, D3DXVECTOR3(0.0f, 0.0f, 0.0f));

  return Py_BuildValue("i", iIndex);
}

PyObject *effectDeleteEffect(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CEffectManager::Instance().DestroyEffectInstance(iIndex);

  return Py_BuildNone();
}

PyObject *effectSetPosition(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  float fx;
  if (!PyTuple_GetFloat(poArgs, 1, &fx))
    return Py_BuildException();
  float fy;
  if (!PyTuple_GetFloat(poArgs, 2, &fy))
    return Py_BuildException();
  float fz;
  if (!PyTuple_GetFloat(poArgs, 3, &fz))
    return Py_BuildException();

  CEffectManager::Instance().SelectEffectInstance(iIndex);
  CEffectManager::Instance().SetEffectInstancePosition(D3DXVECTOR3(fx, fy, fz));

  return Py_BuildNone();
}

PyObject *effectRegisterIndexedFlyData(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  int iType;
  if (!PyTuple_GetInteger(poArgs, 1, &iType))
    return Py_BuildException();
  char *szFlyDataName;
  if (!PyTuple_GetString(poArgs, 2, &szFlyDataName))
    return Py_BuildException();

  CFlyingManager::Instance().RegisterIndexedFlyData(iIndex, iType,
                                                    szFlyDataName);

  return Py_BuildNone();
}

void initeffect() {
  static PyMethodDef s_methods[] = {
      {"RegisterEffect", effectRegisterEffect, METH_VARARGS},
      {"CreateEffect", effectCreateEffect, METH_VARARGS},
      {"DeleteEffect", effectDeleteEffect, METH_VARARGS},
      {"SetPosition", effectSetPosition, METH_VARARGS},

      {"RegisterIndexedFlyData", effectRegisterIndexedFlyData, METH_VARARGS},

      {"Update", effectUpdate, METH_VARARGS},
      {"Render", effectRender, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("effect", s_methods);
  PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_NORMAL",
                          CFlyingManager::INDEX_FLY_TYPE_NORMAL);
  PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_FIRE_CRACKER",
                          CFlyingManager::INDEX_FLY_TYPE_FIRE_CRACKER);
  PyModule_AddIntConstant(poModule, "INDEX_FLY_TYPE_AUTO_FIRE",
                          CFlyingManager::INDEX_FLY_TYPE_AUTO_FIRE);

  enum {
    FLY_NONE,
    FLY_EXP,
    FLY_HP_MEDIUM,
    FLY_HP_BIG,
    FLY_SP_SMALL,
    FLY_SP_MEDIUM,
    FLY_SP_BIG,
    FLY_FIREWORK1,
    FLY_FIREWORK2,
    FLY_FIREWORK3,
    FLY_FIREWORK4,
    FLY_FIREWORK5,
    FLY_FIREWORK6,
    FLY_FIREWORK_XMAS,
    FLY_CHAIN_LIGHTNING,
    FLY_HP_SMALL,
    FLY_SKILL_MUYEONG,
  };

  PyModule_AddIntConstant(poModule, "FLY_NONE", FLY_NONE);
  PyModule_AddIntConstant(poModule, "FLY_EXP", FLY_EXP);
  PyModule_AddIntConstant(poModule, "FLY_HP_MEDIUM", FLY_HP_MEDIUM);
  PyModule_AddIntConstant(poModule, "FLY_HP_BIG", FLY_HP_BIG);
  PyModule_AddIntConstant(poModule, "FLY_SP_SMALL", FLY_SP_SMALL);
  PyModule_AddIntConstant(poModule, "FLY_SP_MEDIUM", FLY_SP_MEDIUM);
  PyModule_AddIntConstant(poModule, "FLY_SP_BIG", FLY_SP_BIG);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK1", FLY_FIREWORK1);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK2", FLY_FIREWORK2);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK3", FLY_FIREWORK3);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK4", FLY_FIREWORK4);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK5", FLY_FIREWORK5);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK6", FLY_FIREWORK6);
  PyModule_AddIntConstant(poModule, "FLY_FIREWORK_XMAS", FLY_FIREWORK_XMAS);
  PyModule_AddIntConstant(poModule, "FLY_CHAIN_LIGHTNING", FLY_CHAIN_LIGHTNING);
  PyModule_AddIntConstant(poModule, "FLY_HP_SMALL", FLY_HP_SMALL);
  PyModule_AddIntConstant(poModule, "FLY_SKILL_MUYEONG", FLY_SKILL_MUYEONG);
}
