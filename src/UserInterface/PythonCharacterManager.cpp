#include "stdafx.h"
#include "pythoncharactermanager.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "AbstractPlayer.h"
#include "packet.h"

#include "EterLib/Camera.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Frame Process

int CHAR_STAGE_VIEW_BOUND = 200 * 100;

struct FCharacterManagerCharacterInstanceUpdate {
  inline void operator()(const std::pair<DWORD, CInstanceBase *> &cr_Pair) {
    cr_Pair.second->Update();
  }
};

void CPythonCharacterManager::AdjustCollisionWithOtherObjects(
    CActorInstance *pInst) {
  if (!pInst->IsPC())
    return;

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  for (CPythonCharacterManager::CharacterIterator i =
           rkChrMgr.CharacterInstanceBegin();
       i != rkChrMgr.CharacterInstanceEnd(); ++i) {
    CInstanceBase *pkInstEach = *i;
    CActorInstance *rkActorEach = pkInstEach->GetGraphicThingInstancePtr();

    if (rkActorEach == pInst)
      continue;

    if (rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy())
      continue;

    if (pInst->TestPhysicsBlendingCollision(*rkActorEach)) {
      // NOTE : 일단 기존위치로 원복
      // TODO : 향후 조금더 잘 처리한다면 physic movement거리를 steping해서
      // iteration처리해야 함.
      TPixelPosition curPos;
      pInst->GetPixelPosition(&curPos);
      pInst->SetBlendingPosition(curPos);
      // Tracef("!!!!!! Collision Adjusted\n");
      break;
    }
  }
}

void CPythonCharacterManager::EnableSortRendering(bool isEnable) {}

void CPythonCharacterManager::InsertPVPKey(DWORD dwVIDSrc, DWORD dwVIDDst) {
  CInstanceBase::InsertPVPKey(dwVIDSrc, dwVIDDst);

  CInstanceBase *pkInstSrc = GetInstancePtr(dwVIDSrc);
  if (pkInstSrc)
    pkInstSrc->RefreshTextTail();

  CInstanceBase *pkInstDst = GetInstancePtr(dwVIDDst);
  if (pkInstDst)
    pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::RemovePVPKey(DWORD dwVIDSrc, DWORD dwVIDDst) {
  CInstanceBase::RemovePVPKey(dwVIDSrc, dwVIDDst);

  CInstanceBase *pkInstSrc = GetInstancePtr(dwVIDSrc);
  if (pkInstSrc)
    pkInstSrc->RefreshTextTail();

  CInstanceBase *pkInstDst = GetInstancePtr(dwVIDDst);
  if (pkInstDst)
    pkInstDst->RefreshTextTail();
}

void CPythonCharacterManager::ChangeGVG(DWORD dwSrcGuildID,
                                        DWORD dwDstGuildID) {
  TCharacterInstanceMap::iterator itor;
  for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); itor++) {
    CInstanceBase *pInstance = itor->second;

    DWORD dwInstanceGuildID = pInstance->GetGuildID();
    if (dwSrcGuildID == dwInstanceGuildID ||
        dwDstGuildID == dwInstanceGuildID) {
      pInstance->RefreshTextTail();
    }
  }
}

void CPythonCharacterManager::ClearMainInstance() { m_pkInstMain = NULL; }

bool CPythonCharacterManager::SetMainInstance(DWORD dwVID) {
  m_pkInstMain = GetInstancePtr(dwVID);

  if (!m_pkInstMain)
    return false;

  return true;
}

CInstanceBase *CPythonCharacterManager::GetMainInstancePtr() {
  return m_pkInstMain;
}

void CPythonCharacterManager::GetInfo(std::string *pstInfo) {
  pstInfo->append("Actor: ");

  CInstanceBase::GetInfo(pstInfo);

  char szInfo[256];
  sprintf(szInfo, "Container - Live %zd, Dead %zd", m_kAliveInstMap.size(),
          m_kDeadInstList.size());
  pstInfo->append(szInfo);
}

