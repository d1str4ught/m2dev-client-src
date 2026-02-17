#include "Stdafx.h"
#include "CollisionData.h"
#include "Pool.h"
#include "GrpScreen.h"
#include "GrpMath.h"
#include "lineintersect_utils.h"
#include "StateManager.h"
const float gc_fReduceMove = 0.5f;

// const float gc_fSlideMoveSpeed = 5.0f;
/*inline D3DXVECTOR3 FitAtSpecifiedLength(const D3DXVECTOR3 & v3Vector, float
length)
{
        D3DXVECTOR3 v;
        D3DXVec3Normalize(&v,&v3Vector);
        return v*length;
}
*/
CDynamicPool<CSphereCollisionInstance> gs_sci;
CDynamicPool<CCylinderCollisionInstance> gs_cci;
CDynamicPool<CPlaneCollisionInstance> gs_pci;
CDynamicPool<CAABBCollisionInstance> gs_aci;
CDynamicPool<COBBCollisionInstance> gs_oci;

void DestroyCollisionInstanceSystem() {
  gs_sci.Destroy();
  gs_cci.Destroy();
  gs_pci.Destroy();
  gs_aci.Destroy();
  gs_oci.Destroy();
}

/////////////////////////////////////////////
// Base
CBaseCollisionInstance *CBaseCollisionInstance::BuildCollisionInstance(
    const CStaticCollisionData *c_pCollisionData, const D3DXMATRIX *pMat) {
  switch (c_pCollisionData->dwType) {
  case COLLISION_TYPE_PLANE: {
    CPlaneCollisionInstance *ppci = gs_pci.Alloc();
    D3DXMATRIX matRotation;
    D3DXMATRIX matTranslationLocal;
    D3DXMatrixRotationQuaternion(&matRotation, &c_pCollisionData->quatRotation);
    D3DXMatrixTranslation(&matTranslationLocal, c_pCollisionData->v3Position.x,
                          c_pCollisionData->v3Position.y,
                          c_pCollisionData->v3Position.z);
    D3DXMATRIX matTransform = matRotation * matTranslationLocal * *pMat;

    TPlaneData &PlaneData = ppci->GetAttribute();
    D3DXVec3TransformCoord(&PlaneData.v3Position, &c_pCollisionData->v3Position,
                           pMat);
    float fHalfWidth = c_pCollisionData->fDimensions[0] / 2.0f;
    float fHalfLength = c_pCollisionData->fDimensions[1] / 2.0f;

    PlaneData.v3QuadPosition[0].x = -fHalfWidth;
    PlaneData.v3QuadPosition[0].y = -fHalfLength;
    PlaneData.v3QuadPosition[0].z = 0.0f;
    PlaneData.v3QuadPosition[1].x = +fHalfWidth;
    PlaneData.v3QuadPosition[1].y = -fHalfLength;
    PlaneData.v3QuadPosition[1].z = 0.0f;
    PlaneData.v3QuadPosition[2].x = -fHalfWidth;
    PlaneData.v3QuadPosition[2].y = +fHalfLength;
    PlaneData.v3QuadPosition[2].z = 0.0f;
    PlaneData.v3QuadPosition[3].x = +fHalfWidth;
    PlaneData.v3QuadPosition[3].y = +fHalfLength;
    PlaneData.v3QuadPosition[3].z = 0.0f;
    for (DWORD i = 0; i < 4; ++i)
      D3DXVec3TransformCoord(&PlaneData.v3QuadPosition[i],
                             &PlaneData.v3QuadPosition[i], &matTransform);
    D3DXVECTOR3 v3Line0 =
        PlaneData.v3QuadPosition[1] - PlaneData.v3QuadPosition[0];
    D3DXVECTOR3 v3Line1 =
        PlaneData.v3QuadPosition[2] - PlaneData.v3QuadPosition[0];
    D3DXVECTOR3 v3Line2 =
        PlaneData.v3QuadPosition[1] - PlaneData.v3QuadPosition[3];
    D3DXVECTOR3 v3Line3 =
        PlaneData.v3QuadPosition[2] - PlaneData.v3QuadPosition[3];
    D3DXVec3Normalize(&v3Line0, &v3Line0);
    D3DXVec3Normalize(&v3Line1, &v3Line1);
    D3DXVec3Normalize(&v3Line2, &v3Line2);
    D3DXVec3Normalize(&v3Line3, &v3Line3);
    D3DXVec3Cross(&PlaneData.v3Normal, &v3Line0, &v3Line1);
    D3DXVec3Normalize(&PlaneData.v3Normal, &PlaneData.v3Normal);

    D3DXVec3Cross(&PlaneData.v3InsideVector[0], &PlaneData.v3Normal, &v3Line0);
    D3DXVec3Cross(&PlaneData.v3InsideVector[1], &v3Line1, &PlaneData.v3Normal);
    D3DXVec3Cross(&PlaneData.v3InsideVector[2], &v3Line2, &PlaneData.v3Normal);
    D3DXVec3Cross(&PlaneData.v3InsideVector[3], &PlaneData.v3Normal, &v3Line3);

    return ppci;
  } break;
  case COLLISION_TYPE_BOX:
    assert(false && "COLLISION_TYPE_BOX not implemented");
    break;
  case COLLISION_TYPE_AABB: {
    CAABBCollisionInstance *paci = gs_aci.Alloc();

    D3DXMATRIX matTranslationLocal;
    D3DXMatrixTranslation(&matTranslationLocal, c_pCollisionData->v3Position.x,
                          c_pCollisionData->v3Position.y,
                          c_pCollisionData->v3Position.z);
    D3DXMATRIX matTransform = *pMat;

    D3DXVECTOR3 v3Pos;
    v3Pos.x = matTranslationLocal._41;
    v3Pos.y = matTranslationLocal._42;
    v3Pos.z = matTranslationLocal._43;

    TAABBData &AABBData = paci->GetAttribute();
    AABBData.v3Min.x = v3Pos.x - c_pCollisionData->fDimensions[0];
    AABBData.v3Min.y = v3Pos.y - c_pCollisionData->fDimensions[1];
    AABBData.v3Min.z = v3Pos.z - c_pCollisionData->fDimensions[2];
    AABBData.v3Max.x = v3Pos.x + c_pCollisionData->fDimensions[0];
    AABBData.v3Max.y = v3Pos.y + c_pCollisionData->fDimensions[1];
    AABBData.v3Max.z = v3Pos.z + c_pCollisionData->fDimensions[2];

    D3DXVec3TransformCoord(&AABBData.v3Min, &AABBData.v3Min, &matTransform);
    D3DXVec3TransformCoord(&AABBData.v3Max, &AABBData.v3Max, &matTransform);

    return paci;
  } break;
  case COLLISION_TYPE_OBB: {
    COBBCollisionInstance *poci = gs_oci.Alloc();

    D3DXMATRIX matTranslationLocal;
    D3DXMatrixTranslation(&matTranslationLocal, c_pCollisionData->v3Position.x,
                          c_pCollisionData->v3Position.y,
                          c_pCollisionData->v3Position.z);
    D3DXMATRIX matRotation;
    D3DXMatrixRotationQuaternion(&matRotation, &c_pCollisionData->quatRotation);

    D3DXMATRIX matTranslationWorld;
    D3DXMatrixIdentity(&matTranslationWorld);
    matTranslationWorld._41 = pMat->_41;
    matTranslationWorld._42 = pMat->_42;
    matTranslationWorld._43 = pMat->_43;
    matTranslationWorld._44 = pMat->_44;

    D3DXVECTOR3 v3Min, v3Max;
    v3Min.x = c_pCollisionData->v3Position.x - c_pCollisionData->fDimensions[0];
    v3Min.y = c_pCollisionData->v3Position.y - c_pCollisionData->fDimensions[1];
    v3Min.z = c_pCollisionData->v3Position.z - c_pCollisionData->fDimensions[2];
    v3Max.x = c_pCollisionData->v3Position.x + c_pCollisionData->fDimensions[0];
    v3Max.y = c_pCollisionData->v3Position.y + c_pCollisionData->fDimensions[1];
    v3Max.z = c_pCollisionData->v3Position.z + c_pCollisionData->fDimensions[2];

    D3DXVec3TransformCoord(&v3Min, &v3Min, pMat);
    D3DXVec3TransformCoord(&v3Max, &v3Max, pMat);
    D3DXVECTOR3 v3Position = (v3Min + v3Max) * 0.5f;

    TOBBData &OBBData = poci->GetAttribute();
    OBBData.v3Min.x = v3Position.x - c_pCollisionData->fDimensions[0];
    OBBData.v3Min.y = v3Position.y - c_pCollisionData->fDimensions[1];
    OBBData.v3Min.z = v3Position.z - c_pCollisionData->fDimensions[2];
    OBBData.v3Max.x = v3Position.x + c_pCollisionData->fDimensions[0];
    OBBData.v3Max.y = v3Position.y + c_pCollisionData->fDimensions[1];
    OBBData.v3Max.z = v3Position.z + c_pCollisionData->fDimensions[2];

    D3DXMATRIX matTransform = *pMat;

    D3DXMatrixIdentity(&OBBData.matRot);
    OBBData.matRot = *pMat;
    OBBData.matRot._41 = 0;
    OBBData.matRot._42 = 0;
    OBBData.matRot._43 = 0;
    OBBData.matRot._44 = 1;

    return poci;
  } break;
  case COLLISION_TYPE_SPHERE: {
    CSphereCollisionInstance *psci = gs_sci.Alloc();

    D3DXMATRIX matTranslationLocal;
    D3DXMatrixTranslation(&matTranslationLocal, c_pCollisionData->v3Position.x,
                          c_pCollisionData->v3Position.y,
                          c_pCollisionData->v3Position.z);
    matTranslationLocal = matTranslationLocal * *pMat;

    TSphereData &SphereData = psci->GetAttribute();
    SphereData.v3Position.x = matTranslationLocal._41;
    SphereData.v3Position.y = matTranslationLocal._42;
    SphereData.v3Position.z = matTranslationLocal._43;
    SphereData.fRadius = c_pCollisionData->fDimensions[0];

    return psci;
  } break;
  case COLLISION_TYPE_CYLINDER: {
    CCylinderCollisionInstance *pcci = gs_cci.Alloc();

    D3DXMATRIX matTranslationLocal;
    D3DXMatrixTranslation(&matTranslationLocal, c_pCollisionData->v3Position.x,
                          c_pCollisionData->v3Position.y,
                          c_pCollisionData->v3Position.z);
    matTranslationLocal = matTranslationLocal * *pMat;

    TCylinderData &CylinderData = pcci->GetAttribute();
    CylinderData.fRadius = c_pCollisionData->fDimensions[0];
    CylinderData.fHeight = c_pCollisionData->fDimensions[1];
    CylinderData.v3Position.x = matTranslationLocal._41;
    CylinderData.v3Position.y = matTranslationLocal._42;
    CylinderData.v3Position.z =
        matTranslationLocal._43 /*+ CylinderData.fHeight/2.0f*/;

    return pcci;
  } break;
  }
  assert(false && "NOT_REACHED");
  return 0;
}

