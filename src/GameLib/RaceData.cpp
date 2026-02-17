#include "StdAfx.h"
#include "EterLib/ResourceManager.h"
#include "EterLib/AttributeInstance.h"
#include "EterBase/Utils.h"
#include "RaceData.h"
#include "RaceMotionData.h"
#include "EterBase/Filename.h"

CDynamicPool<CRaceData> CRaceData::ms_kPool;
CDynamicPool<CRaceData::TMotionModeData> CRaceData::ms_MotionModeDataPool;

const std::string &CRaceData::GetSmokeBone() { return m_strSmokeBoneName; }

DWORD CRaceData::GetSmokeEffectID(UINT eSmoke) {
  if (eSmoke >= SMOKE_NUM) {
    TraceError("CRaceData::GetSmokeEffectID(eSmoke=%d)", eSmoke);
    return 0;
  }

  return m_adwSmokeEffectID[eSmoke];
}

CRaceData::SHair *CRaceData::FindHair(UINT eHair) {
  std::map<DWORD, SHair>::iterator f = m_kMap_dwHairKey_kHair.find(eHair);
  if (m_kMap_dwHairKey_kHair.end() == f) {
    if (eHair != 0)
      TraceError("Hair number %d is not exist.", eHair);
    return NULL;
  }

  return &f->second;
}

void CRaceData::SetHairSkin(UINT eHair, UINT ePart,
                            const char *c_szModelFileName,
                            const char *c_szSrcFileName,
                            const char *c_szDstFileName) {
  SSkin kSkin;
  kSkin.m_ePart = ePart;
  kSkin.m_stSrcFileName = c_szSrcFileName;
  kSkin.m_stDstFileName = c_szDstFileName;

  CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
  m_kMap_dwHairKey_kHair[eHair].m_kVct_kSkin.push_back(kSkin);
  m_kMap_dwHairKey_kHair[eHair].m_stModelFileName = c_szModelFileName;
}

CRaceData::SShape *CRaceData::FindShape(UINT eShape) {
  std::map<DWORD, SShape>::iterator f = m_kMap_dwShapeKey_kShape.find(eShape);
  if (m_kMap_dwShapeKey_kShape.end() == f)
    return NULL;

  return &f->second;
}

void CRaceData::SetShapeModel(UINT eShape, const char *c_szModelFileName) {
  m_kMap_dwShapeKey_kShape[eShape].m_stModelFileName = c_szModelFileName;
}

void CRaceData::AppendShapeSkin(UINT eShape, UINT ePart,
                                const char *c_szSrcFileName,
                                const char *c_szDstFileName) {
  SSkin kSkin;
  kSkin.m_ePart = ePart;
  kSkin.m_stSrcFileName = c_szSrcFileName;
  kSkin.m_stDstFileName = c_szDstFileName;

  CFileNameHelper::ChangeDosPath(kSkin.m_stSrcFileName);
  m_kMap_dwShapeKey_kShape[eShape].m_kVct_kSkin.push_back(kSkin);
}

CRaceData *CRaceData::New() { return ms_kPool.Alloc(); }

void CRaceData::Delete(CRaceData *pkRaceData) {
  pkRaceData->Destroy();
  ms_kPool.Free(pkRaceData);
}

void CRaceData::CreateSystem(UINT uCapacity, UINT uMotModeCapacity) {
  ms_MotionModeDataPool.Create(uMotModeCapacity);
  ms_kPool.Create(uCapacity);
}

void CRaceData::DestroySystem() {
  ms_kPool.Destroy();
  ms_MotionModeDataPool.Destroy();
}

BOOL CRaceData::CreateMotionModeIterator(TMotionModeDataIterator &itor) {
  if (m_pMotionModeDataMap.empty())
    return FALSE;

  itor = m_pMotionModeDataMap.begin();

  return TRUE;
}
BOOL CRaceData::NextMotionModeIterator(TMotionModeDataIterator &itor) {
  ++itor;

  return m_pMotionModeDataMap.end() != itor;
}

