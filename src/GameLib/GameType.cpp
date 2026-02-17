#include "StdAfx.h"
#include "GameType.h"
#include "EffectLib/EffectManager.h"

extern float g_fGameFPS = 60.0f;

CDynamicPool<NRaceData::TCollisionData> NRaceData::g_CollisionDataPool;
CDynamicPool<NRaceData::TAttachingEffectData> NRaceData::g_EffectDataPool;
CDynamicPool<NRaceData::TAttachingObjectData> NRaceData::g_ObjectDataPool;

void NRaceData::DestroySystem() {
  g_CollisionDataPool.Destroy();
  g_EffectDataPool.Destroy();
  g_ObjectDataPool.Destroy();
}

/////////////////////////////////////////////////////////////////////////////////
// Character Attaching Collision Data
BOOL NRaceData::LoadAttackData(CTextFileLoader &rTextFileLoader,
                               TAttackData *pData) {
  if (!rTextFileLoader.GetTokenInteger("attacktype", &pData->iAttackType)) {
    pData->iAttackType = ATTACK_TYPE_SPLASH;
  }
  if (!rTextFileLoader.GetTokenInteger("hittingtype", &pData->iHittingType))
    return false;
  if (!rTextFileLoader.GetTokenFloat("stiffentime", &pData->fStiffenTime))
    return false;
  if (!rTextFileLoader.GetTokenFloat("invisibletime", &pData->fInvisibleTime))
    return false;
  if (!rTextFileLoader.GetTokenFloat("externalforce", &pData->fExternalForce))
    return false;
  if (!rTextFileLoader.GetTokenInteger("hitlimitcount",
                                       &pData->iHitLimitCount)) {
    pData->iHitLimitCount = 0;
  }

  return true;
}

bool NRaceData::THitData::Load(CTextFileLoader &rTextFileLoader) {
  if (!rTextFileLoader.GetTokenFloat("attackingstarttime", &fAttackStartTime))
    return false;
  if (!rTextFileLoader.GetTokenFloat("attackingendtime", &fAttackEndTime))
    return false;

  if (!rTextFileLoader.GetTokenString("attackingbone", &strBoneName))
    strBoneName = "";
  if (!rTextFileLoader.GetTokenFloat("weaponlength", &fWeaponLength))
    fWeaponLength = 0.0f;

  mapHitPosition.clear();

  CTokenVector *tv;
  if (rTextFileLoader.GetTokenVector("hitposition", &tv)) {
    CTokenVector::iterator it = tv->begin();

    while (it != tv->end()) {
      float time;
      NRaceData::THitTimePosition hp;
      time = atof(it++->c_str());
      hp.v3LastPosition.x = atof(it++->c_str());
      hp.v3LastPosition.y = atof(it++->c_str());
      hp.v3LastPosition.z = atof(it++->c_str());
      hp.v3Position.x = atof(it++->c_str());
      hp.v3Position.y = atof(it++->c_str());
      hp.v3Position.z = atof(it++->c_str());

      mapHitPosition[time] = hp;
    }
  }

  return true;
}

BOOL NRaceData::LoadMotionAttackData(CTextFileLoader &rTextFileLoader,
                                     TMotionAttackData *pData) {
  if (!LoadAttackData(rTextFileLoader, pData))
    return FALSE;

  // NOTE : 기존의 AttackingType이 MotionType으로 바뀌었음
  //        기존 데이타의 경우 없으면 기존 것으로 읽게끔..
  if (!rTextFileLoader.GetTokenInteger("motiontype", &pData->iMotionType)) {
    if (!rTextFileLoader.GetTokenInteger("attackingtype", &pData->iMotionType))
      return FALSE;
  }

  pData->HitDataContainer.clear();

  DWORD dwHitDataCount;
  if (!rTextFileLoader.GetTokenDoubleWord("hitdatacount", &dwHitDataCount)) {
    pData->HitDataContainer.push_back(SHitData());
    THitData &rHitData = *(pData->HitDataContainer.rbegin());
    if (!rHitData.Load(rTextFileLoader))
      return FALSE;
  } else {
    for (DWORD i = 0; i < dwHitDataCount; ++i) {
      if (!rTextFileLoader.SetChildNode(i))
        return FALSE;

      pData->HitDataContainer.push_back(SHitData());
      THitData &rHitData = *(pData->HitDataContainer.rbegin());
      if (!rHitData.Load(rTextFileLoader))
        return FALSE;

      rTextFileLoader.SetParentNode();
    }
  }

  return TRUE;
}

