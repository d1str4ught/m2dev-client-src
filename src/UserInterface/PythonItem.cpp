#include "stdafx.h"
#include "EterLib/GrpMath.h"
#include "Gamelib/ItemManager.h"
#include "EffectLib/EffectManager.h"
#include "PythonBackground.h"

#include "pythonitem.h"
#include "PythonTextTail.h"

const float c_fDropStartHeight = 100.0f;
const float c_fDropTime = 0.5f;

std::string
    CPythonItem::TGroundItemInstance::ms_astDropSoundFileName[DROPSOUND_NUM];

void CPythonItem::GetInfo(std::string *pstInfo) {
  char szInfo[256];
  sprintf(szInfo, "Item: Inst %zd, Pool %zd", m_GroundItemInstanceMap.size(),
          m_GroundItemInstancePool.GetCapacity());

  pstInfo->append(szInfo);
}

void CPythonItem::TGroundItemInstance::Clear() {
  stOwnership = "";
  ThingInstance.Clear();
  CEffectManager::Instance().DestroyEffectInstance(dwEffectInstanceIndex);
}

void CPythonItem::TGroundItemInstance::__PlayDropSound(
    DWORD eItemType, const D3DXVECTOR3 &c_rv3Pos) {
  if (eItemType >= DROPSOUND_NUM)
    return;

  SoundEngine::Instance().PlaySound3D(
      ms_astDropSoundFileName[eItemType].c_str(), c_rv3Pos.x, c_rv3Pos.y,
      c_rv3Pos.z);
}

bool CPythonItem::TGroundItemInstance::Update() {
  if (bAnimEnded)
    return false;
  if (dwEndTime < CTimer::Instance().GetCurrentMillisecond()) {
    ThingInstance.SetRotationQuaternion(qEnd);

    /*D3DXVECTOR3 v3Adjust = -v3Center;
    D3DXMATRIX mat;
    D3DXMatrixRotationYawPitchRoll(&mat,
    D3DXToRadian(rEnd.y),
    D3DXToRadian(rEnd.x),
    D3DXToRadian(rEnd.z));
    D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);*/

    D3DXQUATERNION qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
    D3DXQUATERNION qc;
    D3DXQuaternionConjugate(&qc, &qEnd);
    D3DXQuaternionMultiply(&qAdjust, &qAdjust, &qEnd);
    D3DXQuaternionMultiply(&qAdjust, &qc, &qAdjust);

    ThingInstance.SetPosition(v3EndPosition.x + qAdjust.x,
                              v3EndPosition.y + qAdjust.y,
                              v3EndPosition.z + qAdjust.z);
    // ThingInstance.Update();
    bAnimEnded = true;

    __PlayDropSound(eDropSoundType, v3EndPosition);
  } else {
    DWORD time = CTimer::Instance().GetCurrentMillisecond() - dwStartTime;
    DWORD etime = dwEndTime - CTimer::Instance().GetCurrentMillisecond();
    float rate = time * 1.0f / (dwEndTime - dwStartTime);

    D3DXVECTOR3 v3NewPosition =
        v3EndPosition; // = rate*(v3EndPosition - v3StartPosition) +
                       // v3StartPosition;
    v3NewPosition.z +=
        100 - 100 * rate * (3 * rate - 2); //-100*(rate-1)*(3*rate+2);

    D3DXQUATERNION q;
    D3DXQuaternionRotationAxis(&q, &v3RotationAxis,
                               etime * 0.03f * (-1 + rate * (3 * rate - 2)));
    // ThingInstance.SetRotation(rEnd.y + etime*rStart.y, rEnd.x +
    // etime*rStart.x, rEnd.z + etime*rStart.z);
    D3DXQuaternionMultiply(&q, &qEnd, &q);

    ThingInstance.SetRotationQuaternion(q);
    D3DXQUATERNION qAdjust(-v3Center.x, -v3Center.y, -v3Center.z, 0.0f);
    D3DXQUATERNION qc;
    D3DXQuaternionConjugate(&qc, &q);
    D3DXQuaternionMultiply(&qAdjust, &qAdjust, &q);
    D3DXQuaternionMultiply(&qAdjust, &qc, &qAdjust);

    ThingInstance.SetPosition(v3NewPosition.x + qAdjust.x,
                              v3NewPosition.y + qAdjust.y,
                              v3NewPosition.z + qAdjust.z);

    /*D3DXVECTOR3 v3Adjust = -v3Center;
    D3DXMATRIX mat;
    D3DXMatrixRotationYawPitchRoll(&mat,
    D3DXToRadian(rEnd.y + etime*rStart.y),
    D3DXToRadian(rEnd.x + etime*rStart.x),
    D3DXToRadian(rEnd.z + etime*rStart.z));

    D3DXVec3TransformCoord(&v3Adjust,&v3Adjust,&mat);
    //Tracef("%f %f %f\n",v3Adjust.x,v3Adjust.y,v3Adjust.z);
    v3NewPosition += v3Adjust;
    ThingInstance.SetPosition(v3NewPosition.x, v3NewPosition.y,
    v3NewPosition.z);*/
  }
  ThingInstance.Transform();
  ThingInstance.Deform();
  return !bAnimEnded;
}

