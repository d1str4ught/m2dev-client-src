#pragma once

#define SET_EXCEPTION(x) PyErr_SetString(PyExc_RuntimeError, #x)

bool PyTuple_GetString(PyObject *poArgs, int pos, char **ret);
bool PyTuple_GetInteger(PyObject *poArgs, int pos, unsigned char *ret);
bool PyTuple_GetInteger(PyObject *poArgs, int pos, int *ret);
bool PyTuple_GetInteger(PyObject *poArgs, int pos, WORD *ret);
bool PyTuple_GetByte(PyObject *poArgs, int pos, unsigned char *ret);
bool PyTuple_GetUnsignedInteger(PyObject *poArgs, int pos, unsigned int *ret);
bool PyTuple_GetLong(PyObject *poArgs, int pos, long *ret);
bool PyTuple_GetLongLong(PyObject *poArgs, int pos, long long *ret);
bool PyTuple_GetUnsignedLong(PyObject *poArgs, int pos, unsigned long *ret);
bool PyTuple_GetUnsignedLongLong(PyObject *poArgs, int pos,
                                 unsigned long long *ret);
bool PyTuple_GetFloat(PyObject *poArgs, int pos, float *ret);
bool PyTuple_GetDouble(PyObject *poArgs, int pos, double *ret);
bool PyTuple_GetObject(PyObject *poArgs, int pos, PyObject **ret);
bool PyTuple_GetBoolean(PyObject *poArgs, int pos, bool *ret);

template <typename T>
bool PyTuple_GetPointer(PyObject *poArgs, int pos, T **ret) {
  return PyTuple_GetUnsignedLongLong(poArgs, pos, (unsigned long long *)ret);
}

bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs);
bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs, bool *pisRet);
bool PyCallClassMemberFunc(PyObject *poClass, const char *c_szFunc,
                           PyObject *poArgs, long *plRetValue);

bool PyCallClassMemberFunc_ByPyString(PyObject *poClass, PyObject *poFuncName,
                                      PyObject *poArgs);
bool PyCallClassMemberFunc(PyObject *poClass, PyObject *poFunc,
                           PyObject *poArgs);

PyObject *Py_BuildException(const char *c_pszErr = NULL, ...);
PyObject *Py_BadArgument();
PyObject *Py_BuildNone();

// Compatibility functions between Python 2 and 3

inline int PyString_Check(PyObject *obj) {
  return obj && (PyUnicode_Check(obj) || PyBytes_Check(obj));
}

inline char *PyString_AsString(PyObject *obj) {
  if (!obj) {
    PyErr_SetString(PyExc_TypeError, "NULL Python object");
    return nullptr;
  }

  if (PyUnicode_Check(obj))
    return const_cast<char *>(PyUnicode_AsUTF8(obj));

  if (PyBytes_Check(obj))
    return PyBytes_AsString(obj);

  PyErr_SetString(PyExc_TypeError, "Expected str or bytes object");
  return nullptr;
}

inline char *PyString_AS_STRING(PyObject *obj) {
  return PyString_AsString(obj);
}

inline PyObject *PyString_FromStringAndSize(const char *str, Py_ssize_t size) {
  return PyBytes_FromStringAndSize(str, size);
}

inline PyObject *Py_InitModule(const char *name, PyMethodDef *methods) {
  PyModuleDef *moduleDef = new (std::nothrow) PyModuleDef{PyModuleDef_HEAD_INIT,
                                                          name,
                                                          nullptr,
                                                          -1,
                                                          methods,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr,
                                                          nullptr};

  if (!moduleDef)
    return PyErr_NoMemory();

  PyObject *module = PyModule_Create(moduleDef);
  if (!module) {
    delete moduleDef;
    return nullptr;
  }

  PyObject *modules = PyImport_GetModuleDict();
  if (PyDict_SetItemString(modules, name, module) < 0) {
    Py_DECREF(module);
    return nullptr;
  }

  Py_DECREF(module);
  return PyDict_GetItemString(modules, name);
}

#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#define PyInt_FromLong PyLong_FromLong

#define PyString_FromString PyUnicode_FromString
#define PyString_InternFromString PyUnicode_InternFromString
