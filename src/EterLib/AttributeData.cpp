#include "StdAfx.h"
#include "EterBase/Utils.h"
#include "AttributeData.h"

const char c_szAttributeDataFileHeader[] = "AttributeData";
const int c_iAttributeDataFileHeaderLength = 13;

/*DWORD CAttributeData::GetCollisionDataCount() const
{
        return m_CollisionDataVector.size();
}

BOOL CAttributeData::GetCollisionDataPointer(DWORD dwIndex, const TCollisionData
** c_ppCollisionData) const
{
        if (dwIndex >= GetCollisionDataCount())
                return FALSE;

        *c_ppCollisionData = &m_CollisionDataVector[dwIndex];

        return TRUE;
}
*/

const CStaticCollisionDataVector &
CAttributeData::GetCollisionDataVector() const {
  return m_StaticCollisionDataVector;
}

const THeightDataVector &CAttributeData::GetHeightDataVector() const {
  return m_HeightDataVector;
}

DWORD CAttributeData::GetHeightDataCount() const {
  return m_HeightDataVector.size();
}

BOOL CAttributeData::GetHeightDataPointer(
    DWORD dwIndex, const THeightData **c_ppHeightData) const {
  if (dwIndex >= GetHeightDataCount())
    return FALSE;

  *c_ppHeightData = &m_HeightDataVector[dwIndex];

  return TRUE;
}

float CAttributeData::GetMaximizeRadius() { return m_fMaximizeRadius; }

size_t CAttributeData::AddCollisionData(const CStaticCollisionData &data) {
  m_StaticCollisionDataVector.push_back(data);
  return m_StaticCollisionDataVector.size();
}

bool CAttributeData::OnLoad(int /*iSize*/, const void *c_pvBuf) {
  if (!c_pvBuf) {
    // NOTE: 파일이 존재하지 않으면 다른곳에서 그래픽 모델을 기반으로 충돌
    // 데이터를 생성하니 리소스를 파괴하지 않고 유지시킴.
    return true;
  }

  const BYTE *c_pbBuf = static_cast<const BYTE *>(c_pvBuf);

  char szHeader[c_iAttributeDataFileHeaderLength + 1];
  memcpy(szHeader, c_pbBuf, c_iAttributeDataFileHeaderLength + 1);
  c_pbBuf += c_iAttributeDataFileHeaderLength + 1;
  if (strcmp(szHeader, c_szAttributeDataFileHeader))
    return FALSE;

  DWORD dwCollisionDataCount;
  DWORD dwHeightDataCount;
  memcpy(&dwCollisionDataCount, c_pbBuf, sizeof(DWORD));
  c_pbBuf += sizeof(DWORD);
  memcpy(&dwHeightDataCount, c_pbBuf, sizeof(DWORD));
  c_pbBuf += sizeof(DWORD);

  m_StaticCollisionDataVector.clear();
  m_StaticCollisionDataVector.resize(dwCollisionDataCount);
  m_HeightDataVector.clear();
  m_HeightDataVector.resize(dwHeightDataCount);

  for (DWORD i = 0; i < dwCollisionDataCount; ++i) {
    CStaticCollisionData &rCollisionData = m_StaticCollisionDataVector[i];
    memcpy(&rCollisionData.dwType, c_pbBuf, sizeof(DWORD));
    c_pbBuf += sizeof(DWORD);
    memcpy(rCollisionData.szName, c_pbBuf, 32);
    c_pbBuf += 32;
    memcpy(&rCollisionData.v3Position, c_pbBuf, sizeof(D3DXVECTOR3));
    c_pbBuf += sizeof(D3DXVECTOR3);

    switch (rCollisionData.dwType) {
    case COLLISION_TYPE_PLANE:
      memcpy(rCollisionData.fDimensions, c_pbBuf, 2 * sizeof(float));
      c_pbBuf += 2 * sizeof(float);
      break;
    case COLLISION_TYPE_BOX:
      memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
      c_pbBuf += 3 * sizeof(float);
      break;
    case COLLISION_TYPE_SPHERE:
      memcpy(rCollisionData.fDimensions, c_pbBuf, sizeof(float));
      c_pbBuf += sizeof(float);
      break;
    case COLLISION_TYPE_CYLINDER:
      memcpy(rCollisionData.fDimensions, c_pbBuf, 2 * sizeof(float));
      c_pbBuf += 2 * sizeof(float);
      break;
    case COLLISION_TYPE_AABB:
      memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
      c_pbBuf += 3 * sizeof(float);
      break;
    case COLLISION_TYPE_OBB:
      memcpy(rCollisionData.fDimensions, c_pbBuf, 3 * sizeof(float));
      c_pbBuf += 3 * sizeof(float);
      break;
    }

    memcpy(rCollisionData.quatRotation, c_pbBuf, sizeof(D3DXQUATERNION));
    c_pbBuf += sizeof(D3DXQUATERNION);
  }

  for (DWORD j = 0; j < dwHeightDataCount; ++j) {
    THeightData &rHeightData = m_HeightDataVector[j];
    memcpy(rHeightData.szName, c_pbBuf, 32);
    c_pbBuf += 32;

    DWORD dwPrimitiveCount;
    memcpy(&dwPrimitiveCount, c_pbBuf, sizeof(DWORD));
    c_pbBuf += sizeof(DWORD);

    rHeightData.v3VertexVector.clear();
    rHeightData.v3VertexVector.resize(dwPrimitiveCount);
    memcpy(&rHeightData.v3VertexVector[0], c_pbBuf,
           dwPrimitiveCount * sizeof(D3DXVECTOR3));
    c_pbBuf += dwPrimitiveCount * sizeof(D3DXVECTOR3);

    // Getting Maximize Radius
    for (DWORD k = 0; k < rHeightData.v3VertexVector.size(); ++k) {
      m_fMaximizeRadius = fMAX(m_fMaximizeRadius,
                               fabs(rHeightData.v3VertexVector[k].x) + 50.0f);
      m_fMaximizeRadius = fMAX(m_fMaximizeRadius,
                               fabs(rHeightData.v3VertexVector[k].y) + 50.0f);
      m_fMaximizeRadius = fMAX(m_fMaximizeRadius,
                               fabs(rHeightData.v3VertexVector[k].z) + 50.0f);
    }
    // Getting Maximize Radius
  }

  return true;
}

void CAttributeData::OnClear() {
  m_StaticCollisionDataVector.clear();
  m_HeightDataVector.clear();
}

bool CAttributeData::OnIsEmpty() const {
  if (!m_StaticCollisionDataVector.empty())
    return false;
  if (!m_HeightDataVector.empty())
    return false;

  return true;
}

bool CAttributeData::OnIsType(TType type) {
  if (CAttributeData::Type() == type)
    return true;

  return CResource::OnIsType(type);
}

CAttributeData::TType CAttributeData::Type() {
  static TType s_type = StringToType("CAttributeData");
  return s_type;
}

void CAttributeData::OnSelfDestruct() { Clear(); }

CAttributeData::CAttributeData(const char *c_szFileName)
    : CResource(c_szFileName) {
  m_fMaximizeRadius = 0.0f;
}

CAttributeData::~CAttributeData() {}
