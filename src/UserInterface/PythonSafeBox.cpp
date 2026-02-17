#include "StdAfx.h"
#include "PythonSafeBox.h"

void CPythonSafeBox::OpenSafeBox(int iSize) {
  m_dwMoney = 0;
  m_ItemInstanceVector.clear();
  m_ItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

  for (DWORD i = 0; i < m_ItemInstanceVector.size(); ++i) {
    TItemData &rInstance = m_ItemInstanceVector[i];
    ZeroMemory(&rInstance, sizeof(rInstance));
  }
}

void CPythonSafeBox::SetItemData(DWORD dwSlotIndex,
                                 const TItemData &rItemInstance) {
  if (dwSlotIndex >= m_ItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::SetItemData(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return;
  }

  m_ItemInstanceVector[dwSlotIndex] = rItemInstance;
}

void CPythonSafeBox::DelItemData(DWORD dwSlotIndex) {
  if (dwSlotIndex >= m_ItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::DelItemData(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return;
  }

  TItemData &rInstance = m_ItemInstanceVector[dwSlotIndex];
  ZeroMemory(&rInstance, sizeof(rInstance));
}

void CPythonSafeBox::SetMoney(DWORD dwMoney) { m_dwMoney = dwMoney; }

DWORD CPythonSafeBox::GetMoney() { return m_dwMoney; }

int CPythonSafeBox::GetCurrentSafeBoxSize() {
  return m_ItemInstanceVector.size();
}

BOOL CPythonSafeBox::GetSlotItemID(DWORD dwSlotIndex, DWORD *pdwItemID) {
  if (dwSlotIndex >= m_ItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::GetSlotItemID(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return FALSE;
  }

  *pdwItemID = m_ItemInstanceVector[dwSlotIndex].vnum;

  return TRUE;
}

BOOL CPythonSafeBox::GetItemDataPtr(DWORD dwSlotIndex, TItemData **ppInstance) {
  if (dwSlotIndex >= m_ItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::GetItemData(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return FALSE;
  }

  *ppInstance = &m_ItemInstanceVector[dwSlotIndex];

  return TRUE;
}

void CPythonSafeBox::OpenMall(int iSize) {
  m_MallItemInstanceVector.clear();
  m_MallItemInstanceVector.resize(SAFEBOX_SLOT_X_COUNT * iSize);

  for (DWORD i = 0; i < m_MallItemInstanceVector.size(); ++i) {
    TItemData &rInstance = m_MallItemInstanceVector[i];
    ZeroMemory(&rInstance, sizeof(rInstance));
  }
}

void CPythonSafeBox::SetMallItemData(DWORD dwSlotIndex,
                                     const TItemData &rItemData) {
  if (dwSlotIndex >= m_MallItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::SetMallItemData(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return;
  }

  m_MallItemInstanceVector[dwSlotIndex] = rItemData;
}

void CPythonSafeBox::DelMallItemData(DWORD dwSlotIndex) {
  if (dwSlotIndex >= m_MallItemInstanceVector.size()) {
    TraceError(
        "CPythonSafeBox::DelMallItemData(dwSlotIndex=%d) - Strange slot index",
        dwSlotIndex);
    return;
  }

  TItemData &rInstance = m_MallItemInstanceVector[dwSlotIndex];
  ZeroMemory(&rInstance, sizeof(rInstance));
}

BOOL CPythonSafeBox::GetMallItemDataPtr(DWORD dwSlotIndex,
                                        TItemData **ppInstance) {
  if (dwSlotIndex >= m_MallItemInstanceVector.size()) {
    TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange "
               "slot index",
               dwSlotIndex);
    return FALSE;
  }

  *ppInstance = &m_MallItemInstanceVector[dwSlotIndex];

  return TRUE;
}

BOOL CPythonSafeBox::GetSlotMallItemID(DWORD dwSlotIndex, DWORD *pdwItemID) {
  if (dwSlotIndex >= m_MallItemInstanceVector.size()) {
    TraceError("CPythonSafeBox::GetMallSlotItemID(dwSlotIndex=%d) - Strange "
               "slot index",
               dwSlotIndex);
    return FALSE;
  }

  *pdwItemID = m_MallItemInstanceVector[dwSlotIndex].vnum;

  return TRUE;
}

DWORD CPythonSafeBox::GetMallSize() { return m_MallItemInstanceVector.size(); }

CPythonSafeBox::CPythonSafeBox() { m_dwMoney = 0; }

CPythonSafeBox::~CPythonSafeBox() {}

PyObject *safeboxGetCurrentSafeboxSize(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonSafeBox::Instance().GetCurrentSafeBoxSize());
}

PyObject *safeboxGetItemID(PyObject *poSelf, PyObject *poArgs) {
  int ipos;
  if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    return Py_BadArgument();

  TItemData *pInstance;
  if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
    return Py_BuildException();

  return Py_BuildValue("i", pInstance->vnum);
}

PyObject *safeboxGetItemCount(PyObject *poSelf, PyObject *poArgs) {
  int ipos;
  if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    return Py_BadArgument();

  TItemData *pInstance;
  if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
    return Py_BuildException();

  return Py_BuildValue("i", pInstance->count);
}

PyObject *safeboxGetItemFlags(PyObject *poSelf, PyObject *poArgs) {
  int ipos;
  if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    return Py_BadArgument();

  TItemData *pInstance;
  if (!CPythonSafeBox::Instance().GetItemDataPtr(ipos, &pInstance))
    return Py_BuildException();

  return Py_BuildValue("i", pInstance->flags);
}

PyObject *safeboxGetItemMetinSocket(PyObject *poSelf, PyObject *poArgs) {
  int iSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    return Py_BadArgument();
  int iSocketIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
    return Py_BadArgument();

  if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
    return Py_BuildException();

  TItemData *pItemData;
  if (!CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
    return Py_BuildException();

  return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject *safeboxGetItemAttribute(PyObject *poSelf, PyObject *poArgs) {
  int iSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    return Py_BuildException();
  int iAttrSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
    return Py_BuildException();

  if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM) {
    TItemData *pItemData;
    if (CPythonSafeBox::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
      return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType,
                           pItemData->aAttr[iAttrSlotIndex].sValue);
  }

  return Py_BuildValue("ii", 0, 0);
}

PyObject *safeboxGetMoney(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonSafeBox::Instance().GetMoney());
}

PyObject *safeboxGetMallItemID(PyObject *poSelf, PyObject *poArgs) {
  int ipos;
  if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    return Py_BadArgument();

  TItemData *pInstance;
  if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
    return Py_BuildException();

  return Py_BuildValue("i", pInstance->vnum);
}

PyObject *safeboxGetMallItemCount(PyObject *poSelf, PyObject *poArgs) {
  int ipos;
  if (!PyTuple_GetInteger(poArgs, 0, &ipos))
    return Py_BadArgument();

  TItemData *pInstance;
  if (!CPythonSafeBox::Instance().GetMallItemDataPtr(ipos, &pInstance))
    return Py_BuildException();

  return Py_BuildValue("i", pInstance->count);
}

PyObject *safeboxGetMallItemMetinSocket(PyObject *poSelf, PyObject *poArgs) {
  int iSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    return Py_BadArgument();
  int iSocketIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
    return Py_BadArgument();

  if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
    return Py_BuildException();

  TItemData *pItemData;
  if (!CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
    return Py_BuildException();

  return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject *safeboxGetMallItemAttribute(PyObject *poSelf, PyObject *poArgs) {
  int iSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
    return Py_BuildException();
  int iAttrSlotIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
    return Py_BuildException();

  if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM) {
    TItemData *pItemData;
    if (CPythonSafeBox::Instance().GetMallItemDataPtr(iSlotIndex, &pItemData))
      return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType,
                           pItemData->aAttr[iAttrSlotIndex].sValue);
  }

  return Py_BuildValue("ii", 0, 0);
}

PyObject *safeboxGetMallSize(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonSafeBox::Instance().GetMallSize());
}

void initsafebox() {
  static PyMethodDef s_methods[] = {
      // SafeBox
      {"GetCurrentSafeboxSize", safeboxGetCurrentSafeboxSize, METH_VARARGS},
      {"GetItemID", safeboxGetItemID, METH_VARARGS},
      {"GetItemCount", safeboxGetItemCount, METH_VARARGS},
      {"GetItemFlags", safeboxGetItemFlags, METH_VARARGS},
      {"GetItemMetinSocket", safeboxGetItemMetinSocket, METH_VARARGS},
      {"GetItemAttribute", safeboxGetItemAttribute, METH_VARARGS},
      {"GetMoney", safeboxGetMoney, METH_VARARGS},

      // Mall
      {"GetMallItemID", safeboxGetMallItemID, METH_VARARGS},
      {"GetMallItemCount", safeboxGetMallItemCount, METH_VARARGS},
      {"GetMallItemMetinSocket", safeboxGetMallItemMetinSocket, METH_VARARGS},
      {"GetMallItemAttribute", safeboxGetMallItemAttribute, METH_VARARGS},
      {"GetMallSize", safeboxGetMallSize, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("safebox", s_methods);
  PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_X_COUNT",
                          CPythonSafeBox::SAFEBOX_SLOT_X_COUNT);
  PyModule_AddIntConstant(poModule, "SAFEBOX_SLOT_Y_COUNT",
                          CPythonSafeBox::SAFEBOX_SLOT_Y_COUNT);
  PyModule_AddIntConstant(poModule, "SAFEBOX_PAGE_SIZE",
                          CPythonSafeBox::SAFEBOX_PAGE_SIZE);
}
