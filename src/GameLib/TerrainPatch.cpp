// TerrainPatch.cpp: implementation of the CTerrainPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TerrainPatch.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTerrainPatch::SSoftwareTransformPatch::SSoftwareTransformPatch() {
  __Initialize();
}

CTerrainPatch::SSoftwareTransformPatch::~SSoftwareTransformPatch() {
  Destroy();
}

void CTerrainPatch::SSoftwareTransformPatch::Create() {
  assert(NULL == m_akTerrainVertex);
  m_akTerrainVertex =
      new SoftwareTransformPatch_SSourceVertex[TERRAIN_VERTEX_COUNT];
}

void CTerrainPatch::SSoftwareTransformPatch::Destroy() {
  if (m_akTerrainVertex)
    delete[] m_akTerrainVertex;

  __Initialize();
}

void CTerrainPatch::SSoftwareTransformPatch::__Initialize() {
  m_akTerrainVertex = NULL;
}

bool CTerrainPatch::SOFTWARE_TRANSFORM_PATCH_ENABLE = TRUE;

void CTerrainPatch::Clear() {
  m_kHT.m_kVB.Destroy();
  m_kST.Destroy();

  m_WaterVertexBuffer.Destroy();
  ClearID();
  SetUse(false);

  m_bWaterExist = false;
  m_bNeedUpdate = true;

  m_dwWaterPriCount = 0;
  m_byType = PATCH_TYPE_PLAIN;

  m_fMinX = m_fMaxX = m_fMinY = m_fMaxY = m_fMinZ = m_fMaxZ = 0.0f;

  m_dwVersion = 0;
}

void CTerrainPatch::BuildWaterVertexBuffer(SWaterVertex *akSrcVertex,
                                           UINT uWaterVertexCount) {
  CGraphicVertexBuffer &rkVB = m_WaterVertexBuffer;

  if (!rkVB.Create(uWaterVertexCount, D3DFVF_XYZ | D3DFVF_DIFFUSE,
                   D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT))
    return;

  SWaterVertex *akDstWaterVertex;
  if (rkVB.Lock((void **)&akDstWaterVertex)) {
    UINT uVBSize = sizeof(SWaterVertex) * uWaterVertexCount;
    memcpy(akDstWaterVertex, akSrcVertex, uVBSize);
    m_dwWaterPriCount = uWaterVertexCount / 3;

    rkVB.Unlock();
  }
}

void CTerrainPatch::BuildTerrainVertexBuffer(
    HardwareTransformPatch_SSourceVertex *akSrcVertex) {
  if (SOFTWARE_TRANSFORM_PATCH_ENABLE)
    __BuildSoftwareTerrainVertexBuffer(akSrcVertex);
  else
    __BuildHardwareTerrainVertexBuffer(akSrcVertex);
}

void CTerrainPatch::__BuildSoftwareTerrainVertexBuffer(
    HardwareTransformPatch_SSourceVertex *akSrcVertex) {
  // DWORD
  // dwVBSize=sizeof(HardwareTransformPatch_SSourceVertex)*TERRAIN_VERTEX_COUNT;

  m_kST.Create();

  SoftwareTransformPatch_SSourceVertex *akDstVertex =
      SoftwareTransformPatch_GetTerrainVertexDataPtr();
  for (UINT uIndex = 0; uIndex != TERRAIN_VERTEX_COUNT; ++uIndex) {
    *((HardwareTransformPatch_SSourceVertex *)(akDstVertex + uIndex)) =
        *(akSrcVertex + uIndex);
    akDstVertex[uIndex].dwDiffuse = 0xFFFFFFFF;
  }
}

void CTerrainPatch::__BuildHardwareTerrainVertexBuffer(
    HardwareTransformPatch_SSourceVertex *akSrcVertex) {

  CGraphicVertexBuffer &rkVB = m_kHT.m_kVB;
  if (!rkVB.Create(TERRAIN_VERTEX_COUNT, D3DFVF_XYZ | D3DFVF_NORMAL,
                   D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT))
    return;

  HardwareTransformPatch_SSourceVertex *akDstVertex;
  if (rkVB.Lock((void **)&akDstVertex)) {
    UINT uVBSize =
        sizeof(HardwareTransformPatch_SSourceVertex) * TERRAIN_VERTEX_COUNT;

    memcpy(akDstVertex, akSrcVertex, uVBSize);
    rkVB.Unlock();
  }
}