void CBaseCollisionInstance::Destroy() { OnDestroy(); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*------------------------------------------------------Sphere---------------------------------------------------------------*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TSphereData &CSphereCollisionInstance::GetAttribute() { return m_attribute; }

const TSphereData &CSphereCollisionInstance::GetAttribute() const {
  return m_attribute;
}

void CSphereCollisionInstance::Render(D3DFILLMODE d3dFillMode) {
  static CScreen s;
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
  s.RenderSphere(NULL, m_attribute.v3Position.x, m_attribute.v3Position.y,
                 m_attribute.v3Position.z, m_attribute.fRadius, d3dFillMode);
}

void CSphereCollisionInstance::OnDestroy() { gs_sci.Free(this); }

bool CSphereCollisionInstance::OnMovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  if (square_distance_between_linesegment_and_point(
          s.v3LastPosition, s.v3Position, m_attribute.v3Position) <
      (m_attribute.fRadius + s.fRadius) * (m_attribute.fRadius + s.fRadius)) {
    // NOTE : 거리가 가까워 졌을때만.. - [levites]
    if (GetVector3Distance(s.v3Position, m_attribute.v3Position) <
        GetVector3Distance(s.v3LastPosition, m_attribute.v3Position))
      return true;
  }

  return false;
}

bool CSphereCollisionInstance::OnCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  // Tracef("OnCollisionDynamicSphere\n");

  if (square_distance_between_linesegment_and_point(
          s.v3LastPosition, s.v3Position, m_attribute.v3Position) <
      (m_attribute.fRadius + s.fRadius) * (m_attribute.fRadius + s.fRadius)) {
    return true;
  }

  return false;
}

