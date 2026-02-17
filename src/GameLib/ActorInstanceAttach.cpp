#include "StdAfx.h"
#include "EffectLib/EffectManager.h"

#include "ActorInstance.h"
#include "ItemData.h"
#include "ItemManager.h"
#include "RaceData.h"
#include "WeaponTrace.h"

BOOL USE_WEAPON_SPECULAR = TRUE;

BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM = FALSE;

DWORD Vietnam_ConvertWeaponVnum(DWORD vnum) {
  DWORD base = vnum / 10 * 10;
  DWORD rest = vnum % 10;
  switch (base) {
  case 10:
    base = 5000;
    break;
  case 20:
    base = 5010;
    break;
  case 30:
    base = 5020;
    break;
  case 40:
    base = 5030;
    break;
  case 50:
    base = 5030;
    break;
  case 60:
    base = 5040;
    break;
  case 70:
    base = 5040;
    break;
  case 80:
    base = 5050;
    break;
  case 90:
    base = 5050;
    break;
  case 100:
    base = 5060;
    break;
  case 110:
    base = 5060;
    break;
  case 120:
    base = 5070;
    break;
  case 130:
    base = 5070;
    break;
  case 140:
    base = 5080;
    break;
  case 150:
    base = 5080;
    break;
  case 160:
    base = 5090;
    break;
  case 170:
    base = 5090;
    break;
  case 180:
    base = 5100;
    break;
  case 190:
    base = 5100;
    break;
  case 200:
    base = 5110;
    break;
  case 210:
    base = 5110;
    break;
  case 220:
    base = 5120;
    break;
  case 230:
    base = 5120;
    break;
  case 240:
    base = 5130;
    break;
  case 250:
    base = 5130;
    break;
  case 260:
    base = 5140;
    break;
  case 270:
    base = 5140;
    break;
  case 280:
    base = 5150;
    break;
  case 290:
    base = 5150;
    break;
  case 1000:
    base = 5000;
    break;
  case 1010:
    base = 5010;
    break;
  case 1020:
    base = 5020;
    break;
  case 1030:
    base = 5030;
    break;
  case 1040:
    base = 5040;
    break;
  case 1050:
    base = 5050;
    break;
  case 1060:
    base = 5060;
    break;
  case 1070:
    base = 5070;
    break;
  case 1080:
    base = 5080;
    break;
  case 1090:
    base = 5090;
    break;
  case 1100:
    base = 5100;
    break;
  case 1110:
    base = 5110;
    break;
  case 1120:
    base = 5120;
    break;
  case 1130:
    base = 5130;
    break;
  case 1140:
    base = 5140;
    break;
  case 1150:
    base = 5150;
    break;
  case 1160:
    base = 5150;
    break;
  case 1170:
    base = 5150;
    break;
  case 3000:
    base = 5000;
    break;
  case 3010:
    base = 5010;
    break;
  case 3020:
    base = 5020;
    break;
  case 3030:
    base = 5030;
    break;
  case 3040:
    base = 5040;
    break;
  case 3050:
    base = 5050;
    break;
  case 3060:
    base = 5060;
    break;
  case 3070:
    base = 5070;
    break;
  case 3080:
    base = 5080;
    break;
  case 3090:
    base = 5090;
    break;
  case 3100:
    base = 5100;
    break;
  case 3110:
    base = 5100;
    break;
  case 3120:
    base = 5110;
    break;
  case 3130:
    base = 5110;
    break;
  case 3140:
    base = 5120;
    break;
  case 3150:
    base = 5120;
    break;
  case 3160:
    base = 5130;
    break;
  case 3170:
    base = 5130;
    break;
  case 3180:
    base = 5140;
    break;
  case 3190:
    base = 5140;
    break;
  case 3200:
    base = 5150;
    break;
  case 3210:
    base = 5150;
    break;
  }
  return base + rest;
}

DWORD CActorInstance::AttachSmokeEffect(DWORD eSmoke) {
  if (!m_pkCurRaceData)
    return 0;

  DWORD dwSmokeEffectID = m_pkCurRaceData->GetSmokeEffectID(eSmoke);

  return AttachEffectByID(0, m_pkCurRaceData->GetSmokeBone().c_str(),
                          dwSmokeEffectID);
}

