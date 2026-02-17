#include "StdAfx.h"
#include "PythonEventManager.h"
#include "PythonNetworkStream.h"

PyObject *eventRegisterEventSet(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BuildException();

  int iEventIndex =
      CPythonEventManager::Instance().RegisterEventSet(szFileName);
  return Py_BuildValue("i", iEventIndex);
}

PyObject *eventRegisterEventSetFromString(PyObject *poSelf, PyObject *poArgs) {
  char *szEventString;
  if (!PyTuple_GetString(poArgs, 0, &szEventString))
    return Py_BuildException();

  int iEventIndex =
      CPythonEventManager::Instance().RegisterEventSetFromString(szEventString);
  return Py_BuildValue("i", iEventIndex);
}

PyObject *eventClearEventSet(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CPythonEventManager::Instance().ClearEventSeti(iIndex);
  return Py_BuildNone();
}

PyObject *eventSetRestrictedCount(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  int iCount;
  if (!PyTuple_GetInteger(poArgs, 1, &iCount))
    return Py_BuildException();

  CPythonEventManager::Instance().SetRestrictedCount(iIndex, iCount);
  return Py_BuildNone();
}

PyObject *eventGetEventSetLocalYPosition(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  return Py_BuildValue(
      "i", CPythonEventManager::Instance().GetEventSetLocalYPosition(iIndex));
}

PyObject *eventAddEventSetLocalYPosition(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  int iPos;
  if (!PyTuple_GetInteger(poArgs, 1, &iPos))
    return Py_BuildException();

  CPythonEventManager::Instance().AddEventSetLocalYPosition(iIndex, iPos);
  return Py_BuildNone();
}

PyObject *eventInsertText(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  char *szText;
  if (!PyTuple_GetString(poArgs, 1, &szText))
    return Py_BuildException();

  CPythonEventManager::Instance().InsertText(iIndex, szText);
  return Py_BuildNone();
}

PyObject *eventInsertTextInline(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  char *szText;
  if (!PyTuple_GetString(poArgs, 1, &szText))
    return Py_BuildException();
  int iXIndex;
  if (!PyTuple_GetInteger(poArgs, 2, &iXIndex))
    return Py_BuildException();

  CPythonEventManager::Instance().InsertText(iIndex, szText, iXIndex);
  return Py_BuildNone();
}

PyObject *eventUpdateEventSet(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  int ix;
  if (!PyTuple_GetInteger(poArgs, 1, &ix))
    return Py_BuildException();

  int iy;
  if (!PyTuple_GetInteger(poArgs, 2, &iy))
    return Py_BuildException();

  CPythonEventManager::Instance().UpdateEventSet(iIndex, ix, -iy);
  return Py_BuildNone();
}

PyObject *eventRenderEventSet(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CPythonEventManager::Instance().RenderEventSet(iIndex);
  return Py_BuildNone();
}

PyObject *eventSetEventSetWidth(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();
  int iWidth;
  if (!PyTuple_GetInteger(poArgs, 1, &iWidth))
    return Py_BuildException();

  CPythonEventManager::Instance().SetEventSetWidth(iIndex, iWidth);
  return Py_BuildNone();
}

PyObject *eventSkip(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CPythonEventManager::Instance().Skip(iIndex);
  return Py_BuildNone();
}

PyObject *eventIsWait(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  return Py_BuildValue(
      "i", CPythonEventManager::Instance().IsWait(iIndex) == true ? 1 : 0);
}

PyObject *eventEndEventProcess(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CPythonEventManager::Instance().EndEventProcess(iIndex);

  return Py_BuildNone();
}

PyObject *eventSetEventHandler(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  PyObject *poEventHandler;
  if (!PyTuple_GetObject(poArgs, 1, &poEventHandler))
    return Py_BuildException();

  CPythonEventManager::Instance().SetEventHandler(iIndex, poEventHandler);
  return Py_BuildNone();
}

PyObject *eventSelectAnswer(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  int iAnswer;
  if (!PyTuple_GetInteger(poArgs, 1, &iAnswer))
    return Py_BuildException();

  CPythonEventManager::Instance().SelectAnswer(iIndex, iAnswer);
  return Py_BuildNone();
}

PyObject *eventGetLineCount(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  int iLineCount = CPythonEventManager::Instance().GetLineCount(iIndex);
  return Py_BuildValue("i", iLineCount);
}