D3DXVECTOR3 CSphereCollisionInstance::OnGetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {
  const auto _vv__ = (s.v3Position - m_attribute.v3Position);
  if (D3DXVec3LengthSq(&_vv__) >=
      (s.fRadius + m_attribute.fRadius) * (m_attribute.fRadius + s.fRadius))
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  D3DXVECTOR3 c;
  const auto _vv__2 = (s.v3Position - s.v3LastPosition);
  const auto _vv_s_2 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  D3DXVec3Cross(&c, &_vv__2, &_vv_s_2);

  float sum = -D3DXVec3Dot(&c, &_vv__);
  float mul =
      (s.fRadius + m_attribute.fRadius) * (s.fRadius + m_attribute.fRadius) -
      D3DXVec3LengthSq(&_vv__);

  if (sum * sum - 4 * mul <= 0)
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  float sq = sqrt(sum * sum - 4 * mul);
  float t1 = -sum - sq, t2 = -sum + sq;
  t1 *= 0.5f;
  t2 *= 0.5f;

  if (fabs(t1) <= fabs(t2)) {
    return (gc_fReduceMove * t1) * c;
  } else
    return (gc_fReduceMove * t2) * c;

  /*
  D3DXVECTOR3 p1 = s.v3Position+t1*c;
  D3DXVECTOR3 p2 = s.v3Position+t2*c;

  if (D3DXVec3LengthSq(&(p2-s.v3Position))>D3DXVec3LengthSq(&(p1-s.v3Position)))
  {
          return p1-s.v3Position;
  }
  else
  {
          return p2-s.v3Position;
  }
  */
}

