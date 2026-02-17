// AreaLoaderThread.h: interface for the CAreaLoaderThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <deque>
#include <mutex>

class CTerrain;
class CArea;

class TEMP_CAreaLoaderThread {
public:
  TEMP_CAreaLoaderThread();
  virtual ~TEMP_CAreaLoaderThread();

  bool Create(void *arg);
  void Shutdown();

  void Request(CTerrain *pTerrain);
  bool Fetch(CTerrain **ppTerrian);

  void Request(CArea *pArea);
  bool Fetch(CArea **ppArea);

private:
  void ProcessTerrain(CTerrain *pTerrain);
  void ProcessArea(CArea *pArea);

private:
  std::deque<CTerrain *> m_pTerrainCompleteDeque;
  std::mutex m_TerrainCompleteMutex;

  std::deque<CArea *> m_pAreaCompleteDeque;
  std::mutex m_AreaCompleteMutex;

  bool m_bShutdowned;
};
