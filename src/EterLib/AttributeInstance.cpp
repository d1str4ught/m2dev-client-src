#include "StdAfx.h"
#include "EterBase/Utils.h"
#include "AttributeInstance.h"
#include "GrpMath.h"

CDynamicPool<CAttributeInstance> CAttributeInstance::ms_kPool;

const float c_fStepSize = 50.0f;

bool CAttributeInstance::Picking(const D3DXVECTOR3 &v, const D3DXVECTOR3 &dir,
                                 float &out_x, float &out_y) {
  if (IsEmpty())
    return FALSE;
  // fy *= -1.0f;

  bool bPicked = false;
  float nx = 0;
  float ny = 0;

  for (DWORD i = 0; i < m_v3HeightDataVector.size(); ++i)
    for (DWORD j = 0; j < m_v3HeightDataVector[i].size(); j += 3) {
      const D3DXVECTOR3 &cv0 = m_v3HeightDataVector[i][j];
      const D3DXVECTOR3 &cv2 = m_v3HeightDataVector[i][j + 1];
      const D3DXVECTOR3 &cv1 = m_v3HeightDataVector[i][j + 2];

      D3DXVECTOR3 n;
      const auto vvv = (cv1 - cv0);
      const auto vvv2 = (cv2 - cv0);
      const auto vvv3 = (cv2 - cv1);
      D3DXVec3Cross(&n, &vvv, &vvv2);
      D3DXVECTOR3 x;
      float t;
      const auto _vv = (v - cv0);
      t = -D3DXVec3Dot(&_vv, &n) / D3DXVec3Dot(&dir, &n);

      x = v + t * dir;
      const auto vvv4 = (x - cv0);
      const auto vvv5 = (x - cv1);
      const auto vvv6 = (x - cv2);
      D3DXVECTOR3 temp;
      D3DXVec3Cross(&temp, &vvv, &vvv4);
      if (D3DXVec3Dot(&temp, &n) < 0)
        continue;
      D3DXVec3Cross(&temp, &vvv3, &vvv5);
      if (D3DXVec3Dot(&temp, &n) < 0)
        continue;
      const auto _vv_ = (cv0 - cv2);
      D3DXVec3Cross(&temp, &_vv_, &vvv6);
      if (D3DXVec3Dot(&temp, &n) < 0)
        continue;

      if (bPicked) {
        if ((v.x - x.x) * (v.x - x.x) + (v.y - x.y) * (v.y - x.y) <
            (v.x - nx) * (v.x - nx) + (v.y - ny) * (v.y - ny)) {
          nx = x.x;
          ny = x.y;
        }
      } else {
        nx = x.x;
        ny = x.y;
      }
      bPicked = true;
    }
  if (bPicked) {
    out_x = nx;
    out_y = ny;
  }
  return bPicked;
}

BOOL CAttributeInstance::GetHeight(float fx, float fy, float *pfHeight) {
  if (IsEmpty())
    return FALSE;

  fy *= -1.0f;

  if (!IsInHeight(fx, fy))
    return FALSE;

  BOOL bFlag = FALSE;

  for (DWORD i = 0; i < m_v3HeightDataVector.size(); ++i)
    for (DWORD j = 0; j < m_v3HeightDataVector[i].size(); j += 3) {
      const D3DXVECTOR3 &c_rv3Vertex0 = m_v3HeightDataVector[i][j];
      const D3DXVECTOR3 &c_rv3Vertex1 = m_v3HeightDataVector[i][j + 1];
      const D3DXVECTOR3 &c_rv3Vertex2 = m_v3HeightDataVector[i][j + 2];

      if (fx < c_rv3Vertex0.x && fx < c_rv3Vertex1.x && fx < c_rv3Vertex2.x ||
          fx > c_rv3Vertex0.x && fx > c_rv3Vertex1.x && fx > c_rv3Vertex2.x ||
          fy < c_rv3Vertex0.y && fy < c_rv3Vertex1.y && fy < c_rv3Vertex2.y ||
          fy > c_rv3Vertex0.y && fy > c_rv3Vertex1.y && fy > c_rv3Vertex2.y)
        continue;

      if (IsInTriangle2D(c_rv3Vertex0.x, c_rv3Vertex0.y, c_rv3Vertex1.x,
                         c_rv3Vertex1.y, c_rv3Vertex2.x, c_rv3Vertex2.y, fx,
                         fy)) {
        D3DXVECTOR3 v3Line1 = c_rv3Vertex1 - c_rv3Vertex0;
        D3DXVECTOR3 v3Line2 = c_rv3Vertex2 - c_rv3Vertex0;
        D3DXVECTOR3 v3Cross;

        D3DXVec3Cross(&v3Cross, &v3Line1, &v3Line2);
        D3DXVec3Normalize(&v3Cross, &v3Cross);

        if (0.0f != v3Cross.z) {
          float fd = (v3Cross.x * c_rv3Vertex0.x + v3Cross.y * c_rv3Vertex0.y +
                      v3Cross.z * c_rv3Vertex0.z);
          float fm = (v3Cross.x * fx + v3Cross.y * fy);
          *pfHeight = fMAX((fd - fm) / v3Cross.z, *pfHeight);

          bFlag = TRUE;
        }
      }
    }

  return bFlag;
}