bool CPythonCharacterManager::IsCacheMode() {
  static bool s_isOldCacheMode = false;

  bool isCacheMode = s_isOldCacheMode;
  if (s_isOldCacheMode) {
    if (m_kAliveInstMap.size() < 30)
      isCacheMode = false;
  } else {
    if (m_kAliveInstMap.size() > 40)
      isCacheMode = true;
  }
  s_isOldCacheMode = isCacheMode;

  return isCacheMode;
}

void CPythonCharacterManager::Update() {
#ifdef __PERFORMANCE_CHECKER__
  DWORD t1 = timeGetTime();
#endif
  CInstanceBase::ResetPerformanceCounter();

  CInstanceBase *pkInstMain = GetMainInstancePtr();
#ifdef __PERFORMANCE_CHECKER__
  DWORD t2 = timeGetTime();
#endif
  DWORD dwDeadInstCount = 0;
  DWORD dwForceVisibleInstCount = 0;

  TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin();
  while (m_kAliveInstMap.end() != i) {
    TCharacterInstanceMap::iterator c = i++;

    CInstanceBase *pkInstEach = c->second;
    pkInstEach->Update();

    if (pkInstMain) {
      if (pkInstEach->IsForceVisible()) [[unlikely]] {
        dwForceVisibleInstCount++;
        continue;
      }

      // Optimized: Use squared distance to avoid sqrt
      float fDistanceSquared =
          pkInstEach->NEW_GetDistanceFromDestInstanceSquared(*pkInstMain);
      const float fViewBoundSquared =
          (CHAR_STAGE_VIEW_BOUND + 10) * (CHAR_STAGE_VIEW_BOUND + 10);
      if (fDistanceSquared > fViewBoundSquared) [[unlikely]] {
        __DeleteBlendOutInstance(pkInstEach);
        m_kAliveInstMap.erase(c);
        dwDeadInstCount++;
      }
    }
  }
#ifdef __PERFORMANCE_CHECKER__
  DWORD t3 = timeGetTime();
#endif
  UpdateTransform();
#ifdef __PERFORMANCE_CHECKER__
  DWORD t4 = timeGetTime();
#endif

  UpdateDeleting();
#ifdef __PERFORMANCE_CHECKER__
  DWORD t5 = timeGetTime();
#endif

  __NEW_Pick();
#ifdef __PERFORMANCE_CHECKER__
  DWORD t6 = timeGetTime();
#endif

#ifdef __PERFORMANCE_CHECKER__
  {
    static FILE *fp = fopen("perf_chrmgr_update.txt", "w");

    if (t6 - t1 > 1) {
      fprintf(fp, "CU.Total %d (Time %d, Alive %d, Dead %d)\n", t6 - t1,
              ELTimer_GetMSec(), m_kAliveInstMap.size(),
              m_kDeadInstList.size());
      fprintf(fp, "CU.Counter %d\n", t2 - t1);
      fprintf(fp, "CU.ForEach %d\n", t3 - t2);
      fprintf(fp, "CU.Trans %d\n", t4 - t3);
      fprintf(fp, "CU.Del %d\n", t5 - t4);
      fprintf(fp, "CU.Pick %d\n", t6 - t5);
      fprintf(fp, "CU.AI %d\n", m_kAliveInstMap.size());
      fprintf(fp, "CU.DI %d\n", dwDeadInstCount);
      fprintf(fp, "CU.FVI %d\n", dwForceVisibleInstCount);
      fprintf(fp, "-------------------------------- \n");
      fflush(fp);
    }
  }
#endif
}

