#include "StdAfx.h"
#include "PythonIME.h"
#include "PythonApplication.h"

PyObject *imeEnable(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().Initialize(
      CPythonApplication::Instance().GetWindowHandle());
  return Py_BuildNone();
}

PyObject *imeDisable(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().Uninitialize();
  return Py_BuildNone();
}

PyObject *imeEnableCaptureInput(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().EnableCaptureInput();
  return Py_BuildNone();
}

PyObject *imeDisableCaptureInput(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().DisableCaptureInput();
  return Py_BuildNone();
}

PyObject *imeSetMax(PyObject *poSelf, PyObject *poArgs) {
  int iMax;
  if (!PyTuple_GetInteger(poArgs, 0, &iMax))
    return Py_BuildException();

  CPythonIME::Instance().SetMax(iMax);
  return Py_BuildNone();
}

PyObject *imeSetUserMax(PyObject *poSelf, PyObject *poArgs) {
  int iMax;
  if (!PyTuple_GetInteger(poArgs, 0, &iMax))
    return Py_BuildException();

  CPythonIME::Instance().SetUserMax(iMax);
  return Py_BuildNone();
}

PyObject *imeSetText(PyObject *poSelf, PyObject *poArgs) {
  char *szText;
  if (!PyTuple_GetString(poArgs, 0, &szText))
    return Py_BuildException();

  CPythonIME::Instance().SetText(szText, strlen(szText));
  return Py_BuildNone();
}

PyObject *imeGetText(PyObject *poSelf, PyObject *poArgs) {
  std::string strText;
  CPythonIME::Instance().GetText(strText);
  return Py_BuildValue("s", strText.c_str());
}

PyObject *imeGetCandidateCount(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonIME::Instance().GetCandidatePageCount());
}

PyObject *imeGetCandidate(PyObject *poSelf, PyObject *poArgs) {
  int index;
  if (!PyTuple_GetInteger(poArgs, 0, &index))
    return Py_BuildException();

  std::string strText;
  int iLength = CPythonIME::Instance().GetCandidate(index, strText);
  return Py_BuildValue("si", strText.c_str(), iLength);
}

PyObject *imeGetCandidateSelection(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonIME::Instance().GetCandidateSelection());
}

PyObject *imeGetReading(PyObject *poSelf, PyObject *poArgs) {
  std::string strText;
  CPythonIME::Instance().GetReading(strText);
  return Py_BuildValue("s", strText.c_str());
}

PyObject *imeGetReadingError(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonIME::Instance().GetReadingError());
}

PyObject *imeEnableIME(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().EnableIME();
  return Py_BuildNone();
}

PyObject *imeDisableIME(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().DisableIME();
  return Py_BuildNone();
}

PyObject *imeSetInputMode(PyObject *poSelf, PyObject *poArgs) {
  int mode;
  if (!PyTuple_GetInteger(poArgs, 0, &mode))
    return Py_BuildException();

  CPythonIME::Instance().SetInputMode(mode);
  return Py_BuildNone();
}

PyObject *imeSetNumberMode(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().SetNumberMode();
  return Py_BuildNone();
}

PyObject *imeAddExceptKey(PyObject *poSelf, PyObject *poArgs) {
  int key;
  if (!PyTuple_GetInteger(poArgs, 0, &key))
    return Py_BuildException();

  CPythonIME::Instance().AddExceptKey(key);
  return Py_BuildNone();
}

PyObject *imeClearExceptKey(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().ClearExceptKey();
  return Py_BuildNone();
}

PyObject *imeSetStringMode(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().SetStringMode();
  return Py_BuildNone();
}

PyObject *imeGetInputMode(PyObject *poSelf, PyObject *poArgs) {
  return Py_BuildValue("i", CPythonIME::Instance().GetInputMode());
}

PyObject *imeMoveLeft(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().MoveLeft();

  return Py_BuildNone();
}
PyObject *imeMoveRight(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().MoveRight();

  return Py_BuildNone();
}

PyObject *imeMoveHome(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().MoveHome();

  return Py_BuildNone();
}
PyObject *imeMoveEnd(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().MoveEnd();

  return Py_BuildNone();
}