/////////////////////////////////////////////
// Plane
TPlaneData &CPlaneCollisionInstance::GetAttribute() { return m_attribute; }

const TPlaneData &CPlaneCollisionInstance::GetAttribute() const {
  return m_attribute;
}

bool CPlaneCollisionInstance::OnMovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 v3SpherePosition = s.v3Position - m_attribute.v3Position;
  D3DXVECTOR3 v3SphereLastPosition = s.v3LastPosition - m_attribute.v3Position;

  float fPosition1 = D3DXVec3Dot(&m_attribute.v3Normal, &v3SpherePosition);
  float fPosition2 = D3DXVec3Dot(&m_attribute.v3Normal, &v3SphereLastPosition);

  if (fPosition1 > 0.0f && fPosition2 < 0.0f ||
      fPosition1 < 0.0f && fPosition2 > 0.0f ||
      (fPosition1) <= s.fRadius && fPosition1 >= -s.fRadius) {
    D3DXVECTOR3 v3QuadPosition1 = s.v3Position - m_attribute.v3QuadPosition[0];
    D3DXVECTOR3 v3QuadPosition2 = s.v3Position - m_attribute.v3QuadPosition[3];

    if (D3DXVec3Dot(&v3QuadPosition1, &m_attribute.v3InsideVector[0]) >
        -s.fRadius /*0.0f*/)
      if (D3DXVec3Dot(&v3QuadPosition1, &m_attribute.v3InsideVector[1]) >
          -s.fRadius /*0.0f*/)
        if (D3DXVec3Dot(&v3QuadPosition2, &m_attribute.v3InsideVector[2]) >
            -s.fRadius /*0.0f*/)
          if (D3DXVec3Dot(&v3QuadPosition2, &m_attribute.v3InsideVector[3]) >
              -s.fRadius /*0.0f*/) {
            // NOTE : 거리가 가까워 졌을때만.. - [levites]
            const auto _vv__3 = (s.v3Position - m_attribute.v3Position);
            const auto _vv__4 = (s.v3LastPosition - m_attribute.v3Position);
            if (fabs(D3DXVec3Dot(&_vv__3, &m_attribute.v3Normal)) <
                fabs(D3DXVec3Dot(&_vv__4, &m_attribute.v3Normal)))
              return true;
          }
  }

  return false;
}

bool CPlaneCollisionInstance::OnCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  // Tracef("OnCollisionDynamicSphere\n");

  D3DXVECTOR3 v3SpherePosition = s.v3Position - m_attribute.v3Position;
  D3DXVECTOR3 v3SphereLastPosition = s.v3LastPosition - m_attribute.v3Position;

  float fPosition1 = D3DXVec3Dot(&m_attribute.v3Normal, &v3SpherePosition);
  float fPosition2 = D3DXVec3Dot(&m_attribute.v3Normal, &v3SphereLastPosition);

  if (fPosition1 > 0.0f && fPosition2 < 0.0f ||
      fPosition1 < 0.0f && fPosition2 > 0.0f ||
      (fPosition1) <= s.fRadius && fPosition1 >= -s.fRadius) {
    D3DXVECTOR3 v3QuadPosition1 = s.v3Position - m_attribute.v3QuadPosition[0];
    D3DXVECTOR3 v3QuadPosition2 = s.v3Position - m_attribute.v3QuadPosition[3];

    if (D3DXVec3Dot(&v3QuadPosition1, &m_attribute.v3InsideVector[0]) >
        -s.fRadius /*0.0f*/)
      if (D3DXVec3Dot(&v3QuadPosition1, &m_attribute.v3InsideVector[1]) >
          -s.fRadius /*0.0f*/)
        if (D3DXVec3Dot(&v3QuadPosition2, &m_attribute.v3InsideVector[2]) >
            -s.fRadius /*0.0f*/)
          if (D3DXVec3Dot(&v3QuadPosition2, &m_attribute.v3InsideVector[3]) >
              -s.fRadius /*0.0f*/) {
            return true;
          }
  }

  return false;
}