void CTerrainPatch::SoftwareTransformPatch_UpdateTerrainLighting(
    DWORD dwVersion, const D3DLIGHT9 &c_rkLight, const D3DMATERIAL9 &c_rkMtrl) {
  if (m_dwVersion == dwVersion)
    return;

  m_dwVersion = dwVersion;

  SoftwareTransformPatch_SSourceVertex *akSrcVertex =
      SoftwareTransformPatch_GetTerrainVertexDataPtr();
  if (!akSrcVertex)
    return;

  D3DXVECTOR3 kLightDir = c_rkLight.Direction;

  DWORD dwDot;
  DWORD dwAmbientR =
      (c_rkMtrl.Ambient.r * c_rkLight.Ambient.r + c_rkMtrl.Emissive.r) * 255.0f;
  DWORD dwAmbientG =
      (c_rkMtrl.Ambient.g * c_rkLight.Ambient.g + c_rkMtrl.Emissive.g) * 255.0f;
  DWORD dwAmbientB =
      (c_rkMtrl.Ambient.b * c_rkLight.Ambient.b + c_rkMtrl.Emissive.b) * 255.0f;
  DWORD dwDiffuseR = (c_rkMtrl.Diffuse.r * c_rkLight.Diffuse.r) * 255.0f;
  DWORD dwDiffuseG = (c_rkMtrl.Diffuse.g * c_rkLight.Diffuse.g) * 255.0f;
  DWORD dwDiffuseB = (c_rkMtrl.Diffuse.b * c_rkLight.Diffuse.b) * 255.0f;

  if (dwDiffuseR > 255 - dwAmbientR)
    dwDiffuseR = 255 - dwAmbientR;

  if (dwDiffuseG + dwAmbientG > 255)
    dwDiffuseG = 255 - dwAmbientG;

  if (dwDiffuseB + dwAmbientB > 255)
    dwDiffuseB = 255 - dwAmbientB;

  for (UINT uIndex = 0; uIndex != CTerrainPatch::TERRAIN_VERTEX_COUNT;
       ++uIndex) {
    float fDot = D3DXVec3Dot(&akSrcVertex[uIndex].kNormal, &kLightDir);

    const float N = 0xffffff;
    const int S = 24;
    if (fDot < 0.0f)
      dwDot = (N * -fDot);
    else
      dwDot = (N * +fDot);

    akSrcVertex[uIndex].dwDiffuse =
        (0xff000000) | (((dwDiffuseR * dwDot >> S) + dwAmbientR) << 16) |
        (((dwDiffuseG * dwDot >> S) + dwAmbientG) << 8) |
        ((dwDiffuseB * dwDot >> S) + dwAmbientB);
  }
}

UINT CTerrainPatch::GetWaterFaceCount() { return m_dwWaterPriCount; }

CTerrainPatchProxy::CTerrainPatchProxy() { Clear(); }

CTerrainPatchProxy::~CTerrainPatchProxy() { Clear(); }

void CTerrainPatchProxy::SetCenterPosition(const D3DXVECTOR3 &c_rv3Center) {
  m_v3Center = c_rv3Center;
}

bool CTerrainPatchProxy::IsIn(const D3DXVECTOR3 &c_rv3Target, float fRadius) {
  float dx = m_v3Center.x - c_rv3Target.x;
  float dy = m_v3Center.y - c_rv3Target.y;
  float fDist = dx * dx + dy * dy;
  float fCheck = fRadius * fRadius;

  if (fDist < fCheck)
    return true;

  return false;
}

void CTerrainPatchProxy::SoftwareTransformPatch_UpdateTerrainLighting(
    DWORD dwVersion, const D3DLIGHT9 &c_rkLight, const D3DMATERIAL9 &c_rkMtrl) {
  if (m_pTerrainPatch)
    m_pTerrainPatch->SoftwareTransformPatch_UpdateTerrainLighting(
        dwVersion, c_rkLight, c_rkMtrl);
}

CGraphicVertexBuffer *
CTerrainPatchProxy::HardwareTransformPatch_GetVertexBufferPtr() {
  if (m_pTerrainPatch)
    return m_pTerrainPatch->HardwareTransformPatch_GetVertexBufferPtr();

  return NULL;
}

SoftwareTransformPatch_SSourceVertex *
CTerrainPatchProxy::SoftwareTransformPatch_GetTerrainVertexDataPtr() {
  if (m_pTerrainPatch)
    return m_pTerrainPatch->SoftwareTransformPatch_GetTerrainVertexDataPtr();

  return NULL;
}

UINT CTerrainPatchProxy::GetWaterFaceCount() {
  if (m_pTerrainPatch)
    return m_pTerrainPatch->GetWaterFaceCount();

  return 0;
}

void CTerrainPatchProxy::Clear() {
  m_bUsed = false;
  m_sPatchNum = 0;
  m_byTerrainNum = 0xFF;

  m_pTerrainPatch = NULL;
}