BOOL CRaceData::GetMotionKey(WORD wMotionModeIndex, WORD wMotionIndex,
                             MOTION_KEY *pMotionKey) {
  TMotionModeData *pMotionModeData;
  if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData))
    return FALSE;

  if (pMotionModeData->MotionVectorMap.end() ==
      pMotionModeData->MotionVectorMap.find(wMotionIndex)) {
    WORD wGeneralMode = CRaceMotionData::MODE_GENERAL;

    switch (wMotionModeIndex) {
    case CRaceMotionData::MODE_HORSE_ONEHAND_SWORD:
    case CRaceMotionData::MODE_HORSE_TWOHAND_SWORD:
    case CRaceMotionData::MODE_HORSE_DUALHAND_SWORD:
    case CRaceMotionData::MODE_HORSE_FAN:
    case CRaceMotionData::MODE_HORSE_BELL:
    case CRaceMotionData::MODE_HORSE_BOW:
      wGeneralMode = CRaceMotionData::MODE_HORSE;
      break;

    default:
      wGeneralMode = CRaceMotionData::MODE_GENERAL;
      break;
    }

    TMotionModeData *pMotionModeGeneralData;
    if (!GetMotionModeDataPointer(wGeneralMode, &pMotionModeGeneralData))
      return FALSE;

    if (pMotionModeGeneralData->MotionVectorMap.end() ==
        pMotionModeGeneralData->MotionVectorMap.find(wMotionIndex))
      return FALSE;

    *pMotionKey = MAKE_MOTION_KEY(wGeneralMode, wMotionIndex);
  } else {
    *pMotionKey = MAKE_MOTION_KEY(wMotionModeIndex, wMotionIndex);
  }

  return TRUE;
}

BOOL CRaceData::GetMotionModeDataPointer(WORD wMotionMode,
                                         TMotionModeData **ppMotionModeData) {
  TMotionModeDataIterator itor = m_pMotionModeDataMap.find(wMotionMode);
  if (itor == m_pMotionModeDataMap.end())
    return FALSE;

  *ppMotionModeData = itor->second;

  return TRUE;
}

BOOL CRaceData::GetModelDataPointer(DWORD dwModelIndex,
                                    const TModelData **c_ppModelData) {
  TModelDataMapIterator itor = m_ModelDataMap.find(dwModelIndex);
  if (m_ModelDataMap.end() == itor)
    return false;

  *c_ppModelData = &itor->second;

  return true;
}

BOOL CRaceData::GetMotionVectorPointer(WORD wMotionMode, WORD wMotionIndex,
                                       TMotionVector **ppMotionVector) {
  TMotionModeData *pMotionModeData;
  if (!GetMotionModeDataPointer(wMotionMode, &pMotionModeData))
    return FALSE;

  TMotionVectorMap::iterator itor =
      pMotionModeData->MotionVectorMap.find(wMotionIndex);
  if (pMotionModeData->MotionVectorMap.end() == itor)
    return FALSE;

  *ppMotionVector = &itor->second;

  return TRUE;
}

BOOL CRaceData::GetMotionDataPointer(WORD wMotionMode, WORD wMotionIndex,
                                     WORD wMotionSubIndex,
                                     CRaceMotionData **c_ppMotionData) {
  const TMotionVector *c_pMotionVector;
  if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &c_pMotionVector))
    return FALSE;

  if (wMotionSubIndex >= c_pMotionVector->size())
    return FALSE;

  const TMotion &c_rMotion = c_pMotionVector->at(wMotionSubIndex);

  if (!c_rMotion.pMotionData)
    return FALSE;

  *c_ppMotionData = c_rMotion.pMotionData;

  return TRUE;
}

BOOL CRaceData::GetMotionDataPointer(DWORD dwMotionKey,
                                     CRaceMotionData **c_ppMotionData) {
  return GetMotionDataPointer(
      GET_MOTION_MODE(dwMotionKey), GET_MOTION_INDEX(dwMotionKey),
      GET_MOTION_SUB_INDEX(dwMotionKey), c_ppMotionData);
}

BOOL CRaceData::GetMotionVectorPointer(WORD wMotionMode, WORD wMotionIndex,
                                       const TMotionVector **c_ppMotionVector) {
  TMotionVector *pMotionVector;
  if (!GetMotionVectorPointer(wMotionMode, wMotionIndex, &pMotionVector))
    return FALSE;

  *c_ppMotionVector = pMotionVector;

  return TRUE;
}

DWORD CRaceData::GetAttachingDataCount() {
  return m_AttachingDataVector.size();
}