D3DXVECTOR3 CPlaneCollisionInstance::OnGetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 advance = s.v3Position - s.v3LastPosition;

  float d = D3DXVec3Dot(&m_attribute.v3Normal, &advance);
  if (d >= -0.0001 && d <= 0.0001)
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  const auto vv = (s.v3Position - m_attribute.v3Position);
  float t = -D3DXVec3Dot(&m_attribute.v3Normal, &vv) / d;

  if (D3DXVec3Dot(&m_attribute.v3Normal, &advance) >= 0) {
    return t * advance - s.fRadius * m_attribute.v3Normal;
  } else {
    return t * advance + s.fRadius * m_attribute.v3Normal;
  }
}

void CPlaneCollisionInstance::Render(D3DFILLMODE /*d3dFillMode*/) {
  static CScreen s;
  s.RenderBar3d(m_attribute.v3QuadPosition);
}

void CPlaneCollisionInstance::OnDestroy() { gs_pci.Free(this); }

/////////////////////////////////////////////
// Cylinder
TCylinderData &CCylinderCollisionInstance::GetAttribute() {
  return m_attribute;
}

const TCylinderData &CCylinderCollisionInstance::GetAttribute() const {
  return m_attribute;
}

bool CCylinderCollisionInstance::CollideCylinderVSDynamicSphere(
    const TCylinderData &c_rattribute, const CDynamicSphereInstance &s) const {
  if (s.v3Position.z + s.fRadius < c_rattribute.v3Position.z)
    return false;

  if (s.v3Position.z - s.fRadius >
      c_rattribute.v3Position.z + c_rattribute.fHeight)
    return false;

  D3DXVECTOR3 oa, ob;
  IntersectLineSegments(
      c_rattribute.v3Position,
      D3DXVECTOR3(c_rattribute.v3Position.x, c_rattribute.v3Position.y,
                  c_rattribute.v3Position.z + c_rattribute.fHeight),
      s.v3LastPosition, s.v3Position, oa, ob);
  const auto vv = (oa - ob);
  return (D3DXVec3LengthSq(&vv) <= (c_rattribute.fRadius + s.fRadius) *
                                       (c_rattribute.fRadius + s.fRadius));
}

bool CCylinderCollisionInstance::OnMovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  if (CollideCylinderVSDynamicSphere(m_attribute, s)) {
    // NOTE : 거리가 가까워 졌을때만.. - [levites]
    if (GetVector3Distance(s.v3Position, m_attribute.v3Position) <
        GetVector3Distance(s.v3LastPosition, m_attribute.v3Position))
      return true;
  }

  // NOTE : 이동 거리가 클 경우 빈틈없이 (원 크기 단위로) 이동하면서 전부 체크
  // 해 본다 - [levites]
  D3DXVECTOR3 v3Distance = s.v3Position - s.v3LastPosition;
  float fDistance = D3DXVec3Length(&v3Distance);
  if (s.fRadius <= 0.0001f)
    return false;
  if (fDistance >= s.fRadius * 2.0f) {
    TCylinderData cylinder;
    cylinder = m_attribute;
    cylinder.v3Position = s.v3LastPosition;

    int iStep = fDistance / s.fRadius * 2.0f;
    D3DXVECTOR3 v3Step = v3Distance / float(iStep);

    for (int i = 0; i < iStep; ++i) {
      cylinder.v3Position += v3Step;
      if (CollideCylinderVSDynamicSphere(cylinder, s))
        return true;
    }
  }

  return false;
}

bool CCylinderCollisionInstance::OnCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  // Tracef("OnCollisionDynamicSphere\n");

  return (CollideCylinderVSDynamicSphere(m_attribute, s));
}

D3DXVECTOR3 CCylinderCollisionInstance::OnGetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 v3Position = m_attribute.v3Position;
  v3Position.z = s.v3Position.z;
  const auto vv = (s.v3Position - v3Position);
  if (D3DXVec3LengthSq(&vv) >=
      (s.fRadius + m_attribute.fRadius) * (m_attribute.fRadius + s.fRadius))
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  D3DXVECTOR3 c;
  D3DXVECTOR3 advance = s.v3Position - s.v3LastPosition;
  advance.z = 0;
  const auto vssa = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  D3DXVec3Cross(&c, &advance, &vssa);

  const auto svsvs = (s.v3Position - v3Position);
  float sum = -D3DXVec3Dot(&c, &svsvs);
  float mul =
      (s.fRadius + m_attribute.fRadius) * (s.fRadius + m_attribute.fRadius) -
      D3DXVec3LengthSq(&svsvs);

  if (sum * sum - 4 * mul <= 0)
    return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  float sq = sqrt(sum * sum - 4 * mul);
  float t1 = -sum - sq, t2 = -sum + sq;
  t1 *= 0.5f;
  t2 *= 0.5f;

  if (fabs(t1) <= fabs(t2)) {
    return (gc_fReduceMove * t1) * c;
  } else
    return (gc_fReduceMove * t2) * c;

  /*D3DXVECTOR3 p1 = s.v3Position+t1*c;
  D3DXVECTOR3 p2 = s.v3Position+t2*c;

  if (D3DXVec3LengthSq(&(p2-s.v3Position))>D3DXVec3LengthSq(&(p1-s.v3Position)))
  {
          return p1-s.v3Position;
  }
  else
  {
          return p2-s.v3Position;
  }*/
}

