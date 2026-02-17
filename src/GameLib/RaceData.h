#pragma once

#include "EterGrnLib/Thing.h"

class CRaceMotionData;
class CAttributeData;

#define COMBO_KEY DWORD
#define MAKE_COMBO_KEY(motion_mode, combo_type)                                \
  ((DWORD(motion_mode) << 16) | (DWORD(combo_type)))
#define COMBO_KEY_GET_MOTION_MODE(key) (WORD(DWORD(key) >> 16 & 0xFFFF))
#define COMBO_KEY_GET_COMBO_TYPE(key) (WORD(DWORD(key) & 0xFFFF))

class CRaceData {
public:
  enum EParts {
    // Share index with server
    // ECharacterEquipmentPart도 수정해주세요.
    // 패킷 크기가 변합니다 서버와 상의후 추가해주세요.
    PART_MAIN,
    PART_WEAPON,
    PART_HEAD,
    PART_WEAPON_LEFT,
    PART_HAIR,

    PART_MAX_NUM,
  };

  enum {
    SMOKE_NUM = 4,
  };

  /////////////////////////////////////////////////////////////////////////////////
  // Graphic Resource

  // Model
  typedef std::map<WORD, CGraphicThing *> TGraphicThingMap;
  typedef std::map<DWORD, std::string> TAttachingBoneNameMap;

  // Motion
  typedef struct SMotion {
    BYTE byPercentage;
    CGraphicThing *pMotion;
    CRaceMotionData *pMotionData;
  } TMotion;
  typedef std::vector<TMotion> TMotionVector;
  typedef std::map<WORD, TMotionVector> TMotionVectorMap;

  typedef struct SMotionModeData {
    WORD wMotionModeIndex;

    TMotionVectorMap MotionVectorMap;

    SMotionModeData() {}
    virtual ~SMotionModeData() {}
  } TMotionModeData;
  typedef std::map<WORD, TMotionModeData *> TMotionModeDataMap;
  typedef TMotionModeDataMap::iterator TMotionModeDataIterator;

  /////////////////////////////////////////////////////////////////////////////////
  // Model Data
  typedef struct SModelData {
    NRaceData::TAttachingDataVector AttachingDataVector;
  } TModelData;
  typedef std::map<DWORD, TModelData> TModelDataMap;
  typedef TModelDataMap::iterator TModelDataMapIterator;

  /////////////////////////////////////////////////////////////////////////////////
  // Motion Data
  typedef std::map<DWORD, CRaceMotionData *> TMotionDataMap;

  /////////////////////////////////////////////////////////////////////////////////
  // Combo Data
  typedef std::vector<DWORD> TComboIndexVector;
  typedef struct SComboAttackData {
    TComboIndexVector ComboIndexVector;
  } TComboData;
  typedef std::map<DWORD, DWORD> TNormalAttackIndexMap;
  typedef std::map<COMBO_KEY, TComboData> TComboAttackDataMap;
  typedef TComboAttackDataMap::iterator TComboAttackDataIterator;

  struct SSkin {
    int m_ePart;

    std::string m_stSrcFileName;
    std::string m_stDstFileName;

    SSkin() { m_ePart = 0; }
    SSkin(const SSkin &c_rkSkin) { Copy(c_rkSkin); }
    void operator=(const SSkin &c_rkSkin) { Copy(c_rkSkin); }
    void Copy(const SSkin &c_rkSkin) {
      m_ePart = c_rkSkin.m_ePart;
      m_stSrcFileName = c_rkSkin.m_stSrcFileName;
      m_stDstFileName = c_rkSkin.m_stDstFileName;
    }
  };

  struct SHair {
    std::string m_stModelFileName;
    std::vector<SSkin> m_kVct_kSkin;
  };

  struct SShape {
    std::string m_stModelFileName;
    std::vector<SSkin> m_kVct_kSkin;
  };

public:
  static CRaceData *New();
  static void Delete(CRaceData *pkRaceData);
  static void CreateSystem(UINT uCapacity, UINT uMotModeCapacity);
  static void DestroySystem();

public:
  CRaceData();
  virtual ~CRaceData();

  void Destroy();

  // Codes For Client
  DWORD GetRaceIndex() const { return m_dwRaceIndex; }
  const char *GetBaseModelFileName() const;
  const char *GetAttributeFileName() const;
  const char *GetMotionListFileName() const;
  CGraphicThing *GetBaseModelThing();
  CGraphicThing *GetLODModelThing();
  CAttributeData *GetAttributeDataPtr();
  BOOL GetAttachingBoneName(DWORD dwPartIndex, const char **c_pszBoneName);
  BOOL CreateMotionModeIterator(TMotionModeDataIterator &itor);
  BOOL NextMotionModeIterator(TMotionModeDataIterator &itor);

  BOOL GetMotionKey(WORD wMotionModeIndex, WORD wMotionIndex,
                    MOTION_KEY *pMotionKey);

