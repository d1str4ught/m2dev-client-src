#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceManager.h"
#include "ItemManager.h"
#include "RaceData.h"

#include "EterLib/ResourceManager.h"
#include "EterGrnLib/util.h"

DWORD CActorInstance::GetVirtualID() { return m_dwSelfVID; }

void CActorInstance::SetVirtualID(DWORD dwVID) { m_dwSelfVID = dwVID; }

void CActorInstance::UpdateAttribute() {
  if (!m_pAttributeInstance)
    return;

  if (!m_bNeedUpdateCollision)
    return;

  m_bNeedUpdateCollision = FALSE;

  const CStaticCollisionDataVector &c_rkVec_ColliData =
      m_pAttributeInstance->GetObjectPointer()->GetCollisionDataVector();
  UpdateCollisionData(&c_rkVec_ColliData);

  m_pAttributeInstance->RefreshObject(GetTransform());
  UpdateHeightInstance(m_pAttributeInstance);

  // BOOL isHeightData = m_pAttributeInstance->IsEmpty();
}

void CActorInstance::__CreateAttributeInstance(CAttributeData *pData) {
  m_pAttributeInstance = CAttributeInstance::New();
  m_pAttributeInstance->Clear();
  m_pAttributeInstance->SetObjectPointer(pData);
  if (pData->IsEmpty()) {
    m_pAttributeInstance->Clear();
    CAttributeInstance::Delete(m_pAttributeInstance);
  }
}

DWORD CActorInstance::GetRace() { return m_eRace; }

bool CActorInstance::SetRace(DWORD eRace) {
  CRaceData *pRaceData;
  if (!CRaceManager::Instance().GetRaceDataPointer(eRace, &pRaceData)) {
    m_eRace = 0;
    m_pkCurRaceData = NULL;
    return false;
  }

  m_eRace = eRace;
  m_pkCurRaceData = pRaceData;

  CAttributeData *pAttributeData = pRaceData->GetAttributeDataPtr();
  if (pAttributeData) {
    __CreateAttributeInstance(pAttributeData);
  }

  memset(m_adwPartItemID, 0, sizeof(m_adwPartItemID));

  // Setup Graphic ResourceData
  __ClearAttachingEffect();

  CGraphicThingInstance::Clear();

  // NOTE : PC만 Part별로 다 생성하게 해준다.
  if (IsPC()) {
    CGraphicThingInstance::ReserveModelThing(CRaceData::PART_MAX_NUM);
    CGraphicThingInstance::ReserveModelInstance(CRaceData::PART_MAX_NUM);
  } else {
    CGraphicThingInstance::ReserveModelThing(1);
    CGraphicThingInstance::ReserveModelInstance(1);
  }

  CRaceData::TMotionModeDataIterator itor;

  if (pRaceData->CreateMotionModeIterator(itor)) {
    do {
      WORD wMotionMode = itor->first;
      CRaceData::TMotionModeData *pMotionModeData = itor->second;

      CRaceData::TMotionVectorMap::iterator itorMotion =
          pMotionModeData->MotionVectorMap.begin();
      for (; itorMotion != pMotionModeData->MotionVectorMap.end();
           ++itorMotion) {
        WORD wMotionIndex = itorMotion->first;
        const CRaceData::TMotionVector &c_rMotionVector = itorMotion->second;
        CRaceData::TMotionVector::const_iterator it;
        DWORD i;
        for (i = 0, it = c_rMotionVector.begin(); it != c_rMotionVector.end();
             ++i, ++it) {
          DWORD dwMotionKey =
              MAKE_RANDOM_MOTION_KEY(wMotionMode, wMotionIndex, i);
          CGraphicThingInstance::RegisterMotionThing(dwMotionKey, it->pMotion);
        }
      }
    } while (pRaceData->NextMotionModeIterator(itor));
  }

  return true;
}

void CActorInstance::SetHair(DWORD eHair) {
  m_eHair = eHair;

  CRaceData *pRaceData;

  if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
    return;

  CRaceData::SHair *pkHair = pRaceData->FindHair(eHair);
  if (pkHair) {
    if (!pkHair->m_stModelFileName.empty()) {
      CGraphicThing *pkHairThing =
          (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
              pkHair->m_stModelFileName.c_str());
      RegisterModelThing(CRaceData::PART_HAIR, pkHairThing);
      SetModelInstance(CRaceData::PART_HAIR, CRaceData::PART_HAIR, 0,
                       CRaceData::PART_MAIN);
    }

    const std::vector<CRaceData::SSkin> &c_rkVct_kSkin = pkHair->m_kVct_kSkin;
    std::vector<CRaceData::SSkin>::const_iterator i;
    for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i) {
      const CRaceData::SSkin &c_rkSkinItem = *i;

      CResource *pkRes = CResourceManager::Instance().GetResourcePointer(
          c_rkSkinItem.m_stDstFileName.c_str());

      if (pkRes)
        SetMaterialImagePointer(CRaceData::PART_HAIR,
                                c_rkSkinItem.m_stSrcFileName.c_str(),
                                static_cast<CGraphicImage *>(pkRes));
    }
  }
}

