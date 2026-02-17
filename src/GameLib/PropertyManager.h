#pragma once

#include "PackLib/PackManager.h"

class CPropertyManager : public CSingleton<CPropertyManager> {
public:
  CPropertyManager();
  virtual ~CPropertyManager();

  void Clear();

  bool LoadReservedCRC(const char *c_pszFileName);
  void ReserveCRC(DWORD dwCRC);
  DWORD GetUniqueCRC(const char *c_szSeed);

  bool Initialize(const char *c_pszPackFileName = NULL);
  bool Register(const char *c_pszFileName, CProperty **ppProperty = NULL);

  bool Get(DWORD dwCRC, CProperty **ppProperty);
  bool Get(const char *c_pszFileName, CProperty **ppProperty);

protected:
  typedef std::map<DWORD, CProperty *> TPropertyCRCMap;
  typedef std::set<DWORD> TCRCSet;

  bool m_isFileMode;
  TPropertyCRCMap m_PropertyByCRCMap;
  TCRCSet m_ReservedCRCSet;
  std::shared_ptr<CPack> m_pack;
  TPackFileMap m_fileDict;
};