bool CActorInstance::__IsLeftHandWeapon(DWORD type) {
  if (CItemData::WEAPON_DAGGER == type ||
      (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
    return true;
  else if (CItemData::WEAPON_BOW == type)
    return true;
  else
    return false;
}

bool CActorInstance::__IsRightHandWeapon(DWORD type) {
  if (CItemData::WEAPON_DAGGER == type ||
      (CItemData::WEAPON_FAN == type && __IsMountingHorse()))
    return true;
  else if (CItemData::WEAPON_BOW == type)
    return false;
  else
    return true;
}

bool CActorInstance::__IsWeaponTrace(DWORD weaponType) {
  switch (weaponType) {
  case CItemData::WEAPON_BELL:
  case CItemData::WEAPON_FAN:
  case CItemData::WEAPON_BOW:
    return false;
  default:
    return true;
  }
}

void CActorInstance::AttachWeapon(DWORD dwItemIndex, DWORD dwParentPartIndex,
                                  DWORD dwPartIndex) {
  if (dwPartIndex >= CRaceData::PART_MAX_NUM)
    return;

  m_adwPartItemID[dwPartIndex] = dwItemIndex;

  if (USE_VIETNAM_CONVERT_WEAPON_VNUM)
    dwItemIndex = Vietnam_ConvertWeaponVnum(dwItemIndex);

  CItemData *pItemData;
  if (!CItemManager::Instance().GetItemDataPointer(dwItemIndex, &pItemData)) {
    RegisterModelThing(dwPartIndex, NULL);
    SetModelInstance(dwPartIndex, dwPartIndex, 0);

    RegisterModelThing(CRaceData::PART_WEAPON_LEFT, NULL);
    SetModelInstance(CRaceData::PART_WEAPON_LEFT, CRaceData::PART_WEAPON_LEFT,
                     0);

    RefreshActorInstance();
    return;
  }

  __DestroyWeaponTrace();
  // 양손무기(자객 이도류) 왼손,오른손 모두에 장착.
  if (__IsRightHandWeapon(pItemData->GetWeaponType()))
    AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON, pItemData);
  if (__IsLeftHandWeapon(pItemData->GetWeaponType()))
    AttachWeapon(dwParentPartIndex, CRaceData::PART_WEAPON_LEFT, pItemData);
}

BOOL CActorInstance::GetAttachingBoneName(DWORD dwPartIndex,
                                          const char **c_pszBoneName) {
  return m_pkCurRaceData->GetAttachingBoneName(dwPartIndex, c_pszBoneName);
}

void CActorInstance::AttachWeapon(DWORD dwParentPartIndex, DWORD dwPartIndex,
                                  CItemData *pItemData) {
  //	assert(m_pkCurRaceData);
  if (!pItemData)
    return;

  const char *szBoneName;
  if (!GetAttachingBoneName(dwPartIndex, &szBoneName))
    return;

  // NOTE : (이도류처리)단도일 경우 형태가 다른 것으로 얻는다. 없을 경우
  // 디폴트를 리턴
  if (CRaceData::PART_WEAPON_LEFT == dwPartIndex) {
    RegisterModelThing(dwPartIndex, pItemData->GetSubModelThing());
  } else {
    RegisterModelThing(dwPartIndex, pItemData->GetModelThing());
  }

  for (DWORD i = 0; i < pItemData->GetLODModelThingCount(); ++i) {
    CGraphicThing *pThing;

    if (!pItemData->GetLODModelThingPointer(i, &pThing))
      continue;

    RegisterLODThing(dwPartIndex, pThing);
  }

  SetModelInstance(dwPartIndex, dwPartIndex, 0);
  AttachModelInstance(dwParentPartIndex, szBoneName, dwPartIndex);

  // 20041208.myevan.무기스펙큘러(값옷은 SetShape에서 직접 해준다.)
  if (USE_WEAPON_SPECULAR) {
    SMaterialData kMaterialData;
    kMaterialData.pImage = NULL;
    kMaterialData.isSpecularEnable = TRUE;
    kMaterialData.fSpecularPower = pItemData->GetSpecularPowerf();
    kMaterialData.bSphereMapIndex = 1;
    SetMaterialData(dwPartIndex, NULL, kMaterialData);
  }

  // Weapon Trace
  if (__IsWeaponTrace(pItemData->GetWeaponType())) {
    CWeaponTrace *pWeaponTrace = CWeaponTrace::New();
    pWeaponTrace->SetWeaponInstance(this, dwPartIndex, szBoneName);
    m_WeaponTraceVector.push_back(pWeaponTrace);
  }
}

void CActorInstance::DettachEffect(DWORD dwEID) {
  std::list<TAttachingEffect>::iterator i = m_AttachingEffectList.begin();

  while (i != m_AttachingEffectList.end()) {
    TAttachingEffect &rkAttEft = (*i);

    if (rkAttEft.dwEffectIndex == dwEID) {
      i = m_AttachingEffectList.erase(i);
      CEffectManager::Instance().DestroyEffectInstance(dwEID);
    } else {
      ++i;
    }
  }
}

DWORD CActorInstance::AttachEffectByName(DWORD dwParentPartIndex,
                                         const char *c_pszBoneName,
                                         const char *c_pszEffectName) {
  std::string str;
  DWORD dwCRC;
  StringPath(c_pszEffectName, str);
  dwCRC = GetCaseCRC32(str.c_str(), str.length());

  return AttachEffectByID(dwParentPartIndex, c_pszBoneName, dwCRC);
}

DWORD CActorInstance::AttachEffectByID(DWORD dwParentPartIndex,
                                       const char *c_pszBoneName,
                                       DWORD dwEffectID,
                                       const D3DXVECTOR3 *c_pv3Position) {
  TAttachingEffect ae;
  ae.iLifeType = EFFECT_LIFE_INFINITE;
  ae.dwEndTime = 0;
  ae.dwModelIndex = dwParentPartIndex;
  ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
  ae.isAttaching = TRUE;
  if (c_pv3Position) {
    D3DXMatrixTranslation(&ae.matTranslation, c_pv3Position->x,
                          c_pv3Position->y, c_pv3Position->z);
  } else {
    D3DXMatrixIdentity(&ae.matTranslation);
  }
  CEffectManager &rkEftMgr = CEffectManager::Instance();
  rkEftMgr.CreateEffectInstance(ae.dwEffectIndex, dwEffectID);

  if (c_pszBoneName) {
    int iBoneIndex;

    if (!FindBoneIndex(dwParentPartIndex, c_pszBoneName, &iBoneIndex)) {
      ae.iBoneIndex = -1;
      // Tracef("Cannot get Bone Index\n");
      // assert(false && "Cannot get Bone Index");
    } else {
      ae.iBoneIndex = iBoneIndex;
    }
  } else {
    ae.iBoneIndex = -1;
  }

  m_AttachingEffectList.push_back(ae);

  return ae.dwEffectIndex;
}

void CActorInstance::RefreshActorInstance() {
  if (!m_pkCurRaceData) {
    TraceError(
        "void CActorInstance::RefreshActorInstance() - m_pkCurRaceData=NULL");
    return;
  }

  // This is Temporary place before making the weapon detection system
  // Setup Collison Detection Data
  m_BodyPointInstanceList.clear();
  // m_AttackingPointInstanceList.clear();
  m_DefendingPointInstanceList.clear();

  // Base
  for (DWORD i = 0; i < m_pkCurRaceData->GetAttachingDataCount(); ++i) {
    const NRaceData::TAttachingData *c_pAttachingData;

    if (!m_pkCurRaceData->GetAttachingDataPointer(i, &c_pAttachingData))
      continue;

    switch (c_pAttachingData->dwType) {
    case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA: {
      const NRaceData::TCollisionData *c_pCollisionData =
          c_pAttachingData->pCollisionData;

      TCollisionPointInstance PointInstance;
      if (NRaceData::COLLISION_TYPE_ATTACKING ==
          c_pCollisionData->iCollisionType)
        continue;

      if (!CreateCollisionInstancePiece(CRaceData::PART_MAIN, c_pAttachingData,
                                        &PointInstance))
        continue;

      switch (c_pCollisionData->iCollisionType) {
      case NRaceData::COLLISION_TYPE_ATTACKING:
        // m_AttackingPointInstanceList.push_back(PointInstance);
        break;
      case NRaceData::COLLISION_TYPE_DEFENDING:
        m_DefendingPointInstanceList.push_back(PointInstance);
        break;
      case NRaceData::COLLISION_TYPE_BODY:
        m_BodyPointInstanceList.push_back(PointInstance);
        break;
      }
    } break;

    case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
      //				if (!m_bEffectInitialized)
      //				{
      //					DWORD dwCRC;
      //					StringPath(c_pAttachingData->pEffectData->strFileName);
      //					dwCRC =
      // GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),c_pAttachingData->pEffectData->strFileName.length());
      //
      //					TAttachingEffect ae;
      //					ae.iLifeType =
      // EFFECT_LIFE_INFINITE; 					ae.dwEndTime =
      // 0; 					ae.dwModelIndex = 0;
      //					ae.dwEffectIndex =
      // CEffectManager::Instance().GetEmptyIndex();
      // ae.isAttaching = TRUE;
      //					CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex,
      // dwCRC);
      //
      //					if
      //(c_pAttachingData->isAttaching)
      //					{
      //						int iBoneIndex;
      //						if
      //(!FindBoneIndex(0,c_pAttachingData->strAttachingBoneName.c_str(),
      //&iBoneIndex))
      //						{
      //							Tracef("Cannot
      // get Bone Index\n");
      // assert(false/*Cannot get Bone Index*/);
      //						}
      //
      //						ae.iBoneIndex =
      // iBoneIndex;
      //					}
      //					else
      //					{
      //						ae.iBoneIndex = -1;
      //					}
      //
      //					m_AttachingEffectList.push_back(ae);
      //				}

      if (c_pAttachingData->isAttaching) {
        AttachEffectByName(0, c_pAttachingData->strAttachingBoneName.c_str(),
                           c_pAttachingData->pEffectData->strFileName.c_str());
      } else {
        AttachEffectByName(0, 0,
                           c_pAttachingData->pEffectData->strFileName.c_str());
      }
      break;

    case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
      break;

    default:
      assert(false /*NOT_IMPLEMENTED*/);
      break;
    }
  }

  for (DWORD j = 0; j < CRaceData::PART_MAX_NUM; ++j) {
    if (0 == m_adwPartItemID[j])
      continue;

    CItemData *pItemData;
    if (!CItemManager::Instance().GetItemDataPointer(m_adwPartItemID[j],
                                                     &pItemData))
      return;

    for (DWORD k = 0; k < pItemData->GetAttachingDataCount(); ++k) {
      const NRaceData::TAttachingData *c_pAttachingData;

      if (!pItemData->GetAttachingDataPointer(k, &c_pAttachingData))
        continue;

      switch (c_pAttachingData->dwType) {
      case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA: {
        const NRaceData::TCollisionData *c_pCollisionData =
            c_pAttachingData->pCollisionData;

        // FIXME : 첫번째 인자는 Part의 번호다.
        //         Base는 무조건 0인가? - [levites]
        TCollisionPointInstance PointInstance;
        if (NRaceData::COLLISION_TYPE_ATTACKING ==
            c_pCollisionData->iCollisionType)
          continue;
        if (!CreateCollisionInstancePiece(j, c_pAttachingData, &PointInstance))
          continue;

        switch (c_pCollisionData->iCollisionType) {
        case NRaceData::COLLISION_TYPE_ATTACKING:
          // m_AttackingPointInstanceList.push_back(PointInstance);
          break;
        case NRaceData::COLLISION_TYPE_DEFENDING:
          m_DefendingPointInstanceList.push_back(PointInstance);
          break;
        case NRaceData::COLLISION_TYPE_BODY:
          m_BodyPointInstanceList.push_back(PointInstance);
          break;
        }
      } break;

      case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
        if (!m_bEffectInitialized) {
          DWORD dwCRC;
          StringPath(c_pAttachingData->pEffectData->strFileName);
          dwCRC =
              GetCaseCRC32(c_pAttachingData->pEffectData->strFileName.c_str(),
                           c_pAttachingData->pEffectData->strFileName.length());

          TAttachingEffect ae;
          ae.iLifeType = EFFECT_LIFE_INFINITE;
          ae.dwEndTime = 0;
          ae.dwModelIndex = j;
          ae.dwEffectIndex = CEffectManager::Instance().GetEmptyIndex();
          ae.isAttaching = TRUE;
          CEffectManager::Instance().CreateEffectInstance(ae.dwEffectIndex,
                                                          dwCRC);

          int iBoneIndex;
          if (!FindBoneIndex(j, c_pAttachingData->strAttachingBoneName.c_str(),
                             &iBoneIndex)) {
            Tracef("Cannot get Bone Index\n");
            assert(false /*Cannot get Bone Index*/);
          }
          Tracef("Creating %p %d %d\n", this, j, k);

          ae.iBoneIndex = iBoneIndex;

          m_AttachingEffectList.push_back(ae);
        }
        break;

      case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
        break;

      default:
        assert(false /*NOT_IMPLEMENTED*/);
        break;
      }
    }
  }

  m_bEffectInitialized = true;
}

void CActorInstance::SetWeaponTraceTexture(const char *szTextureName) {
  std::vector<CWeaponTrace *>::iterator it;
  for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end();
       ++it) {
    (*it)->SetTexture(szTextureName);
  }
}