BOOL NRaceData::LoadCollisionData(CTextFileLoader &rTextFileLoader,
                                  TCollisionData *pCollisionData) {
  if (!rTextFileLoader.GetTokenInteger("collisiontype",
                                       &pCollisionData->iCollisionType))
    return false;

  DWORD dwSphereDataCount;
  if (!rTextFileLoader.GetTokenDoubleWord("spheredatacount",
                                          &dwSphereDataCount))
    return false;

  pCollisionData->SphereDataVector.clear();
  pCollisionData->SphereDataVector.resize(dwSphereDataCount);
  for (DWORD i = 0; i < dwSphereDataCount; ++i) {
    TSphereData &r = pCollisionData->SphereDataVector[i].GetAttribute();

    if (!rTextFileLoader.SetChildNode("spheredata", i))
      return false;

    if (!rTextFileLoader.GetTokenFloat("radius", &r.fRadius))
      return false;
    if (!rTextFileLoader.GetTokenPosition("position", &r.v3Position))
      return false;

    rTextFileLoader.SetParentNode();
  }

  return true;
}

void NRaceData::SaveEffectData(FILE *File, int iTabCount,
                               const TAttachingEffectData &c_rEffectData) {
  PrintfTabs(File, iTabCount, "EffectScriptName     \"%s\"\n",
             c_rEffectData.strFileName.c_str());

  PrintfTabs(File, iTabCount, "EffectPosition       %f %f %f\n",
             c_rEffectData.v3Position.x, c_rEffectData.v3Position.y,
             c_rEffectData.v3Position.z);
  PrintfTabs(File, iTabCount, "EffectRotation       %f %f %f\n",
             c_rEffectData.v3Rotation.x, c_rEffectData.v3Rotation.y,
             c_rEffectData.v3Rotation.z);
}

void NRaceData::SaveObjectData(FILE *File, int iTabCount,
                               const TAttachingObjectData &c_rObjectData) {
  PrintfTabs(File, iTabCount, "ObjectScriptName     \"%s\"\n",
             c_rObjectData.strFileName.c_str());
}

BOOL NRaceData::LoadEffectData(CTextFileLoader &rTextFileLoader,
                               TAttachingEffectData *pEffectData) {
  if (!rTextFileLoader.GetTokenString("effectscriptname",
                                      &pEffectData->strFileName))
    return false;

  if (!rTextFileLoader.GetTokenPosition("effectposition",
                                        &pEffectData->v3Position)) {
    pEffectData->v3Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  }

  if (!rTextFileLoader.GetTokenPosition("effectrotation",
                                        &pEffectData->v3Rotation)) {
    // pEffectData->qRotation = D3DXQUATERNION(0.0f,0.0f,0.0f,1.0f);
    pEffectData->v3Rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  }

  /*if (!*/
  // TODO DELETEME FIXME
  CEffectManager::Instance().RegisterEffect(pEffectData->strFileName.c_str());
  /*) return false;*/

  return true;
}

BOOL NRaceData::LoadObjectData(CTextFileLoader &rTextFileLoader,
                               TAttachingObjectData *pObjectData) {
  if (!rTextFileLoader.GetTokenString("objectscriptname",
                                      &pObjectData->strFileName))
    return false;

  return true;
}

void NRaceData::SaveAttackData(FILE *File, int iTabCount,
                               const TAttackData &c_rData) {
  PrintfTabs(File, iTabCount, "AttackType           %d\n", c_rData.iAttackType);
  PrintfTabs(File, iTabCount, "HittingType          %d\n",
             c_rData.iHittingType);
  PrintfTabs(File, iTabCount, "StiffenTime          %f\n",
             c_rData.fStiffenTime);
  PrintfTabs(File, iTabCount, "InvisibleTime        %f\n",
             c_rData.fInvisibleTime);
  PrintfTabs(File, iTabCount, "ExternalForce        %f\n",
             c_rData.fExternalForce);
  PrintfTabs(File, iTabCount, "HitLimitCount        %d\n",
             c_rData.iHitLimitCount);
}

