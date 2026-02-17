#pragma once

class CFlyingData //: public CPooledObject<CFlyingData>
{
public:
  enum {
    FLY_ATTACH_NONE,
    FLY_ATTACH_EFFECT,
    FLY_ATTACH_OBJECT, // NOT IMPLEMENTED
  };

  enum {
    FLY_ATTACH_TYPE_NONE,
    FLY_ATTACH_TYPE_LINE,
    FLY_ATTACH_TYPE_MULTI_LINE,
    FLY_ATTACH_TYPE_SINE,
    FLY_ATTACH_TYPE_EXP,
  };

  struct TFlyingAttachData {
    int iType;
    int iFlyType;
    std::string strFilename;

    bool bHasTail;
    DWORD dwTailColor;
    float fTailLength;
    float fTailSize;
    bool bRectShape;

    float fRoll;
    // nothing for LINE
    // for MULTI_LINE
    float fDistance;
    // for SINE, EXP
    float fPeriod;
    float fAmplitude;
  };

  friend class CFlyingInstance;

  friend class CSceneFly;
  friend class CFlyTabPage;

  CFlyingData();
  virtual ~CFlyingData();

  void Destroy();

  bool LoadScriptFile(const char *c_szFilename);
  bool SaveScriptFile(const char *c_szFilename);

  const D3DXVECTOR3 &GetAcceleration() { return m_v3Accel; }

  void SetBombEffect(const char *szEffectName);

  DWORD AttachFlyEffect(int iType, const std::string &strFilename, float fRoll,
                        float fArg1, float fArg2);

  TFlyingAttachData &GetAttachDataReference(int iIndex);
  int GetAttachDataCount() { return m_AttachDataVector.size(); }

  void DuplicateAttach(int iIndex);
  void RemoveAttach(int iIndex);
  void RemoveAllAttach();

protected:
  void __Initialize();

  std::string m_strFilename;

  bool m_bSpreading;
  bool m_bMaintainParallel;

  float m_fInitVel;
  float m_fConeAngle;
  float m_fRollAngle;

  D3DXVECTOR3 m_v3AngVel;

  float m_fGravity;

  float m_fBombRange;
  std::string m_strBombEffectName;
  DWORD m_dwBombEffectID;

  bool m_bIsHoming;
  float m_fHomingMaxAngle;
  float m_fHomingStartTime;

  bool m_bHitOnBackground;
  bool m_bHitOnAnotherMonster;
  int m_iPierceCount;
  float m_fCollisionSphereRadius;

  float m_fRange;
  D3DXVECTOR3 m_v3Accel;

  std::vector<TFlyingAttachData> m_AttachDataVector;

public:
  static CFlyingData *New() { return ms_kPool.Alloc(); }
  static void Delete(CFlyingData *pData) {
    pData->Destroy();
    ms_kPool.Free(pData);
  }

  static void DestroySystem() { ms_kPool.Destroy(); }

  static CDynamicPool<CFlyingData> ms_kPool;
};