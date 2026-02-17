#include "StdAfx.h"

bool PyTuple_GetThingInstance(PyObject *poArgs, int pos,
                              CGraphicThingInstance **ppRetThingInstance) {
  unsigned long long handle;
  if (!PyTuple_GetUnsignedLongLong(poArgs, pos, &handle))
    return false;

  if (!handle)
    return false;

  *ppRetThingInstance = (CGraphicThingInstance *)handle;
  return true;
}

PyObject *grpThingGenerate(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  if (strlen(szFileName) <= 0)
    return Py_BuildValue("K", 0);

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szFileName);

  if (!pResource->IsType(CGraphicThing::Type()))
    return Py_BuildException();

  CGraphicThingInstance *pThingInstance = CGraphicThingInstance::New();
  pThingInstance->ReserveModelThing(1);
  pThingInstance->ReserveModelInstance(1);
  pThingInstance->RegisterModelThing(0,
                                     static_cast<CGraphicThing *>(pResource));
  pThingInstance->SetModelInstance(0, 0, 0);
  return Py_BuildValue("K", pThingInstance);
}

PyObject *grpThingDelete(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  CGraphicThingInstance::Delete(pThingInstance);
  return Py_BuildNone();
}

PyObject *grpThingSetFileName(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  char *szFileName;
  if (!PyTuple_GetString(poArgs, 1, &szFileName))
    return Py_BuildException();

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szFileName);

  if (!pResource->IsType(CGraphicThing::Type()))
    return Py_BuildException();

  pThingInstance->Clear();
  pThingInstance->ReserveModelThing(1);
  pThingInstance->ReserveModelInstance(1);
  pThingInstance->RegisterModelThing(0,
                                     static_cast<CGraphicThing *>(pResource));
  pThingInstance->SetModelInstance(0, 0, 0);
  return Py_BuildNone();
}

PyObject *grpThingRender(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  pThingInstance->Render();
  return Py_BuildNone();
}

PyObject *grpThingUpdate(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  pThingInstance->Update();
  pThingInstance->Deform();
  return Py_BuildNone();
}

PyObject *grpSetThingPosition(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  float x;
  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BuildException();

  float y;
  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BuildException();

  float z;
  if (!PyTuple_GetFloat(poArgs, 3, &z))
    return Py_BuildException();

  pThingInstance->SetPosition(x, y, z);
  return Py_BuildNone();
}

PyObject *grpSetThingRotation(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  float fDegree;
  if (!PyTuple_GetFloat(poArgs, 1, &fDegree))
    return Py_BuildException();

  pThingInstance->SetRotation(fDegree);
  return Py_BuildNone();
}

PyObject *grpSetThingScale(PyObject *poSelf, PyObject *poArgs) {
  CGraphicThingInstance *pThingInstance;
  if (!PyTuple_GetThingInstance(poArgs, 0, &pThingInstance))
    return Py_BuildException();

  float x;
  if (!PyTuple_GetFloat(poArgs, 1, &x))
    return Py_BuildException();
  float y;
  if (!PyTuple_GetFloat(poArgs, 2, &y))
    return Py_BuildException();
  float z;
  if (!PyTuple_GetFloat(poArgs, 3, &z))
    return Py_BuildException();

  pThingInstance->SetScale(x, y, z);
  return Py_BuildNone();
}

void initgrpThing() {
  static PyMethodDef s_methods[] = {
      {"Update", grpThingUpdate, METH_VARARGS},
      {"Render", grpThingRender, METH_VARARGS},
      {"SetPosition", grpSetThingPosition, METH_VARARGS},
      {"Generate", grpThingGenerate, METH_VARARGS},
      {"Delete", grpThingDelete, METH_VARARGS},
      {"SetFileName", grpThingSetFileName, METH_VARARGS},
      {"SetRotation", grpSetThingRotation, METH_VARARGS},
      {"SetScale", grpSetThingScale, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  Py_InitModule("grpThing", s_methods);
}