void CActorInstance::SetShape(DWORD eShape, float fSpecular) {
  m_eShape = eShape;

  CRaceData *pRaceData;
  if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
    return;

  CRaceData::SShape *pkShape = pRaceData->FindShape(eShape);
  if (pkShape) {
    CResourceManager &rkResMgr = CResourceManager::Instance();

    if (pkShape->m_stModelFileName.empty()) {
      CGraphicThing *pModelThing = pRaceData->GetBaseModelThing();
      RegisterModelThing(0, pModelThing);
    } else {
      CGraphicThing *pModelThing = (CGraphicThing *)rkResMgr.GetResourcePointer(
          pkShape->m_stModelFileName.c_str());
      RegisterModelThing(0, pModelThing);
    }

    {
      std::string stLODModelFileName;

      char szLODModelFileNameEnd[256];
      for (UINT uLODIndex = 1; uLODIndex <= 3; ++uLODIndex) {
        sprintf(szLODModelFileNameEnd, "_lod_%.2d.gr2", uLODIndex);
        stLODModelFileName =
            CFileNameHelper::NoExtension(pkShape->m_stModelFileName) +
            szLODModelFileNameEnd;
        if (!rkResMgr.IsFileExist(stLODModelFileName.c_str()))
          break;

        CGraphicThing *pLODModelThing =
            (CGraphicThing *)rkResMgr.GetResourcePointer(
                stLODModelFileName.c_str());
        if (!pLODModelThing)
          break;

        RegisterLODThing(0, pLODModelThing);
      }
    }

    SetModelInstance(0, 0, 0);

    const std::vector<CRaceData::SSkin> &c_rkVct_kSkin = pkShape->m_kVct_kSkin;
    std::vector<CRaceData::SSkin>::const_iterator i;
    for (i = c_rkVct_kSkin.begin(); i != c_rkVct_kSkin.end(); ++i) {
      const CRaceData::SSkin &c_rkSkinItem = *i;

      CResource *pkRes = CResourceManager::Instance().GetResourcePointer(
          c_rkSkinItem.m_stDstFileName.c_str());

      if (pkRes) {
        if (fSpecular > 0.0f) {
          SMaterialData kMaterialData;
          kMaterialData.pImage = static_cast<CGraphicImage *>(pkRes);
          kMaterialData.isSpecularEnable = TRUE;
          kMaterialData.fSpecularPower = fSpecular;
          kMaterialData.bSphereMapIndex = 0;
          SetMaterialData(c_rkSkinItem.m_ePart,
                          c_rkSkinItem.m_stSrcFileName.c_str(), kMaterialData);
        } else {
          SetMaterialImagePointer(c_rkSkinItem.m_ePart,
                                  c_rkSkinItem.m_stSrcFileName.c_str(),
                                  static_cast<CGraphicImage *>(pkRes));
        }
      }
    }
  } else {
    if (pRaceData->IsTree()) {
      __CreateTree(pRaceData->GetTreeFileName());
    } else {
      CGraphicThing *pModelThing = pRaceData->GetBaseModelThing();
      RegisterModelThing(0, pModelThing);

      CGraphicThing *pLODModelThing = pRaceData->GetLODModelThing();
      RegisterLODThing(0, pLODModelThing);

      SetModelInstance(0, 0, 0);
    }
  }

  // Attaching Objects
  for (DWORD i = 0; i < pRaceData->GetAttachingDataCount(); ++i) {
    const NRaceData::TAttachingData *c_pAttachingData;
    if (!pRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
      continue;

    switch (c_pAttachingData->dwType) {
    case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
      if (c_pAttachingData->isAttaching) {
        AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(),
                           c_pAttachingData->pEffectData->strFileName.c_str());
      } else {
        AttachEffectByName(0, 0,
                           c_pAttachingData->pEffectData->strFileName.c_str());
      }
      break;
    }
  }
}

void CActorInstance::ChangeMaterial(const char *c_szFileName) {
  CRaceData *pRaceData;
  if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pRaceData))
    return;

  CRaceData::SShape *pkShape = pRaceData->FindShape(m_eShape);
  if (!pkShape)
    return;

  const std::vector<CRaceData::SSkin> &c_rkVct_kSkin = pkShape->m_kVct_kSkin;
  if (c_rkVct_kSkin.empty())
    return;

  std::vector<CRaceData::SSkin>::const_iterator i = c_rkVct_kSkin.begin();
  const CRaceData::SSkin &c_rkSkinItem = *i;

  std::string dstFileName = "d:/ymir work/npc/guild_symbol/guild_symbol.dds";
  dstFileName = c_szFileName;

  CResource *pkRes =
      CResourceManager::Instance().GetResourcePointer(dstFileName.c_str());
  if (!pkRes)
    return;

  SetMaterialImagePointer(c_rkSkinItem.m_ePart,
                          c_rkSkinItem.m_stSrcFileName.c_str(),
                          static_cast<CGraphicImage *>(pkRes));
}

DWORD CActorInstance::GetPartItemID(DWORD dwPartIndex) {
  if (dwPartIndex >= CRaceData::PART_MAX_NUM) {
    TraceError("CActorInstance::GetPartIndex(dwPartIndex=%d/"
               "CRaceData::PART_MAX_NUM=%d)",
               dwPartIndex, CRaceData::PART_MAX_NUM);
    return 0;
  }

  return m_adwPartItemID[dwPartIndex];
}

void CActorInstance::SetSpecularInfo(BOOL bEnable, int iPart, float fAlpha) {
  CRaceData *pkRaceData;
  if (!CRaceManager::Instance().GetRaceDataPointer(m_eRace, &pkRaceData))
    return;

  CRaceData::SShape *pkShape = pkRaceData->FindShape(m_eShape);
  if (pkShape->m_kVct_kSkin.empty())
    return;

  std::string filename = pkShape->m_kVct_kSkin[0].m_stSrcFileName.c_str();
  CFileNameHelper::ChangeDosPath(filename);

  CGraphicThingInstance::SetSpecularInfo(iPart, filename.c_str(), bEnable,
                                         fAlpha);
}

void CActorInstance::SetSpecularInfoForce(BOOL bEnable, int iPart,
                                          float fAlpha) {
  CGraphicThingInstance::SetSpecularInfo(iPart, NULL, bEnable, fAlpha);
}
