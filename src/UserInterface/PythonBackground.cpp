// PythonBackground.cpp: implementation of the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EterLib/CullingManager.h"
#include "EterLib/Camera.h"
#include "PackLib/PackManager.h"
#include "GameLib/MapOutDoor.h"
#include "GameLib/PropertyLoader.h"

#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "PythonMiniMap.h"
#include "PythonSystem.h"

std::string g_strEffectName =
    "d:/ymir work/effect/etc/direction/direction_land.mse";

DWORD CPythonBackground::GetRenderShadowTime() { return m_dwRenderShadowTime; }

bool CPythonBackground::SetVisiblePart(int eMapOutDoorPart, bool isVisible) {
  if (!m_pkMap)
    return false;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.SetVisiblePart(eMapOutDoorPart, isVisible);
  return true;
}

void CPythonBackground::EnableTerrainOnlyForHeight() {
  if (!m_pkMap)
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.EnableTerrainOnlyForHeight(TRUE);
}

bool CPythonBackground::SetSplatLimit(int iSplatNum) {
  if (!m_pkMap)
    return false;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.SetSplatLimit(iSplatNum);
  return true;
}

void CPythonBackground::CreateCharacterShadowTexture() {
  if (!m_pkMap)
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.CreateCharacterShadowTexture();
}

void CPythonBackground::ReleaseCharacterShadowTexture() {
  if (!m_pkMap)
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.ReleaseCharacterShadowTexture();
}

void CPythonBackground::RefreshShadowLevel() {
  SetShadowLevel(CPythonSystem::Instance().GetShadowLevel());
}

bool CPythonBackground::SetShadowLevel(int eLevel) {
  if (!m_pkMap)
    return false;

  if (m_eShadowLevel == eLevel)
    return true;

  CMapOutdoor &rkMap = GetMapOutdoorRef();

  m_eShadowLevel = eLevel;

  switch (m_eShadowLevel) {
  case SHADOW_NONE:
    rkMap.SetDrawShadow(false);
    rkMap.SetShadowTextureSize(512);
    break;

  case SHADOW_GROUND:
    rkMap.SetDrawShadow(true);
    rkMap.SetDrawCharacterShadow(false);
    rkMap.SetShadowTextureSize(512);
    break;

  case SHADOW_GROUND_AND_SOLO:
    rkMap.SetDrawShadow(true);
    rkMap.SetDrawCharacterShadow(true);
    rkMap.SetShadowTextureSize(512);
    break;

  case SHADOW_ALL:
    rkMap.SetDrawShadow(true);
    rkMap.SetDrawCharacterShadow(true);
    rkMap.SetShadowTextureSize(512);
    break;

  case SHADOW_ALL_HIGH:
    rkMap.SetDrawShadow(true);
    rkMap.SetDrawCharacterShadow(true);
    rkMap.SetShadowTextureSize(1024);
    break;

  case SHADOW_ALL_MAX:
    rkMap.SetDrawShadow(true);
    rkMap.SetDrawCharacterShadow(true);
    rkMap.SetShadowTextureSize(2048);
    break;
  }

  return true;
}

void CPythonBackground::SelectViewDistanceNum(int eNum) {
  if (!m_pkMap)
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();

  if (!mc_pcurEnvironmentData) {
    TraceError("CPythonBackground::SelectViewDistanceNum(int eNum=%d) "
               "mc_pcurEnvironmentData is NULL",
               eNum);
    return;
  }

  m_eViewDistanceNum = eNum;

  TEnvironmentData *env = ((TEnvironmentData *)mc_pcurEnvironmentData);

  // 게임 분위기를 바꿔놓을 수 있으므로 reserve로 되어있으면 고치지 않는다.
  if (env->bReserve) {
    env->m_fFogNearDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
    env->m_fFogFarDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
    env->v3SkyBoxScale = m_ViewDistanceSet[m_eViewDistanceNum].m_v3SkyBoxScale;
    rkMap.SetEnvironmentSkyBox();
  }
}

