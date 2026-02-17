// MonsterAreaInfo.cpp: implementation of the CMonsterAreaInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAreaInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAreaInfo::CMonsterAreaInfo() { Clear(); }

CMonsterAreaInfo::~CMonsterAreaInfo() { Clear(); }

void CMonsterAreaInfo::Clear() {
  SetOrigin(-1, -1);
  SetSize(-1, -1);
  RemoveAllMonsters();
}

void CMonsterAreaInfo::SetOrigin(long lOriginX, long lOriginY) {
  m_lOriginX = lOriginX;
  m_lOriginY = lOriginY;
  SetLRTB();
}

void CMonsterAreaInfo::SetSize(long lSizeX, long lSizeY) {
  m_lSizeX = lSizeX;
  m_lSizeY = lSizeY;
  SetLRTB();
}

void CMonsterAreaInfo::GetOrigin(long *plOriginX, long *plOriginY) {
  *plOriginX = m_lOriginX;
  *plOriginY = m_lOriginY;
}

void CMonsterAreaInfo::GetSize(long *plSizeX, long *plSizeY) {
  *plSizeX = m_lSizeX;
  *plSizeY = m_lSizeY;
}

void CMonsterAreaInfo::SetLRTB() {
  m_lLeft = m_lOriginX - m_lSizeX;
  m_lTop = m_lOriginY - m_lSizeY;
  m_lRight = m_lOriginX + m_lSizeX;
  m_lBottom = m_lOriginY + m_lSizeY;
}

void CMonsterAreaInfo::SetMonsterCount(DWORD dwCount) {
  m_dwMonsterCount = dwCount;

  if (m_TempMonsterPosVector.size() == dwCount)
    return;

  m_TempMonsterPosVector.clear();
  m_TempMonsterPosVector.resize(dwCount);

  for (DWORD dwI = 0; dwI < dwCount; ++dwI) {
    m_TempMonsterPosVector[dwI].x = (float)random_range(m_lLeft, m_lRight);
    m_TempMonsterPosVector[dwI].y = (float)random_range(m_lTop, m_lBottom);
  }
}

void CMonsterAreaInfo::SetMonsterDirection(EMonsterDir eMonsterDir) {
  m_eMonsterDir = eMonsterDir;
  D3DXMATRIX matRotation;
  D3DXVECTOR3 v3Direction(0.0f, 1.0f, 0.0f);
  float fDegree = 0.0f;
  switch (m_eMonsterDir) {
  case DIR_RANDOM:
    fDegree = (float)random_range(0, 7) * 45.0f;
    break;
  case DIR_NORTH:
    fDegree = 0.0f;
    break;
  case DIR_NORTHEAST:
    fDegree = 45.0f;
    break;
  case DIR_EAST:
    fDegree = 90.0f;
    break;
  case DIR_SOUTHEAST:
    fDegree = 135.0f;
    break;
  case DIR_SOUTH:
    fDegree = 180.0f;
    break;
  case DIR_SOUTHWEST:
    fDegree = 225.0f;
    break;
  case DIR_WEST:
    fDegree = 270.0f;
    break;
  case DIR_NORTHWEST:
    fDegree = 315.0f;
    break;
  }
  D3DXMatrixRotationZ(&matRotation, -D3DXToRadian(fDegree));
  D3DXVec3TransformCoord(&v3Direction, &v3Direction, &matRotation);
  m_v2Monsterdirection.x = v3Direction.x;
  m_v2Monsterdirection.y = v3Direction.y;
  D3DXVec2Normalize(&m_v2Monsterdirection, &m_v2Monsterdirection);
}

void CMonsterAreaInfo::RemoveAllMonsters() {
  SetMonsterAreaInfoType(MONSTERAREAINFOTYPE_INVALID);

  SetMonsterGroupID(0);

  m_strGroupName.assign("이름없음");
  m_strLeaderName.assign("이름없음");
  SetMonsterGroupFollowerCount(0);

  SetMonsterVID(0);
  m_strMonsterName.assign("이름없음");

  SetMonsterCount(0);
  SetMonsterDirection(DIR_NORTH);

  m_TempMonsterPosVector.clear();
}

D3DXVECTOR2 CMonsterAreaInfo::GetTempMonsterPos(DWORD dwIndex) {
  if (dwIndex >= m_TempMonsterPosVector.size())
    return D3DXVECTOR2(0.0f, 0.0f);
  return m_TempMonsterPosVector[dwIndex];
}