void CPythonCharacterManager::ShowPointEffect(DWORD ePoint, DWORD dwVID) {
  CInstanceBase *pkInstSel =
      (dwVID == 0xffffffff) ? GetMainInstancePtr() : GetInstancePtr(dwVID);

  if (!pkInstSel)
    return;

  switch (ePoint) {
  case POINT_LEVEL:
    pkInstSel->LevelUp();
    break;
  case POINT_LEVEL_STEP:
    pkInstSel->SkillUp();
    break;
  }
}

bool CPythonCharacterManager::RegisterPointEffect(DWORD ePoint,
                                                  const char *c_szFileName) {
  if (ePoint >= POINT_MAX_NUM)
    return false;

  CEffectManager &rkEftMgr = CEffectManager::Instance();
  rkEftMgr.RegisterEffect2(c_szFileName, &m_adwPointEffect[ePoint]);

  return true;
}

void CPythonCharacterManager::UpdateTransform() {
#ifdef __PERFORMANCE_CHECKER__
  DWORD t1 = timeGetTime();
  DWORD t2 = timeGetTime();
#endif

  CInstanceBase *pMainInstance = GetMainInstancePtr();
  if (pMainInstance) {
    CPythonBackground &rkBG = CPythonBackground::Instance();
    for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin();
         i != m_kAliveInstMap.end(); ++i) {
      CInstanceBase *pSrcInstance = i->second;

      pSrcInstance->CheckAdvancing();

      // 2004.08.02.myevan.IsAttacked 일 경우 죽었을때도 체크하므로,
      // 실질적으로 거리가 변경되는 IsPushing일때만 체크하도록 한다
      if (pSrcInstance->IsPushing())
        rkBG.CheckAdvancing(pSrcInstance);
    }
#ifdef __PERFORMANCE_CHECKER__
    t2 = timeGetTime();
#endif

#ifdef __MOVIE_MODE__
    if (!m_pkInstMain->IsMovieMode()) {
      rkBG.CheckAdvancing(m_pkInstMain);
    }
#else
    rkBG.CheckAdvancing(m_pkInstMain);
#endif
  }

#ifdef __PERFORMANCE_CHECKER__
  DWORD t3 = timeGetTime();
#endif

  {
    for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
         itor != m_kAliveInstMap.end(); ++itor) {
      CInstanceBase *pInstance = itor->second;
      pInstance->Transform();
    }
  }

#ifdef __PERFORMANCE_CHECKER__
  DWORD t4 = timeGetTime();
#endif

#ifdef __PERFORMANCE_CHECKER__
  {
    static FILE *fp = fopen("perf_chrmgr_updatetransform.txt", "w");

    if (t4 - t1 > 5) {
      fprintf(fp, "CUT.Total %d (Time %f, Alive %d, Dead %d)\n", t4 - t1,
              ELTimer_GetMSec() / 1000.0f, m_kAliveInstMap.size(),
              m_kDeadInstList.size());
      fprintf(fp, "CUT.ChkAdvInst %d\n", t2 - t1);
      fprintf(fp, "CUT.ChkAdvBG %d\n", t3 - t2);
      fprintf(fp, "CUT.Trans %d\n", t4 - t3);

      fprintf(fp, "-------------------------------- \n");
      fflush(fp);
    }

    fflush(fp);
  }
#endif
}
void CPythonCharacterManager::UpdateDeleting() {
  TCharacterInstanceList::iterator itor = m_kDeadInstList.begin();
  for (; itor != m_kDeadInstList.end();) {
    CInstanceBase *pInstance = *itor;

    if (pInstance->UpdateDeleting()) [[likely]] {
      ++itor;
    } else [[unlikely]] {
      CInstanceBase::Delete(pInstance);
      itor = m_kDeadInstList.erase(itor);
    }
  }
}

struct FCharacterManagerCharacterInstanceDeform {
  inline void operator()(const std::pair<DWORD, CInstanceBase *> &cr_Pair) {
    cr_Pair.second->Deform();
    // pInstance->Update();
  }
};
struct FCharacterManagerCharacterInstanceListDeform {
  inline void operator()(CInstanceBase *pInstance) { pInstance->Deform(); }
};