void CPythonBackground::SetViewDistanceSet(int eNum, float fFarClip) {
  if (!m_pkMap)
    return;

  m_ViewDistanceSet[eNum].m_fFogStart = fFarClip * 0.5f; // 0.3333333f;
  m_ViewDistanceSet[eNum].m_fFogEnd = fFarClip * 0.7f;   // 0.6666667f;

  float fSkyBoxScale = fFarClip * 0.6f; // 0.5773502f;
  m_ViewDistanceSet[eNum].m_v3SkyBoxScale =
      D3DXVECTOR3(fSkyBoxScale, fSkyBoxScale, fSkyBoxScale);
  m_ViewDistanceSet[eNum].m_fFarClip = fFarClip;

  if (eNum == m_eViewDistanceNum)
    SelectViewDistanceNum(eNum);
}

float CPythonBackground::GetFarClip() {
  if (!m_pkMap)
    return 50000.0f;

  if (m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip == 0.0f) {
    TraceError("CPythonBackground::GetFarClip m_eViewDistanceNum=%d",
               m_eViewDistanceNum);
    m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip = 25600.0f;
  }

  return m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

void CPythonBackground::GetDistanceSetInfo(int *peNum, float *pfStart,
                                           float *pfEnd, float *pfFarClip) {
  if (!m_pkMap) {
    *peNum = 4;
    *pfStart = 10000.0f;
    *pfEnd = 15000.0f;
    *pfFarClip = 50000.0f;
    return;
  }
  *peNum = m_eViewDistanceNum;
  *pfStart = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
  *pfEnd = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
  *pfFarClip = m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPythonBackground::CPythonBackground() {
  m_dwRenderShadowTime = 0;
  m_eViewDistanceNum = 0;
  m_eViewDistanceNum = 0;
  m_eViewDistanceNum = 0;
  m_eShadowLevel = SHADOW_NONE;
  m_dwBaseX = 0;
  m_dwBaseY = 0;
  m_strMapName = "";
  m_iDayMode = DAY_MODE_LIGHT;
  m_iXMasTreeGrade = 0;
  m_bVisibleGuildArea = FALSE;

  SetViewDistanceSet(4, 25600.0f);
  SetViewDistanceSet(3, 25600.0f);
  SetViewDistanceSet(2, 25600.0f);
  SetViewDistanceSet(1, 25600.0f);
  SetViewDistanceSet(0, 25600.0f);
  Initialize();
}

CPythonBackground::~CPythonBackground() { Tracen("CPythonBackground Clear"); }

void CPythonBackground::Initialize() {
  std::string stAtlasInfoFileName(GetLocalePath());
  stAtlasInfoFileName += "/AtlasInfo.txt";
  SetAtlasInfoFileName(stAtlasInfoFileName.c_str());
  CMapManager::Initialize();
}

void CPythonBackground::__CreateProperty() {
  m_PropertyManager.Initialize("pack/property.pck");
}

//////////////////////////////////////////////////////////////////////
// Normal Functions
//////////////////////////////////////////////////////////////////////

bool CPythonBackground::GetPickingPoint(D3DXVECTOR3 *v3IntersectPt) {
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  return rkMap.GetPickingPoint(v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRay(const CRay &rRay,
                                               D3DXVECTOR3 *v3IntersectPt) {
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  return rkMap.GetPickingPointWithRay(rRay, v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRayOnlyTerrain(
    const CRay &rRay, D3DXVECTOR3 *v3IntersectPt) {
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  return rkMap.GetPickingPointWithRayOnlyTerrain(rRay, v3IntersectPt);
}

BOOL CPythonBackground::GetLightDirection(D3DXVECTOR3 &rv3LightDirection) {
  if (!mc_pcurEnvironmentData)
    return FALSE;

  rv3LightDirection.x =
      mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
  rv3LightDirection.y =
      mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
  rv3LightDirection.z =
      mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;
  return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CPythonBackground::Destroy() {
  CMapManager::Destroy();
  m_SnowEnvironment.Destroy();
  m_bVisibleGuildArea = FALSE;
}

void CPythonBackground::Create() {
  static int s_isCreateProperty = false;

  if (!s_isCreateProperty) {
    s_isCreateProperty = true;
    __CreateProperty();
  }

  CMapManager::Create();

  m_SnowEnvironment.Create();
}

struct FGetPortalID {
  float m_fRequestX, m_fRequestY;
  std::set<int> m_kSet_iPortalID;
  FGetPortalID(float fRequestX, float fRequestY) {
    m_fRequestX = fRequestX;
    m_fRequestY = fRequestY;
  }
  void operator()(CGraphicObjectInstance *pObject) {
    for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i) {
      int iID = pObject->GetPortal(i);
      if (0 == iID)
        break;

      m_kSet_iPortalID.insert(iID);
    }
  }
};

void CPythonBackground::Update(float fCenterX, float fCenterY, float fCenterZ) {
  if (!IsMapReady())
    return;
#ifdef __PERFORMANCE_CHECKER__
  DWORD t1 = ELTimer_GetMSec();
#endif
  UpdateMap(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
  DWORD t2 = ELTimer_GetMSec();
#endif
  UpdateAroundAmbience(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
  DWORD t3 = ELTimer_GetMSec();
#endif
  m_SnowEnvironment.Update(D3DXVECTOR3(fCenterX, -fCenterY, fCenterZ));

#ifdef __PERFORMANCE_CHECKER__
  {
    static FILE *fp = fopen("perf_bg_update.txt", "w");
    if (t3 - t1 > 5) {
      fprintf(fp, "BG.Total %d (Time %f)\n", t3 - t1,
              ELTimer_GetMSec() / 1000.0f);
      fprintf(fp, "BG.UpdateMap %d\n", t2 - t1);
      fprintf(fp, "BG.UpdateAmb %d\n", t3 - t2);
      fflush(fp);
    }
  }
#endif

  // Portal Process
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  if (rkMap.IsEnablePortal()) {
    CCullingManager &rkCullingMgr = CCullingManager::Instance();
    FGetPortalID kGetPortalID(fCenterX, -fCenterY);

    Vector3d aVector3d;
    aVector3d.Set(fCenterX, -fCenterY, fCenterZ);

    Vector3d toTop;
    toTop.Set(0, 0, 25000.0f);

    rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

    std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
    if (!__IsSame(kGetPortalID.m_kSet_iPortalID, m_kSet_iShowingPortalID)) {
      ClearPortal();
      std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
      for (; itor != kGetPortalID.m_kSet_iPortalID.end(); ++itor) {
        AddShowingPortalID(*itor);
      }
      RefreshPortal();

      m_kSet_iShowingPortalID = kGetPortalID.m_kSet_iPortalID;
    }
  }

  // Target Effect Process
  {
    std::map<DWORD, DWORD>::iterator itor = m_kMap_dwTargetID_dwChrID.begin();
    for (; itor != m_kMap_dwTargetID_dwChrID.end(); ++itor) {
      DWORD dwTargetID = itor->first;
      DWORD dwChrID = itor->second;

      CInstanceBase *pInstance =
          CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

      if (!pInstance)
        continue;

      TPixelPosition kPixelPosition;
      pInstance->NEW_GetPixelPosition(&kPixelPosition);

      CreateSpecialEffect(dwTargetID, +kPixelPosition.x, -kPixelPosition.y,
                          +kPixelPosition.z, g_strEffectName.c_str());
    }
  }

  // Reserve Target Effect
  {
    std::map<DWORD, SReserveTargetEffect>::iterator itor =
        m_kMap_dwID_kReserveTargetEffect.begin();
    for (; itor != m_kMap_dwID_kReserveTargetEffect.end();) {
      DWORD dwID = itor->first;
      SReserveTargetEffect &rReserveTargetEffect = itor->second;

      float ilx = float(rReserveTargetEffect.ilx);
      float ily = float(rReserveTargetEffect.ily);

      float fHeight = rkMap.GetHeight(ilx, ily);
      if (0.0f == fHeight) {
        ++itor;
        continue;
      }

      CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());

      itor = m_kMap_dwID_kReserveTargetEffect.erase(itor);
    }
  }
}

bool CPythonBackground::__IsSame(std::set<int> &rleft, std::set<int> &rright) {
  std::set<int>::iterator itor_l;
  std::set<int>::iterator itor_r;

  for (itor_l = rleft.begin(); itor_l != rleft.end(); ++itor_l) {
    if (rright.end() == rright.find(*itor_l))
      return false;
  }

  for (itor_r = rright.begin(); itor_r != rright.end(); ++itor_r) {
    if (rleft.end() == rleft.find(*itor_r))
      return false;
  }

  return true;
}

void CPythonBackground::Render() {
  if (!IsMapReady())
    return;

  m_SnowEnvironment.Deform();

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.Render();
  if (m_bVisibleGuildArea)
    rkMap.RenderMarkedArea();
}

void CPythonBackground::RenderSnow() { m_SnowEnvironment.Render(); }

void CPythonBackground::RenderPCBlocker() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderPCBlocker();
}

void CPythonBackground::RenderCollision() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderCollision();
}

void CPythonBackground::RenderCharacterShadowToTexture() {
  extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
  if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
    return;

  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  DWORD t1 = ELTimer_GetMSec();

  if (m_eShadowLevel == SHADOW_ALL || m_eShadowLevel == SHADOW_ALL_HIGH ||
      m_eShadowLevel == SHADOW_ALL_MAX ||
      m_eShadowLevel == SHADOW_GROUND_AND_SOLO) {
    D3DXMATRIX matWorld;
    STATEMANAGER.GetTransform(D3DTS_WORLD, &matWorld);

    bool canRender = rkMap.BeginRenderCharacterShadowToTexture();
    if (canRender) {
      CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();

      if (m_eShadowLevel == SHADOW_GROUND_AND_SOLO)
        rkChrMgr.RenderShadowMainInstance();
      else
        rkChrMgr.RenderShadowAllInstances();
    }
    rkMap.EndRenderCharacterShadowToTexture();

    STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorld);
  }

  DWORD t2 = ELTimer_GetMSec();

  m_dwRenderShadowTime = t2 - t1;
}

inline float Interpolate(float fStart, float fEnd, float fPercent) {
  return fStart + (fEnd - fStart) * fPercent;
}
struct CollisionChecker {
  bool isBlocked;
  CInstanceBase *pInstance;
  CollisionChecker(CInstanceBase *pInstance)
      : pInstance(pInstance), isBlocked(false) {}
  void operator()(CGraphicObjectInstance *pOpponent) {
    if (isBlocked)
      return;

    if (!pOpponent)
      return;

    if (pInstance->IsBlockObject(*pOpponent))
      isBlocked = true;
  }
};

struct CollisionAdjustChecker {
  bool isBlocked;
  CInstanceBase *pInstance;
  CollisionAdjustChecker(CInstanceBase *pInstance)
      : pInstance(pInstance), isBlocked(false) {}
  void operator()(CGraphicObjectInstance *pOpponent) {
    if (!pOpponent)
      return;

    if (pInstance->AvoidObject(*pOpponent))
      isBlocked = true;
  }
};

bool CPythonBackground::CheckAdvancing(CInstanceBase *pInstance) {
  if (!IsMapReady())
    return true;

  Vector3d center;
  float radius;
  pInstance->GetGraphicThingInstanceRef().GetBoundingSphere(center, radius);

  CCullingManager &rkCullingMgr = CCullingManager::Instance();

  CollisionAdjustChecker kCollisionAdjustChecker(pInstance);
  rkCullingMgr.ForInRange(center, radius, &kCollisionAdjustChecker);
  if (kCollisionAdjustChecker.isBlocked) {
    CollisionChecker kCollisionChecker(pInstance);
    rkCullingMgr.ForInRange(center, radius, &kCollisionChecker);
    if (kCollisionChecker.isBlocked) {
      pInstance->BlockMovement();
      return true;
    } else {
      pInstance->NEW_MoveToDestPixelPositionDirection(
          pInstance->NEW_GetDstPixelPositionRef());
    }
    return false;
  }
  return false;
}

void CPythonBackground::RenderSky() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderSky();
}

void CPythonBackground::RenderCloud() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderCloud();
}

void CPythonBackground::RenderWater() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderWater();
}

void CPythonBackground::RenderEffect() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderEffect();
}