void NRaceData::SaveMotionAttackData(FILE *File, int iTabCount,
                                     const TMotionAttackData &c_rData) {
  SaveAttackData(File, iTabCount, c_rData);

  PrintfTabs(File, iTabCount, "\n");
  PrintfTabs(File, iTabCount, "MotionType           %d\n", c_rData.iMotionType);
  PrintfTabs(File, iTabCount, "HitDataCount         %d\n",
             c_rData.HitDataContainer.size());

  DWORD dwHitDataNumber = 0;
  THitDataContainer::const_iterator itor = c_rData.HitDataContainer.begin();
  for (; itor != c_rData.HitDataContainer.end(); ++itor, ++dwHitDataNumber) {
    const THitData &c_rHitData = *itor;

    PrintfTabs(File, iTabCount, "Group HitData%02d\n", dwHitDataNumber);
    PrintfTabs(File, iTabCount, "{\n");
    PrintfTabs(File, iTabCount + 1, "AttackingStartTime   %f\n",
               c_rHitData.fAttackStartTime);
    PrintfTabs(File, iTabCount + 1, "AttackingEndTime     %f\n",
               c_rHitData.fAttackEndTime);

    PrintfTabs(File, iTabCount + 1, "AttackingBone        \"%s\"\n",
               c_rHitData.strBoneName.c_str());
    PrintfTabs(File, iTabCount + 1, "WeaponLength         %f\n",
               c_rHitData.fWeaponLength);
    PrintfTabs(File, iTabCount + 1, "List HitPosition\n");
    PrintfTabs(File, iTabCount + 1, "{\n");

    NRaceData::THitTimePositionMap::const_iterator it;
    for (it = c_rHitData.mapHitPosition.begin();
         it != c_rHitData.mapHitPosition.end(); ++it) {
      PrintfTabs(File, iTabCount + 2, "%f %f %f %f %f %f %f\n", it->first,
                 it->second.v3LastPosition.x, it->second.v3LastPosition.y,
                 it->second.v3LastPosition.z, it->second.v3Position.x,
                 it->second.v3Position.y, it->second.v3Position.z);
    }
    PrintfTabs(File, iTabCount + 1, "}\n");
    PrintfTabs(File, iTabCount, "}\n");
  }
}