void CPythonCharacterManager::Deform() {
  std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(),
                FCharacterManagerCharacterInstanceDeform());
  std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(),
                FCharacterManagerCharacterInstanceListDeform());
}

bool CPythonCharacterManager::OLD_GetPickedInstanceVID(
    DWORD *pdwPickedActorID) {
  if (!m_pkInstPick)
    return false;

  *pdwPickedActorID = m_pkInstPick->GetVirtualID();
  return true;
}

CInstanceBase *CPythonCharacterManager::OLD_GetPickedInstancePtr() {
  return m_pkInstPick;
}

D3DXVECTOR2 &CPythonCharacterManager::OLD_GetPickedInstPosReference() {
  return m_v2PickedInstProjPos;
}

bool CPythonCharacterManager::IsRegisteredVID(DWORD dwVID) {
  if (m_kAliveInstMap.end() == m_kAliveInstMap.find(dwVID))
    return false;

  return true;
}

bool CPythonCharacterManager::IsAliveVID(DWORD dwVID) {
  return m_kAliveInstMap.find(dwVID) != m_kAliveInstMap.end();
}

bool CPythonCharacterManager::IsDeadVID(DWORD dwVID) {
  for (TCharacterInstanceList::iterator f = m_kDeadInstList.begin();
       f != m_kDeadInstList.end(); ++f) {
    if ((*f)->GetVirtualID() == dwVID)
      return true;
  }

  return false;
}

// let's sort character instances by their distance to the camera
// to avoid overdrawing
struct LessCharacterInstancePtrRenderOrder {
  D3DXVECTOR3 v3CameraPosition;
  bool operator()(CInstanceBase *pkLeft, CInstanceBase *pkRight) {
    D3DXVECTOR3 v3Left, v3Right;
    pkLeft->NEW_GetPixelPosition(&v3Left);
    pkRight->NEW_GetPixelPosition(&v3Right);

    v3Left.y *= -1;
    v3Right.y *= -1;

    D3DXVECTOR3 v3LeftDiff = v3Left - v3CameraPosition;
    D3DXVECTOR3 v3RightDiff = v3Right - v3CameraPosition;

    return D3DXVec3Dot(&v3LeftDiff, &v3LeftDiff) <
           D3DXVec3Dot(&v3RightDiff, &v3RightDiff);
  }
};

struct FCharacterManagerCharacterInstanceRender {
  inline void operator()(const std::pair<DWORD, CInstanceBase *> &cr_Pair) {
    cr_Pair.second->Render();
    cr_Pair.second->RenderTrace();
  }
};
struct FCharacterInstanceRender {
  inline void operator()(CInstanceBase *pInstance) { pInstance->Render(); }
};
struct FCharacterInstanceRenderTrace {
  inline void operator()(CInstanceBase *pInstance) { pInstance->RenderTrace(); }
};

void CPythonCharacterManager::__RenderSortedAliveActorList() {
  static std::vector<CInstanceBase *> s_kVct_pkInstAliveSort;
  s_kVct_pkInstAliveSort.clear();

  CCamera *pCamera = CCameraManager::instance().GetCurrentCamera();
  if (!pCamera) [[unlikely]]
    return;

  TCharacterInstanceMap &rkMap_pkInstAlive = m_kAliveInstMap;
  TCharacterInstanceMap::iterator i;
  for (i = rkMap_pkInstAlive.begin(); i != rkMap_pkInstAlive.end(); ++i)
    s_kVct_pkInstAliveSort.push_back(i->second);

  LessCharacterInstancePtrRenderOrder fSortFunc;
  fSortFunc.v3CameraPosition = pCamera->GetEye();

  std::sort(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(),
            fSortFunc);
  std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(),
                FCharacterInstanceRender());
  std::for_each(s_kVct_pkInstAliveSort.begin(), s_kVct_pkInstAliveSort.end(),
                FCharacterInstanceRenderTrace());
}