void CPythonItem::Update(const POINT &c_rkPtMouse) {
  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
  for (; itor != m_GroundItemInstanceMap.end(); ++itor) {
    itor->second->Update();
  }

  m_dwPickedItemID = __Pick(c_rkPtMouse);
}

void CPythonItem::Render() {
  CPythonGraphic::Instance().SetDiffuseOperation();
  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
  for (; itor != m_GroundItemInstanceMap.end(); ++itor) {
    CGraphicThingInstance &rInstance = itor->second->ThingInstance;
    // rInstance.Update();
    rInstance.Render();
    rInstance.BlendRender();
  }
}

void CPythonItem::SetUseSoundFileName(DWORD eItemType,
                                      const std::string &c_rstFileName) {
  if (eItemType >= USESOUND_NUM)
    return;

  // Tracenf("SetUseSoundFile %d : %s", eItemType, c_rstFileName.c_str());

  m_astUseSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::SetDropSoundFileName(DWORD eItemType,
                                       const std::string &c_rstFileName) {
  if (eItemType >= DROPSOUND_NUM)
    return;

  Tracenf("SetDropSoundFile %d : %s", eItemType, c_rstFileName.c_str());

  SGroundItemInstance::ms_astDropSoundFileName[eItemType] = c_rstFileName;
}

void CPythonItem::PlayUseSound(DWORD dwItemID) {
  // CItemManager& rkItemMgr=CItemManager::Instance();

  CItemData *pkItemData;
  if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
    return;

  DWORD eItemType = __GetUseSoundType(*pkItemData);
  if (eItemType == USESOUND_NONE)
    return;
  if (eItemType >= USESOUND_NUM)
    return;

  SoundEngine::Instance().PlaySound2D(m_astUseSoundFileName[eItemType].c_str());
}

void CPythonItem::PlayDropSound(DWORD dwItemID) {
  // CItemManager& rkItemMgr=CItemManager::Instance();

  CItemData *pkItemData;
  if (!CItemManager::Instance().GetItemDataPointer(dwItemID, &pkItemData))
    return;

  DWORD eItemType = __GetDropSoundType(*pkItemData);
  if (eItemType >= DROPSOUND_NUM)
    return;

  SoundEngine::Instance().PlaySound2D(
      SGroundItemInstance::ms_astDropSoundFileName[eItemType].c_str());
}

void CPythonItem::PlayUsePotionSound() {
  SoundEngine::Instance().PlaySound2D(
      m_astUseSoundFileName[USESOUND_POTION].c_str());
}

DWORD CPythonItem::__GetDropSoundType(const CItemData &c_rkItemData) {
  switch (c_rkItemData.GetType()) {
  case CItemData::ITEM_TYPE_WEAPON:
    switch (c_rkItemData.GetWeaponType()) {
    case CItemData::WEAPON_BOW:
      return DROPSOUND_BOW;
      break;
    case CItemData::WEAPON_ARROW:
      return DROPSOUND_DEFAULT;
      break;
    default:
      return DROPSOUND_WEAPON;
      break;
    }
    break;
  case CItemData::ITEM_TYPE_ARMOR:
    switch (c_rkItemData.GetSubType()) {
    case CItemData::ARMOR_NECK:
    case CItemData::ARMOR_EAR:
      return DROPSOUND_ACCESSORY;
      break;
    case CItemData::ARMOR_BODY:
      return DROPSOUND_ARMOR;
    default:
      return DROPSOUND_DEFAULT;
      break;
    }
    break;
  default:
    return DROPSOUND_DEFAULT;
    break;
  }

  return DROPSOUND_DEFAULT;
}

DWORD CPythonItem::__GetUseSoundType(const CItemData &c_rkItemData) {
  switch (c_rkItemData.GetType()) {
  case CItemData::ITEM_TYPE_WEAPON:
    switch (c_rkItemData.GetWeaponType()) {
    case CItemData::WEAPON_BOW:
      return USESOUND_BOW;
      break;
    case CItemData::WEAPON_ARROW:
      return USESOUND_DEFAULT;
      break;
    default:
      return USESOUND_WEAPON;
      break;
    }
    break;
  case CItemData::ITEM_TYPE_ARMOR:
    switch (c_rkItemData.GetSubType()) {
    case CItemData::ARMOR_NECK:
    case CItemData::ARMOR_EAR:
      return USESOUND_ACCESSORY;
      break;
    case CItemData::ARMOR_BODY:
      return USESOUND_ARMOR;
    default:
      return USESOUND_DEFAULT;
      break;
    }
    break;
  case CItemData::ITEM_TYPE_USE:
    switch (c_rkItemData.GetSubType()) {
    case CItemData::USE_ABILITY_UP:
      return USESOUND_POTION;
      break;
    case CItemData::USE_POTION:
      return USESOUND_NONE;
      break;
    case CItemData::USE_TALISMAN:
      return USESOUND_PORTAL;
      break;
    default:
      return USESOUND_DEFAULT;
      break;
    }
    break;
  default:
    return USESOUND_DEFAULT;
    break;
  }

  return USESOUND_DEFAULT;
}

void CPythonItem::CreateItem(DWORD dwVirtualID, DWORD dwVirtualNumber, float x,
                             float y, float z, bool bDrop) {
  // CItemManager& rkItemMgr=CItemManager::Instance();

  CItemData *pItemData;
  if (!CItemManager::Instance().GetItemDataPointer(dwVirtualNumber, &pItemData))
    return;

  CGraphicThing *pItemModel = pItemData->GetDropModelThing();

  TGroundItemInstance *pGroundItemInstance = m_GroundItemInstancePool.Alloc();
  pGroundItemInstance->dwVirtualNumber = dwVirtualNumber;

  bool bStabGround = false;

  if (bDrop) {
    z = CPythonBackground::Instance().GetHeight(x, y) + 10.0f;

    if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON &&
        (pItemData->GetWeaponType() == CItemData::WEAPON_SWORD ||
         pItemData->GetWeaponType() == CItemData::WEAPON_ARROW))
      bStabGround = true;

    bStabGround = false;
    pGroundItemInstance->bAnimEnded = false;
  } else {
    pGroundItemInstance->bAnimEnded = true;
  }

  {
    // attaching effect
    CEffectManager &rem = CEffectManager::Instance();
    pGroundItemInstance->dwEffectInstanceIndex = rem.CreateEffect(
        m_dwDropItemEffectID, D3DXVECTOR3(x, -y, z), D3DXVECTOR3(0, 0, 0));

    pGroundItemInstance->eDropSoundType = __GetDropSoundType(*pItemData);
  }

  D3DXVECTOR3 normal;
  if (!CPythonBackground::Instance().GetNormal(int(x), int(y), &normal))
    normal = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

  pGroundItemInstance->ThingInstance.Clear();
  pGroundItemInstance->ThingInstance.ReserveModelThing(1);
  pGroundItemInstance->ThingInstance.ReserveModelInstance(1);
  pGroundItemInstance->ThingInstance.RegisterModelThing(0, pItemModel);
  pGroundItemInstance->ThingInstance.SetModelInstance(0, 0, 0);
  if (bDrop) {
    pGroundItemInstance->v3EndPosition = D3DXVECTOR3(x, -y, z);
    pGroundItemInstance->ThingInstance.SetPosition(0, 0, 0);
  } else
    pGroundItemInstance->ThingInstance.SetPosition(x, -y, z);

  pGroundItemInstance->ThingInstance.Update();
  pGroundItemInstance->ThingInstance.Transform();
  pGroundItemInstance->ThingInstance.Deform();

  if (bDrop) {
    D3DXVECTOR3 vMin, vMax;
    pGroundItemInstance->ThingInstance.GetBoundBox(&vMin, &vMax);
    pGroundItemInstance->v3Center = (vMin + vMax) * 0.5f;

    std::pair<float, int> f[3] = {std::make_pair(vMax.x - vMin.x, 0),
                                  std::make_pair(vMax.y - vMin.y, 1),
                                  std::make_pair(vMax.z - vMin.z, 2)};

    std::sort(f, f + 3);

    // int no_rotation_axis=-1;

    D3DXVECTOR3 rEnd;

    if (/*f[1].first-f[0].first < (f[2].first-f[0].first)*0.30f*/ bStabGround) {
      // 뾰족
      if (f[2].second == 0) // axis x
      {
        rEnd.y = 90.0f + frandom(-15.0f, 15.0f);
        rEnd.x = frandom(0.0f, 360.0f);
        rEnd.z = frandom(-15.0f, 15.0f);
      } else if (f[2].second == 1) // axis y
      {
        rEnd.y = frandom(0.0f, 360.0f);
        rEnd.x = frandom(-15.0f, 15.0f);
        rEnd.z = 180.0f + frandom(-15.0f, 15.0f);
      } else // axis z
      {
        rEnd.y = 180.0f + frandom(-15.0f, 15.0f);
        rEnd.x = 0.0f + frandom(-15.0f, 15.0f);
        rEnd.z = frandom(0.0f, 360.0f);
      }
    } else {
      // 넓적
      // 땅의 노말의 영향을 받을 것
      if (f[0].second == 0) {
        // y,z = by normal
        pGroundItemInstance->qEnd = RotationArc(
            D3DXVECTOR3(((float)(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f),
                        0 + frandom(-0.1f, 0.1f), 0 + frandom(-0.1f, 0.1f)),
            D3DXVECTOR3(0, 0, 1) /*normal*/);
      } else if (f[0].second == 1) {
        pGroundItemInstance->qEnd = RotationArc(
            D3DXVECTOR3(0 + frandom(-0.1f, 0.1f),
                        ((float)(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f),
                        0 + frandom(-0.1f, 0.1f)),
            D3DXVECTOR3(0, 0, 1) /*normal*/);
      } else {
        pGroundItemInstance->qEnd = RotationArc(
            D3DXVECTOR3(0 + frandom(-0.1f, 0.1f), 0 + frandom(-0.1f, 0.1f),
                        ((float)(random() % 2)) * 2 - 1 + frandom(-0.1f, 0.1f)),
            D3DXVECTOR3(0, 0, 1) /*normal*/);
      }
    }
    // D3DXQuaternionRotationYawPitchRoll(&pGroundItemInstance->qEnd, rEnd.y,
    // rEnd.x, rEnd.z );
    float rot = frandom(0, 2 * 3.1415926535f);
    D3DXQUATERNION q(0, 0, cosf(rot), sinf(rot));
    D3DXQuaternionMultiply(&pGroundItemInstance->qEnd,
                           &pGroundItemInstance->qEnd, &q);
    q = RotationArc(D3DXVECTOR3(0, 0, 1), normal);
    D3DXQuaternionMultiply(&pGroundItemInstance->qEnd,
                           &pGroundItemInstance->qEnd, &q);

    pGroundItemInstance->dwStartTime =
        CTimer::Instance().GetCurrentMillisecond();
    pGroundItemInstance->dwEndTime = pGroundItemInstance->dwStartTime + 300;
    pGroundItemInstance->v3RotationAxis.x =
        sinf(rot + 0); // frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
    pGroundItemInstance->v3RotationAxis.y =
        cosf(rot + 0); // frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);
    pGroundItemInstance->v3RotationAxis.z =
        0; // frandom(0.4f,0.7f) * (2*(int)(random()%2) - 1);

    /*
    switch (no_rotation_axis)
    {
    case 0:
            pGroundItemInstance->rStart.x = 0;
            break;
    case 1:
            pGroundItemInstance->rStart.y = 0;
            break;
    case 2:
            pGroundItemInstance->rStart.z = 0;
            break;
    }*/

    D3DXVECTOR3 v3Adjust = -pGroundItemInstance->v3Center;
    D3DXMATRIX mat;
    D3DXMatrixRotationQuaternion(&mat, &pGroundItemInstance->qEnd);
    /*D3DXMatrixRotationYawPitchRoll(&mat,
            D3DXToRadian(pGroundItemInstance->rEnd.y),
            D3DXToRadian(pGroundItemInstance->rEnd.x),
            D3DXToRadian(pGroundItemInstance->rEnd.z));*/

    D3DXVec3TransformCoord(&v3Adjust, &v3Adjust, &mat);
    // Tracef("%f %f %f\n",v3Adjust.x,v3Adjust.y,v3Adjust.z);
    // pGroundItemInstance->v3EndPosition += v3Adjust;
    // pGroundItemInstance->rEnd.z += pGroundItemInstance->v3Center.z;
  }

  pGroundItemInstance->ThingInstance.Show();

  m_GroundItemInstanceMap.insert(
      TGroundItemInstanceMap::value_type(dwVirtualID, pGroundItemInstance));

  CPythonTextTail &rkTextTail = CPythonTextTail::Instance();
  rkTextTail.RegisterItemTextTail(dwVirtualID, pItemData->GetName(),
                                  &pGroundItemInstance->ThingInstance);
}

void CPythonItem::SetOwnership(DWORD dwVID, const char *c_pszName) {
  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

  if (m_GroundItemInstanceMap.end() == itor)
    return;

  TGroundItemInstance *pGroundItemInstance = itor->second;
  pGroundItemInstance->stOwnership.assign(c_pszName);

  CPythonTextTail &rkTextTail = CPythonTextTail::Instance();
  rkTextTail.SetItemTextTailOwner(dwVID, c_pszName);
}

bool CPythonItem::GetOwnership(DWORD dwVID, const char **c_pszName) {
  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

  if (m_GroundItemInstanceMap.end() == itor)
    return false;

  TGroundItemInstance *pGroundItemInstance = itor->second;
  *c_pszName = pGroundItemInstance->stOwnership.c_str();

  return true;
}

void CPythonItem::DeleteAllItems() {
  CPythonTextTail &rkTextTail = CPythonTextTail::Instance();

  TGroundItemInstanceMap::iterator i;
  for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end();
       ++i) {
    TGroundItemInstance *pGroundItemInst = i->second;
    rkTextTail.DeleteItemTextTail(i->first);
    pGroundItemInst->Clear();
    m_GroundItemInstancePool.Free(pGroundItemInst);
  }
  m_GroundItemInstanceMap.clear();
}

void CPythonItem::DeleteItem(DWORD dwVirtualID) {
  TGroundItemInstanceMap::iterator itor =
      m_GroundItemInstanceMap.find(dwVirtualID);
  if (m_GroundItemInstanceMap.end() == itor)
    return;

  TGroundItemInstance *pGroundItemInstance = itor->second;
  pGroundItemInstance->Clear();
  m_GroundItemInstancePool.Free(pGroundItemInstance);
  m_GroundItemInstanceMap.erase(itor);

  // Text Tail
  CPythonTextTail::Instance().DeleteItemTextTail(dwVirtualID);
}

bool CPythonItem::GetCloseMoney(const TPixelPosition &c_rPixelPosition,
                                DWORD *pdwItemID, DWORD dwDistance) {
  DWORD dwCloseItemID = 0;
  DWORD dwCloseItemDistance = 1000 * 1000;

  TGroundItemInstanceMap::iterator i;
  for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end();
       ++i) {
    TGroundItemInstance *pInstance = i->second;

    if (pInstance->dwVirtualNumber != VNUM_MONEY)
      continue;

    DWORD dwxDistance = DWORD(c_rPixelPosition.x - pInstance->v3EndPosition.x);
    DWORD dwyDistance =
        DWORD(c_rPixelPosition.y - (-pInstance->v3EndPosition.y));
    DWORD dwDistance =
        DWORD(dwxDistance * dwxDistance + dwyDistance * dwyDistance);

    if (dwxDistance * dwxDistance + dwyDistance * dwyDistance <
        dwCloseItemDistance) {
      dwCloseItemID = i->first;
      dwCloseItemDistance = dwDistance;
    }
  }

  if (dwCloseItemDistance > float(dwDistance) * float(dwDistance))
    return false;

  *pdwItemID = dwCloseItemID;

  return true;
}

