#pragma once

#pragma warning(disable : 4786) // character 255 넘어가는거 끄기

#include <windows.h>
#include <vector>
#include <map>

#include "Stl.h"

class CMemoryTextFileLoader {
public:
  CMemoryTextFileLoader();
  virtual ~CMemoryTextFileLoader();

  void Bind(int bufSize, const void *c_pvBuf);
  DWORD GetLineCount();
  bool CheckLineIndex(DWORD dwLine);
  bool SplitLine(DWORD dwLine, CTokenVector *pstTokenVector,
                 const char *c_szDelimeter = " \t");
  int SplitLine2(DWORD dwLine, CTokenVector *pstTokenVector,
                 const char *c_szDelimeter = " \t");
  bool SplitLineByTab(DWORD dwLine, CTokenVector *pstTokenVector);
  const std::string &GetLineString(DWORD dwLine);

protected:
  std::vector<std::string> m_stLineVector;
};

class CMemoryFileLoader {
public:
  CMemoryFileLoader(int size, const void *c_pvMemoryFile);
  virtual ~CMemoryFileLoader();

  bool Read(int size, void *pvDst);

  int GetPosition();
  int GetSize();

protected:
  bool IsReadableSize(int size);
  const char *GetCurrentPositionPointer();

protected:
  const char *m_pcBase;
  int m_size;
  int m_pos;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
class CDiskFileLoader {
public:
  CDiskFileLoader();
  virtual ~CDiskFileLoader();

  void Close();
  bool Open(const char *c_szFileName);
  bool Read(int size, void *pvDst);

  int GetSize();

protected:
  void Initialize();

protected:
  FILE *m_fp;
  int m_size;
};

typedef std::map<std::string, std::string> TStringMap;