#include "StdAfx.h"
#include <python/frameobject.h>
#ifdef BYTE
#undef BYTE
#endif

#include "PackLib/PackManager.h"

#include "PythonLauncher.h"
#include "PythonModules/frozen_modules.h"

#include <utf8.h>

CPythonLauncher::CPythonLauncher() {
  InitStandardPythonModules();
  Py_FrozenFlag = 1;

  Py_Initialize();
}

CPythonLauncher::~CPythonLauncher() { Clear(); }

void CPythonLauncher::Clear() { Py_Finalize(); }

std::string g_stTraceBuffer[512];
int g_nCurTraceN = 0;

namespace {
const char *SafePyString(PyObject *obj, const char *fallback = "") {
  if (!obj)
    return fallback;

  const char *value = PyString_AsString(obj);
  if (value)
    return value;

  PyErr_Clear();
  return fallback;
}

bool ReadCompiledFileHeader(FILE *fp) {
  const long magic = PyMarshal_ReadLongFromFile(fp);
  if (magic != PyImport_GetMagicNumber()) {
    PyErr_SetString(PyExc_RuntimeError, "Bad magic number in .pyc file");
    return false;
  }

  PyMarshal_ReadLongFromFile(fp); // flags
  PyMarshal_ReadLongFromFile(fp); // hash / timestamp
  PyMarshal_ReadLongFromFile(fp); // hash / source size

  if (PyErr_Occurred())
    return false;

  return true;
}
} // namespace

void Traceback() {
  std::string str;

  for (int i = 0; i < g_nCurTraceN; ++i) {
    str.append(g_stTraceBuffer[i]);
    str.append("\n");
  }

  if (!PyErr_Occurred()) {
    str.append("(No Python error set - failure occurred at C++ level)");
    LogBoxf("Traceback:\n\n%s\n", str.c_str());
    return;
  }

  PyObject *exc;
  PyObject *v;
  PyObject *tb;

  PyErr_Fetch(&exc, &v, &tb);
  PyErr_NormalizeException(&exc, &v, &tb);

  // Try using traceback.format_exception for full details
  PyObject *tbMod = PyImport_ImportModule("traceback");
  if (tbMod) {
    PyObject *fmtFunc = PyObject_GetAttrString(tbMod, "format_exception");
    if (fmtFunc) {
      PyObject *result =
          PyObject_CallFunction(fmtFunc, (char *)"OOO", exc ? exc : Py_None,
                                v ? v : Py_None, tb ? tb : Py_None);
      if (result && PyList_Check(result)) {
        Py_ssize_t n = PyList_Size(result);
        for (Py_ssize_t i = 0; i < n; ++i) {
          PyObject *line = PyList_GetItem(result, i);
          str.append(SafePyString(line));
        }
        Py_DECREF(result);
        Py_DECREF(fmtFunc);
        Py_DECREF(tbMod);
        Py_XDECREF(exc);
        Py_XDECREF(v);
        Py_XDECREF(tb);
        LogBoxf("Traceback:\n\n%s\n", str.c_str());
        return;
      }
      Py_XDECREF(result);
      Py_DECREF(fmtFunc);
    }
    Py_DECREF(tbMod);
  }

  if (exc) {
    PyObject *excName = PyObject_GetAttrString(exc, "__name__");
    if (excName) {
      str.append(SafePyString(excName));
      str.append(": ");
    }
    Py_XDECREF(excName);
  }

  if (v) {
    PyObject *vStr = PyObject_Str(v);
    if (vStr) {
      const char *errStr = SafePyString(vStr);
      str.append(errStr);
      Tracef("%s\n", errStr);
    }
    Py_XDECREF(vStr);
  }

  Py_XDECREF(exc);
  Py_XDECREF(v);
  Py_XDECREF(tb);
  LogBoxf("Traceback:\n\n%s\n", str.c_str());
}

int TraceFunc(PyObject *obj, PyFrameObject *f, int what, PyObject *arg) {
  char szTraceBuffer[128];

  switch (what) {
  case PyTrace_CALL: {
    if (g_nCurTraceN >= 512)
      return 0;

    PyCodeObject *code = PyFrame_GetCode(f);
    const int lineNo = PyFrame_GetLineNumber(f);

    PyObject *fileNameObj =
        code ? PyObject_GetAttrString((PyObject *)code, "co_filename") : NULL;
    PyObject *funcNameObj =
        code ? PyObject_GetAttrString((PyObject *)code, "co_name") : NULL;

    _snprintf(szTraceBuffer, sizeof(szTraceBuffer),
              "Call: File \"%s\", line %d, in %s",
              SafePyString(fileNameObj, "<unknown>"), lineNo,
              SafePyString(funcNameObj, "<unknown>"));

    g_stTraceBuffer[g_nCurTraceN++] = szTraceBuffer;

    Py_XDECREF(fileNameObj);
    Py_XDECREF(funcNameObj);
    Py_XDECREF((PyObject *)code);
  } break;

  case PyTrace_RETURN: {
    if (g_nCurTraceN > 0)
      --g_nCurTraceN;
  } break;

  case PyTrace_EXCEPTION: {
    if (g_nCurTraceN >= 512)
      return 0;

    PyCodeObject *code = PyFrame_GetCode(f);
    const int lineNo = PyFrame_GetLineNumber(f);

    PyObject *fileNameObj =
        code ? PyObject_GetAttrString((PyObject *)code, "co_filename") : NULL;
    PyObject *funcNameObj =
        code ? PyObject_GetAttrString((PyObject *)code, "co_name") : NULL;

    _snprintf(szTraceBuffer, sizeof(szTraceBuffer),
              "Exception: File \"%s\", line %d, in %s",
              SafePyString(fileNameObj, "<unknown>"), lineNo,
              SafePyString(funcNameObj, "<unknown>"));

    g_stTraceBuffer[g_nCurTraceN++] = szTraceBuffer;

    Py_XDECREF(fileNameObj);
    Py_XDECREF(funcNameObj);
    Py_XDECREF((PyObject *)code);
  } break;
  }
  return 0;
}