void CPythonCharacterManager::__RenderSortedDeadActorList() {
  static std::vector<CInstanceBase *> s_kVct_pkInstDeadSort;
  s_kVct_pkInstDeadSort.clear();

  CCamera *pCamera = CCameraManager::instance().GetCurrentCamera();
  if (!pCamera) [[unlikely]]
    return;

  TCharacterInstanceList &rkLst_pkInstDead = m_kDeadInstList;
  TCharacterInstanceList::iterator i;
  for (i = rkLst_pkInstDead.begin(); i != rkLst_pkInstDead.end(); ++i)
    s_kVct_pkInstDeadSort.push_back(*i);

  LessCharacterInstancePtrRenderOrder fSortFunc;
  fSortFunc.v3CameraPosition = pCamera->GetEye();

  std::sort(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(),
            fSortFunc);
  std::for_each(s_kVct_pkInstDeadSort.begin(), s_kVct_pkInstDeadSort.end(),
                FCharacterInstanceRender());
}

void CPythonCharacterManager::Render() {
  STATEMANAGER.SetTexture(0, NULL);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  __RenderSortedAliveActorList();
  __RenderSortedDeadActorList();

  CInstanceBase *pkPickedInst = OLD_GetPickedInstancePtr();
  if (pkPickedInst) {
    const D3DXVECTOR3 &c_rv3Position =
        pkPickedInst->GetGraphicThingInstanceRef().GetPosition();
    CPythonGraphic::Instance().ProjectPosition(
        c_rv3Position.x, c_rv3Position.y, c_rv3Position.z,
        &m_v2PickedInstProjPos.x, &m_v2PickedInstProjPos.y);
  }
}

void CPythonCharacterManager::RenderShadowMainInstance() {
  CInstanceBase *pkInstMain = GetMainInstancePtr();
  if (pkInstMain)
    pkInstMain->RenderToShadowMap();
}

struct FCharacterManagerCharacterInstanceRenderToShadowMap {
  inline void operator()(const std::pair<DWORD, CInstanceBase *> &cr_Pair) {
    cr_Pair.second->RenderToShadowMap();
  }
};

void CPythonCharacterManager::RenderShadowAllInstances() {
  std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(),
                FCharacterManagerCharacterInstanceRenderToShadowMap());
}

struct FCharacterManagerCharacterInstanceRenderCollision {
  inline void operator()(const std::pair<DWORD, CInstanceBase *> &cr_Pair) {
    cr_Pair.second->RenderCollision();
  }
};