// bool CPythonItem::GetCloseItem(const TPixelPosition & c_rPixelPosition, DWORD
// * pdwItemID, DWORD dwDistance)
// {
// DWORD dwCloseItemID = 0;
// DWORD dwCloseItemDistance = 1000 * 1000;

// TGroundItemInstanceMap::iterator i;
// for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end();
// ++i)
// {
// TGroundItemInstance * pInstance = i->second;

// DWORD dwxDistance = DWORD(c_rPixelPosition.x-pInstance->v3EndPosition.x);
// DWORD dwyDistance = DWORD(c_rPixelPosition.y-(-pInstance->v3EndPosition.y));
// DWORD dwDistance = DWORD(dwxDistance*dwxDistance + dwyDistance*dwyDistance);

// if (dwxDistance*dwxDistance + dwyDistance*dwyDistance < dwCloseItemDistance)
// {
// dwCloseItemID = i->first;
// dwCloseItemDistance = dwDistance;
// }
// }

// if (dwCloseItemDistance>float(dwDistance)*float(dwDistance))
// return false;

// *pdwItemID=dwCloseItemID;

// return true;
// }

// Fix for picking up items
// https://metin2.dev/board/topic/24691-fix-pickup-distances-bug/
int DISTANCE_APPROX(int dx, int dy) {
  int min, max;

  if (dx < 0)
    dx = -dx;

  if (dy < 0)
    dy = -dy;

  if (dx < dy) {
    min = dx;
    max = dy;
  } else {
    min = dy;
    max = dx;
  }

  // coefficients equivalent to ( 123/128 * max ) and ( 51/128 * min )
  return (((max << 8) + (max << 3) - (max << 4) - (max << 1) + (min << 7) -
           (min << 5) + (min << 3) - (min << 1)) >>
          8);
}

