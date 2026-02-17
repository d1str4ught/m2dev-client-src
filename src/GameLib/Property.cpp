#include "StdAfx.h"
#include <string.h>
#include "EterBase/TempFile.h"

#include "PropertyManager.h"
#include "Property.h"
/*
 *	CProperty 파일 포맷
 *
 *  0 ~ 4 bytes: fourcc
 *  5 ~ 6 bytes: \r\n
 *
 *  그 이후의 바이트들은 텍스트 파일 로더와 같은 구조
 */
CProperty::CProperty(const char *c_pszFileName)
    : mc_pFileName(NULL), m_dwCRC(0) {
  m_stFileName = c_pszFileName;
  StringPath(m_stFileName);

  mc_pFileName = strrchr(m_stFileName.c_str(), '/');

  if (!mc_pFileName)
    mc_pFileName = m_stFileName.c_str();
  else
    ++mc_pFileName;
}

CProperty::~CProperty() {}

DWORD CProperty::GetCRC() { return m_dwCRC; }

const char *CProperty::GetFileName() { return (m_stFileName.c_str()); }

bool CProperty::GetString(const char *c_pszKey, const char **c_ppString) {
  std::string stTempKey = c_pszKey;
  stl_lowers(stTempKey);
  CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());

  //	printf("GetString %s %d\n", stTempKey.c_str(), m_stTokenMap.size());

  if (m_stTokenMap.end() == it)
    return false;

  *c_ppString = it->second[0].c_str();
  return true;
}

DWORD CProperty::GetSize() { return m_stTokenMap.size(); }

bool CProperty::GetVector(const char *c_pszKey, CTokenVector &rTokenVector) {
  std::string stTempKey = c_pszKey;
  stl_lowers(stTempKey);
  CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());

  if (m_stTokenMap.end() == it)
    return false;

  // NOTE : 튕김 현상 발견
  //	std::copy(rTokenVector.begin(), it->second.begin(), it->second.end());
  // NOTE : 레퍼런스에는 이런 식으로 하게끔 되어 있음
  ///////////////////////////////////////////////////////////////////////////////
  //	template <class InputIterator, class OutputIterator>
  //	OutputIterator copy(InputIterator first, InputIterator last,
  //                    OutputIterator result);
  //
  //	vector<int> V(5);
  //	iota(V.begin(), V.end(), 1);
  //	list<int> L(V.size());
  //	copy(V.begin(), V.end(), L.begin());
  //	assert(equal(V.begin(), V.end(), L.begin()));
  ///////////////////////////////////////////////////////////////////////////////
  // 헌데 그래도 튕김. - [levites]
  //	std::copy(it->second.begin(), it->second.end(), rTokenVector.begin());

  // 결국 이렇게.. - [levites]
  // 현재 사용하는 곳 : WorldEditor/Dialog/MapObjectPropertyPageBuilding.cpp
  CTokenVector &rSourceTokenVector = it->second;
  CTokenVector::iterator itor = rSourceTokenVector.begin();
  for (; itor != rSourceTokenVector.end(); ++itor) {
    rTokenVector.push_back(*itor);
  }

  return true;
}

void CProperty::PutString(const char *c_pszKey, const char *c_pszString) {
  std::string stTempKey = c_pszKey;
  stl_lowers(stTempKey);

  // 이미 있는걸 지움
  CTokenVectorMap::iterator itor = m_stTokenMap.find(stTempKey);

  if (itor != m_stTokenMap.end())
    m_stTokenMap.erase(itor);

  CTokenVector tokenVector;
  tokenVector.push_back(c_pszString);

  m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, tokenVector));
}

void CProperty::PutVector(const char *c_pszKey,
                          const CTokenVector &c_rTokenVector) {
  std::string stTempKey = c_pszKey;
  stl_lowers(stTempKey);

  m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, c_rTokenVector));
}

void GetTimeString(char *str, time_t ct) {
  struct tm tm;
  tm = *localtime(&ct);

  _snprintf(str, 15, "%04d%02d%02d%02d%02d%02d", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

bool CProperty::ReadFromMemory(const void *c_pvData, int iLen,
                               const char *c_pszFileName) {
  const char *pStart = (const char *)c_pvData;
  const char *pcData = pStart;

  if (*(DWORD *)pcData != MAKEFOURCC('Y', 'P', 'R', 'T'))
    return false;

  pcData += sizeof(DWORD);

  while (pcData < pStart + iLen && (*pcData == '\r' || *pcData == '\n' ||
                                    *pcData == ' ' || *pcData == '\t'))
    ++pcData;

  int textLen = iLen - int(pcData - pStart);
  if (textLen <= 0) {
    TraceError("CProperty::ReadFromMemory: textLen <= 0 in %s\n",
               c_pszFileName);
    return false;
  }

  CTokenVector stTokenVector;

  CMemoryTextFileLoader textFileLoader;
  textFileLoader.Bind(textLen, pcData);

  m_stCRC.clear();
  m_dwCRC = 0;

  if (textFileLoader.GetLineCount() > 0) {
    m_stCRC = textFileLoader.GetLineString(0);

    bool bAllDigits = !m_stCRC.empty() &&
                      std::all_of(m_stCRC.begin(), m_stCRC.end(), [](char c) {
                        return isdigit((unsigned char)c);
                      });

    DWORD startLine = 0;

    if (bAllDigits) {
      m_dwCRC = atoi(m_stCRC.c_str());
      startLine = 1;
    }

    for (DWORD i = startLine; i < textFileLoader.GetLineCount(); ++i) {
      if (!textFileLoader.SplitLine(i, &stTokenVector))
        continue;

      stl_lowers(stTokenVector[0]);
      std::string stKey = stTokenVector[0];

      stTokenVector.erase(stTokenVector.begin());
      PutVector(stKey.c_str(), stTokenVector);
    }
  }

  return true;
}

void CProperty::Clear() { m_stTokenMap.clear(); }
