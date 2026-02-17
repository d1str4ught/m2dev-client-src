#include "StdAfx.h"
#include "TextFileLoader.h"
#include "PackLib/PackManager.h"

void PrintfTabs(FILE *File, int iTabCount, const char *c_szString, ...) {
  va_list args;
  va_start(args, c_szString);

  static char szBuf[1024];
  _vsnprintf(szBuf, sizeof(szBuf), c_szString, args);
  va_end(args);

  for (int i = 0; i < iTabCount; ++i)
    fprintf(File, "    ");

  fprintf(File, szBuf);
}

bool LoadTextData(const char *c_szFileName, CTokenMap &rstTokenMap) {
  TPackFile File;

  if (!CPackManager::Instance().GetFile(c_szFileName, File))
    return false;

  CMemoryTextFileLoader textFileLoader;
  CTokenVector stTokenVector;

  textFileLoader.Bind(File.size(), File.data());

  for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i) {
    if (!textFileLoader.SplitLine(i, &stTokenVector))
      continue;

    if (2 != stTokenVector.size())
      return false;

    stl_lowers(stTokenVector[0]);
    stl_lowers(stTokenVector[1]);

    rstTokenMap[stTokenVector[0]] = stTokenVector[1];
  }

  return true;
}

bool LoadMultipleTextData(const char *c_szFileName,
                          CTokenVectorMap &rstTokenVectorMap) {
  TPackFile File;

  if (!CPackManager::Instance().GetFile(c_szFileName, File))
    return false;

  DWORD i;

  CMemoryTextFileLoader textFileLoader;
  CTokenVector stTokenVector;

  textFileLoader.Bind(File.size(), File.data());

  for (i = 0; i < textFileLoader.GetLineCount(); ++i) {
    if (!textFileLoader.SplitLine(i, &stTokenVector))
      continue;

    stl_lowers(stTokenVector[0]);

    // Start or End
    if (0 == stTokenVector[0].compare("start")) {
      CTokenVector stSubTokenVector;

      stl_lowers(stTokenVector[1]);
      std::string key = stTokenVector[1];
      stTokenVector.clear();

      for (i = i + 1; i < textFileLoader.GetLineCount(); ++i) {
        if (!textFileLoader.SplitLine(i, &stSubTokenVector))
          continue;

        stl_lowers(stSubTokenVector[0]);

        if (0 == stSubTokenVector[0].compare("end")) {
          break;
        }

        for (DWORD j = 0; j < stSubTokenVector.size(); ++j) {
          stTokenVector.push_back(stSubTokenVector[j]);
        }
      }

      rstTokenVectorMap.insert(CTokenVectorMap::value_type(key, stTokenVector));
    } else {
      std::string key = stTokenVector[0];
      stTokenVector.erase(stTokenVector.begin());
      rstTokenVectorMap.insert(CTokenVectorMap::value_type(key, stTokenVector));
    }
  }

  return true;
}

D3DXVECTOR3 TokenToVector(CTokenVector &rVector) {
  if (3 != rVector.size()) {
    assert(!"Size of token vector which will be converted to vector is not 3");
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  }

  return D3DXVECTOR3(atof(rVector[0].c_str()), atof(rVector[1].c_str()),
                     atof(rVector[2].c_str()));
}

D3DXCOLOR TokenToColor(CTokenVector &rVector) {
  if (4 != rVector.size()) {
    assert(!"Size of token vector which will be converted to color is not 4");
    return D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
  }

  return D3DXCOLOR(atof(rVector[0].c_str()), atof(rVector[1].c_str()),
                   atof(rVector[2].c_str()), atof(rVector[3].c_str()));
}
