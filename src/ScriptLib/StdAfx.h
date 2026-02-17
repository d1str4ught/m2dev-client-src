#pragma once

#include "EterLib/StdAfx.h"
#include "EterGrnLib/StdAfx.h"

// #include <crtdbg.h>
#ifdef _DEBUG
#undef _DEBUG
#include <python/python.h>
#define _DEBUG
#else
#include <python/python.h>
#endif
#include <python/errcode.h>
#include <python/compile.h>
#include <python/marshal.h>

#ifdef BYTE
#undef BYTE
#endif

#include "PythonUtils.h"
#include "PythonLauncher.h"
#include "Resource.h"

void initdbg();

// PYTHON_EXCEPTION_SENDER
class IPythonExceptionSender {
public:
  void Clear() { m_strExceptionString = ""; }

  void RegisterExceptionString(const char *c_szString) {
    m_strExceptionString += c_szString;
  }

  virtual void Send() = 0;

protected:
  std::string m_strExceptionString;
};

extern IPythonExceptionSender *g_pkExceptionSender;

void SetExceptionSender(IPythonExceptionSender *pkExceptionSender);
// END_OF_PYTHON_EXCEPTION_SENDER
