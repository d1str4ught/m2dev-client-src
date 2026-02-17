#include "StdAfx.h"
#include "PropertyLoader.h"
#include "PropertyManager.h"
#include "Property.h"

bool CPropertyLoader::OnFolder(const char *c_szFilter, const char *c_szPathName,
                               const char *c_szFileName) {
  std::string stPathName = "";
  stPathName += c_szPathName;
  stPathName += c_szFileName;

  CPropertyLoader PropertyLoader;
  PropertyLoader.SetPropertyManager(m_pPropertyManager);
  PropertyLoader.Create(c_szFilter, stPathName.c_str());
  return true;
}

bool CPropertyLoader::OnFile(const char *c_szPathName,
                             const char *c_szFileName) {
  RegisterFile(c_szPathName, c_szFileName);
  return true;
}

DWORD CPropertyLoader::RegisterFile(const char *c_szPathName,
                                    const char *c_szFileName) {
  std::string strFileName = "";
  strFileName += c_szPathName;
  strFileName += c_szFileName;

  std::string stExt;
  GetFileExtension(strFileName.c_str(), strFileName.length(), &stExt);

  stl_lowers(stExt);
  stl_lowers(strFileName);

  // 패스를 소문자로 만들고 \\ 는 / 로 바꾼다.
  StringPath(strFileName);

  // 예약된 CRC 처리 (지워진 CRC)
  if (0 == strFileName.compare("property/reserve")) {
    m_pPropertyManager->LoadReservedCRC(strFileName.c_str());
    return 1;
  } else {
    CProperty *pProperty;

    if (m_pPropertyManager->Register(strFileName.c_str(), &pProperty))
      return pProperty->GetCRC();
    else
      return 0;
  }
}

void CPropertyLoader::SetPropertyManager(CPropertyManager *pPropertyManager) {
  m_pPropertyManager = pPropertyManager;
}

CPropertyLoader::CPropertyLoader() { m_pPropertyManager = NULL; }

CPropertyLoader::~CPropertyLoader() {}
