#pragma once

#include "SpeedTreeLib/SpeedTreeWrapper.h"
#include "EterGrnLib/ThingInstance.h"
#include "MapType.h"
#include "DungeonBlock.h"
#include "SpeedTreeLib/SpeedTreeForest.h"

class CMapOutdoor;
class CEffectInstance;

class CArea {
public:
  enum {
    ROTATION_STEP_COUNT = 24,
    ROTATION_STEP_AMOUNT = 360 / ROTATION_STEP_COUNT,
    YAW_STEP_COUNT = 24,
    YAW_STEP_AMOUNT = 360 / YAW_STEP_COUNT,
    PITCH_STEP_COUNT = YAW_STEP_COUNT,
    PITCH_STEP_AMOUNT = YAW_STEP_AMOUNT,
  };

  typedef D3DXVECTOR3 TObjectPosition;

  ///////////////////////////////////////////////////////////////////////////////////
  // Object Static Data
  typedef struct SObjectData {
    TObjectPosition Position;
    DWORD dwCRC;
    BYTE abyPortalID[PORTAL_ID_MAX_NUM];

    // For Tree
    //  - Nothing

    // For Granny Thing
    float m_fYaw;
    float m_fPitch;
    float m_fRoll;
    float m_fHeightBias;

    // For Effect
    //  - Nothing

    // For Ambience
    DWORD dwRange;
    float fMaxVolumeAreaPercentage;

    void InitializeRotation();
  } TObjectData;

  struct ObjectDataComp {
    bool operator()(TObjectData &lhs, TObjectData &rhs) const {
      return lhs.dwCRC < rhs.dwCRC;
    }
  };

  typedef std::vector<TObjectData> TObjectDataVector;
  typedef TObjectDataVector::iterator TObjectDataIterator;

  ///////////////////////////////////////////////////////////////////////////////////
  // Object Dynamic Data
  typedef struct SAmbienceInstance : public CScreen {
    float fx, fy, fz;
    DWORD dwRange;
    float fMaxVolumeAreaPercentage;
    MaSoundInstance *playSoundInstance;
    float fNextPlayTime;
    prt::TPropertyAmbience AmbienceData;

    SAmbienceInstance();
    virtual ~SAmbienceInstance() {}
    void __Update(float fxCenter, float fyCenter, float fzCenter);
    void (SAmbienceInstance::*Update)(float fxCenter, float fyCenter,
                                      float fzCenter);
    void UpdateOnceSound(float fxCenter, float fyCenter, float fzCenter);
    void UpdateStepSound(float fxCenter, float fyCenter, float fzCenter);
    void UpdateLoopSound(float fxCenter, float fyCenter, float fzCenter);
    void Render();
    bool Picking();
    float __GetVolumeFromDistance(float fDistance);
  } TAmbienceInstance;

  typedef struct SObjectInstance {
    DWORD dwType;
    CAttributeInstance *pAttributeInstance;

    // Data For Tree
    CSpeedTreeForest::SpeedTreeWrapperPtr pTree;

    // Data For Normal Object or Building
    BOOL isShadowFlag;
    CGraphicThingInstance *pThingInstance;

    // Data for Effect
    DWORD dwEffectID;
    DWORD dwEffectInstanceIndex;

    // Data For Ambience
    TAmbienceInstance *pAmbienceInstance;

    // Dungeon
    CDungeonBlock *pDungeonBlock;

    void Clear() {
      if (pTree)
        pTree->Clear();

      if (pThingInstance)
        pThingInstance->Clear();

      if (pAttributeInstance)
        pAttributeInstance->Clear();

      if (pAmbienceInstance)
        pAmbienceInstance->Clear();

      if (pDungeonBlock)
        pDungeonBlock->Clear();

      __Initialize();
    }

    void SetTree(float x, float y, float z, DWORD dwTreeCRC,
                 const char *c_szTreeName);

    void __Initialize() {
      dwType = prt::PROPERTY_TYPE_NONE;

      dwEffectID = 0xffffffff;
      dwEffectInstanceIndex = 0xffffffff;
      isShadowFlag = false;

      pTree = NULL;
      pThingInstance = NULL;
      pAttributeInstance = NULL;
      pAmbienceInstance = NULL;
      pDungeonBlock = NULL;
    }

    SObjectInstance() { __Initialize(); }
    virtual ~SObjectInstance() {}
  } TObjectInstance;

  typedef std::vector<CSpeedTreeForest::SpeedTreeWrapperPtr>
      TTreeInstanceVector;
  typedef std::vector<CGraphicThingInstance *> TThingInstanceVector;
  typedef std::vector<CDungeonBlock *> TDungeonBlockInstanceVector;
  typedef std::vector<TObjectInstance *> TObjectInstanceVector;
  typedef std::vector<TAmbienceInstance *> TAmbienceInstanceVector;
  typedef std::map<CGraphicThingInstance *, DWORD>
      TGraphicThingInstanceCRCMap; // 최적화용 by 동현
  typedef std::set<int> TShowingPortalIDSet;

  typedef struct SCRCWithNumber {
    DWORD dwCRC;
    DWORD dwNumber;
  } TCRCWithNumber;

  typedef std::vector<TCRCWithNumber> TCRCWithNumberVector;

  struct FFindIfCRC {

    FFindIfCRC(DWORD dwCRC) { m_dwCRC = dwCRC; }

    bool operator()(TCRCWithNumber &rCRCWithNumber) {
      if (rCRCWithNumber.dwCRC == m_dwCRC)
        return true;
      return false;
    }

