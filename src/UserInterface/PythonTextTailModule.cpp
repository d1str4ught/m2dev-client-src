#include "StdAfx.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"

PyObject *textTailClear(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().Clear();
  return Py_BuildNone();
}

PyObject *textTailUpdateAllTextTail(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().UpdateAllTextTail();
  return Py_BuildNone();
}

PyObject *textTailUpdateShowingTextTail(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().UpdateShowingTextTail();
  return Py_BuildNone();
}

PyObject *textTailRender(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().Render();
  return Py_BuildNone();
}

PyObject *textTailRegisterCharacterTextTail(PyObject *poSelf,
                                            PyObject *poArgs) {
  int iGuildID;
  if (!PyTuple_GetInteger(poArgs, 0, &iGuildID))
    return Py_BuildException();
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 1, &iVirtualID))
    return Py_BuildException();

  CPythonTextTail::Instance().RegisterCharacterTextTail(
      iGuildID, iVirtualID, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));

  return Py_BuildNone();
}

PyObject *textTailGetPosition(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  bool isData =
      CPythonTextTail::Instance().GetTextTailPosition(VirtualID, &x, &y, &z);
  if (!isData) {
    CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
    CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
    if (pkInstMain) {
      const D3DXVECTOR3 &c_rv3Position =
          pkInstMain->GetGraphicThingInstanceRef().GetPosition();
      CPythonGraphic::Instance().ProjectPosition(
          c_rv3Position.x, c_rv3Position.y, c_rv3Position.z, &x, &y);
    }
  }

  return Py_BuildValue("fff", x, y, z);
}

PyObject *textTailIsChat(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  return Py_BuildValue("i",
                       CPythonTextTail::Instance().IsChatTextTail(VirtualID));
}

PyObject *textTailRegisterChatTail(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  char *szText;
  if (!PyTuple_GetString(poArgs, 1, &szText))
    return Py_BuildException();

  CPythonTextTail::Instance().RegisterChatTail(VirtualID, szText);

  return Py_BuildNone();
}

PyObject *textTailRegisterInfoTail(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  char *szText;
  if (!PyTuple_GetString(poArgs, 1, &szText))
    return Py_BuildException();

  CPythonTextTail::Instance().RegisterInfoTail(VirtualID, szText);

  return Py_BuildNone();
}

PyObject *textTailAttachTitle(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
    return Py_BuildException();
  char *szName;
  if (!PyTuple_GetString(poArgs, 1, &szName))
    return Py_BuildException();
  float fr;
  if (!PyTuple_GetFloat(poArgs, 2, &fr))
    return Py_BuildException();
  float fg;
  if (!PyTuple_GetFloat(poArgs, 3, &fg))
    return Py_BuildException();
  float fb;
  if (!PyTuple_GetFloat(poArgs, 4, &fb))
    return Py_BuildException();

  CPythonTextTail::Instance().AttachTitle(iVirtualID, szName,
                                          D3DXCOLOR(fr, fg, fb, 1.0f));

  return Py_BuildNone();
}

PyObject *textTailShowCharacterTextTail(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  CPythonTextTail::Instance().ShowCharacterTextTail(VirtualID);
  return Py_BuildNone();
}

PyObject *textTailShowItemTextTail(PyObject *poSelf, PyObject *poArgs) {
  int VirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
    return Py_BuildException();

  CPythonTextTail::Instance().ShowItemTextTail(VirtualID);
  return Py_BuildNone();
}

PyObject *textTailArrangeTextTail(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().ArrangeTextTail();
  return Py_BuildNone();
}

PyObject *textTailHideAllTextTail(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().HideAllTextTail();
  return Py_BuildNone();
}

PyObject *textTailShowAllTextTail(PyObject *poSelf, PyObject *poArgs) {
  CPythonTextTail::Instance().ShowAllTextTail();
  return Py_BuildNone();
}

PyObject *textTailPick(PyObject *poSelf, PyObject *poArgs) {
  int ix;
  if (!PyTuple_GetInteger(poArgs, 0, &ix))
    return Py_BuildException();
  int iy;
  if (!PyTuple_GetInteger(poArgs, 1, &iy))
    return Py_BuildException();

  int iValue = CPythonTextTail::Instance().Pick(ix, iy);
  return Py_BuildValue("i", iValue);
}

PyObject *textTailSelectItemName(PyObject *poSelf, PyObject *poArgs) {
  int iVirtualID;
  if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
    return Py_BuildException();

  CPythonTextTail::Instance().SelectItemName(iVirtualID);
  return Py_BuildNone();
}

PyObject *textTailEnablePKTitle(PyObject *poSelf, PyObject *poArgs) {
  int iFlag;
  if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    return Py_BuildException();

  CPythonTextTail::Instance().EnablePKTitle(iFlag);
  return Py_BuildNone();
}

void initTextTail() {
  static PyMethodDef s_methods[] = {
      {"Clear", textTailClear, METH_VARARGS},

      {"UpdateAllTextTail", textTailUpdateAllTextTail, METH_VARARGS},
      {"UpdateShowingTextTail", textTailUpdateShowingTextTail, METH_VARARGS},
      {"Render", textTailRender, METH_VARARGS},

      {"ShowCharacterTextTail", textTailShowCharacterTextTail, METH_VARARGS},
      {"ShowItemTextTail", textTailShowItemTextTail, METH_VARARGS},

      {"GetPosition", textTailGetPosition, METH_VARARGS},
      {"IsChat", textTailIsChat, METH_VARARGS},

      {"ArrangeTextTail", textTailArrangeTextTail, METH_VARARGS},
      {"HideAllTextTail", textTailHideAllTextTail, METH_VARARGS},
      {"ShowAllTextTail", textTailShowAllTextTail, METH_VARARGS},

      {"Pick", textTailPick, METH_VARARGS},
      {"SelectItemName", textTailSelectItemName, METH_VARARGS},

      {"EnablePKTitle", textTailEnablePKTitle, METH_VARARGS},

      // For Test
      {"RegisterCharacterTextTail", textTailRegisterCharacterTextTail,
       METH_VARARGS},
      {"RegisterChatTail", textTailRegisterChatTail, METH_VARARGS},
      {"RegisterInfoTail", textTailRegisterInfoTail, METH_VARARGS},
      {"AttachTitle", textTailAttachTitle, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  Py_InitModule("textTail", s_methods);
}