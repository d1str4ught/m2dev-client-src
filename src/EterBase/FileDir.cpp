#include "StdAfx.h"
#include "FileDir.h"
#include <string>
#include <utf8.h>

CDir::CDir() { Initialize(); }

CDir::~CDir() { Destroy(); }

void CDir::Destroy() {
  if (m_hFind)
    FindClose(m_hFind);

  Initialize();
}

bool CDir::Create(const char *c_szFilter, const char *c_szPath,
                  BOOL bCheckedExtension) {
  Destroy();

  std::string stPath = c_szPath ? c_szPath : "";

  if (!stPath.empty()) {
    char end = stPath.back();
    if (end != '\\')
      stPath += '\\';
  }

  // Query: UTF-8 -> UTF-16 for WinAPI
  std::string stQueryUtf8 = stPath + "*.*";
  std::wstring stQueryW = Utf8ToWide(stQueryUtf8);

  m_wfd.dwFileAttributes = 0;
  m_wfd.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

  m_hFind = FindFirstFileW(stQueryW.c_str(), &m_wfd);
  if (m_hFind == INVALID_HANDLE_VALUE)
    return true;

  do {
    // Convert filename to UTF-8 for existing logic/callbacks
    std::string fileNameUtf8 = WideToUtf8(m_wfd.cFileName);

    if (!fileNameUtf8.empty() && fileNameUtf8[0] == '.')
      continue;

    if (IsFolder()) {
      if (!OnFolder(c_szFilter, stPath.c_str(), fileNameUtf8.c_str()))
        return false;
    } else {
      const char *c_szExtension = strchr(fileNameUtf8.c_str(), '.');
      if (!c_szExtension)
        continue;

      // NOTE : 임시 변수 - [levites]
      //        최종적으로는 무조건 TRUE 형태로 만든다.
      //        그전에 전 프로젝트의 CDir을 사용하는 곳에서 Extension을 "wav",
      //        "gr2" 이런식으로 넣게끔 한다. - [levites]
      if (bCheckedExtension) {
        std::string strFilter = c_szFilter ? c_szFilter : "";
        int iPos = (int)strFilter.find_first_of(';', 0);

        if (iPos > 0) {
          std::string first = strFilter.substr(0, iPos);
          std::string second = strFilter.substr(iPos + 1);

          if (0 != first.compare(c_szExtension + 1) &&
              0 != second.compare(c_szExtension + 1))
            continue;
        } else {
          if (0 != _stricmp(c_szExtension + 1, c_szFilter))
            continue;
        }
      }

      if (!OnFile(stPath.c_str(), fileNameUtf8.c_str()))
        return false;
    }
  } while (FindNextFileW(m_hFind, &m_wfd));

  return true;
}

bool CDir::IsFolder() {
  if (m_wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    return true;

  return false;
}

void CDir::Initialize() {
  memset(&m_wfd, 0, sizeof(m_wfd));
  m_hFind = NULL;
}
