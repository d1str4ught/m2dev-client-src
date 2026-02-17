// TerrainPatch.h: interface for the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(                                                                  \
    AFX_TERRAINPATCH_H__CDD52438_D542_433C_8748_3A15C910A65E__INCLUDED_)
#define AFX_TERRAINPATCH_H__CDD52438_D542_433C_8748_3A15C910A65E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EterLib/GrpVertexBuffer.h"
#include "PRTerrainLib/Terrain.h"

#pragma pack(push)
#pragma pack(1)

struct HardwareTransformPatch_SSourceVertex {
  D3DXVECTOR3 kPosition;
  D3DXVECTOR3 kNormal;
};

struct SoftwareTransformPatch_SSourceVertex {
  D3DXVECTOR3 kPosition;
  D3DXVECTOR3 kNormal;
  DWORD dwDiffuse;
};

struct SWaterVertex {
  float x, y, z; // position
  DWORD dwDiffuse;
};
#pragma pack(pop)

class CTerrainPatch {
public:
  enum {
    PATCH_TYPE_PLAIN = 0,
    PATCH_TYPE_HILL,
    PATCH_TYPE_CLIFF,
  };

  enum {
    TERRAIN_VERTEX_COUNT =
        (CTerrainImpl::PATCH_XSIZE + 1) * (CTerrainImpl::PATCH_YSIZE + 1)
  };

  static bool SOFTWARE_TRANSFORM_PATCH_ENABLE;

public:
  CTerrainPatch() { Clear(); }
  ~CTerrainPatch() { Clear(); }

  void Clear();

  void ClearID() { SetID(0xFFFFFFFF); }

  void SetMinX(float fMinX) { m_fMinX = fMinX; }
  float GetMinX() { return m_fMinX; }

  void SetMaxX(float fMaxX) { m_fMaxX = fMaxX; }
  float GetMaxX() { return m_fMaxX; }

  void SetMinY(float fMinY) { m_fMinY = fMinY; }
  float GetMinY() { return m_fMinY; }

  void SetMaxY(float fMaxY) { m_fMaxY = fMaxY; }
  float GetMaxY() { return m_fMaxY; }

  void SetMinZ(float fMinZ) { m_fMinZ = fMinZ; }
  float GetMinZ() { return m_fMinZ; }

  void SetMaxZ(float fMaxZ) { m_fMaxZ = fMaxZ; }
  float GetMaxZ() { return m_fMaxZ; }

  bool IsUse() { return m_bUse; }
  void SetUse(bool bUse) { m_bUse = bUse; }

  bool IsWaterExist() { return m_bWaterExist; }
  void SetWaterExist(bool bWaterExist) { m_bWaterExist = bWaterExist; }

  DWORD GetID() { return m_dwID; }
  void SetID(DWORD dwID) { m_dwID = dwID; }

  void SetType(BYTE byType) { m_byType = byType; }
  BYTE GetType() { return m_byType; }

  void NeedUpdate(bool bNeedUpdate) { m_bNeedUpdate = bNeedUpdate; }
  bool NeedUpdate() { return m_bNeedUpdate; }

  UINT GetWaterFaceCount();

  void
  SoftwareTransformPatch_UpdateTerrainLighting(DWORD dwVersion,
                                               const D3DLIGHT9 &c_rkLight,
                                               const D3DMATERIAL9 &c_rkMtrl);

  void
  BuildTerrainVertexBuffer(HardwareTransformPatch_SSourceVertex *akSrcVertex);
  void BuildWaterVertexBuffer(SWaterVertex *akSrcVertex,
                              UINT uWaterVertexCount);

protected:
  void __BuildHardwareTerrainVertexBuffer(
      HardwareTransformPatch_SSourceVertex *akSrcVertex);
  void __BuildSoftwareTerrainVertexBuffer(
      HardwareTransformPatch_SSourceVertex *akSrcVertex);

private:
  float m_fMinX;
  float m_fMaxX;
  float m_fMinY;
  float m_fMaxY;
  float m_fMinZ;
  float m_fMaxZ;
  bool m_bUse;
  bool m_bWaterExist;
  DWORD m_dwID;
  DWORD m_dwWaterPriCount;

