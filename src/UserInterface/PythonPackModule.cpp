#include "StdAfx.h"
#include "PackLib/PackManager.h"

PyObject *packExist(PyObject *poSelf, PyObject *poArgs) {
  char *strFileName;

  if (!PyTuple_GetString(poArgs, 0, &strFileName))
    return Py_BuildException();

  return Py_BuildValue("i",
                       CPackManager::Instance().IsExist(strFileName) ? 1 : 0);
}

PyObject *packGet(PyObject *poSelf, PyObject *poArgs) {
  char *strFileName;

  if (!PyTuple_GetString(poArgs, 0, &strFileName))
    return Py_BuildException();

  // 파이썬에서 읽어드리는 패킹 파일은 python 파일과 txt 파일에 한정한다
  const char *pcExt = strrchr(strFileName, '.');
  if (pcExt) // 확장자가 있고
  {
    if ((stricmp(pcExt, ".py") == 0) || (stricmp(pcExt, ".pyc") == 0) ||
        (stricmp(pcExt, ".txt") == 0)) {
      TPackFile file;
      if (CPackManager::Instance().GetFile(strFileName, file))
        return Py_BuildValue("s#", file.data(), file.size());
    }
  }

  return Py_BuildException();
}

void initpack() {
  static PyMethodDef s_methods[] = {
      {"Exist", packExist, METH_VARARGS},
      {"Get", packGet, METH_VARARGS},
      {NULL, NULL},
  };

  Py_InitModule("pack", s_methods);
}