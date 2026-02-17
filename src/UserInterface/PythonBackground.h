// PythonBackground.h: interface for the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(                                                                  \
    AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
#define AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameLib/MapManager.h"
#include "GameLib/TerrainDecal.h"
#include "GameLib/SnowEnvironment.h"

class CInstanceBase;

class CPythonBackground : public CMapManager,
                          public CSingleton<CPythonBackground> {
public:
  enum {
    SHADOW_NONE,
    SHADOW_GROUND,
    SHADOW_GROUND_AND_SOLO,
    SHADOW_ALL,
    SHADOW_ALL_HIGH,
    SHADOW_ALL_MAX,
  };

  enum {
    DISTANCE0,
    DISTANCE1,
    DISTANCE2,
    DISTANCE3,
    DISTANCE4,
    NUM_DISTANCE_SET
  };

  enum {
    DAY_MODE_LIGHT,
    DAY_MODE_DARK,
  };

  typedef struct SVIEWDISTANCESET {
    float m_fFogStart;
    float m_fFogEnd;
    float m_fFarClip;
    D3DXVECTOR3 m_v3SkyBoxScale;
  } TVIEWDISTANCESET;

public:
  CPythonBackground();
  virtual ~CPythonBackground();

  void Initialize();

  void Destroy();
  void Create();

  void GlobalPositionToLocalPosition(int32_t &rGlobalX, int32_t &rGlobalY);
  void LocalPositionToGlobalPosition(int32_t &rLocalX, int32_t &rLocalY);

  void GlobalPositionToLocalPosition(uint32_t &rGlobalX, uint32_t &rGlobalY);
  void LocalPositionToGlobalPosition(uint32_t &rLocalX, uint32_t &rLocalY);

  void EnableTerrainOnlyForHeight();
  bool SetSplatLimit(int iSplatNum);
  bool SetVisiblePart(int ePart, bool isVisible);
  bool SetShadowLevel(int eLevel);
  void RefreshShadowLevel();
  void SelectViewDistanceNum(int eNum);
  void SetViewDistanceSet(int eNum, float fFarClip);
  float GetFarClip();

  DWORD GetRenderShadowTime();
  void GetDistanceSetInfo(int *peNum, float *pfStart, float *pfEnd,
                          float *pfFarClip);

  bool GetPickingPoint(D3DXVECTOR3 *v3IntersectPt);
  bool GetPickingPointWithRay(const CRay &rRay, D3DXVECTOR3 *v3IntersectPt);
  bool GetPickingPointWithRayOnlyTerrain(const CRay &rRay,
                                         D3DXVECTOR3 *v3IntersectPt);
  BOOL GetLightDirection(D3DXVECTOR3 &rv3LightDirection);

  void Update(float fCenterX, float fCenterY, float fCenterZ);

  void CreateCharacterShadowTexture();
  void ReleaseCharacterShadowTexture();
  void Render();
  void RenderSnow();
  void RenderPCBlocker();
  void RenderCollision();
  void RenderCharacterShadowToTexture();
  void RenderSky();
  void RenderCloud();
  void RenderWater();
  void RenderEffect();
  void RenderBeforeLensFlare();
  void RenderAfterLensFlare();

  bool CheckAdvancing(CInstanceBase *pInstance);

  void SetCharacterDirLight();
  void SetBackgroundDirLight();

  void ChangeToDay();
  void ChangeToNight();
  void EnableSnowEnvironment();
  void DisableSnowEnvironment();
  void SetXMaxTree(int iGrade);

  void ClearGuildArea();
  void RegisterGuildArea(int isx, int isy, int iex, int iey);

  void CreateTargetEffect(DWORD dwID, DWORD dwChrVID);
  void CreateTargetEffect(DWORD dwID, long lx, long ly);
  void DeleteTargetEffect(DWORD dwID);

  void CreateSpecialEffect(DWORD dwID, float fx, float fy, float fz,
                           const char *c_szFileName);
  void DeleteSpecialEffect(DWORD dwID);

  void Warp(DWORD dwX, DWORD dwY);

  void VisibleGuildArea();
  void DisableGuildArea();

  void RegisterDungeonMapName(const char *c_szMapName);
  TMapInfo *GlobalPositionToMapInfo(DWORD dwGlobalX, DWORD dwGlobalY);
  const char *GetWarpMapName();

protected:
  void __CreateProperty();
  bool __IsSame(std::set<int> &rleft, std::set<int> &rright);

protected:
  std::string m_strMapName;

private:
  CSnowEnvironment m_SnowEnvironment;

  int m_iDayMode;
  int m_iXMasTreeGrade;

  int m_eShadowLevel;
  int m_eViewDistanceNum;

  BOOL m_bVisibleGuildArea;

  DWORD m_dwRenderShadowTime;

  DWORD m_dwBaseX;
  DWORD m_dwBaseY;

  TVIEWDISTANCESET m_ViewDistanceSet[NUM_DISTANCE_SET];

  std::set<int> m_kSet_iShowingPortalID;
  std::set<std::string> m_kSet_strDungeonMapName;
  std::map<DWORD, DWORD> m_kMap_dwTargetID_dwChrID;

  struct SReserveTargetEffect {
    int ilx;
    int ily;
  };
  std::map<DWORD, SReserveTargetEffect> m_kMap_dwID_kReserveTargetEffect;

  struct FFindWarpMapName {
    DWORD m_dwX, m_dwY;
    FFindWarpMapName(DWORD dwX, DWORD dwY) {
      m_dwX = dwX;
      m_dwY = dwY;
    }
    bool operator()(TMapInfo &rMapInfo) {
      if (m_dwX < rMapInfo.m_dwBaseX || m_dwX >= rMapInfo.m_dwEndX ||
          m_dwY < rMapInfo.m_dwBaseY || m_dwY >= rMapInfo.m_dwEndY)
        return false;
      return true;
    }
  };
};

#endif // !defined(AFX_PYTHONBACKGROUND_H__A202CB18_9553_4CF3_8500_5D7062B55432__INCLUDED_)