CAttributeData *CAttributeInstance::GetObjectPointer() const {
  return m_roAttributeData.GetPointer();
}
BOOL CAttributeInstance::IsInHeight(float fx, float fy) {
  float fdx = m_matGlobal._41 - fx;
  float fdy = m_matGlobal._42 - fy;
  if (sqrtf(fdx * fdx + fdy * fdy) > m_fHeightRadius)
    return FALSE;

  return TRUE;
}

void CAttributeInstance::SetObjectPointer(CAttributeData *pAttributeData) {
  Clear();
  m_roAttributeData.SetPointer(pAttributeData);
}

void CAttributeInstance::RefreshObject(const D3DXMATRIX &c_rmatGlobal) {
  assert(!m_roAttributeData.IsNull());

  m_matGlobal = c_rmatGlobal;

  // Height
  m_fHeightRadius = m_roAttributeData->GetMaximizeRadius();

  DWORD dwHeightDataCount = m_roAttributeData->GetHeightDataCount();
  m_v3HeightDataVector.clear();
  m_v3HeightDataVector.resize(dwHeightDataCount);
  for (DWORD i = 0; i < dwHeightDataCount; ++i) {
    const THeightData *c_pHeightData;
    if (!m_roAttributeData->GetHeightDataPointer(i, &c_pHeightData))
      continue;

    DWORD dwVertexCount = c_pHeightData->v3VertexVector.size();
    m_v3HeightDataVector[i].clear();
    m_v3HeightDataVector[i].resize(dwVertexCount);
    for (DWORD j = 0; j < dwVertexCount; ++j) {
      D3DXVec3TransformCoord(&m_v3HeightDataVector[i][j],
                             &c_pHeightData->v3VertexVector[j], &m_matGlobal);
    }
  }
}

const char *CAttributeInstance::GetDataFileName() const {
  return m_roAttributeData->GetFileName();
}

void CAttributeInstance::CreateSystem(UINT uCapacity) {
  ms_kPool.Create(uCapacity);
}

void CAttributeInstance::DestroySystem() { ms_kPool.Destroy(); }

CAttributeInstance *CAttributeInstance::New() { return ms_kPool.Alloc(); }

void CAttributeInstance::Delete(CAttributeInstance *pkInst) {
  ms_kPool.Free(pkInst);
}

BOOL CAttributeInstance::IsEmpty() const {
  if (!m_v3HeightDataVector.empty())
    return FALSE;

  return TRUE;
}

void CAttributeInstance::Clear() {
  m_fHeightRadius = 0.0f;
  m_fCollisionRadius = 0.0f;
  D3DXMatrixIdentity(&m_matGlobal);

  m_v3HeightDataVector.clear();

  m_roAttributeData.SetPointer(NULL);
}

CAttributeInstance::CAttributeInstance() {}
CAttributeInstance::~CAttributeInstance() {}