void CPythonCharacterManager::RenderCollision() {
  std::for_each(m_kAliveInstMap.begin(), m_kAliveInstMap.end(),
                FCharacterManagerCharacterInstanceRenderCollision());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Managing Process

CInstanceBase *CPythonCharacterManager::CreateInstance(
    const CInstanceBase::SCreateData &c_rkCreateData) {
  CInstanceBase *pCharacterInstance = RegisterInstance(c_rkCreateData.m_dwVID);
  if (!pCharacterInstance) [[unlikely]] {
    TraceError(
        "CPythonCharacterManager::CreateInstance: VID[%d] - ALREADY EXIST\n",
        c_rkCreateData);
    return NULL;
  }

  if (!pCharacterInstance->Create(c_rkCreateData)) [[unlikely]] {
    TraceError("CPythonCharacterManager::CreateInstance VID[%d] Race[%d]",
               c_rkCreateData.m_dwVID, c_rkCreateData.m_dwRace);
    DeleteInstance(c_rkCreateData.m_dwVID);
    return NULL;
  }

  if (c_rkCreateData.m_isMain)
    SelectInstance(c_rkCreateData.m_dwVID);

  return (pCharacterInstance);
}

CInstanceBase *CPythonCharacterManager::RegisterInstance(DWORD VirtualID) {
  TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

  if (m_kAliveInstMap.end() != itor) {
    return NULL;
  }

  CInstanceBase *pCharacterInstance = CInstanceBase::New();
  m_kAliveInstMap.insert(
      TCharacterInstanceMap::value_type(VirtualID, pCharacterInstance));

  return (pCharacterInstance);
}

void CPythonCharacterManager::DeleteInstance(DWORD dwDelVID) {
  TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(dwDelVID);

  if (m_kAliveInstMap.end() == itor) {
    Tracef("DeleteCharacterInstance: no vid by %d\n", dwDelVID);
    return;
  }

  CInstanceBase *pkInstDel = itor->second;

  if (pkInstDel == m_pkInstBind)
    m_pkInstBind = NULL;

  if (pkInstDel == m_pkInstMain)
    m_pkInstMain = NULL;

  if (pkInstDel == m_pkInstPick)
    m_pkInstPick = NULL;

  CInstanceBase::Delete(pkInstDel);

  m_kAliveInstMap.erase(itor);
}

void CPythonCharacterManager::__DeleteBlendOutInstance(
    CInstanceBase *pkInstDel) {
  pkInstDel->DeleteBlendOut();
  m_kDeadInstList.push_back(pkInstDel);

  IAbstractPlayer &rkPlayer = IAbstractPlayer::GetSingleton();
  rkPlayer.NotifyCharacterDead(pkInstDel->GetVirtualID());
}

void CPythonCharacterManager::DeleteInstanceByFade(DWORD dwVID) {
  TCharacterInstanceMap::iterator f = m_kAliveInstMap.find(dwVID);
  if (m_kAliveInstMap.end() == f) {
    return;
  }
  __DeleteBlendOutInstance(f->second);
  m_kAliveInstMap.erase(f);
}

void CPythonCharacterManager::SelectInstance(DWORD VirtualID) {
  TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

  if (m_kAliveInstMap.end() == itor) {
    Tracef("SelectCharacterInstance: no vid by %d\n", VirtualID);
    return;
  }

  m_pkInstBind = itor->second;
}

CInstanceBase *CPythonCharacterManager::GetInstancePtr(DWORD VirtualID) {
  TCharacterInstanceMap::iterator itor = m_kAliveInstMap.find(VirtualID);

  if (m_kAliveInstMap.end() == itor)
    return NULL;

  return itor->second;
}

CInstanceBase *CPythonCharacterManager::GetInstancePtrByName(const char *name) {
  TCharacterInstanceMap::iterator itor;

  for (itor = m_kAliveInstMap.begin(); itor != m_kAliveInstMap.end(); itor++) {
    CInstanceBase *pInstance = itor->second;

    if (!strcmp(pInstance->GetNameString(), name))
      return pInstance;
  }

  return NULL;
}

CInstanceBase *CPythonCharacterManager::GetSelectedInstancePtr() {
  return m_pkInstBind;
}

CInstanceBase *CPythonCharacterManager::FindClickableInstancePtr() {
  return NULL;
}

void CPythonCharacterManager::__UpdateSortPickedActorList() {
  __UpdatePickedActorList();
  __SortPickedActorList();
}

void CPythonCharacterManager::__UpdatePickedActorList() {
  m_kVct_pkInstPicked.clear();

  TCharacterInstanceMap::iterator i;
  for (i = m_kAliveInstMap.begin(); i != m_kAliveInstMap.end(); ++i) {
    CInstanceBase *pkInstEach = i->second;
    // 2004.07.17.levites.isShow를 ViewFrustumCheck로 변경
    if (pkInstEach->CanPickInstance()) {
      if (pkInstEach->IsDead()) {
        if (pkInstEach->IntersectBoundingBox())
          m_kVct_pkInstPicked.push_back(pkInstEach);
      } else {
        if (pkInstEach->IntersectDefendingSphere())
          m_kVct_pkInstPicked.push_back(pkInstEach);
      }
    }
  }
}

struct CInstanceBase_SLessCameraDistance {
  TPixelPosition m_kPPosEye;

  bool operator()(CInstanceBase *pkInstLeft, CInstanceBase *pkInstRight) {
    int nLeftDeadPoint = pkInstLeft->IsDead();
    int nRightDeadPoint = pkInstRight->IsDead();

    if (nLeftDeadPoint < nRightDeadPoint)
      return true;

    if (pkInstLeft->CalculateDistanceSq3d(m_kPPosEye) <
        pkInstRight->CalculateDistanceSq3d(m_kPPosEye))
      return true;

    return false;
  }
};

void CPythonCharacterManager::__SortPickedActorList() {
  CCamera *pCamera = CCameraManager::Instance().GetCurrentCamera();
  const D3DXVECTOR3 &c_rv3EyePos = pCamera->GetEye();

  CInstanceBase_SLessCameraDistance kLess;
  kLess.m_kPPosEye =
      TPixelPosition(+c_rv3EyePos.x, -c_rv3EyePos.y, +c_rv3EyePos.z);

  std::sort(m_kVct_pkInstPicked.begin(), m_kVct_pkInstPicked.end(), kLess);
}

void CPythonCharacterManager::__NEW_Pick() {
  __UpdateSortPickedActorList();

  CInstanceBase *pkInstMain = GetMainInstancePtr();

#ifdef __MOVIE_MODE
  if (pkInstMain)
    if (pkInstMain->IsMovieMode()) {
      if (m_pkInstPick)
        m_pkInstPick->OnUnselected();
      return;
    }
#endif

  // 정밀한 체크
  {
    std::vector<CInstanceBase *>::iterator f;
    for (f = m_kVct_pkInstPicked.begin(); f != m_kVct_pkInstPicked.end(); ++f) {
      CInstanceBase *pkInstEach = *f;
      if (pkInstEach != pkInstMain && pkInstEach->IntersectBoundingBox()) {
        if (m_pkInstPick)
          if (m_pkInstPick != pkInstEach)
            m_pkInstPick->OnUnselected();

        if (pkInstEach->CanPickInstance()) {
          m_pkInstPick = pkInstEach;
          m_pkInstPick->OnSelected();
          return;
        }
      }
    }
  }

  // 못찾겠으면 걍 순서대로
  {
    std::vector<CInstanceBase *>::iterator f;
    for (f = m_kVct_pkInstPicked.begin(); f != m_kVct_pkInstPicked.end(); ++f) {
      CInstanceBase *pkInstEach = *f;
      if (pkInstEach != pkInstMain) {
        if (m_pkInstPick)
          if (m_pkInstPick != pkInstEach)
            m_pkInstPick->OnUnselected();

        if (pkInstEach->CanPickInstance()) {
          m_pkInstPick = pkInstEach;
          m_pkInstPick->OnSelected();
          return;
        }
      }
    }
  }

  if (pkInstMain)
    if (pkInstMain->CanPickInstance())
      if (m_kVct_pkInstPicked.end() != std::find(m_kVct_pkInstPicked.begin(),
                                                 m_kVct_pkInstPicked.end(),
                                                 pkInstMain)) {
        if (m_pkInstPick)
          if (m_pkInstPick != pkInstMain)
            m_pkInstPick->OnUnselected();

        m_pkInstPick = pkInstMain;
        m_pkInstPick->OnSelected();
        return;
      }

  if (m_pkInstPick) {
    m_pkInstPick->OnUnselected();
    m_pkInstPick = NULL;
  }
}

void CPythonCharacterManager::__OLD_Pick() {
  for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
       itor != m_kAliveInstMap.end(); ++itor) {
    CInstanceBase *pkInstEach = itor->second;

    if (pkInstEach == m_pkInstMain)
      continue;

    if (pkInstEach->IntersectDefendingSphere()) {
      if (m_pkInstPick)
        if (m_pkInstPick != pkInstEach)
          m_pkInstPick->OnUnselected();

      m_pkInstPick = pkInstEach;
      m_pkInstPick->OnSelected();

      return;
    }
  }

  if (m_pkInstPick) {
    m_pkInstPick->OnUnselected();
    m_pkInstPick = NULL;
  }
}

