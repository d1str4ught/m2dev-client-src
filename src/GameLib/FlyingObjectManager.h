#pragma once

#include "FlyTarget.h"

#include <set>

class CFlyingInstance;
class CFlyingData;
class CMapManager;
class CActorInstance;

class CFlyingManager : public CSingleton<CFlyingManager> {
public:
  enum EIndexFlyType {
    INDEX_FLY_TYPE_NORMAL,
    INDEX_FLY_TYPE_FIRE_CRACKER,
    INDEX_FLY_TYPE_AUTO_FIRE,
  };

public:
  CFlyingManager();
  virtual ~CFlyingManager();

  void Destroy();

  void DeleteAllInstances();

  bool RegisterFlyingData(const char *c_szFilename);
  bool RegisterFlyingData(const char *c_szFilename, DWORD &r_dwRetCRC);

  CFlyingInstance *
  CreateFlyingInstanceFlyTarget(const DWORD dwID,
                                const D3DXVECTOR3 &v3StartPosition,
                                const CFlyTarget &cr_FlyTarget, bool canAttack);

  void Update();
  void Render();

  void SetMapManagerPtr(CMapManager *pMapManager) {
    m_pMapManager = pMapManager;
  }
  CMapManager *GetMapManagerPtr() { return m_pMapManager; }

public: // Controlled by Server
  bool RegisterIndexedFlyData(DWORD dwIndex, BYTE byType,
                              const char *c_szFileName);
  void CreateIndexedFly(DWORD dwIndex, CActorInstance *pStartActor,
                        CActorInstance *pEndActor);

private:
  void __DestroyFlyingInstanceList();
  void __DestroyFlyingDataMap();

  typedef std::map<DWORD, CFlyingData *> TFlyingDataMap;
  typedef std::list<CFlyingInstance *> TFlyingInstanceList;

  typedef struct SIndexFlyData {
    BYTE byType;
    DWORD dwCRC;
  } TIndexFlyData;
  typedef std::map<DWORD, TIndexFlyData> TIndexFlyDataMap;

  TFlyingDataMap m_kMap_pkFlyData;
  TFlyingInstanceList m_kLst_pkFlyInst;
  TIndexFlyDataMap m_kMap_dwIndexFlyData;

  CMapManager *m_pMapManager;

  DWORD m_IDCounter;
};