    DWORD m_dwCRC;
  };

  struct CRCNumComp {
    bool operator()(TCRCWithNumber &lhs, TCRCWithNumber &rhs) const {
      return lhs.dwNumber > rhs.dwNumber;
    }
  };

public:
  CArea();
  virtual ~CArea();

  void CollectRenderingObject(
      std::vector<CGraphicThingInstance *> &rkVct_pkOpaqueThingInst);
  void CollectBlendRenderingObject(
      std::vector<CGraphicThingInstance *> &rkVct_pkBlendThingInst);

  void SetMapOutDoor(CMapOutdoor *pOwnerOutdoorMap);
  void Clear();

  bool Load(const char *c_szPathName);

  DWORD GetObjectDataCount();
  bool GetObjectDataPointer(DWORD dwIndex,
                            const TObjectData **ppObjectData) const;

  const DWORD GetObjectInstanceCount() const;
  const bool
  GetObjectInstancePointer(const DWORD &dwIndex,
                           const TObjectInstance **ppObjectInstance) const;

  void EnablePortal(BOOL bFlag = TRUE);
  void ClearPortal();
  void AddShowingPortalID(int iNum);
  void RefreshPortal();

  //////////////////////////////////////////////////////////////////////////

  void Update();
  void UpdateAroundAmbience(float fX, float fY, float fZ);

  void Render();
  void RenderEffect();
  void RenderCollision();
  void RenderAmbience();
  void RenderDungeon();
  void Refresh();

  //////////////////////////////////////////////////////////////////////////

  void SetCoordinate(const WORD &c_usCoordX, const WORD &c_usCoordY);
  void GetCoordinate(WORD *usCoordX, WORD *usCoordY);

  //////////////////////////////////////////////////////////////////////////

  DWORD DEBUG_GetRenderedCRCNum();
  TCRCWithNumberVector &DEBUG_GetRenderedCRCWithNumVector();
  DWORD DEBUG_GetRenderedGrapphicThingInstanceNum();

  CMapOutdoor *GetOwner() { return m_pOwnerOutdoorMap; }

protected:
  bool CheckObjectIndex(DWORD dwIndex) const;

  bool __Load_LoadObject(const char *c_szFileName);
  bool __Load_LoadAmbience(const char *c_szFileName);
  void __Load_BuildObjectInstances();

  void __UpdateAniThingList();
  void __UpdateEffectList();

  void __LoadAttribute(TObjectInstance *pObjectInstance,
                       const char *c_szAttributeFileName);

protected:
  void __Clear_DestroyObjectInstance(TObjectInstance *pObjectInstance);

  void __SetObjectInstance(TObjectInstance *pObjectInstance,
                           const TObjectData *c_pData);
  void __SetObjectInstance_SetTree(TObjectInstance *pObjectInstance,
                                   const TObjectData *c_pData,
                                   CProperty *pProperty);
  void __SetObjectInstance_SetBuilding(TObjectInstance *pObjectInstance,
                                       const TObjectData *c_pData,
                                       CProperty *pProperty);
  void __SetObjectInstance_SetEffect(TObjectInstance *pObjectInstance,
                                     const TObjectData *c_pData,
                                     CProperty *pProperty);
  void __SetObjectInstance_SetAmbience(TObjectInstance *pObjectInstance,
                                       const TObjectData *c_pData,
                                       CProperty *pProperty);
  void __SetObjectInstance_SetDungeonBlock(TObjectInstance *pObjectInstance,
                                           const TObjectData *c_pData,
                                           CProperty *pProperty);

protected:
  // Static Data
  TObjectDataVector m_ObjectDataVector; // Area 상에 있는 오브젝트들

  // Rendering Instances
  TObjectInstanceVector m_ObjectInstanceVector;

  // Clone Instance Vector
  TTreeInstanceVector m_TreeCloneInstaceVector;
  TThingInstanceVector m_ThingCloneInstaceVector;
  TDungeonBlockInstanceVector m_DungeonBlockCloneInstanceVector;
  TThingInstanceVector m_AniThingCloneInstanceVector;
  TThingInstanceVector m_ShadowThingCloneInstaceVector;
  TAmbienceInstanceVector m_AmbienceCloneInstanceVector;
  TGraphicThingInstanceCRCMap m_GraphicThingInstanceCRCMap;
  TCRCWithNumberVector m_kRenderedThingInstanceCRCWithNumberVector;
  TThingInstanceVector m_kRenderedGrapphicThingInstanceVector;

  // 좌표 : 심리스 맵 전체 중에서의 좌표... 둘다 0~999
  WORD m_wX;
  WORD m_wY;

  // Owner COutdoorMap poineter
  CMapOutdoor *m_pOwnerOutdoorMap;

protected:
  static CDynamicPool<TObjectInstance> ms_ObjectInstancePool;
  static CDynamicPool<CAttributeInstance> ms_AttributeInstancePool;
  static CDynamicPool<TAmbienceInstance> ms_AmbienceInstancePool;
  static CDynamicPool<CDungeonBlock> ms_DungeonBlockInstancePool;

  typedef std::map<DWORD, CEffectInstance *> TEffectInstanceMap;
  typedef TEffectInstanceMap::iterator TEffectInstanceIterator;
  TEffectInstanceMap m_EffectInstanceMap;

  BOOL m_bPortalEnable;
  TShowingPortalIDSet m_kSet_ShowingPortalID;

public:
  static void DestroySystem();

  static CArea *New();
  static void Delete(CArea *pkArea);

  static CDynamicPool<CArea> ms_kPool;
};
