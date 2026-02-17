// AreaLoaderThread.cpp: implementation of the CAreaLoaderThread class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "EterLib/ResourceManager.h"
#include "EterLib/GameThreadPool.h"

#include "AreaLoaderThread.h"
#include "AreaTerrain.h"
#include "MapOutdoor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TEMP_CAreaLoaderThread::TEMP_CAreaLoaderThread() : m_bShutdowned(false) {}

TEMP_CAreaLoaderThread::~TEMP_CAreaLoaderThread() { Shutdown(); }

bool TEMP_CAreaLoaderThread::Create(void *arg) {
  // Modern implementation doesn't need explicit thread creation
  // The global CGameThreadPool handles threading
  m_bShutdowned = false;
  return true;
}

void TEMP_CAreaLoaderThread::Shutdown() {
  m_bShutdowned = true;

  // Clear any pending completed items
  {
    std::lock_guard<std::mutex> lock(m_TerrainCompleteMutex);
    m_pTerrainCompleteDeque.clear();
  }

  {
    std::lock_guard<std::mutex> lock(m_AreaCompleteMutex);
    m_pAreaCompleteDeque.clear();
  }
}

void TEMP_CAreaLoaderThread::Request(CTerrain *pTerrain) {
  if (m_bShutdowned)
    return;

  // Enqueue terrain loading to the global thread pool
  CGameThreadPool *pThreadPool = CGameThreadPool::InstancePtr();
  if (pThreadPool) {
    pThreadPool->Enqueue([this, pTerrain]() { ProcessTerrain(pTerrain); });
  } else {
    // Fallback to synchronous loading if thread pool not available
    ProcessTerrain(pTerrain);
  }
}

bool TEMP_CAreaLoaderThread::Fetch(CTerrain **ppTerrain) {
  std::lock_guard<std::mutex> lock(m_TerrainCompleteMutex);

  if (m_pTerrainCompleteDeque.empty())
    return false;

  *ppTerrain = m_pTerrainCompleteDeque.front();
  m_pTerrainCompleteDeque.pop_front();

  return true;
}

void TEMP_CAreaLoaderThread::Request(CArea *pArea) {
  if (m_bShutdowned)
    return;

  // Enqueue area loading to the global thread pool
  CGameThreadPool *pThreadPool = CGameThreadPool::InstancePtr();
  if (pThreadPool) {
    pThreadPool->Enqueue([this, pArea]() { ProcessArea(pArea); });
  } else {
    // Fallback to synchronous loading if thread pool not available
    ProcessArea(pArea);
  }
}

bool TEMP_CAreaLoaderThread::Fetch(CArea **ppArea) {
  std::lock_guard<std::mutex> lock(m_AreaCompleteMutex);

  if (m_pAreaCompleteDeque.empty())
    return false;

  *ppArea = m_pAreaCompleteDeque.front();
  m_pAreaCompleteDeque.pop_front();

  return true;
}

void TEMP_CAreaLoaderThread::ProcessArea(CArea *pArea) {
  if (m_bShutdowned)
    return;

  DWORD dwStartTime = ELTimer_GetMSec();

  // Area Load
  WORD wAreaCoordX, wAreaCoordY;
  pArea->GetCoordinate(&wAreaCoordX, &wAreaCoordY);
  DWORD dwID = (DWORD)(wAreaCoordX) * 1000L + (DWORD)(wAreaCoordY);

  const std::string &c_rStrMapName = pArea->GetOwner()->GetName();

  char szAreaPathName[64 + 1];
  _snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\",
            c_rStrMapName.c_str(), dwID);

  pArea->Load(szAreaPathName);

  Tracef("TEMP_CAreaLoaderThread::ProcessArea LoadArea : %d ms elapsed\n",
         ELTimer_GetMSec() - dwStartTime);

  // Add to completed queue
  {
    std::lock_guard<std::mutex> lock(m_AreaCompleteMutex);
    m_pAreaCompleteDeque.push_back(pArea);
  }
}

void TEMP_CAreaLoaderThread::ProcessTerrain(CTerrain *pTerrain) {
  if (m_bShutdowned)
    return;

  DWORD dwStartTime = ELTimer_GetMSec();

  // Terrain Load
  WORD wCoordX, wCoordY;
  pTerrain->GetCoordinate(&wCoordX, &wCoordY);
  DWORD dwID = (DWORD)(wCoordX) * 1000L + (DWORD)(wCoordY);

  const std::string &c_rStrMapName = pTerrain->GetOwner()->GetName();
  char filename[256];
  sprintf(filename, "%s\\%06u\\AreaProperty.txt", c_rStrMapName.c_str(), dwID);

  CTokenVectorMap stTokenVectorMap;

  if (!LoadMultipleTextData(filename, stTokenVectorMap))
    return;

  if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
    return;

  if (stTokenVectorMap.end() == stTokenVectorMap.find("areaname"))
    return;

  const std::string &c_rstrType = stTokenVectorMap["scripttype"][0];
  const std::string &c_rstrAreaName = stTokenVectorMap["areaname"][0];

  if (c_rstrType != "AreaProperty")
    return;

  char szRawHeightFieldname[64 + 1];
  char szWaterMapName[64 + 1];
  char szAttrMapName[64 + 1];
  char szShadowTexName[64 + 1];
  char szShadowMapName[64 + 1];
  char szMiniMapTexName[64 + 1];
  char szSplatName[64 + 1];

  _snprintf(szRawHeightFieldname, sizeof(szRawHeightFieldname),
            "%s\\%06u\\height.raw", c_rStrMapName.c_str(), dwID);
  _snprintf(szSplatName, sizeof(szSplatName), "%s\\%06u\\tile.raw",
            c_rStrMapName.c_str(), dwID);
  _snprintf(szAttrMapName, sizeof(szAttrMapName), "%s\\%06u\\attr.atr",
            c_rStrMapName.c_str(), dwID);
  _snprintf(szWaterMapName, sizeof(szWaterMapName), "%s\\%06u\\water.wtr",
            c_rStrMapName.c_str(), dwID);
  _snprintf(szShadowTexName, sizeof(szShadowTexName), "%s\\%06u\\shadowmap.dds",
            c_rStrMapName.c_str(), dwID);
  _snprintf(szShadowMapName, sizeof(szShadowMapName), "%s\\%06u\\shadowmap.raw",
            c_rStrMapName.c_str(), dwID);
  _snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds",
            c_rStrMapName.c_str(), dwID);

  pTerrain->CopySettingFromGlobalSetting();

  pTerrain->LoadWaterMap(szWaterMapName);
  pTerrain->LoadHeightMap(szRawHeightFieldname);
  pTerrain->LoadAttrMap(szAttrMapName);
  pTerrain->RAW_LoadTileMap(szSplatName, true);
  pTerrain->LoadShadowTexture(szShadowTexName);
  pTerrain->LoadShadowMap(szShadowMapName);
  pTerrain->LoadMiniMapTexture(szMiniMapTexName);
  pTerrain->SetName(c_rstrAreaName.c_str());
  pTerrain->CalculateTerrainPatch();

  pTerrain->SetReady();

  Tracef("TEMP_CAreaLoaderThread::ProcessTerrain LoadTerrain : %d ms elapsed\n",
         ELTimer_GetMSec() - dwStartTime);

  // Add to completed queue
  {
    std::lock_guard<std::mutex> lock(m_TerrainCompleteMutex);
    m_pTerrainCompleteDeque.push_back(pTerrain);
  }
}