void CPythonBackground::RenderBeforeLensFlare() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderBeforeLensFlare();
}

void CPythonBackground::RenderAfterLensFlare() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RenderAfterLensFlare();
}

void CPythonBackground::ClearGuildArea() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.ClearGuildArea();
}

void CPythonBackground::RegisterGuildArea(int isx, int isy, int iex, int iey) {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.RegisterGuildArea(isx, isy, iex, iey);
}

void CPythonBackground::SetCharacterDirLight() {
  if (!IsMapReady())
    return;

  if (!mc_pcurEnvironmentData)
    return;

  STATEMANAGER.SetLight(
      0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_CHARACTER]);
}

void CPythonBackground::SetBackgroundDirLight() {
  if (!IsMapReady())
    return;
  if (!mc_pcurEnvironmentData)
    return;

  STATEMANAGER.SetLight(
      0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND]);
}

void CPythonBackground::GlobalPositionToLocalPosition(int32_t &rGlobalX,
                                                      int32_t &rGlobalY) {
  rGlobalX -= m_dwBaseX;
  rGlobalY -= m_dwBaseY;
}

void CPythonBackground::LocalPositionToGlobalPosition(int32_t &rLocalX,
                                                      int32_t &rLocalY) {
  rLocalX += m_dwBaseX;
  rLocalY += m_dwBaseY;
}

