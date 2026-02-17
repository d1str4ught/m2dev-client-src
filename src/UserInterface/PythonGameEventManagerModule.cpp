#include "StdAfx.h"
#include "GameLib/GameEventManager.h"

PyObject *eventMgrUpdate(PyObject *poSelf, PyObject *poArgs) {
  float fx;
  if (!PyTuple_GetFloat(poArgs, 0, &fx))
    return Py_BuildException();
  float fy;
  if (!PyTuple_GetFloat(poArgs, 1, &fy))
    return Py_BuildException();
  float fz;
  if (!PyTuple_GetFloat(poArgs, 2, &fz))
    return Py_BuildException();

  CGameEventManager::Instance().SetCenterPosition(fx, fy, fz);
  CGameEventManager::Instance().Update();
  return Py_BuildNone();
}

void initeventmgr() {
  static PyMethodDef s_methods[] = {
      {"Update", eventMgrUpdate, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  Py_InitModule("eventMgr", s_methods);
}