BOOL CRaceData::GetAttachingDataPointer(
    DWORD dwIndex, const NRaceData::TAttachingData **c_ppAttachingData) {
  if (dwIndex >= GetAttachingDataCount())
    return FALSE;

  *c_ppAttachingData = &m_AttachingDataVector[dwIndex];

  return TRUE;
}

BOOL CRaceData::GetCollisionDataPointer(
    DWORD dwIndex, const NRaceData::TAttachingData **c_ppAttachingData) {
  if (dwIndex >= GetAttachingDataCount())
    return FALSE;

  if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA !=
      m_AttachingDataVector[dwIndex].dwType)
    return FALSE;

  *c_ppAttachingData = &m_AttachingDataVector[dwIndex];

  return TRUE;
}

BOOL CRaceData::GetBodyCollisionDataPointer(
    const NRaceData::TAttachingData **c_ppAttachingData) {
  for (DWORD i = 0; i < m_AttachingDataVector.size(); ++i) {
    const NRaceData::TAttachingData *pAttachingData = &m_AttachingDataVector[i];

    if (NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA == pAttachingData->dwType)
      if (NRaceData::COLLISION_TYPE_BODY ==
          pAttachingData->pCollisionData->iCollisionType) {
        *c_ppAttachingData = pAttachingData;
        return true;
      }
  }

  return FALSE;
}

void CRaceData::SetRace(DWORD dwRaceIndex) { m_dwRaceIndex = dwRaceIndex; }

void CRaceData::RegisterAttachingBoneName(DWORD dwPartIndex,
                                          const char *c_szBoneName) {
  m_AttachingBoneNameMap.insert(
      TAttachingBoneNameMap::value_type(dwPartIndex, c_szBoneName));
}

void CRaceData::RegisterMotionMode(WORD wMotionModeIndex) {
  TMotionModeData *pMotionModeData = ms_MotionModeDataPool.Alloc();
  pMotionModeData->wMotionModeIndex = wMotionModeIndex;
  pMotionModeData->MotionVectorMap.clear();
  m_pMotionModeDataMap.insert(
      TMotionModeDataMap::value_type(wMotionModeIndex, pMotionModeData));
}

CGraphicThing *CRaceData::NEW_RegisterMotion(CRaceMotionData *pkMotionData,
                                             WORD wMotionModeIndex,
                                             WORD wMotionIndex,
                                             const char *c_szFileName,
                                             BYTE byPercentage) {
  CGraphicThing *pMotionThing =
      (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
          c_szFileName);

  TMotionModeData *pMotionModeData;
  if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData)) {
    AssertLog("Failed getting motion mode data!");
    return NULL;
  }

  TMotion kMotion;
  kMotion.byPercentage = byPercentage;
  kMotion.pMotion = pMotionThing;
  kMotion.pMotionData = pkMotionData;
  pMotionModeData->MotionVectorMap[wMotionIndex].push_back(kMotion);

  return pMotionThing;
}

CGraphicThing *CRaceData::RegisterMotionData(WORD wMotionMode,
                                             WORD wMotionIndex,
                                             const char *c_szFileName,
                                             BYTE byPercentage) {
  CRaceMotionData *pRaceMotionData = CRaceMotionData::New();
  if (!pRaceMotionData->LoadMotionData(c_szFileName)) {
    TraceError(
        "CRaceData::RegisterMotionData - LoadMotionData(c_szFileName=%s) ERROR",
        c_szFileName);
    CRaceMotionData::Delete(pRaceMotionData);
    pRaceMotionData = NULL;
    return NULL;
  }

  // 2004. 3. 15. myevan. 원래는 모션내 인덱스 정보가 있어야 한다.
  pRaceMotionData->SetName(wMotionIndex);

  /////

  // FIXME : 이미 GR2가 로드 되어 있을 경우에는 어떻게 해야 하는가?
  //         현재는 똑같은 것이 하나 더 추가 되어 버린다. - [levites]

  return NEW_RegisterMotion(pRaceMotionData, wMotionMode, wMotionIndex,
                            pRaceMotionData->GetMotionFileName(), byPercentage);

  // 2004.09.03.myevan.새로운함수로 대처
  // TMotion	Motion;
  // Motion.byPercentage	= byPercentage;
  // Motion.pMotion = (CGraphicThing
  // *)CResourceManager::Instance().GetResourcePointer(pRaceMotionData->GetMotionFileName());
  // Motion.pMotionData = pRaceMotionData;
  //__OLD_RegisterMotion(wMotionMode, wMotionIndex, Motion);
  // return true;
}