void CActorInstance::UseTextureWeaponTrace() {
  for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
           std::mem_fn(&CWeaponTrace::UseTexture));
}

void CActorInstance::UseAlphaWeaponTrace() {
  for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
           std::mem_fn(&CWeaponTrace::UseAlpha));
}

void CActorInstance::UpdateAttachingInstances() {
  CEffectManager &rkEftMgr = CEffectManager::Instance();

  std::list<TAttachingEffect>::iterator it;
  DWORD dwCurrentTime = CTimer::Instance().GetCurrentMillisecond();
  for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();) {
    if (EFFECT_LIFE_WITH_MOTION == it->iLifeType) {
      ++it;
      continue;
    }

    if ((EFFECT_LIFE_NORMAL == it->iLifeType &&
         it->dwEndTime < dwCurrentTime) ||
        !rkEftMgr.IsAliveEffect(it->dwEffectIndex)) {
      rkEftMgr.DestroyEffectInstance(it->dwEffectIndex);
      it = m_AttachingEffectList.erase(it);
    } else {
      if (it->isAttaching) {
        rkEftMgr.SelectEffectInstance(it->dwEffectIndex);

        if (it->iBoneIndex == -1) {
          D3DXMATRIX matTransform;
          matTransform = it->matTranslation;
          matTransform *= m_worldMatrix;
          rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
        } else {
          D3DXMATRIX *pBoneMat;
          if (GetBoneMatrix(it->dwModelIndex, it->iBoneIndex, &pBoneMat)) {
            D3DXMATRIX matTransform;
            matTransform = *pBoneMat;
            matTransform *= it->matTranslation;
            matTransform *= m_worldMatrix;
            rkEftMgr.SetEffectInstanceGlobalMatrix(matTransform);
          } else {
            // TraceError("GetBoneMatrix(modelIndex(%d),
            // boneIndex(%d)).NOT_FOUND_BONE", 	it->dwModelIndex,
            // it->iBoneIndex);
          }
        }
      }

      ++it;
    }
  }
}

