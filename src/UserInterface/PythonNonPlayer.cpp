#include "stdafx.h"
#include "EterBase/lzo.h"
#include "PackLib/PackManager.h"
#include "pythonnonplayer.h"
#include "InstanceBase.h"
#include "PythonCharacterManager.h"

bool CPythonNonPlayer::LoadNonPlayerData(const char *c_szFileName) {
  static DWORD s_adwMobProtoKey[4] = {4813894, 18955, 552631, 6822045};

  TPackFile file;

  Tracef("CPythonNonPlayer::LoadNonPlayerData: %s, sizeof(TMobTable)=%u\n",
         c_szFileName, sizeof(TMobTable));

  if (!CPackManager::Instance().GetFile(c_szFileName, file))
    return false;

  DWORD dwFourCC, dwElements, dwDataSize;
  memcpy(&dwFourCC, file.data(), sizeof(DWORD));

  if (dwFourCC != MAKEFOURCC('M', 'M', 'P', 'T')) {
    TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid Mob proto type %s",
               c_szFileName);
    return false;
  }

  memcpy(&dwElements, file.data() + sizeof(DWORD), sizeof(DWORD));
  memcpy(&dwDataSize, file.data() + sizeof(DWORD) * 2, sizeof(DWORD));

  BYTE *pbData = new BYTE[dwDataSize];
  memcpy(pbData, file.data() + sizeof(DWORD) * 3, dwDataSize);
  /////

  CLZObject zObj;

  if (!CLZO::Instance().Decompress(zObj, pbData, s_adwMobProtoKey)) {
    delete[] pbData;
    return false;
  }

  if ((zObj.GetSize() % sizeof(TMobTable)) != 0) {
    TraceError("CPythonNonPlayer::LoadNonPlayerData: invalid size %u check "
               "data format.",
               zObj.GetSize());
    return false;
  }

  TMobTable *pTable = (TMobTable *)zObj.GetBuffer();
  for (DWORD i = 0; i < dwElements; ++i, ++pTable) {
    TMobTable *pNonPlayerData = new TMobTable;

    memcpy(pNonPlayerData, pTable, sizeof(TMobTable));

    // TraceError("%d : %s type[%d] color[%d]", pNonPlayerData->dwVnum,
    // pNonPlayerData->szLocaleName, pNonPlayerData->bType,
    // pNonPlayerData->dwMonsterColor);
    m_NonPlayerDataMap.insert(
        TNonPlayerDataMap::value_type(pNonPlayerData->dwVnum, pNonPlayerData));
  }

  delete[] pbData;
  return true;
}

bool CPythonNonPlayer::GetName(DWORD dwVnum, const char **c_pszName) {
  const TMobTable *p = GetTable(dwVnum);

  if (!p)
    return false;

  *c_pszName = p->szLocaleName;

  return true;
}

bool CPythonNonPlayer::GetInstanceType(DWORD dwVnum, BYTE *pbType) {
  const TMobTable *p = GetTable(dwVnum);

  // dwVnum를 찾을 수 없으면 플레이어 캐릭터로 간주 한다. 문제성 코드 -_-
  // [cronan]
  if (!p)
    return false;

  *pbType = p->bType;

  return true;
}

const CPythonNonPlayer::TMobTable *CPythonNonPlayer::GetTable(DWORD dwVnum) {
  TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.find(dwVnum);

  if (itor == m_NonPlayerDataMap.end())
    return NULL;

  return itor->second;
}

BYTE CPythonNonPlayer::GetEventType(DWORD dwVnum) {
  const TMobTable *p = GetTable(dwVnum);

  if (!p) {
    // Tracef("CPythonNonPlayer::GetEventType - Failed to find virtual
    // number\n");
    return ON_CLICK_EVENT_NONE;
  }

  return p->bOnClickType;
}

BYTE CPythonNonPlayer::GetEventTypeByVID(DWORD dwVID) {
  CInstanceBase *pInstance =
      CPythonCharacterManager::Instance().GetInstancePtr(dwVID);

  if (NULL == pInstance) {
    // Tracef("CPythonNonPlayer::GetEventTypeByVID - There is no Virtual
    // Number\n");
    return ON_CLICK_EVENT_NONE;
  }

  WORD dwVnum = pInstance->GetVirtualNumber();
  return GetEventType(dwVnum);
}

const char *CPythonNonPlayer::GetMonsterName(DWORD dwVnum) {
  const CPythonNonPlayer::TMobTable *c_pTable = GetTable(dwVnum);
  if (!c_pTable) {
    static const char *sc_szEmpty = "";
    return sc_szEmpty;
  }

  return c_pTable->szLocaleName;
}

DWORD CPythonNonPlayer::GetMonsterColor(DWORD dwVnum) {
  const CPythonNonPlayer::TMobTable *c_pTable = GetTable(dwVnum);
  if (!c_pTable)
    return 0;

  return c_pTable->dwMonsterColor;
}

void CPythonNonPlayer::GetMatchableMobList(int iLevel, int iInterval,
                                           TMobTableList *pMobTableList) {
  /*
          pMobTableList->clear();

          TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
          for (; itor != m_NonPlayerDataMap.end(); ++itor)
          {
                  TMobTable * pMobTable = itor->second;

                  int iLowerLevelLimit = iLevel-iInterval;
                  int iUpperLevelLimit = iLevel+iInterval;

                  if ((pMobTable->abLevelRange[0] >= iLowerLevelLimit &&
     pMobTable->abLevelRange[0] <= iUpperLevelLimit) ||
                          (pMobTable->abLevelRange[1] >= iLowerLevelLimit &&
     pMobTable->abLevelRange[1] <= iUpperLevelLimit))
                  {
                          pMobTableList->push_back(pMobTable);
                  }
          }
  */
}

void CPythonNonPlayer::Clear() {}

void CPythonNonPlayer::Destroy() {
  for (TNonPlayerDataMap::iterator itor = m_NonPlayerDataMap.begin();
       itor != m_NonPlayerDataMap.end(); ++itor) {
    delete itor->second;
  }
  m_NonPlayerDataMap.clear();
}

CPythonNonPlayer::CPythonNonPlayer() { Clear(); }

CPythonNonPlayer::~CPythonNonPlayer(void) { Destroy(); }