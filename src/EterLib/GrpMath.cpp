#include "StdAfx.h"
#include "GrpMath.h"

float CrossProduct2D(float x1, float y1, float x2, float y2) {
  return x1 * y2 - y1 * x2;
}

bool IsInTriangle2D(float ax, float ay, float bx, float by, float cx, float cy,
                    float tx, float ty) {
  float c1 = CrossProduct2D(bx - ax, by - ay, tx - ax, ty - ay);
  float c2 = CrossProduct2D(cx - bx, cy - by, tx - bx, ty - by);
  float c3 = CrossProduct2D(ax - cx, ay - cy, tx - cx, ty - cy);

  if (c1 * c2 > 0.0f && c1 * c3 > 0.0f)
    return true;

  if (c1 * c2 * c3 == 0.0f) {
    if (tx < ax)
      if (tx < bx)
        if (tx < cx)
          return false;

    if (tx > ax)
      if (tx > bx)
        if (tx > cx)
          return false;

    if (ty < ay)
      if (ty < by)
        if (ty < cy)
          return false;

    if (ty > ay)
      if (ty > by)
        if (ty > cy)
          return false;

    return true;
  }

  return false;
}

D3DXVECTOR3 *D3DXVec3Rotation(D3DXVECTOR3 *pvtOut, const D3DXVECTOR3 *c_pvtSrc,
                              const D3DXQUATERNION *c_pqtRot) {
  D3DXQUATERNION qtSrc(c_pvtSrc->x, c_pvtSrc->y, c_pvtSrc->z, 0);
  D3DXQUATERNION qtRet;
  D3DXQuaternionConjugate(&qtRet, c_pqtRot);
  D3DXQuaternionMultiply(&qtRet, &qtSrc, &qtRet);
  D3DXQuaternionMultiply(&qtRet, c_pqtRot, &qtRet);

  pvtOut->x = qtRet.x;
  pvtOut->y = qtRet.y;
  pvtOut->z = qtRet.z;

  return pvtOut;
}

void GetRotationFromMatrix(D3DXVECTOR3 *pRotation,
                           const D3DXMATRIX *c_pMatrix) {
  float sx = c_pMatrix->_32;
  float cx = sqrtf(1.0f - sx * sx);

  if (cx < 0.00001f) {
    if (sx > 0)
      pRotation->x = D3DX_PI / 2;
    else
      pRotation->x = -D3DX_PI / 2;

    pRotation->y = atan2f(c_pMatrix->_31, c_pMatrix->_11);
    pRotation->z = 0.0f;
  } else {
    pRotation->x = atan2f(sx, cx);
    pRotation->y = atan2f(-c_pMatrix->_31, c_pMatrix->_33);
    pRotation->z = atan2f(-c_pMatrix->_12, c_pMatrix->_22);
  }
}

void GetPivotAndRotationFromMatrix(D3DXMATRIX *pMatrix, D3DXVECTOR3 *pPivot,
                                   D3DXVECTOR3 *pRotation) {
  float sx = pMatrix->_32;
  float cx = sqrtf(1.0f - sx * sx);
  float x, y, z;

  if (cx < 0.00001f) {
    if (sx > 0)
      x = D3DX_PI / 2;
    else
      x = -D3DX_PI / 2;

    y = atan2f(pMatrix->_31, pMatrix->_11);
    z = 0.0f;
  } else {
    x = atan2f(sx, cx);
    y = atan2f(-pMatrix->_31, pMatrix->_33);
    z = atan2f(-pMatrix->_12, pMatrix->_22);
  }

  pRotation->x = x;
  pRotation->y = y;
  pRotation->z = z;

  pPivot->x = pMatrix->_41;
  pPivot->y = pMatrix->_42;
  pPivot->z = pMatrix->_43;
}

// NOTE : must be optimized!
void ExtractMovement(D3DXMATRIX *pTargetMatrix, D3DXMATRIX *pSourceMatrix) {
  D3DXVECTOR3 v3Pivot;
  D3DXVECTOR3 v3Rotation;
  GetPivotAndRotationFromMatrix(pSourceMatrix, &v3Pivot, &v3Rotation);

  D3DXMATRIX matRotationX;
  D3DXMatrixRotationX(&matRotationX, v3Rotation.x);
  D3DXMATRIX matRotationY;
  D3DXMatrixRotationY(&matRotationY, v3Rotation.y);
  D3DXMATRIX matRotationZ;
  D3DXMatrixRotationZ(&matRotationZ, v3Rotation.z);

  D3DXMATRIX matTranslation;
  D3DXMatrixTranslation(&matTranslation, v3Pivot.x, v3Pivot.y, v3Pivot.z);

  *pTargetMatrix = matRotationX * matRotationY * matRotationZ * matTranslation;
}