int CPythonCharacterManager::PickAll() {
  for (TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
       itor != m_kAliveInstMap.end(); ++itor) {
    CInstanceBase *pInstance = itor->second;

    if (pInstance->IntersectDefendingSphere())
      return pInstance->GetVirtualID();
  }

  return -1;
}

CInstanceBase *
CPythonCharacterManager::GetCloseInstance(CInstanceBase *pInstance) {
  float fMinDistance = 10000.0f;
  CInstanceBase *pCloseInstance = NULL;

  TCharacterInstanceMap::iterator itor = m_kAliveInstMap.begin();
  for (; itor != m_kAliveInstMap.end(); ++itor) {
    CInstanceBase *pTargetInstance = itor->second;

    if (pTargetInstance == pInstance)
      continue;

    DWORD dwVirtualNumber = pTargetInstance->GetVirtualNumber();
    if (CPythonNonPlayer::ON_CLICK_EVENT_BATTLE !=
        CPythonNonPlayer::Instance().GetEventType(dwVirtualNumber))
      continue;

    float fDistance = pInstance->GetDistance(pTargetInstance);
    if (fDistance < fMinDistance) {
      fMinDistance = fDistance;
      pCloseInstance = pTargetInstance;
    }
  }

  return pCloseInstance;
}

void CPythonCharacterManager::RefreshAllPCTextTail() {
  CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
  CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
  for (; itor != itorEnd; ++itor) {
    CInstanceBase *pInstance = *itor;
    if (!pInstance->IsPC())
      continue;

    pInstance->RefreshTextTail();
  }
}