void CCylinderCollisionInstance::Render(D3DFILLMODE d3dFillMode) {
  static CScreen s;
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
  s.RenderCylinder(NULL, m_attribute.v3Position.x, m_attribute.v3Position.y,
                   m_attribute.v3Position.z + m_attribute.fHeight / 2,
                   m_attribute.fRadius, m_attribute.fHeight, d3dFillMode);
}

void CCylinderCollisionInstance::OnDestroy() { gs_cci.Free(this); }

/////////////////////////////////////////////
// AABB (Aligned Axis Bounding Box)
TAABBData &CAABBCollisionInstance::GetAttribute() { return m_attribute; }

const TAABBData &CAABBCollisionInstance::GetAttribute() const {

  return m_attribute;
}

bool CAABBCollisionInstance::OnMovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 v;
  D3DXVECTOR3 v3center = (m_attribute.v3Min + m_attribute.v3Max) * 0.5f;

  memcpy(&v, &s.v3Position, sizeof(D3DXVECTOR3));

  if (v.x < m_attribute.v3Min.x)
    v.x = m_attribute.v3Min.x;
  if (v.x > m_attribute.v3Max.x)
    v.x = m_attribute.v3Max.x;
  if (v.y < m_attribute.v3Min.y)
    v.x = m_attribute.v3Min.y;
  if (v.y > m_attribute.v3Max.y)
    v.x = m_attribute.v3Max.y;
  if (v.z < m_attribute.v3Min.z)
    v.z = m_attribute.v3Min.z;
  if (v.z > m_attribute.v3Max.z)
    v.z = m_attribute.v3Max.z;

  if (GetVector3Distance(v, s.v3Position) <= s.fRadius * s.fRadius) {

    return true;
  }

  memcpy(&v, &s.v3LastPosition, sizeof(D3DXVECTOR3));

  if (v.x < m_attribute.v3Min.x)
    v.x = m_attribute.v3Min.x;
  if (v.x > m_attribute.v3Max.x)
    v.x = m_attribute.v3Max.x;
  if (v.y < m_attribute.v3Min.y)
    v.x = m_attribute.v3Min.y;
  if (v.y > m_attribute.v3Max.y)
    v.x = m_attribute.v3Max.y;
  if (v.z < m_attribute.v3Min.z)
    v.z = m_attribute.v3Min.z;
  if (v.z > m_attribute.v3Max.z)
    v.z = m_attribute.v3Max.z;

  if (GetVector3Distance(v, s.v3LastPosition) <= s.fRadius * s.fRadius) {

    return true;
  }

  return false;
}

bool CAABBCollisionInstance::OnCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 v;
  memcpy(&v, &s.v3Position, sizeof(D3DXVECTOR3));

  if (v.x < m_attribute.v3Min.x)
    v.x = m_attribute.v3Min.x;
  if (v.x > m_attribute.v3Max.x)
    v.x = m_attribute.v3Max.x;
  if (v.y < m_attribute.v3Min.y)
    v.x = m_attribute.v3Min.y;
  if (v.y > m_attribute.v3Max.y)
    v.x = m_attribute.v3Max.y;
  if (v.z < m_attribute.v3Min.z)
    v.z = m_attribute.v3Min.z;
  if (v.z > m_attribute.v3Max.z)
    v.z = m_attribute.v3Max.z;

  if (v.x > m_attribute.v3Min.x && v.x < m_attribute.v3Max.x &&
      v.y > m_attribute.v3Min.y && v.y < m_attribute.v3Max.y &&
      v.z > m_attribute.v3Min.z && v.z < m_attribute.v3Max.z) {
    return true;
  }

  if (GetVector3Distance(v, s.v3Position) <= s.fRadius * s.fRadius) {
    return true;
  }

  memcpy(&v, &s.v3LastPosition, sizeof(D3DXVECTOR3));

  if (v.x < m_attribute.v3Min.x)
    v.x = m_attribute.v3Min.x;
  if (v.x > m_attribute.v3Max.x)
    v.x = m_attribute.v3Max.x;
  if (v.y < m_attribute.v3Min.y)
    v.x = m_attribute.v3Min.y;
  if (v.y > m_attribute.v3Max.y)
    v.x = m_attribute.v3Max.y;
  if (v.z < m_attribute.v3Min.z)
    v.z = m_attribute.v3Min.z;
  if (v.z > m_attribute.v3Max.z)
    v.z = m_attribute.v3Max.z;

  if (v.x > m_attribute.v3Min.x && v.x < m_attribute.v3Max.x &&
      v.y > m_attribute.v3Min.y && v.y < m_attribute.v3Max.y &&
      v.z > m_attribute.v3Min.z && v.z < m_attribute.v3Max.z) {
    return true;
  }

  if (GetVector3Distance(v, s.v3LastPosition) <= s.fRadius * s.fRadius) {
    return true;
  }

  return false;
}

