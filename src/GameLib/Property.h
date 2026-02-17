#pragma once

#include <string>

class CProperty {
public:
  CProperty(const char *c_pszFileName);
  ~CProperty();

  void Clear();
  bool ReadFromMemory(const void *c_pvData, int iLen,
                      const char *c_pszFileName);

  const char *GetFileName();

  bool GetVector(const char *c_pszKey, CTokenVector &rTokenVector);
  bool GetString(const char *c_pszKey, const char **c_ppString);

  void PutVector(const char *c_pszKey, const CTokenVector &c_rTokenVector);
  void PutString(const char *c_pszKey, const char *c_pszString);

  DWORD GetSize();
  DWORD GetCRC();

protected:
  std::string m_stFileName;
  std::string m_stCRC;
  const char *mc_pFileName;
  DWORD m_dwCRC;

  CTokenVectorMap m_stTokenMap;
};