void CPythonCharacterManager::RefreshAllGuildMark() {
  CPythonCharacterManager::CharacterIterator itor = CharacterInstanceBegin();
  CPythonCharacterManager::CharacterIterator itorEnd = CharacterInstanceEnd();
  for (; itor != itorEnd; ++itor) {
    CInstanceBase *pInstance = *itor;
    if (!pInstance->IsPC())
      continue;

    pInstance->ChangeGuild(pInstance->GetGuildID());
    pInstance->RefreshTextTail();
  }
}

void CPythonCharacterManager::DeleteAllInstances() {
  DestroyAliveInstanceMap();
  DestroyDeadInstanceList();
}

void CPythonCharacterManager::DestroyAliveInstanceMap() {
  for (TCharacterInstanceMap::iterator i = m_kAliveInstMap.begin();
       i != m_kAliveInstMap.end(); ++i)
    CInstanceBase::Delete(i->second);

  m_kAliveInstMap.clear();
}

void CPythonCharacterManager::DestroyDeadInstanceList() {
  std::for_each(m_kDeadInstList.begin(), m_kDeadInstList.end(),
                CInstanceBase::Delete);
  m_kDeadInstList.clear();
}

void CPythonCharacterManager::Destroy() {
  DeleteAllInstances();

  CInstanceBase::DestroySystem();

  __Initialize();
}

void CPythonCharacterManager::__Initialize() {
  memset(m_adwPointEffect, 0, sizeof(m_adwPointEffect));
  m_pkInstMain = NULL;
  m_pkInstBind = NULL;
  m_pkInstPick = NULL;
  m_v2PickedInstProjPos = D3DXVECTOR2(0.0f, 0.0f);
}

CPythonCharacterManager::CPythonCharacterManager() { __Initialize(); }

CPythonCharacterManager::~CPythonCharacterManager() { Destroy(); }