D3DXVECTOR3 CAABBCollisionInstance::OnGetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {

  D3DXVECTOR3 v3Temp;
  if (s.v3Position.x + s.fRadius <= m_attribute.v3Min.x) {
    v3Temp.x = m_attribute.v3Min.x;
  } else if (s.v3Position.x - s.fRadius >= m_attribute.v3Max.x) {
    v3Temp.x = m_attribute.v3Max.x;
  } else if (s.v3Position.x + s.fRadius >= m_attribute.v3Min.x &&
             s.v3Position.x + s.fRadius <= m_attribute.v3Max.x) {
    v3Temp.x = s.v3Position.x + s.fRadius;
  } else {
    v3Temp.x = s.v3Position.x - s.fRadius;
  }

  if (s.v3Position.y + s.fRadius <= m_attribute.v3Min.y) {
    v3Temp.y = m_attribute.v3Min.y;
  } else if (s.v3Position.y - s.fRadius >= m_attribute.v3Max.y) {
    v3Temp.y = m_attribute.v3Max.y;
  } else if (s.v3Position.y + s.fRadius >= m_attribute.v3Min.y &&
             s.v3Position.y + s.fRadius <= m_attribute.v3Max.y) {
    v3Temp.y = s.v3Position.y + s.fRadius;
  } else {
    v3Temp.y = s.v3Position.y - s.fRadius;
  }

  if (s.v3Position.z + s.fRadius <= m_attribute.v3Min.z) {
    v3Temp.z = m_attribute.v3Min.z;
  } else if (s.v3Position.z - s.fRadius >= m_attribute.v3Max.z) {
    v3Temp.z = m_attribute.v3Max.z;
  } else if (s.v3Position.z + s.fRadius >= m_attribute.v3Min.z &&
             s.v3Position.z + s.fRadius <= m_attribute.v3Max.z) {
    v3Temp.z = s.v3Position.z + s.fRadius;
  } else {
    v3Temp.z = s.v3Position.z - s.fRadius;
  }

  const auto vv = (v3Temp - s.v3Position);
  if (D3DXVec3LengthSq(&vv) < s.fRadius * s.fRadius)
    return D3DXVECTOR3(.0f, .0f, .0f);

  return D3DXVECTOR3(.0f, .0f, .0f);
}

void CAABBCollisionInstance::Render(D3DFILLMODE d3dFillMode) {
  static CScreen s;
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
  s.RenderCube(m_attribute.v3Min.x, m_attribute.v3Min.y, m_attribute.v3Min.z,
               m_attribute.v3Max.x, m_attribute.v3Max.y, m_attribute.v3Max.z);
  return;
}

void CAABBCollisionInstance::OnDestroy() { gs_aci.Free(this); }

/////////////////////////////////////////////
// OBB

TOBBData &COBBCollisionInstance::GetAttribute() { return m_attribute; }

const TOBBData &COBBCollisionInstance::GetAttribute() const {

  return m_attribute;
}