void CRaceData::OLD_RegisterMotion(WORD wMotionModeIndex, WORD wMotionIndex,
                                   const char *c_szFileName,
                                   BYTE byPercentage) {
  CGraphicThing *pThing =
      (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
          c_szFileName);

  TMotion Motion;
  Motion.byPercentage = byPercentage;
  Motion.pMotion = pThing;
  Motion.pMotionData = NULL;
  __OLD_RegisterMotion(wMotionModeIndex, wMotionIndex, Motion);
}

void CRaceData::__OLD_RegisterMotion(WORD wMotionMode, WORD wMotionIndex,
                                     const TMotion &rMotion) {
  TMotionModeData *pMotionModeData;
  if (!GetMotionModeDataPointer(wMotionMode, &pMotionModeData)) {
    AssertLog("Failed getting motion mode data!");
    return;
  }

  TMotionVectorMap::iterator itor =
      pMotionModeData->MotionVectorMap.find(wMotionIndex);
  if (pMotionModeData->MotionVectorMap.end() == itor) {
    TMotionVector MotionVector;
    MotionVector.push_back(rMotion);

    pMotionModeData->MotionVectorMap.insert(
        TMotionVectorMap::value_type(wMotionIndex, MotionVector));
  } else {
    TMotionVector &rMotionVector = itor->second;
    rMotionVector.push_back(rMotion);
  }
}

bool CRaceData::SetMotionRandomWeight(WORD wMotionModeIndex, WORD wMotionIndex,
                                      WORD wMotionSubIndex, BYTE byPercentage) {
  TMotionModeData *pMotionModeData;

  if (!GetMotionModeDataPointer(wMotionModeIndex, &pMotionModeData)) {
    // AssertLog("Failed getting motion mode data!");
    return false;
  }

  TMotionVectorMap::iterator itor =
      pMotionModeData->MotionVectorMap.find(wMotionIndex);

  if (pMotionModeData->MotionVectorMap.end() != itor) {
    TMotionVector &rMotionVector = itor->second;
    if (wMotionSubIndex < rMotionVector.size()) {
      rMotionVector[wMotionSubIndex].byPercentage = byPercentage;
    } else {
      return false;
    }
  } else {
    TraceError("CRaceData::SetMotionRandomWeight(wMotionModeIndex=%d, "
               "wMotionIndex=%d, wMotionSubIndex=%d, byPercentage=%d) - Find "
               "Motion(wMotionIndex=%d) FAILED",
               wMotionModeIndex, wMotionIndex, wMotionSubIndex, byPercentage,
               wMotionModeIndex);

    return false;
  }

  return true;
}

void CRaceData::RegisterNormalAttack(WORD wMotionModeIndex, WORD wMotionIndex) {
  m_NormalAttackIndexMap.insert(
      TNormalAttackIndexMap::value_type(wMotionModeIndex, wMotionIndex));
}
BOOL CRaceData::GetNormalAttackIndex(WORD wMotionModeIndex,
                                     WORD *pwMotionIndex) {
  TNormalAttackIndexMap::iterator itor =
      m_NormalAttackIndexMap.find(wMotionModeIndex);

  if (m_NormalAttackIndexMap.end() == itor)
    return FALSE;

  *pwMotionIndex = itor->second;

  return TRUE;
}

void CRaceData::ReserveComboAttack(WORD wMotionModeIndex, WORD wComboType,
                                   DWORD dwComboCount) {
  TComboData ComboData;
  ComboData.ComboIndexVector.clear();
  ComboData.ComboIndexVector.resize(dwComboCount);
  m_ComboAttackDataMap.insert(TComboAttackDataMap::value_type(
      MAKE_COMBO_KEY(wMotionModeIndex, wComboType), ComboData));
}

void CRaceData::RegisterComboAttack(WORD wMotionModeIndex, WORD wComboType,
                                    DWORD dwComboIndex, WORD wMotionIndex) {
  TComboAttackDataIterator itor =
      m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));
  if (m_ComboAttackDataMap.end() == itor)
    return;

  TComboIndexVector &rComboIndexVector = itor->second.ComboIndexVector;
  if (dwComboIndex >= rComboIndexVector.size()) {
    AssertLog("CRaceData::RegisterCombo - Strange combo index!");
    return;
  }

  rComboIndexVector[dwComboIndex] = wMotionIndex;
}