void NRaceData::SaveCollisionData(FILE *File, int iTabCount,
                                  const TCollisionData &c_rCollisionData) {
  PrintfTabs(File, iTabCount, "CollisionType        %d\n",
             c_rCollisionData.iCollisionType);
  PrintfTabs(File, iTabCount, "\n");

  const CSphereCollisionInstanceVector &c_rSphereDataVector =
      c_rCollisionData.SphereDataVector;

  PrintfTabs(File, iTabCount, "SphereDataCount      %d\n",
             c_rSphereDataVector.size());

  for (DWORD i = 0; i < c_rSphereDataVector.size(); ++i) {
    const TSphereData &c_rAttr = c_rSphereDataVector[i].GetAttribute();

    PrintfTabs(File, iTabCount, "Group SphereData%02d\n", i);

    PrintfTabs(File, iTabCount, "{\n");

    PrintfTabs(File, iTabCount + 1, "Radius           %f\n", c_rAttr.fRadius);

    PrintfTabs(File, iTabCount + 1, "Position         %f %f %f\n",
               c_rAttr.v3Position.x, c_rAttr.v3Position.y,
               c_rAttr.v3Position.z);

    PrintfTabs(File, iTabCount, "}\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Attaching Data
void NRaceData::SaveAttachingData(
    FILE *File, int iTabCount,
    const TAttachingDataVector &c_rAttachingDataVector) {
  DWORD dwAttachingDataCount = c_rAttachingDataVector.size();

  PrintfTabs(File, iTabCount, "AttachingDataCount       %d\n",
             dwAttachingDataCount);
  PrintfTabs(File, iTabCount, "\n");

  for (DWORD i = 0; i < dwAttachingDataCount; ++i) {
    const NRaceData::TAttachingData &c_rAttachingData =
        c_rAttachingDataVector[i];

    PrintfTabs(File, iTabCount, "Group AttachingData%02d\n", i);
    PrintfTabs(File, iTabCount, "{\n", i);

    PrintfTabs(File, iTabCount + 1, "AttachingDataType    %d\n",
               c_rAttachingData.dwType);
    PrintfTabs(File, iTabCount + 1, "\n", i);

    PrintfTabs(File, iTabCount + 1, "isAttaching          %d\n",
               c_rAttachingData.isAttaching);
    PrintfTabs(File, iTabCount + 1, "AttachingModelIndex  %d\n",
               c_rAttachingData.dwAttachingModelIndex);
    PrintfTabs(File, iTabCount + 1, "AttachingBoneName    \"%s\"\n",
               c_rAttachingData.strAttachingBoneName.c_str());
    PrintfTabs(File, iTabCount + 1, "\n");

    switch (c_rAttachingData.dwType) {
    case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
      NRaceData::SaveCollisionData(File, iTabCount + 1,
                                   *c_rAttachingData.pCollisionData);
      break;
    case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
      NRaceData::SaveEffectData(File, iTabCount + 1,
                                *c_rAttachingData.pEffectData);
      break;
    case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
      NRaceData::SaveObjectData(File, iTabCount + 1,
                                *c_rAttachingData.pObjectData);
      break;
    }

    PrintfTabs(File, iTabCount, "}\n");

    if (i != dwAttachingDataCount - 1)
      PrintfTabs(File, iTabCount, "\n");
  }
}

BOOL NRaceData::LoadAttachingData(CTextFileLoader &rTextFileLoader,
                                  TAttachingDataVector *pAttachingDataVector) {
  DWORD dwDataCount;
  if (!rTextFileLoader.GetTokenDoubleWord("attachingdatacount", &dwDataCount))
    return false;

  pAttachingDataVector->clear();
  pAttachingDataVector->resize(dwDataCount);

  for (DWORD i = 0; i < dwDataCount; ++i) {
    NRaceData::TAttachingData &rAttachingData = pAttachingDataVector->at(i);
    if (!rTextFileLoader.SetChildNode("attachingdata", i))
      return false;

    if (!rTextFileLoader.GetTokenDoubleWord("attachingdatatype",
                                            &rAttachingData.dwType))
      return false;

    if (!rTextFileLoader.GetTokenInteger("isattaching",
                                         &rAttachingData.isAttaching))
      return false;
    if (!rTextFileLoader.GetTokenDoubleWord(
            "attachingmodelindex", &rAttachingData.dwAttachingModelIndex))
      return false;
    if (!rTextFileLoader.GetTokenString("attachingbonename",
                                        &rAttachingData.strAttachingBoneName))
      return false;

    switch (rAttachingData.dwType) {
    case NRaceData::ATTACHING_DATA_TYPE_COLLISION_DATA:
      rAttachingData.pCollisionData = g_CollisionDataPool.Alloc();
      if (!NRaceData::LoadCollisionData(rTextFileLoader,
                                        rAttachingData.pCollisionData))
        return false;
      break;
    case NRaceData::ATTACHING_DATA_TYPE_EFFECT:
      rAttachingData.pEffectData = g_EffectDataPool.Alloc();
      if (!NRaceData::LoadEffectData(rTextFileLoader,
                                     rAttachingData.pEffectData))
        return false;
      break;
    case NRaceData::ATTACHING_DATA_TYPE_OBJECT:
      rAttachingData.pObjectData = g_ObjectDataPool.Alloc();
      if (!NRaceData::LoadObjectData(rTextFileLoader,
                                     rAttachingData.pObjectData))
        return false;
      break;
    }

    rTextFileLoader.SetParentNode();
  }
  return TRUE;
}
