#include "Stdafx.h"

#include "FlyingData.h"
#include "FlyingInstance.h"
#include "FlyingObjectManager.h"

#include "ActorInstance.h"

CFlyingManager::CFlyingManager() { m_pMapManager = 0; }

CFlyingManager::~CFlyingManager() { Destroy(); }

void CFlyingManager::__DestroyFlyingInstanceList() {
  TFlyingInstanceList::iterator i;
  for (i = m_kLst_pkFlyInst.begin(); i != m_kLst_pkFlyInst.end(); ++i) {
    CFlyingInstance *pkFlyInst = *i;
    CFlyingInstance::Delete(pkFlyInst);
  }
  m_kLst_pkFlyInst.clear();
}

void CFlyingManager::__DestroyFlyingDataMap() {
  TFlyingDataMap::iterator i;
  for (i = m_kMap_pkFlyData.begin(); i != m_kMap_pkFlyData.end(); ++i) {
    CFlyingData *pkFlyData = i->second;
    CFlyingData::Delete(pkFlyData);
  }
  m_kMap_pkFlyData.clear();
}

void CFlyingManager::DeleteAllInstances() { __DestroyFlyingInstanceList(); }

void CFlyingManager::Destroy() {
  __DestroyFlyingInstanceList();
  __DestroyFlyingDataMap();

  m_pMapManager = 0;
}

bool CFlyingManager::RegisterFlyingData(const char *c_szFilename) {
  std::string s;
  StringPath(c_szFilename, s);
  DWORD dwRetCRC = GetCaseCRC32(s.c_str(), s.size());

  if (m_kMap_pkFlyData.find(dwRetCRC) != m_kMap_pkFlyData.end()) {
    return false;
  }

  CFlyingData *pFlyingData = CFlyingData::New();
  if (!pFlyingData->LoadScriptFile(c_szFilename)) {
    Tracenf("CEffectManager::RegisterFlyingData %s - Failed to load flying "
            "data file",
            c_szFilename);
    CFlyingData::Delete(pFlyingData);
    return false;
  }

  m_kMap_pkFlyData.insert(std::make_pair(dwRetCRC, pFlyingData));
  return true;
}

bool CFlyingManager::RegisterFlyingData(const char *c_szFilename,
                                        DWORD &r_dwRetCRC) {
  std::string s;
  StringPath(c_szFilename, s);
  r_dwRetCRC = GetCaseCRC32(s.c_str(), s.size());

  if (m_kMap_pkFlyData.find(r_dwRetCRC) != m_kMap_pkFlyData.end()) {
    TraceError("CFlyingManager::RegisterFlyingData - Already exists flying "
               "data named [%s]",
               c_szFilename);
    return false;
  }

  CFlyingData *pFlyingData = CFlyingData::New();
  if (!pFlyingData->LoadScriptFile(c_szFilename)) {
    TraceError("CEffectManager::RegisterFlyingData %s - Failed to load flying "
               "data file",
               c_szFilename);
    CFlyingData::Delete(pFlyingData);
    return false;
  }

  m_kMap_pkFlyData.insert(std::make_pair(r_dwRetCRC, pFlyingData));
  return true;
}

CFlyingInstance *CFlyingManager::CreateFlyingInstanceFlyTarget(
    const DWORD dwID, const D3DXVECTOR3 &v3StartPosition,
    const CFlyTarget &cr_FlyTarget, bool canAttack) {
  if (m_kMap_pkFlyData.find(dwID) == m_kMap_pkFlyData.end()) {
    // TraceError("CFlyingManager::CreateFlyingInstanceFlyTarget - No data with
    // CRC [%d]", dwID);
    return NULL;
  }

  CFlyingInstance *pFlyingInstance = CFlyingInstance::New();
  pFlyingInstance->Create(m_kMap_pkFlyData[dwID], v3StartPosition, cr_FlyTarget,
                          canAttack);

  m_kLst_pkFlyInst.push_back(pFlyingInstance);

  pFlyingInstance->ID = m_IDCounter++;
  return pFlyingInstance;
}

void CFlyingManager::Update() {
  TFlyingInstanceList::iterator i = m_kLst_pkFlyInst.begin();

  while (i != m_kLst_pkFlyInst.end()) {
    CFlyingInstance *pkFlyInst = *i;
    if (!pkFlyInst->Update()) {
      CFlyingInstance::Delete(pkFlyInst);
      i = m_kLst_pkFlyInst.erase(i);
    } else {
      ++i;
    }
  }
}

void CFlyingManager::Render() {
  std::for_each(m_kLst_pkFlyInst.begin(), m_kLst_pkFlyInst.end(),
                std::mem_fn(&CFlyingInstance::Render));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

bool CFlyingManager::RegisterIndexedFlyData(DWORD dwIndex, BYTE byType,
                                            const char *c_szFileName) {
  DWORD dwCRC;
  if (!RegisterFlyingData(c_szFileName, dwCRC)) {
    TraceError("CFlyingManager::RegisterIndexFlyData(dwIndex=%d, "
               "c_szFileName=%s) - Failed to load flying data file",
               dwIndex, c_szFileName);
    return false;
  }

  TIndexFlyData IndexFlyData;
  IndexFlyData.byType = byType;
  IndexFlyData.dwCRC = dwCRC;
  m_kMap_dwIndexFlyData.insert(std::make_pair(dwIndex, IndexFlyData));

  return true;
}

void CFlyingManager::CreateIndexedFly(DWORD dwIndex,
                                      CActorInstance *pStartActor,
                                      CActorInstance *pEndActor) {
  if (m_kMap_dwIndexFlyData.end() == m_kMap_dwIndexFlyData.find(dwIndex)) {
    TraceError(
        "CFlyingManager::CreateIndexedFly(dwIndex=%d) - Not registered index",
        dwIndex);
    return;
  }

  TPixelPosition posStart;
  pStartActor->GetPixelPosition(&posStart);

  TIndexFlyData &rIndexFlyData = m_kMap_dwIndexFlyData[dwIndex];
  switch (rIndexFlyData.byType) {
  case INDEX_FLY_TYPE_NORMAL: {
    CreateFlyingInstanceFlyTarget(
        rIndexFlyData.dwCRC, D3DXVECTOR3(posStart.x, posStart.y, posStart.z),
        pEndActor, false);
    break;
  }
  case INDEX_FLY_TYPE_FIRE_CRACKER: {
    float fRot = fmod(pStartActor->GetRotation() - 90.0f + 360.0f, 360.0f) +
                 frandom(-30.0f, 30.0f);

    float fDistance = frandom(2000.0f, 5000.0f);
    float fxRand = fDistance * cosf(D3DXToRadian(fRot));
    float fyRand = fDistance * sinf(D3DXToRadian(fRot));
    float fzRand = frandom(1000.0f, 2500.0f);

    CreateFlyingInstanceFlyTarget(
        rIndexFlyData.dwCRC,
        D3DXVECTOR3(posStart.x, posStart.y, posStart.z + 200),
        D3DXVECTOR3(posStart.x + fxRand, posStart.y + fyRand,
                    posStart.z + fzRand),
        false);
    break;
  }
  case INDEX_FLY_TYPE_AUTO_FIRE: {
    CreateFlyingInstanceFlyTarget(
        rIndexFlyData.dwCRC,
        D3DXVECTOR3(posStart.x, posStart.y, posStart.z + 100.0f), pEndActor,
        false);
    break;
  }
  }
}