  CGraphicVertexBuffer m_WaterVertexBuffer;
  BYTE m_byType;

  bool m_bNeedUpdate;
  DWORD m_dwVersion;

public:
  CGraphicVertexBuffer *GetWaterVertexBufferPointer() {
    return &m_WaterVertexBuffer;
  }

public:
  CGraphicVertexBuffer *HardwareTransformPatch_GetVertexBufferPtr() {
    return &m_kHT.m_kVB;
  }

protected:
  struct SHardwareTransformPatch {
    CGraphicVertexBuffer m_kVB;
  } m_kHT;

public:
  SoftwareTransformPatch_SSourceVertex *
  SoftwareTransformPatch_GetTerrainVertexDataPtr() {
    return m_kST.m_akTerrainVertex;
  }

protected:
  struct SSoftwareTransformPatch {
    SoftwareTransformPatch_SSourceVertex *m_akTerrainVertex;

    SSoftwareTransformPatch();
    ~SSoftwareTransformPatch();

    void Create();
    void Destroy();

    void __Initialize();
  } m_kST;
};

class CTerrainPatchProxy {
public:
  CTerrainPatchProxy();
  virtual ~CTerrainPatchProxy();

  void Clear();

  void SetCenterPosition(const D3DXVECTOR3 &c_rv3Center);

  bool IsIn(const D3DXVECTOR3 &c_rv3Target, float fRadius);

  bool isUsed() { return m_bUsed; }
  void SetUsed(bool bUsed) { m_bUsed = bUsed; }

  short GetPatchNum() { return m_sPatchNum; }
  void SetPatchNum(short sPatchNum) { m_sPatchNum = sPatchNum; }

  BYTE GetTerrainNum() { return m_byTerrainNum; }
  void SetTerrainNum(BYTE byTerrainNum) { m_byTerrainNum = byTerrainNum; }

  void SetTerrainPatch(CTerrainPatch *pTerrainPatch) {
    m_pTerrainPatch = pTerrainPatch;
  }

  bool isWaterExists();

  UINT GetWaterFaceCount();

  float GetMinX();
  float GetMaxX();
  float GetMinY();
  float GetMaxY();
  float GetMinZ();
  float GetMaxZ();

  // Vertex Buffer
  CGraphicVertexBuffer *GetWaterVertexBufferPointer();
  SoftwareTransformPatch_SSourceVertex *
  SoftwareTransformPatch_GetTerrainVertexDataPtr();
  CGraphicVertexBuffer *HardwareTransformPatch_GetVertexBufferPtr();

  void
  SoftwareTransformPatch_UpdateTerrainLighting(DWORD dwVersion,
                                               const D3DLIGHT9 &c_rkLight,
                                               const D3DMATERIAL9 &c_rkMtrl);

protected:
  bool m_bUsed;
  short m_sPatchNum; // Patch Number

  BYTE m_byTerrainNum;

  CTerrainPatch *m_pTerrainPatch;

  D3DXVECTOR3 m_v3Center;
};

inline bool CTerrainPatchProxy::isWaterExists() {
  return m_pTerrainPatch->IsWaterExist();
}

inline float CTerrainPatchProxy::GetMinX() {
  return m_pTerrainPatch->GetMinX();
}

inline float CTerrainPatchProxy::GetMaxX() {
  return m_pTerrainPatch->GetMaxX();
}

inline float CTerrainPatchProxy::GetMinY() {
  return m_pTerrainPatch->GetMinY();
}

inline float CTerrainPatchProxy::GetMaxY() {
  return m_pTerrainPatch->GetMaxY();
}

inline float CTerrainPatchProxy::GetMinZ() {
  return m_pTerrainPatch->GetMinZ();
}

inline float CTerrainPatchProxy::GetMaxZ() {
  return m_pTerrainPatch->GetMaxZ();
}

inline CGraphicVertexBuffer *CTerrainPatchProxy::GetWaterVertexBufferPointer() {
  return m_pTerrainPatch->GetWaterVertexBufferPointer();
}

#endif // !defined(AFX_TERRAINPATCH_H__CDD52438_D542_433C_8748_3A15C910A65E__INCLUDED_)
