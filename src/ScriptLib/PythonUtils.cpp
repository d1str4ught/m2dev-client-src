#include "StdAfx.h"
#include "PythonUtils.h"

#define PyLong_AsLong PyLong_AsLongLong
#define PyLong_AsUnsignedLong PyLong_AsUnsignedLongLong

IPythonExceptionSender *g_pkExceptionSender = NULL;

bool __PyCallClassMemberFunc_ByCString(PyObject *poClass, const char *c_szFunc,
                                       PyObject *poArgs, PyObject **poRet);
bool __PyCallClassMemberFunc_ByPyString(PyObject *poClass, PyObject *poFuncName,
                                        PyObject *poArgs, PyObject **poRet);
bool __PyCallClassMemberFunc(PyObject *poClass, PyObject *poFunc,
                             PyObject *poArgs, PyObject **poRet);

PyObject *Py_BadArgument() {
  PyErr_BadArgument();
  return NULL;
}

PyObject *Py_BuildException(const char *c_pszErr, ...) {
  if (!c_pszErr) {
    PyErr_Clear();
    return Py_BuildNone();
  }

  char szErrBuf[512 + 1];
  va_list args;
  va_start(args, c_pszErr);
  vsnprintf(szErrBuf, sizeof(szErrBuf), c_pszErr, args);
  va_end(args);

  PyErr_SetString(PyExc_RuntimeError, szErrBuf);
  return NULL;
}

PyObject *Py_BuildNone() {
  Py_INCREF(Py_None);
  return Py_None;
}

void Py_ReleaseNone() { Py_DECREF(Py_None); }

bool PyTuple_GetObject(PyObject *poArgs, int pos, PyObject **ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = poItem;
  return true;
}

bool PyTuple_GetLong(PyObject *poArgs, int pos, long *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsLong(poItem);
  return true;
}

bool PyTuple_GetLongLong(PyObject *poArgs, int pos, long long *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsLongLong(poItem);
  return true;
}

bool PyTuple_GetDouble(PyObject *poArgs, int pos, double *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyFloat_AsDouble(poItem);
  return true;
}

bool PyTuple_GetFloat(PyObject *poArgs, int pos, float *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = float(PyFloat_AsDouble(poItem));
  return true;
}

bool PyTuple_GetByte(PyObject *poArgs, int pos, unsigned char *ret) {
  int val;
  bool result = PyTuple_GetInteger(poArgs, pos, &val);
  *ret = unsigned char(val);
  return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, unsigned char *ret) {
  int val;
  bool result = PyTuple_GetInteger(poArgs, pos, &val);
  *ret = unsigned char(val);
  return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, WORD *ret) {
  int val;
  bool result = PyTuple_GetInteger(poArgs, pos, &val);
  *ret = WORD(val);
  return result;
}

bool PyTuple_GetInteger(PyObject *poArgs, int pos, int *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsLong(poItem);
  return true;
}

bool PyTuple_GetUnsignedLong(PyObject *poArgs, int pos, unsigned long *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsUnsignedLong(poItem);
  return true;
}

bool PyTuple_GetUnsignedLongLong(PyObject *poArgs, int pos,
                                 unsigned long long *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsUnsignedLongLong(poItem);
  return true;
}

bool PyTuple_GetUnsignedInteger(PyObject *poArgs, int pos, unsigned int *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsUnsignedLong(poItem);
  return true;
}

bool PyTuple_GetString(PyObject *poArgs, int pos, char **ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  if (!PyString_Check(poItem))
    return false;

  *ret = PyString_AsString(poItem);
  return true;
}

bool PyTuple_GetBoolean(PyObject *poArgs, int pos, bool *ret) {
  if (pos >= PyTuple_Size(poArgs))
    return false;

  PyObject *poItem = PyTuple_GetItem(poArgs, pos);

  if (!poItem)
    return false;

  *ret = PyLong_AsLong(poItem) ? true : false;
  return true;
}

bool PyCallClassMemberFunc(PyObject *poClass, PyObject *poFunc,
                           PyObject *poArgs) {
  PyObject *poRet;

  // NOTE : NULL 체크 추가.. - [levites]
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  if (!__PyCallClassMemberFunc(poClass, poFunc, poArgs, &poRet))
    return false;

  Py_DECREF(poRet);
  return true;
}

bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs) {
  PyObject *poRet;

  // NOTE : NULL 체크 추가.. - [levites]
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
    return false;

  Py_DECREF(poRet);
  return true;
}

bool PyCallClassMemberFunc_ByPyString(PyObject *poClass, PyObject *poFuncName,
                                      PyObject *poArgs) {
  PyObject *poRet;

  // NOTE : NULL 체크 추가.. - [levites]
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  if (!__PyCallClassMemberFunc_ByPyString(poClass, poFuncName, poArgs, &poRet))
    return false;

  Py_DECREF(poRet);
  return true;
}

bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs, bool *pisRet) {
  PyObject *poRet;

  if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
    return false;

  if (PyNumber_Check(poRet))
    *pisRet = (PyLong_AsLong(poRet) != 0);
  else
    *pisRet = true;

  Py_DECREF(poRet);
  return true;
}

bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs, long *plRetValue) {
  PyObject *poRet;

  if (!__PyCallClassMemberFunc_ByCString(poClass, c_szFunc, poArgs, &poRet))
    return false;

  if (PyNumber_Check(poRet)) {
    *plRetValue = PyLong_AsLong(poRet);
    Py_DECREF(poRet);
    return true;
  }

  Py_DECREF(poRet);
  return false;
}

/*
 *	이 함수를 직접 호출하지 않도록 한다.
 *	부득이 하게 직접 호출할 경우에는 반드시 false 가 리턴 됐을 때
 *	Py_DECREF(poArgs); 를 해준다.
 */
bool __PyCallClassMemberFunc_ByCString(PyObject *poClass, const char *c_szFunc,
                                       PyObject *poArgs, PyObject **ppoRet) {
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  PyObject *poFunc =
      PyObject_GetAttrString(poClass, (char *)c_szFunc); // New Reference

  if (!poFunc) {
    PyErr_Clear();
    Py_XDECREF(poArgs);
    return false;
  }

  if (!PyCallable_Check(poFunc)) {
    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  PyObject *poRet = PyObject_CallObject(poFunc, poArgs); // New Reference

  if (!poRet) {
    if (g_pkExceptionSender)
      g_pkExceptionSender->Clear();

    PyErr_Print();

    if (g_pkExceptionSender)
      g_pkExceptionSender->Send();

    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  *ppoRet = poRet;

  Py_DECREF(poFunc);
  Py_XDECREF(poArgs);
  return true;
}

bool __PyCallClassMemberFunc_ByPyString(PyObject *poClass, PyObject *poFuncName,
                                        PyObject *poArgs, PyObject **ppoRet) {
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  PyObject *poFunc = PyObject_GetAttr(poClass, poFuncName); // New Reference

  if (!poFunc) {
    PyErr_Clear();
    Py_XDECREF(poArgs);
    return false;
  }

  if (!PyCallable_Check(poFunc)) {
    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  PyObject *poRet = PyObject_CallObject(poFunc, poArgs); // New Reference

  if (!poRet) {
    if (g_pkExceptionSender)
      g_pkExceptionSender->Clear();

    PyErr_Print();

    if (g_pkExceptionSender)
      g_pkExceptionSender->Send();

    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  *ppoRet = poRet;

  Py_DECREF(poFunc);
  Py_XDECREF(poArgs);
  return true;
}

bool __PyCallClassMemberFunc(PyObject *poClass, PyObject *poFunc,
                             PyObject *poArgs, PyObject **ppoRet) {
  if (!poClass) {
    Py_XDECREF(poArgs);
    return false;
  }

  if (!poFunc) {
    PyErr_Clear();
    Py_XDECREF(poArgs);
    return false;
  }

  if (!PyCallable_Check(poFunc)) {
    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  PyObject *poRet = PyObject_CallObject(poFunc, poArgs); // New Reference

  if (!poRet) {
    PyErr_Print();
    Py_DECREF(poFunc);
    Py_XDECREF(poArgs);
    return false;
  }

  *ppoRet = poRet;

  Py_DECREF(poFunc);
  Py_XDECREF(poArgs);
  return true;
}