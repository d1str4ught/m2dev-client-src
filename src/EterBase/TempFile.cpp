#include "StdAfx.h"
#include "TempFile.h"
#include "Utils.h"
#include "Debug.h"
#include <utf8.h>

CTempFile::~CTempFile() {
  Destroy();

  if (m_szFileName[0]) {
    std::wstring wPath = Utf8ToWide(m_szFileName);
    DeleteFileW(wPath.c_str());
  }
}

CTempFile::CTempFile(const char *c_pszPrefix) {
  strncpy(m_szFileName, CreateTempFileName(c_pszPrefix), MAX_PATH);

  if (!Create(m_szFileName, CFileBase::FILEMODE_WRITE)) {
    TraceError(
        "CTempFile::CTempFile cannot create temporary file. (filename: %s)",
        m_szFileName);
    return;
  }
}
