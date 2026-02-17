#include "StdAfx.h"
#include "PackLib/PackManager.h"

#include "PropertyManager.h"
#include "Property.h"

CPropertyManager::CPropertyManager() : m_isFileMode(true) {}

CPropertyManager::~CPropertyManager() { Clear(); }

bool CPropertyManager::Initialize(const char *c_pszPackFileName) {
  if (c_pszPackFileName) {
    m_pack = std::make_shared<CPack>();
    if (!m_pack->Load(c_pszPackFileName)) {
      LogBoxf("Cannot open property pack file (filename %s)",
              c_pszPackFileName);
      return false;
    }

    m_isFileMode = false;

    const auto &index = m_pack->GetIndex();
    for (const auto &entry : index) {
      m_fileDict.emplace(entry.file_name, std::make_pair(m_pack, entry));
    }

    for (auto it = m_fileDict.begin(); it != m_fileDict.end(); ++it) {
      std::string stFileName = it->second.second.file_name;
      if (!stricmp("property/reserve", stFileName.c_str())) {
        LoadReservedCRC(stFileName.c_str());
      } else {
        Register(stFileName.c_str());
      }
    }
  } else {
    m_isFileMode = true;
    // NOTE : 여기서 Property를 등록시키면 WorldEditor에서 이상이 생김 ;
    //        또한, Property Tree List에도 등록을 시켜야 되기 때문에
    //        바깥쪽에서.. - [levites]
  }

  return true;
}

bool CPropertyManager::LoadReservedCRC(const char *c_pszFileName) {
  TPackFile file;

  if (!CPackManager::Instance().GetFile(c_pszFileName, file))
    return false;

  CMemoryTextFileLoader textFileLoader;
  textFileLoader.Bind(file.size(), file.data());

  for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i) {
    const char *pszLine = textFileLoader.GetLineString(i).c_str();

    if (!pszLine || !*pszLine)
      continue;

    ReserveCRC(atoi(pszLine));
  }

  return true;
}

void CPropertyManager::ReserveCRC(DWORD dwCRC) {
  m_ReservedCRCSet.insert(dwCRC);
}

DWORD CPropertyManager::GetUniqueCRC(const char *c_szSeed) {
  std::string stTmp = c_szSeed;

  while (1) {
    DWORD dwCRC = GetCRC32(stTmp.c_str(), stTmp.length());

    if (m_ReservedCRCSet.find(dwCRC) == m_ReservedCRCSet.end() &&
        m_PropertyByCRCMap.find(dwCRC) == m_PropertyByCRCMap.end())
      return dwCRC;

    char szAdd[2];
    _snprintf(szAdd, sizeof(szAdd), "%d", random() % 10);
    stTmp += szAdd;
  }
}

bool CPropertyManager::Register(const char *c_pszFileName,
                                CProperty **ppProperty) {
  TPackFile file;

  if (!CPackManager::Instance().GetFile(c_pszFileName, file))
    return false;

  CProperty *pProperty = new CProperty(c_pszFileName);

  if (!pProperty->ReadFromMemory(file.data(), file.size(), c_pszFileName)) {
    delete pProperty;
    return false;
  }

  DWORD dwCRC = pProperty->GetCRC();

  TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

  if (m_PropertyByCRCMap.end() != itor) {
    Tracef("Property already registered, replace %s to %s\n",
           itor->second->GetFileName(), c_pszFileName);

    delete itor->second;
    itor->second = pProperty;
  } else
    m_PropertyByCRCMap.insert(TPropertyCRCMap::value_type(dwCRC, pProperty));

  if (ppProperty)
    *ppProperty = pProperty;

  return true;
}

bool CPropertyManager::Get(const char *c_pszFileName, CProperty **ppProperty) {
  return Register(c_pszFileName, ppProperty);
}

bool CPropertyManager::Get(DWORD dwCRC, CProperty **ppProperty) {
  TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

  if (m_PropertyByCRCMap.end() == itor)
    return false;

  *ppProperty = itor->second;
  return true;
}

void CPropertyManager::Clear() { stl_wipe_second(m_PropertyByCRCMap); }