BOOL CRaceData::GetComboDataPointer(WORD wMotionModeIndex, WORD wComboType,
                                    TComboData **ppComboData) {
  TComboAttackDataIterator itor =
      m_ComboAttackDataMap.find(MAKE_COMBO_KEY(wMotionModeIndex, wComboType));

  if (m_ComboAttackDataMap.end() == itor)
    return FALSE;

  *ppComboData = &itor->second;
  return TRUE;
}

const char *CRaceData::GetBaseModelFileName() const {
  return m_strBaseModelFileName.c_str();
}

const char *CRaceData::GetAttributeFileName() const {
  return m_strAttributeFileName.c_str();
}

const char *CRaceData::GetMotionListFileName() const {
  return m_strMotionListFileName.c_str();
}

CGraphicThing *CRaceData::GetBaseModelThing() {
  if (!m_pBaseModelThing) {
    m_pBaseModelThing =
        (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
            m_strBaseModelFileName.c_str());
  }

  return m_pBaseModelThing;
}

CGraphicThing *CRaceData::GetLODModelThing() {
  if (!m_pLODModelThing) {
    std::string strLODFileName =
        CFileNameHelper::NoExtension(m_strBaseModelFileName) + "_lod_01.gr2";
    if (CResourceManager::Instance().IsFileExist(strLODFileName.c_str())) {
      m_pLODModelThing =
          (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
              strLODFileName.c_str());
    }
  }

  return m_pLODModelThing;
}

CAttributeData *CRaceData::GetAttributeDataPtr() {
  if (m_strAttributeFileName.empty())
    return NULL;

  if (!CResourceManager::Instance().IsFileExist(m_strAttributeFileName.c_str()))
    return NULL;

  return (CAttributeData *)CResourceManager::Instance().GetResourcePointer(
      m_strAttributeFileName.c_str());
}

BOOL CRaceData::GetAttachingBoneName(DWORD dwPartIndex,
                                     const char **c_pszBoneName) {
  TAttachingBoneNameMap::iterator itor =
      m_AttachingBoneNameMap.find(dwPartIndex);
  if (itor == m_AttachingBoneNameMap.end())
    return FALSE;

  const std::string &c_rstrBoneName = itor->second;

  *c_pszBoneName = c_rstrBoneName.c_str();

  return TRUE;
}

BOOL CRaceData::IsTree() { return !m_strTreeFileName.empty(); }

const char *CRaceData::GetTreeFileName() { return m_strTreeFileName.c_str(); }

void CRaceData::Destroy() {
  m_kMap_dwHairKey_kHair.clear();
  m_kMap_dwShapeKey_kShape.clear();

  m_strBaseModelFileName = "";
  m_strTreeFileName = "";
  m_strAttributeFileName = "";
  m_strMotionListFileName = "motlist.txt";

  m_AttachingBoneNameMap.clear();
  m_ModelDataMap.clear();

  m_NormalAttackIndexMap.clear();
  m_ComboAttackDataMap.clear();

  TMotionModeDataMap::iterator itorMode = m_pMotionModeDataMap.begin();
  for (; itorMode != m_pMotionModeDataMap.end(); ++itorMode) {
    TMotionModeData *pMotionModeData = itorMode->second;

    TMotionVectorMap::iterator itorMotion =
        pMotionModeData->MotionVectorMap.begin();
    for (; itorMotion != pMotionModeData->MotionVectorMap.end(); ++itorMotion) {
      TMotionVector &rMotionVector = itorMotion->second;
      for (DWORD i = 0; i < rMotionVector.size(); ++i) {
        CRaceMotionData::Delete(rMotionVector[i].pMotionData);
      }
    }

    ms_MotionModeDataPool.Free(pMotionModeData);
  }
  m_pMotionModeDataMap.clear();

  __Initialize();
}

void CRaceData::__Initialize() {
  m_strMotionListFileName = "motlist.txt";

  m_pBaseModelThing = NULL;
  m_pLODModelThing = NULL;

  m_dwRaceIndex = 0;

  memset(m_adwSmokeEffectID, 0, sizeof(m_adwSmokeEffectID));
}

CRaceData::CRaceData() { __Initialize(); }
CRaceData::~CRaceData() { Destroy(); }