void CPythonBackground::GlobalPositionToLocalPosition(uint32_t &rGlobalX,
                                                      uint32_t &rGlobalY) {
  rGlobalX -= m_dwBaseX;
  rGlobalY -= m_dwBaseY;
}

void CPythonBackground::LocalPositionToGlobalPosition(uint32_t &rLocalX,
                                                      uint32_t &rLocalY) {
  rLocalX += m_dwBaseX;
  rLocalY += m_dwBaseY;
}

void CPythonBackground::RegisterDungeonMapName(const char *c_szMapName) {
  m_kSet_strDungeonMapName.insert(c_szMapName);
}

CPythonBackground::TMapInfo *
CPythonBackground::GlobalPositionToMapInfo(DWORD dwGlobalX, DWORD dwGlobalY) {
  TMapInfoVector::iterator f =
      std::find_if(m_kVct_kMapInfo.begin(), m_kVct_kMapInfo.end(),
                   FFindWarpMapName(dwGlobalX, dwGlobalY));
  if (f == m_kVct_kMapInfo.end())
    return NULL;

  return &(*f);
}

void CPythonBackground::Warp(DWORD dwX, DWORD dwY) {
  TMapInfo *pkMapInfo = GlobalPositionToMapInfo(dwX, dwY);
  if (!pkMapInfo) {
    TraceError("NOT_FOUND_GLOBAL_POSITION(%d, %d)", dwX, dwY);
    return;
  }

  RefreshShadowLevel();
  TMapInfo &rMapInfo = *pkMapInfo;
  assert((dwX >= rMapInfo.m_dwBaseX) && (dwY >= rMapInfo.m_dwBaseY));

  if (!LoadMap(rMapInfo.m_strName, float(dwX - rMapInfo.m_dwBaseX),
               float(dwY - rMapInfo.m_dwBaseY), 0)) {
    // LOAD_MAP_ERROR_HANDLING
    PostQuitMessage(0);
    // END_OF_LOAD_MAP_ERROR_HANDLING
    return;
  }

  CPythonMiniMap::Instance().LoadAtlas();

  m_dwBaseX = rMapInfo.m_dwBaseX;
  m_dwBaseY = rMapInfo.m_dwBaseY;

  m_strMapName = rMapInfo.m_strName;

  SetXMaxTree(m_iXMasTreeGrade);

  if (m_kSet_strDungeonMapName.end() !=
      m_kSet_strDungeonMapName.find(m_strMapName)) {
    EnableTerrainOnlyForHeight();

    CMapOutdoor &rkMap = GetMapOutdoorRef();
    rkMap.EnablePortal(TRUE);
  }

  m_kSet_iShowingPortalID.clear();
  m_kMap_dwTargetID_dwChrID.clear();
  m_kMap_dwID_kReserveTargetEffect.clear();
}

