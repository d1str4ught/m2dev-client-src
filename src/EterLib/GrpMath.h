#pragma once

float CrossProduct2D(float x1, float y1, float x2, float y2);

bool IsInTriangle2D(float ax, float ay, float bx, float by, float cx, float cy,
                    float tx, float ty);

D3DXVECTOR3 *D3DXVec3Rotation(D3DXVECTOR3 *pvtOut, const D3DXVECTOR3 *c_pvtSrc,
                              const D3DXQUATERNION *c_pqtRot);
D3DXVECTOR3 *D3DXVec3Translation(D3DXVECTOR3 *pvtOut,
                                 const D3DXVECTOR3 *c_pvtSrc,
                                 const D3DXVECTOR3 *c_pvtTrans);

void GetRotationFromMatrix(D3DXVECTOR3 *pRotation, const D3DXMATRIX *c_pMatrix);
void GetPivotAndRotationFromMatrix(D3DXMATRIX *pMatrix, D3DXVECTOR3 *pPivot,
                                   D3DXVECTOR3 *pRotation);
void ExtractMovement(D3DXMATRIX *pTargetMatrix, D3DXMATRIX *pSourceMatrix);

inline D3DXVECTOR3 *D3DXVec3Blend(D3DXVECTOR3 *pvtOut,
                                  const D3DXVECTOR3 *c_pvtSrc1,
                                  const D3DXVECTOR3 *c_pvtSrc2, float d) {
  pvtOut->x = c_pvtSrc1->x + d * (c_pvtSrc2->x - c_pvtSrc1->x);
  pvtOut->y = c_pvtSrc1->y + d * (c_pvtSrc2->y - c_pvtSrc1->y);
  pvtOut->z = c_pvtSrc1->z + d * (c_pvtSrc2->z - c_pvtSrc1->z);

  return pvtOut;
}

inline D3DXQUATERNION *D3DXQuaternionBlend(D3DXQUATERNION *pqtOut,
                                           const D3DXQUATERNION *c_pqtSrc1,
                                           const D3DXQUATERNION *c_pqtSrc2,
                                           float d) {
  pqtOut->x = c_pqtSrc1->x + d * (c_pqtSrc2->x - c_pqtSrc1->x);
  pqtOut->y = c_pqtSrc1->y + d * (c_pqtSrc2->y - c_pqtSrc1->y);
  pqtOut->z = c_pqtSrc1->z + d * (c_pqtSrc2->z - c_pqtSrc1->z);
  pqtOut->w = c_pqtSrc1->w + d * (c_pqtSrc2->w - c_pqtSrc1->w);
  return pqtOut;
}

inline float ClampDegree(float fDegree) {
  if (fDegree >= 360.0f)
    fDegree -= 360.0f;
  if (fDegree < 0.0f)
    fDegree += 360.0f;

  return fDegree;
}

inline float GetVector3Distance(const D3DXVECTOR3 &c_rv3Source,
                                const D3DXVECTOR3 &c_rv3Target) {
  return (c_rv3Source.x - c_rv3Target.x) * (c_rv3Source.x - c_rv3Target.x) +
         (c_rv3Source.y - c_rv3Target.y) * (c_rv3Source.y - c_rv3Target.y);
}

inline D3DXQUATERNION SafeRotationNormalizedArc(const D3DXVECTOR3 &vFrom,
                                                const D3DXVECTOR3 &vTo) {
  if (vFrom == vTo)
    return D3DXQUATERNION(0.0f, 0.0f, 0.0f, 1.0f);
  if (vFrom == -vTo)
    return D3DXQUATERNION(0.0f, 0.0f, 1.0f, 0.0f);
  D3DXVECTOR3 c;
  D3DXVec3Cross(&c, &vFrom, &vTo);
  float d = D3DXVec3Dot(&vFrom, &vTo);
  float s = sqrtf((1 + d) * 2);

  return D3DXQUATERNION(c.x / s, c.y / s, c.z / s, s * 0.5f);
}

inline D3DXQUATERNION RotationNormalizedArc(const D3DXVECTOR3 &vFrom,
                                            const D3DXVECTOR3 &vTo)

{
  D3DXVECTOR3 c;
  D3DXVec3Cross(&c, &vFrom, &vTo);
  float d = D3DXVec3Dot(&vFrom, &vTo);
  float s = sqrtf((1 + d) * 2);

  return D3DXQUATERNION(c.x / s, c.y / s, c.z / s, s * 0.5f);
}

inline D3DXQUATERNION RotationArc(const D3DXVECTOR3 &vFrom,
                                  const D3DXVECTOR3 &vTo) {
  D3DXVECTOR3 vnFrom, vnTo;
  D3DXVec3Normalize(&vnFrom, &vFrom);
  D3DXVec3Normalize(&vnTo, &vTo);
  return RotationNormalizedArc(vnFrom, vnTo);
}

inline float square_distance_between_linesegment_and_point(
    const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &x) {
  const auto v1 = p2 - p1;
  float l = D3DXVec3LengthSq(&v1);
  const auto v2 = x - p1;
  const auto v3 = p2 - p1;
  float d = D3DXVec3Dot(&(v2), &(v3));
  if (d <= 0.0f) {
    return D3DXVec3LengthSq(&(v2));
  } else if (d >= l) {
    const auto v4 = x - p2;
    return D3DXVec3LengthSq(&(v4));
  } else {
    D3DXVECTOR3 c;
    return D3DXVec3LengthSq(D3DXVec3Cross(&c, &(v2), &(v3))) / l;
  }
}

inline D3DXVECTOR3 *Vec3TransformQuaternionSafe(D3DXVECTOR3 *pvout,
                                                const D3DXVECTOR3 *pv,
                                                const D3DXQUATERNION *pq) {
  D3DXVECTOR3 v;
  D3DXVec3Cross(&v, pv, (D3DXVECTOR3 *)pq);
  v *= -2 * pq->w;
  v += (pq->w * pq->w - D3DXVec3LengthSq((D3DXVECTOR3 *)pq)) * (*pv);
  v += 2 * D3DXVec3Dot((D3DXVECTOR3 *)pq, pv) * (*(D3DXVECTOR3 *)pq);
  *pvout = v;
  return pvout;
}

inline D3DXVECTOR3 *Vec3TransformQuaternion(D3DXVECTOR3 *pvout,
                                            const D3DXVECTOR3 *pv,
                                            const D3DXQUATERNION *pq) {
  D3DXVec3Cross(pvout, pv, (D3DXVECTOR3 *)pq);
  *pvout *= -2 * pq->w;
  *pvout += (pq->w * pq->w - D3DXVec3LengthSq((D3DXVECTOR3 *)pq)) * (*pv);
  *pvout += 2 * D3DXVec3Dot((D3DXVECTOR3 *)pq, pv) * (*(D3DXVECTOR3 *)pq);

  return pvout;
}
