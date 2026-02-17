#include "StdAfx.h"

extern IPythonExceptionSender *g_pkExceptionSender;

PyObject *dbgLogBox(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  char *szCaption;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();
  if (!PyTuple_GetString(poArgs, 1, &szCaption)) {
    LogBox(szMsg);
  } else {
    LogBox(szMsg, szCaption);
  }
  return Py_BuildNone();
}

PyObject *dbgTrace(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  Trace(szMsg);
  return Py_BuildNone();
}

PyObject *dbgTracen(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  Tracen(szMsg);
  return Py_BuildNone();
}

PyObject *dbgTraceError(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  TraceError("%s", szMsg);
  return Py_BuildNone();
}

// MR-11: Temporary trace functions for debugging (not for regular logging)
PyObject *dbgTraceTemp(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  TempTrace(szMsg, false);
  return Py_BuildNone();
}

PyObject *dbgTraceTempn(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  TempTracen(szMsg, false);
  return Py_BuildNone();
}
// MR-11: -- END OF -- Temporary trace functions for debugging (not for regular
// logging)

PyObject *dbgRegisterExceptionString(PyObject *poSelf, PyObject *poArgs) {
  char *szMsg;
  if (!PyTuple_GetString(poArgs, 0, &szMsg))
    return Py_BuildException();

  if (g_pkExceptionSender)
    g_pkExceptionSender->RegisterExceptionString(szMsg);

  return Py_BuildNone();
}

void initdbg() {
  static PyMethodDef s_methods[] = {
      {"LogBox", dbgLogBox, METH_VARARGS},
      {"Trace", dbgTrace, METH_VARARGS},
      {"Tracen", dbgTracen, METH_VARARGS},
      {"TraceError", dbgTraceError, METH_VARARGS},
      // MR-11: Temporary trace functions for debugging (not for regular
      // logging)
      {"TraceTemp", dbgTraceTemp, METH_VARARGS},
      {"TraceTempn", dbgTraceTempn, METH_VARARGS},
      // MR-11: -- END OF -- Temporary trace functions for debugging (not for
      // regular logging)
      {"RegisterExceptionString", dbgRegisterExceptionString, METH_VARARGS},
      {NULL, NULL},
  };

  Py_InitModule("dbg", s_methods);
}