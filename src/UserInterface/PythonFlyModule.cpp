#include "stdafx.h"
#include "GameLib/FlyingObjectManager.h"

PyObject *flyUpdate(PyObject *poSelf, PyObject *poArgs) {
  CFlyingManager::Instance().Update();
  return Py_BuildNone();
}

PyObject *flyRender(PyObject *poSelf, PyObject *poArgs) {
  CFlyingManager::Instance().Render();
  return Py_BuildNone();
}

void initfly() {
  static PyMethodDef s_methods[] = {
      {"Update", flyUpdate, METH_VARARGS},
      {"Render", flyRender, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  Py_InitModule("fly", s_methods);
}