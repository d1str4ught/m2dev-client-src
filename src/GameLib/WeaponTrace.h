#pragma once

// change CatMull to cubic spline
#include "EterGrnLib/ThingInstance.h"

class CWeaponTrace {
  /*
  public:
          typedef struct SSplineValue
          {
                  float fValue1;
                  float fValue2;
                  float fValue3;
          } TSplineValue;
  */
public:
  static void DestroySystem();
  static void Delete(CWeaponTrace *pkWTDel);
  static CWeaponTrace *New();

public:
  CWeaponTrace();
  virtual ~CWeaponTrace();

  void Clear();

  void TurnOn();
  void TurnOff();

  void UseAlpha();
  void UseTexture();

  void SetTexture(const char *c_szFileName);
  bool SetWeaponInstance(CGraphicThingInstance *pInstance, DWORD dwModelIndex,
                         const char *c_szBoneName);
  void SetPosition(float fx, float fy, float fz);
  void SetRotation(float fRotation);

  void SetLifeTime(float fLifeTime);
  void SetSamplingTime(float fSamplingTime);

  void Update(float fReachScale);
  void Render();

  void Initialize();

protected:
  bool BuildVertex();

protected:
  float m_fLastUpdate;

  typedef std::pair<float, D3DXVECTOR3> TTimePoint;
  typedef std::deque<TTimePoint> TTimePointList;
  TTimePointList m_ShortTimePointList;
  TTimePointList m_LongTimePointList;

  std::vector<TPDTVertex> m_PDTVertexVector;

  float m_fLifeTime;
  float m_fSamplingTime;

  // std::vector<TPDTVertex> m_PDTVertexVector;
  // std::vector<TPDTVertex> m_CurvingTraceVector;
  // std::vector<TSplineValue> m_SplineValueVector;

  CGraphicThingInstance *m_pInstance;
  DWORD m_dwModelInstanceIndex;

  CGraphicImageInstance m_ImageInstance;

  float m_fx;
  float m_fy;
  float m_fz;
  float m_fRotation;
  float m_fLength;

  BOOL m_isPlaying;
  bool m_bUseTexture;

  int m_iBoneIndex;

protected:
  static CDynamicPool<CWeaponTrace> ms_kPool;
};
