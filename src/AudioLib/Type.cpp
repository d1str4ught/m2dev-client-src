#include "StdAfx.h"
#include "Type.h"
#include "EterLib/TextFileLoader.h"
#include <utf8.h>

std::string NSound::strResult;

const char *NSound::GetResultString() { return strResult.c_str(); }

void NSound::SetResultString(const char *c_pszStr) {
  strResult.assign(c_pszStr);
}

bool NSound::LoadSoundInformationPiece(
    const char *c_szFileName, NSound::TSoundDataVector &rSoundDataVector,
    const char *c_szPathHeader) {
  std::string strResult;
  strResult = c_szFileName;

  CTextFileLoader *pkTextFileLoader = CTextFileLoader::Cache(c_szFileName);
  if (!pkTextFileLoader)
    return false;

  CTextFileLoader &rkTextFileLoader = *pkTextFileLoader;
  if (rkTextFileLoader.IsEmpty()) {
    SetResultString((strResult + " Can not open file for reading").c_str());
    return false;
  }

  rkTextFileLoader.SetTop();

  int iCount;
  if (!rkTextFileLoader.GetTokenInteger("sounddatacount", &iCount)) {
    SetResultString(
        (strResult + " File format error, SoundDataCount Unable to find.")
            .c_str());
    return false;
  }

  rSoundDataVector.clear();
  rSoundDataVector.resize(iCount);

  char szSoundDataHeader[32 + 1];
  for (uint32_t i = 0; i < rSoundDataVector.size(); ++i) {
    _snprintf_s(szSoundDataHeader, sizeof(szSoundDataHeader), "sounddata%02d",
                i);
    CTokenVector *pTokenVector;
    if (!rkTextFileLoader.GetTokenVector(szSoundDataHeader, &pTokenVector)) {
      SetResultString((strResult + " File format error: " + szSoundDataHeader +
                       " Unable to find")
                          .c_str());
      return false;
    }

    if (2 != pTokenVector->size()) {
      SetResultString(
          (strResult + " File format error: The size of the vector is not 2")
              .c_str());
      return false;
    }

    rSoundDataVector[i].fTime = (float)atof(pTokenVector->at(0).c_str());
    if (c_szPathHeader) {
      rSoundDataVector[i].strSoundFileName = c_szPathHeader;
      rSoundDataVector[i].strSoundFileName += pTokenVector->at(1).c_str();
    } else {
      rSoundDataVector[i].strSoundFileName = pTokenVector->at(1).c_str();
    }

    // TraceError("LoadSoundInformation %s -- %f -- %s", c_szFileName,
    // rSoundDataVector[i].fTime, rSoundDataVector[i].strSoundFileName.c_str());
  }

  SetResultString((strResult + " Loaded").c_str());
  return true;
}

bool NSound::SaveSoundInformationPiece(
    const char *c_szFileName, NSound::TSoundDataVector &rSoundDataVector) {
  if (rSoundDataVector.empty()) // If no data is considered success
  {
    if (IsFile(c_szFileName)) // If the data is empty but there is a file
    {
      _unlink(c_szFileName); // erase.
    }
    return true;
  }

  std::string strResult;
  strResult = c_szFileName;

  // UTF-8 → UTF-16 conversion for Unicode path support
  std::wstring wFileName = Utf8ToWide(c_szFileName);
  FILE *File = _wfopen(wFileName.c_str(), L"wt");

  if (!File) {
    char szErrorText[256 + 1];
    _snprintf_s(szErrorText, sizeof(szErrorText),
                "Failed to save file (%s).\nPlease check if it is read-only or "
                "you have no space on the disk.\n",
                c_szFileName);
    LogBox(szErrorText, "Error");
    SetResultString((strResult + " Cannot open file for writing").c_str());
    return false;
  }

  fprintf(File, "ScriptType        CharacterSoundInformation\n");
  fprintf(File, "\n");

  fprintf(File, "SoundDataCount    %llu\n", rSoundDataVector.size());

  for (uint32_t i = 0; i < rSoundDataVector.size(); ++i) {
    NSound::TSoundData &rSoundData = rSoundDataVector[i];
    fprintf(File, "SoundData%02d       %f \"%s\"\n", i, rSoundData.fTime,
            rSoundData.strSoundFileName.c_str());
  }

  fclose(File);
  return true;
}

void NSound::DataToInstance(const TSoundDataVector &c_rSoundDataVector,
                            TSoundInstanceVector *pSoundInstanceVector) {
  if (c_rSoundDataVector.empty())
    return;

  uint32_t dwFPS = 60;
  const float c_fFrameTime = 1.0f / float(dwFPS);

  pSoundInstanceVector->clear();
  pSoundInstanceVector->resize(c_rSoundDataVector.size());
  for (uint32_t i = 0; i < c_rSoundDataVector.size(); ++i) {
    const TSoundData &c_rSoundData = c_rSoundDataVector[i];
    TSoundInstance &rSoundInstance = pSoundInstanceVector->at(i);

    rSoundInstance.dwFrame = (uint32_t)(c_rSoundData.fTime / c_fFrameTime);
    rSoundInstance.strSoundFileName = c_rSoundData.strSoundFileName;
  }
}
