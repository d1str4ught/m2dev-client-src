#pragma once

#include <vector>
#include "AttributeData.h"
#include "Pool.h"

class CAttributeInstance {
public:
  CAttributeInstance();
  virtual ~CAttributeInstance();

  void Clear();
  BOOL IsEmpty() const;

  const char *GetDataFileName() const;

  // NOTE : Object 전용
  void SetObjectPointer(CAttributeData *pAttributeData);
  void RefreshObject(const D3DXMATRIX &c_rmatGlobal);
  CAttributeData *GetObjectPointer() const;

  bool Picking(const D3DXVECTOR3 &v, const D3DXVECTOR3 &dir, float &out_x,
               float &out_y);

  BOOL IsInHeight(float fx, float fy);
  BOOL GetHeight(float fx, float fy, float *pfHeight);

  BOOL IsHeightData() const;

protected:
  void SetGlobalMatrix(const D3DXMATRIX &c_rmatGlobal);
  void SetGlobalPosition(const D3DXVECTOR3 &c_rv3Position);

protected:
  float m_fCollisionRadius;
  float m_fHeightRadius;

  D3DXMATRIX m_matGlobal;

  std::vector<std::vector<D3DXVECTOR3>> m_v3HeightDataVector;

  CAttributeData::TRef m_roAttributeData;

  /*
  BOOL m_isHeightCached;
  struct SHeightCacheData
  {
          float fxMin;
          float fyMin;
          float fxMax;
          float fyMax;
          DWORD dwxStep;
          DWORD dwyStep;
          std::vector<float> kVec_fHeight;
  } m_kHeightCacheData;
  */

public:
  static void CreateSystem(UINT uCapacity);
  static void DestroySystem();

  static CAttributeInstance *New();
  static void Delete(CAttributeInstance *pkInst);

  static CDynamicPool<CAttributeInstance> ms_kPool;
};