  BOOL GetMotionModeDataPointer(WORD wMotionMode,
                                TMotionModeData **ppMotionModeData);
  BOOL GetModelDataPointer(DWORD dwModelIndex,
                           const TModelData **c_ppModelData);
  BOOL GetMotionVectorPointer(WORD wMotionMode, WORD wMotionIndex,
                              const TMotionVector **c_ppMotionVector);
  BOOL GetMotionDataPointer(WORD wMotionMode, WORD wMotionIndex,
                            WORD wMotionSubIndex,
                            CRaceMotionData **ppMotionData);
  BOOL GetMotionDataPointer(DWORD dwMotionKey, CRaceMotionData **ppMotionData);

  DWORD GetAttachingDataCount();
  BOOL
  GetAttachingDataPointer(DWORD dwIndex,
                          const NRaceData::TAttachingData **c_ppAttachingData);
  BOOL
  GetCollisionDataPointer(DWORD dwIndex,
                          const NRaceData::TAttachingData **c_ppAttachingData);
  BOOL GetBodyCollisionDataPointer(
      const NRaceData::TAttachingData **c_ppAttachingData);

  BOOL IsTree();
  const char *GetTreeFileName();

  ///////////////////////////////////////////////////////////////////
  // Setup by Script
  BOOL LoadRaceData(const char *c_szFileName);

  CGraphicThing *RegisterMotionData(WORD wMotionMode, WORD wMotionIndex,
                                    const char *c_szFileName,
                                    BYTE byPercentage = 100);

  ///////////////////////////////////////////////////////////////////
  // Setup by Python
  void SetRace(DWORD dwRaceIndex);
  void RegisterAttachingBoneName(DWORD dwPartIndex, const char *c_szBoneName);

  void RegisterMotionMode(WORD wMotionModeIndex);
  void SetMotionModeParent(WORD wParentMotionModeIndex, WORD wMotionModeIndex);
  void OLD_RegisterMotion(WORD wMotionModeIndex, WORD wMotionIndex,
                          const char *c_szFileName, BYTE byPercentage = 100);
  CGraphicThing *NEW_RegisterMotion(CRaceMotionData *pkMotionData,
                                    WORD wMotionModeIndex, WORD wMotionIndex,
                                    const char *c_szFileName,
                                    BYTE byPercentage = 100);
  bool SetMotionRandomWeight(WORD wMotionModeIndex, WORD wMotionIndex,
                             WORD wMotionSubIndex, BYTE byPercentage);

  void RegisterNormalAttack(WORD wMotionModeIndex, WORD wMotionIndex);
  BOOL GetNormalAttackIndex(WORD wMotionModeIndex, WORD *pwMotionIndex);

  void ReserveComboAttack(WORD wMotionModeIndex, WORD wComboType,
                          DWORD dwComboCount);
  void RegisterComboAttack(WORD wMotionModeIndex, WORD wComboType,
                           DWORD dwComboIndex, WORD wMotionIndex);
  BOOL GetComboDataPointer(WORD wMotionModeIndex, WORD wComboType,
                           TComboData **ppComboData);

  void SetShapeModel(UINT eShape, const char *c_szModelFileName);
  void AppendShapeSkin(UINT eShape, UINT ePart, const char *c_szSrcFileName,
                       const char *c_szDstFileName);

  void SetHairSkin(UINT eHair, UINT ePart, const char *c_szModelFileName,
                   const char *c_szSrcFileName, const char *c_szDstFileName);

  /////

  DWORD GetSmokeEffectID(UINT eSmoke);

  const std::string &GetSmokeBone();

  SHair *FindHair(UINT eHair);
  SShape *FindShape(UINT eShape);

protected:
  void __Initialize();

  void __OLD_RegisterMotion(WORD wMotionMode, WORD wMotionIndex,
                            const TMotion &rMotion);

  BOOL GetMotionVectorPointer(WORD wMotionMode, WORD wMotionIndex,
                              TMotionVector **ppMotionVector);

protected:
  DWORD m_dwRaceIndex;
  DWORD m_adwSmokeEffectID[SMOKE_NUM];

  CGraphicThing *m_pBaseModelThing;
  CGraphicThing *m_pLODModelThing;

  std::string m_strBaseModelFileName;
  std::string m_strTreeFileName;
  std::string m_strAttributeFileName;
  std::string m_strMotionListFileName;
  std::string m_strSmokeBoneName;

  TModelDataMap m_ModelDataMap;
  TMotionModeDataMap m_pMotionModeDataMap;
  TAttachingBoneNameMap m_AttachingBoneNameMap;
  TComboAttackDataMap m_ComboAttackDataMap;
  TNormalAttackIndexMap m_NormalAttackIndexMap;

  std::map<DWORD, SHair> m_kMap_dwHairKey_kHair;
  std::map<DWORD, SShape> m_kMap_dwShapeKey_kShape;

  NRaceData::TAttachingDataVector m_AttachingDataVector;

protected:
  static CDynamicPool<TMotionModeData> ms_MotionModeDataPool;
  static CDynamicPool<CRaceData> ms_kPool;
};