bool COBBCollisionInstance::OnMovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  D3DXVECTOR3 v3Center = 0.5f * (m_attribute.v3Min + m_attribute.v3Max);
  D3DXVECTOR3 v3Sphere = s.v3Position - v3Center;
  D3DXVec3TransformCoord(&v3Sphere, &v3Sphere, &m_attribute.matRot);
  v3Sphere = v3Sphere + v3Center;

  D3DXVECTOR3 v3Point = v3Sphere;
  if (v3Point.x < m_attribute.v3Min.x) {
    v3Point.x = m_attribute.v3Min.x;
  }
  if (v3Point.x > m_attribute.v3Max.x) {
    v3Point.x = m_attribute.v3Max.x;
  }
  if (v3Point.y < m_attribute.v3Min.y) {
    v3Point.y = m_attribute.v3Min.y;
  }
  if (v3Point.y > m_attribute.v3Max.y) {
    v3Point.y = m_attribute.v3Max.y;
  }
  if (v3Point.z < m_attribute.v3Min.z) {
    v3Point.z = m_attribute.v3Min.z;
  }
  if (v3Point.z > m_attribute.v3Max.z) {
    v3Point.z = m_attribute.v3Max.z;
  }

  if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius) {
    return true;
  }

  v3Sphere = s.v3LastPosition - v3Center;
  D3DXVec3TransformCoord(&v3Sphere, &v3Sphere, &m_attribute.matRot);
  v3Sphere = v3Sphere + v3Center;

  v3Point = v3Sphere;
  if (v3Point.x < m_attribute.v3Min.x) {
    v3Point.x = m_attribute.v3Min.x;
  }
  if (v3Point.x > m_attribute.v3Max.x) {
    v3Point.x = m_attribute.v3Max.x;
  }
  if (v3Point.y < m_attribute.v3Min.y) {
    v3Point.y = m_attribute.v3Min.y;
  }
  if (v3Point.y > m_attribute.v3Max.y) {
    v3Point.y = m_attribute.v3Max.y;
  }
  if (v3Point.z < m_attribute.v3Min.z) {
    v3Point.z = m_attribute.v3Min.z;
  }
  if (v3Point.z > m_attribute.v3Max.z) {
    v3Point.z = m_attribute.v3Max.z;
  }

  if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius) {
    return true;
  }

  return false;
}

bool COBBCollisionInstance::OnCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {

  D3DXVECTOR3 v3Center = 0.5f * (m_attribute.v3Min + m_attribute.v3Max);
  D3DXVECTOR3 v3Sphere = s.v3Position - v3Center;
  D3DXVec3TransformCoord(&v3Sphere, &v3Sphere, &m_attribute.matRot);
  v3Sphere = v3Sphere + v3Center;

  D3DXVECTOR3 v3Point = v3Sphere;
  if (v3Point.x < m_attribute.v3Min.x) {
    v3Point.x = m_attribute.v3Min.x;
  }
  if (v3Point.x > m_attribute.v3Max.x) {
    v3Point.x = m_attribute.v3Max.x;
  }
  if (v3Point.y < m_attribute.v3Min.y) {
    v3Point.y = m_attribute.v3Min.y;
  }
  if (v3Point.y > m_attribute.v3Max.y) {
    v3Point.y = m_attribute.v3Max.y;
  }
  if (v3Point.z < m_attribute.v3Min.z) {
    v3Point.z = m_attribute.v3Min.z;
  }
  if (v3Point.z > m_attribute.v3Max.z) {
    v3Point.z = m_attribute.v3Max.z;
  }

  if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius) {
    return true;
  }

  v3Sphere = s.v3LastPosition - v3Center;
  D3DXVec3TransformCoord(&v3Sphere, &v3Sphere, &m_attribute.matRot);
  v3Sphere = v3Sphere + v3Center;

  v3Point = v3Sphere;
  if (v3Point.x < m_attribute.v3Min.x) {
    v3Point.x = m_attribute.v3Min.x;
  }
  if (v3Point.x > m_attribute.v3Max.x) {
    v3Point.x = m_attribute.v3Max.x;
  }
  if (v3Point.y < m_attribute.v3Min.y) {
    v3Point.y = m_attribute.v3Min.y;
  }
  if (v3Point.y > m_attribute.v3Max.y) {
    v3Point.y = m_attribute.v3Max.y;
  }
  if (v3Point.z < m_attribute.v3Min.z) {
    v3Point.z = m_attribute.v3Min.z;
  }
  if (v3Point.z > m_attribute.v3Max.z) {
    v3Point.z = m_attribute.v3Max.z;
  }

  if (GetVector3Distance(v3Point, v3Sphere) <= s.fRadius * s.fRadius) {
    return true;
  }

  return false;
}

D3DXVECTOR3 COBBCollisionInstance::OnGetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {

  return D3DXVECTOR3(.0f, .0f, .0f);
}

void COBBCollisionInstance::Render(D3DFILLMODE d3dFillMode) {
  static CScreen s;
  STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
  s.RenderCube(m_attribute.v3Min.x, m_attribute.v3Min.y, m_attribute.v3Min.z,
               m_attribute.v3Max.x, m_attribute.v3Max.y, m_attribute.v3Max.z,
               m_attribute.matRot);
  return;
}

void COBBCollisionInstance::OnDestroy() { gs_oci.Free(this); }