PyObject *eventSetVisibleStartLine(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  int iStartLine;
  if (!PyTuple_GetInteger(poArgs, 1, &iStartLine))
    return Py_BuildException();

  CPythonEventManager::Instance().SetVisibleStartLine(iIndex, iStartLine);
  return Py_BuildNone();
}

PyObject *eventGetVisibleStartLine(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  return Py_BuildValue(
      "i", CPythonEventManager::Instance().GetVisibleStartLine(iIndex));
}

PyObject *eventQuestButtonClick(PyObject *poSelf, PyObject *poArgs) {
  int iIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
    return Py_BuildException();

  CPythonNetworkStream::Instance().SendScriptButtonPacket(iIndex);

  return Py_BuildNone();
}

PyObject *eventSetInterfaceWindow(PyObject *poSelf, PyObject *poArgs) {
  PyObject *pyHandle;
  if (!PyTuple_GetObject(poArgs, 0, &pyHandle))
    return Py_BadArgument();

  CPythonEventManager &rpem = CPythonEventManager::Instance();
  rpem.SetInterfaceWindow(pyHandle);
  return Py_BuildNone();
}

PyObject *eventSetLeftTimeString(PyObject *poSelf, PyObject *poArgs) {
  char *szText;
  if (!PyTuple_GetString(poArgs, 0, &szText))
    return Py_BuildException();

  CPythonEventManager &rpem = CPythonEventManager::Instance();
  rpem.SetLeftTimeString(szText);
  return Py_BuildNone();
}

PyObject *eventDestroy(PyObject *poSelf, PyObject *poArgs) {
  CPythonEventManager &rpem = CPythonEventManager::Instance();
  rpem.Destroy();
  return Py_BuildNone();
}

void initEvent() {
  static PyMethodDef s_methods[] = {
      {"RegisterEventSet", eventRegisterEventSet, METH_VARARGS},
      {"RegisterEventSetFromString", eventRegisterEventSetFromString,
       METH_VARARGS},
      {"ClearEventSet", eventClearEventSet, METH_VARARGS},

      {"SetRestrictedCount", eventSetRestrictedCount, METH_VARARGS},

      {"GetEventSetLocalYPosition", eventGetEventSetLocalYPosition,
       METH_VARARGS},
      {"AddEventSetLocalYPosition", eventAddEventSetLocalYPosition,
       METH_VARARGS},
      {"InsertText", eventInsertText, METH_VARARGS},
      {"InsertTextInline", eventInsertTextInline, METH_VARARGS},

      {"UpdateEventSet", eventUpdateEventSet, METH_VARARGS},
      {"RenderEventSet", eventRenderEventSet, METH_VARARGS},
      {"SetEventSetWidth", eventSetEventSetWidth, METH_VARARGS},

      {"Skip", eventSkip, METH_VARARGS},
      {"IsWait", eventIsWait, METH_VARARGS},
      {"EndEventProcess", eventEndEventProcess, METH_VARARGS},

      {"SelectAnswer", eventSelectAnswer, METH_VARARGS},
      {"GetLineCount", eventGetLineCount, METH_VARARGS},
      {"SetVisibleStartLine", eventSetVisibleStartLine, METH_VARARGS},
      {"GetVisibleStartLine", eventGetVisibleStartLine, METH_VARARGS},

      {"SetEventHandler", eventSetEventHandler, METH_VARARGS},
      {"SetInterfaceWindow", eventSetInterfaceWindow, METH_VARARGS},
      {"SetLeftTimeString", eventSetLeftTimeString, METH_VARARGS},

      {"QuestButtonClick", eventQuestButtonClick, METH_VARARGS},
      {"Destroy", eventDestroy, METH_VARARGS},
      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("event", s_methods);

  PyModule_AddIntConstant(poModule, "BOX_VISIBLE_LINE_COUNT",
                          CPythonEventManager::BOX_VISIBLE_LINE_COUNT);
  PyModule_AddIntConstant(poModule, "BUTTON_TYPE_NEXT",
                          CPythonEventManager::BUTTON_TYPE_NEXT);
  PyModule_AddIntConstant(poModule, "BUTTON_TYPE_DONE",
                          CPythonEventManager::BUTTON_TYPE_DONE);
  PyModule_AddIntConstant(poModule, "BUTTON_TYPE_CANCEL",
                          CPythonEventManager::BUTTON_TYPE_CANCEL);
}