void CPythonLauncher::SetTraceFunc(int (*pFunc)(PyObject *obj, PyFrameObject *f,
                                                int what, PyObject *arg)) {
  PyEval_SetTrace(pFunc, NULL);
}

bool CPythonLauncher::Create() {
#ifdef _DEBUG
  PyEval_SetTrace(TraceFunc, NULL);
#endif
  m_poModule = PyImport_AddModule("__main__");

  if (!m_poModule)
    return false;

  m_poDic = PyModule_GetDict(m_poModule);

  PyObject *builtins = PyImport_ImportModule("builtins");
  if (builtins) {
    PyModule_AddIntConstant(builtins, "TRUE", 1);
    PyModule_AddIntConstant(builtins, "FALSE", 0);
    PyDict_SetItemString(m_poDic, "__builtins__", builtins);
    Py_DECREF(builtins);
  }

  if (!RunLine("import __main__"))
    return false;

  if (!RunLine("import sys"))
    return false;

  return true;
}

bool CPythonLauncher::RunCompiledFile(const char *c_szFileName) {
  std::wstring wFileName = Utf8ToWide(c_szFileName);
  FILE *fp = _wfopen(wFileName.c_str(), L"rb");

  if (!fp)
    return false;

  if (!ReadCompiledFileHeader(fp)) {
    fclose(fp);
    return false;
  }

  PyObject *code = PyMarshal_ReadLastObjectFromFile(fp);

  fclose(fp);

  if (!code || !PyCode_Check(code)) {
    Py_XDECREF(code);
    PyErr_SetString(PyExc_RuntimeError, "Bad code object in .pyc file");
    return false;
  }

  PyObject *result = PyEval_EvalCode(code, m_poDic, m_poDic);
  Py_DECREF(code);
  if (!result) {
    Traceback();
    return false;
  }

  Py_DECREF(result);

  return true;
}

bool CPythonLauncher::RunMemoryTextFile(const char *c_szFileName,
                                        UINT uFileSize,
                                        const VOID *c_pvFileData) {
  NANOBEGIN
  const CHAR *c_pcFileData = (const CHAR *)c_pvFileData;

  std::string stConvFileData;
  stConvFileData.reserve(uFileSize);
  stConvFileData += "exec(compile('''";

  for (UINT i = 0; i < uFileSize; ++i) {
    if (c_pcFileData[i] != 13)
      stConvFileData += c_pcFileData[i];
  }

  stConvFileData += "''', ";
  stConvFileData += "'";
  stConvFileData += c_szFileName;
  stConvFileData += "', ";
  stConvFileData += "'exec'))";

  const CHAR *c_pcConvFileData = stConvFileData.c_str();
  NANOEND
  return RunLine(c_pcConvFileData);
}

bool CPythonLauncher::RunFile(const char *c_szFileName) {
  TPackFile file;
  CPackManager::Instance().GetFile(c_szFileName, file);

  if (file.empty())
    return false;

  std::string source;
  source.reserve(file.size());
  for (size_t i = 0; i < file.size(); ++i) {
    if (file[i] != '\r')
      source += (char)file[i];
  }

  PyObject *code =
      Py_CompileString(source.c_str(), c_szFileName, Py_file_input);
  if (!code) {
    Traceback();
    return false;
  }

  PyObject *result = PyEval_EvalCode(code, m_poDic, m_poDic);
  Py_DECREF(code);

  if (!result) {
    Traceback();
    return false;
  }

  Py_DECREF(result);

  return true;
}

bool CPythonLauncher::RunLine(const char *c_szSrc) {
  PyObject *v = PyRun_String((char *)c_szSrc, Py_file_input, m_poDic, m_poDic);

  if (!v) {
    Traceback();
    return false;
  }

  Py_DECREF(v);
  return true;
}

const char *CPythonLauncher::GetError() {
  static std::string s_error;
  s_error.clear();

  PyObject *exc;
  PyObject *v;
  PyObject *tb;

  PyErr_Fetch(&exc, &v, &tb);

  if (v) {
    s_error = SafePyString(v);
    Py_XDECREF(exc);
    Py_XDECREF(v);
    Py_XDECREF(tb);
    return s_error.c_str();
  }

  Py_XDECREF(exc);
  Py_XDECREF(v);
  Py_XDECREF(tb);

  return "";
}