PyObject *imeSetCursorPosition(PyObject *poSelf, PyObject *poArgs) {
  int iPosition;
  if (!PyTuple_GetInteger(poArgs, 0, &iPosition))
    return Py_BuildException();

  CPythonIME::Instance().SetCursorPosition(iPosition);

  return Py_BuildNone();
}

PyObject *imeDelete(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().Delete();

  return Py_BuildNone();
}

PyObject *imePasteTextFromClipBoard(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().PasteTextFromClipBoard();
  return Py_BuildNone();
}

PyObject *imeSelectAll(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().SelectAll();
  return Py_BuildNone();
}

PyObject *imeCopySelectionToClipboard(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().CopySelectionToClipboard();
  return Py_BuildNone();
}

PyObject *imeCutSelection(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().CopySelectionToClipboard();
  CPythonIME::Instance().DeleteSelection();
  return Py_BuildNone();
}

PyObject *imeEnablePaste(PyObject *poSelf, PyObject *poArgs) {
  int iFlag;
  if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
    return Py_BuildException();

  CPythonIME::Instance().EnablePaste(iFlag ? true : false);
  return Py_BuildNone();
}

PyObject *imePasteString(PyObject *poSelf, PyObject *poArgs) {
  char *szText;
  if (!PyTuple_GetString(poArgs, 0, &szText))
    return Py_BuildException();
  CPythonIME::Instance().PasteString(szText);
  return Py_BuildNone();
}

PyObject *imePasteBackspace(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().WMChar(NULL, WM_CHAR, 0x08, NULL);
  return Py_BuildNone();
}

PyObject *imePasteReturn(PyObject *poSelf, PyObject *poArgs) {
  CPythonIME::Instance().WMChar(NULL, WM_CHAR, 0x0D, NULL);
  return Py_BuildNone();
}

void initime() {
  static PyMethodDef s_methods[] = {
      {"Enable", imeEnable, METH_VARARGS},
      {"Disable", imeDisable, METH_VARARGS},

      {"EnableCaptureInput", imeEnableCaptureInput, METH_VARARGS},
      {"DisableCaptureInput", imeDisableCaptureInput, METH_VARARGS},
      {"SetMax", imeSetMax, METH_VARARGS},
      {"SetUserMax", imeSetUserMax, METH_VARARGS},
      {"SetText", imeSetText, METH_VARARGS},
      {"GetText", imeGetText, METH_VARARGS},
      {"GetCandidateCount", imeGetCandidateCount, METH_VARARGS},
      {"GetCandidate", imeGetCandidate, METH_VARARGS},
      {"GetCandidateSelection", imeGetCandidateSelection, METH_VARARGS},
      {"GetReading", imeGetReading, METH_VARARGS},
      {"GetReadingError", imeGetReadingError, METH_VARARGS},
      {"EnableIME", imeEnableIME, METH_VARARGS},
      {"DisableIME", imeDisableIME, METH_VARARGS},
      {"GetInputMode", imeGetInputMode, METH_VARARGS},
      {"SetInputMode", imeSetInputMode, METH_VARARGS},

      {"SetNumberMode", imeSetNumberMode, METH_VARARGS},
      {"SetStringMode", imeSetStringMode, METH_VARARGS},
      {"AddExceptKey", imeAddExceptKey, METH_VARARGS},
      {"ClearExceptKey", imeClearExceptKey, METH_VARARGS},

      {"MoveLeft", imeMoveLeft, METH_VARARGS},
      {"MoveRight", imeMoveRight, METH_VARARGS},
      {"MoveHome", imeMoveHome, METH_VARARGS},
      {"MoveEnd", imeMoveEnd, METH_VARARGS},
      {"SetCursorPosition", imeSetCursorPosition, METH_VARARGS},
      {"Delete", imeDelete, METH_VARARGS},
      {"PasteString", imePasteString, METH_VARARGS},
      {"PasteBackspace", imePasteBackspace, METH_VARARGS},
      {"PasteReturn", imePasteReturn, METH_VARARGS},
      {"PasteTextFromClipBoard", imePasteTextFromClipBoard, METH_VARARGS},
      {"SelectAll", imeSelectAll, METH_VARARGS},
      {"CutSelection", imeCutSelection, METH_VARARGS},
      {"CopySelection", imeCopySelectionToClipboard, METH_VARARGS},
      {"EnablePaste", imeEnablePaste, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  Py_InitModule("ime", s_methods);
}