bool CPythonItem::GetCloseItem(const TPixelPosition &c_rPixelPosition,
                               DWORD *pdwItemID, DWORD dwDistance) {
  DWORD dwCloseItemID = 0;
  DWORD dwCloseItemDistance = 0;
  int aproMin = 0;

  TGroundItemInstanceMap::iterator i;
  for (i = m_GroundItemInstanceMap.begin(); i != m_GroundItemInstanceMap.end();
       ++i) {
    TGroundItemInstance *pInstance = i->second;

    int iDist = DISTANCE_APPROX(
        (int)c_rPixelPosition.x - (int)pInstance->v3EndPosition.x,
        (int)c_rPixelPosition.y - (-(int)pInstance->v3EndPosition.y));
    if (aproMin == 0)
      aproMin = iDist;
    if (aproMin >= iDist) {
      aproMin = iDist;
      dwCloseItemID = i->first;
      dwCloseItemDistance = iDist;
    }
  }

  if (dwCloseItemDistance > 300)
    return false;

  *pdwItemID = dwCloseItemID;

  return true;
}

BOOL CPythonItem::GetGroundItemPosition(DWORD dwVirtualID,
                                        TPixelPosition *pPosition) {
  TGroundItemInstanceMap::iterator itor =
      m_GroundItemInstanceMap.find(dwVirtualID);
  if (m_GroundItemInstanceMap.end() == itor)
    return FALSE;

  TGroundItemInstance *pInstance = itor->second;

  const D3DXVECTOR3 &rkD3DVct3 = pInstance->ThingInstance.GetPosition();

  pPosition->x = +rkD3DVct3.x;
  pPosition->y = -rkD3DVct3.y;
  pPosition->z = +rkD3DVct3.z;

  return TRUE;
}