void CPythonBackground::VisibleGuildArea() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.VisibleMarkedArea();

  m_bVisibleGuildArea = TRUE;
}

void CPythonBackground::DisableGuildArea() {
  if (!IsMapReady())
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.DisableMarkedArea();

  m_bVisibleGuildArea = FALSE;
}

const char *CPythonBackground::GetWarpMapName() { return m_strMapName.c_str(); }

void CPythonBackground::ChangeToDay() { m_iDayMode = DAY_MODE_LIGHT; }

void CPythonBackground::ChangeToNight() { m_iDayMode = DAY_MODE_DARK; }

void CPythonBackground::EnableSnowEnvironment() { m_SnowEnvironment.Enable(); }

void CPythonBackground::DisableSnowEnvironment() {
  m_SnowEnvironment.Disable();
}

const D3DXVECTOR3 c_v3TreePos = D3DXVECTOR3(76500.0f, -60900.0f, 20215.0f);

void CPythonBackground::SetXMaxTree(int iGrade) {
  if (!m_pkMap)
    return;

  assert(iGrade >= 0 && iGrade <= 3);
  m_iXMasTreeGrade = iGrade;

  CMapOutdoor &rkMap = GetMapOutdoorRef();

  if ("map_n_snowm_01" != m_strMapName) {
    rkMap.XMasTree_Destroy();
    return;
  }

  if (0 == iGrade) {
    rkMap.XMasTree_Destroy();
    return;
  }

  //////////////////////////////////////////////////////////////////////

  iGrade -= 1;
  iGrade = std::max(iGrade, 0);
  iGrade = std::min(iGrade, 2);

  static std::string s_strTreeName[3] = {
      "d:/ymir work/tree/christmastree1.spt",
      "d:/ymir work/tree/christmastree2.spt",
      "d:/ymir work/tree/christmastree3.spt"};
  static std::string s_strEffectName[3] = {
      "d:/ymir work/effect/etc/christmas_tree/tree_1s.mse",
      "d:/ymir work/effect/etc/christmas_tree/tree_2s.mse",
      "d:/ymir work/effect/etc/christmas_tree/tree_3s.mse",
  };
  rkMap.XMasTree_Set(c_v3TreePos.x, c_v3TreePos.y, c_v3TreePos.z,
                     s_strTreeName[iGrade].c_str(),
                     s_strEffectName[iGrade].c_str());
}

