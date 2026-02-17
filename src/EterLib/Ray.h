#pragma once

#include <d3dx9.h>

class CRay {
public:
  CRay(const D3DXVECTOR3 &v3Start, const D3DXVECTOR3 &v3Dir, float fRayRange)
      : m_v3Start(v3Start), m_v3Direction(v3Dir) {
    assert(fRayRange >= 0);
    m_fRayRange = fRayRange;
    D3DXVec3Normalize(&m_v3Direction, &m_v3Direction);
    m_v3End = m_v3Start + fRayRange * m_v3Direction;
  }

  CRay() {}

  void SetStartPoint(const D3DXVECTOR3 &v3Start) { m_v3Start = v3Start; }

  void SetDirection(const D3DXVECTOR3 &v3Dir, float fRayRange) {
    assert(fRayRange >= 0);
    m_v3Direction = v3Dir;
    D3DXVec3Normalize(&m_v3Direction, &m_v3Direction);
    m_fRayRange = fRayRange;
    m_v3End = m_v3Start + m_fRayRange * m_v3Direction;
  }

  void GetStartPoint(D3DXVECTOR3 *pv3Start) const { *pv3Start = m_v3Start; }

  void GetDirection(D3DXVECTOR3 *pv3Dir, float *pfRayRange) const {
    *pv3Dir = m_v3Direction;
    *pfRayRange = m_fRayRange;
  }

  void GetEndPoint(D3DXVECTOR3 *pv3End) const { *pv3End = m_v3End; }

  const CRay &operator=(const CRay &rhs) {
    assert(rhs.m_fRayRange >= 0);
    m_v3Start = rhs.m_v3Start;
    m_v3Direction = rhs.m_v3Direction;
    m_fRayRange = rhs.m_fRayRange;
    D3DXVec3Normalize(&m_v3Direction, &m_v3Direction);
    m_v3End = m_v3Start + m_fRayRange * m_v3Direction;
  }

private:
  D3DXVECTOR3 m_v3Start;
  D3DXVECTOR3 m_v3End;
  D3DXVECTOR3 m_v3Direction;
  float m_fRayRange;
};