DWORD CPythonItem::__Pick(const POINT &c_rkPtMouse) {
  float fu, fv, ft;

  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.begin();
  for (; itor != m_GroundItemInstanceMap.end(); ++itor) {
    TGroundItemInstance *pInstance = itor->second;

    if (pInstance->ThingInstance.Intersect(&fu, &fv, &ft)) {
      return itor->first;
    }
  }

  CPythonTextTail &rkTextTailMgr = CPythonTextTail::Instance();
  return rkTextTailMgr.Pick(c_rkPtMouse.x, c_rkPtMouse.y);
}

bool CPythonItem::GetPickedItemID(DWORD *pdwPickedItemID) {
  if (INVALID_ID == m_dwPickedItemID)
    return false;

  *pdwPickedItemID = m_dwPickedItemID;
  return true;
}

DWORD CPythonItem::GetVirtualNumberOfGroundItem(DWORD dwVID) {
  TGroundItemInstanceMap::iterator itor = m_GroundItemInstanceMap.find(dwVID);

  if (itor == m_GroundItemInstanceMap.end())
    return 0;
  else
    return itor->second->dwVirtualNumber;
}

void CPythonItem::BuildNoGradeNameData(int iType) {
  /*
  CMapIterator<std::string, CItemData *> itor =
  CItemManager::Instance().GetItemNameMapIterator();

  m_NoGradeNameItemData.clear();
  m_NoGradeNameItemData.reserve(1024);

  while (++itor)
  {
          CItemData * pItemData = *itor;
          if (iType == pItemData->GetType())
                  m_NoGradeNameItemData.push_back(pItemData);
  }
  */
}

DWORD CPythonItem::GetNoGradeNameDataCount() {
  return m_NoGradeNameItemData.size();
}

CItemData *CPythonItem::GetNoGradeNameDataPtr(DWORD dwIndex) {
  if (dwIndex >= m_NoGradeNameItemData.size())
    return NULL;

  return m_NoGradeNameItemData[dwIndex];
}

void CPythonItem::Destroy() {
  DeleteAllItems();
  m_GroundItemInstancePool.Clear();
}

void CPythonItem::Create() {
  CEffectManager::Instance().RegisterEffect2(
      "d:/ymir work/effect/etc/dropitem/dropitem.mse", &m_dwDropItemEffectID);
}

CPythonItem::CPythonItem() { m_dwPickedItemID = INVALID_ID; }

CPythonItem::~CPythonItem() { assert(m_GroundItemInstanceMap.empty()); }