void CPythonBackground::CreateTargetEffect(DWORD dwID, DWORD dwChrVID) {
  m_kMap_dwTargetID_dwChrID.insert(std::make_pair(dwID, dwChrVID));
}

void CPythonBackground::CreateTargetEffect(DWORD dwID, long lx, long ly) {
  if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
    return;

  CMapOutdoor &rkMap = GetMapOutdoorRef();

  DWORD dwBaseX;
  DWORD dwBaseY;
  rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

  int ilx = +(lx - int(dwBaseX));
  int ily = -(ly - int(dwBaseY));

  float fHeight = rkMap.GetHeight(float(ilx), float(ily));

  if (0.0f == fHeight) {
    SReserveTargetEffect ReserveTargetEffect;
    ReserveTargetEffect.ilx = ilx;
    ReserveTargetEffect.ily = ily;
    m_kMap_dwID_kReserveTargetEffect.insert(
        std::make_pair(dwID, ReserveTargetEffect));
    return;
  }

  CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());
}

void CPythonBackground::DeleteTargetEffect(DWORD dwID) {
  if (m_kMap_dwID_kReserveTargetEffect.end() !=
      m_kMap_dwID_kReserveTargetEffect.find(dwID)) {
    m_kMap_dwID_kReserveTargetEffect.erase(dwID);
  }
  if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID)) {
    m_kMap_dwTargetID_dwChrID.erase(dwID);
  }

  DeleteSpecialEffect(dwID);
}

void CPythonBackground::CreateSpecialEffect(DWORD dwID, float fx, float fy,
                                            float fz,
                                            const char *c_szFileName) {
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.SpecialEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialEffect(DWORD dwID) {
  CMapOutdoor &rkMap = GetMapOutdoorRef();
  rkMap.SpecialEffect_Delete(dwID);
}