void CActorInstance::ShowAllAttachingEffect() {
  std::list<TAttachingEffect>::iterator it;
  for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();
       ++it) {
    CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
    CEffectManager::Instance().ShowEffect();
    CEffectManager::Instance().ReleaseAlwaysHidden();
  }
}

void CActorInstance::HideAllAttachingEffect() {
  std::list<TAttachingEffect>::iterator it;

  for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();
       ++it) {
    CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
    CEffectManager::Instance().HideEffect();
    CEffectManager::Instance().ApplyAlwaysHidden();
  }
}

void CActorInstance::HideAllAttachingEffectForEunhyeong() {
  std::list<TAttachingEffect>::iterator it;
  CEffectManager &mgr = CEffectManager::Instance();

  for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();
       ++it) {
    mgr.SelectEffectInstance(it->dwEffectIndex);
    DWORD crc = mgr.GetSelectedEffectDataCRC();
    CEffectData *pData = nullptr;
    bool isEunh = false;

    if (crc != 0 && mgr.GetEffectData(crc, &pData) && pData) {
      std::string fname = pData->GetFileName();
      std::string fnameLower = fname;

      // normalize for case-insensitive search and path separators
      for (char &c : fnameLower) {
        if (c == '\\')
          c = '/';
        c = (char)tolower((unsigned char)c);
      }

      // Match any eunhyeongbeop variant (eunhyeongbeop, eunhyeongbeop_2,
      // eunhyeongbeop_3, etc.)
      if (fnameLower.find("eunhyeongbeop") != std::string::npos)
        isEunh = true;

      Tracef("Effect Instance %u -> file=%s crc=0x%08X\n", it->dwEffectIndex,
             fname.c_str(), crc);
    } else {
      Tracef("Effect Instance %u -> no effect-data (crc=0x%08X)\n",
             it->dwEffectIndex, crc);
    }

    // If this attaching effect is an eunhyeongbeop variant, do NOT hide it
    // (show it).
    if (isEunh) {
      mgr.SelectEffectInstance(it->dwEffectIndex);
      mgr.ShowEffect();
      mgr.ReleaseAlwaysHidden();

      continue;
    }

    CEffectManager::Instance().SelectEffectInstance(it->dwEffectIndex);
    CEffectManager::Instance().HideEffect();
    CEffectManager::Instance().ApplyAlwaysHidden();
  }
}

void CActorInstance::__ClearAttachingEffect() {
  m_bEffectInitialized = false;

  std::list<TAttachingEffect>::iterator it;

  for (it = m_AttachingEffectList.begin(); it != m_AttachingEffectList.end();
       ++it) {
    CEffectManager::Instance().DestroyEffectInstance(it->dwEffectIndex);
  }

  m_AttachingEffectList.clear();
}
