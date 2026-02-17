#include "StdAfx.h"
#include "PythonBackground.h"
#include "EterLib/Camera.h"

//////////////////////////////////////////////////////////////////////////
// 메세지

extern void SetHeightLog(bool isLog);

float CCamera::CAMERA_MIN_DISTANCE = 200.0f;
float CCamera::CAMERA_MAX_DISTANCE = 2500.0f;

void CCamera::ProcessTerrainCollision() {
  CPythonBackground &rPythonBackground = CPythonBackground::Instance();
  D3DXVECTOR3 v3CollisionPoint;

  if (rPythonBackground.GetPickingPointWithRayOnlyTerrain(
          m_kTargetToCameraBottomRay, &v3CollisionPoint)) {
    SetCameraState(CAMERA_STATE_CANTGODOWN);
    D3DXVECTOR3 v3CheckVector =
        m_v3Eye - 2.0f * m_fTerrainCollisionRadius * m_v3Up;
    v3CheckVector.z = rPythonBackground.GetHeight(floorf(v3CheckVector.x),
                                                  floorf(v3CheckVector.y));
    D3DXVECTOR3 v3NewEye =
        v3CheckVector + 2.0f * m_fTerrainCollisionRadius * m_v3Up;
    if (v3NewEye.z > m_v3Eye.z) {
      // printf("ToCameraBottom(%f, %f, %f) TCR %f, UP(%f, %f, %f), new %f > old
      // %f", 	v3CheckVector.x, v3CheckVector.y, v3CheckVector.z,
      //	m_fTerrainCollisionRadius,
      //	m_v3Up.x, m_v3Up.y, m_v3Up.z,
      //	v3NewEye.z, m_v3Eye.z);
      SetEye(v3NewEye);
    }
    /*
    SetCameraState(CAMERA_STATE_NORMAL);
    D3DXVECTOR3 v3NewEye = v3CollisionPoint;
    SetEye(v3NewEye);
    */
  } else
    SetCameraState(CAMERA_STATE_NORMAL);

  if (rPythonBackground.GetPickingPointWithRayOnlyTerrain(
          m_kCameraBottomToTerrainRay, &v3CollisionPoint)) {
    SetCameraState(CAMERA_STATE_CANTGODOWN);
    auto d3dd = (m_v3Eye - v3CollisionPoint);
    if (D3DXVec3Length(&d3dd) < 2.0f * m_fTerrainCollisionRadius) {
      D3DXVECTOR3 v3NewEye =
          v3CollisionPoint + 2.0f * m_fTerrainCollisionRadius * m_v3Up;
      // printf("CameraBottomToTerrain new %f > old %f", v3NewEye.z, m_v3Eye.z);
      SetEye(v3NewEye);
    }
  } else
    SetCameraState(CAMERA_STATE_NORMAL);
}

struct CameraCollisionChecker {
  bool m_isBlocked;
  std::vector<D3DXVECTOR3> *m_pkVct_v3Position;
  CDynamicSphereInstance *m_pdsi;

  CameraCollisionChecker(CDynamicSphereInstance *pdsi,
                         std::vector<D3DXVECTOR3> *pkVct_v3Position)
      : m_pdsi(pdsi), m_pkVct_v3Position(pkVct_v3Position), m_isBlocked(false) {
  }
  void operator()(CGraphicObjectInstance *pOpponent) {
    if (pOpponent->CollisionDynamicSphere(*m_pdsi)) {
      m_pkVct_v3Position->push_back(pOpponent->GetPosition());
      m_isBlocked = true;
    }
  }
};

void CCamera::ProcessBuildingCollision() {
  float fMoveAmountSmall = 2.0f;
  float fMoveAmountLarge = 4.0f;

  D3DXVECTOR3 v3CheckVector;

  CDynamicSphereInstance s;
  s.fRadius = m_fObjectCollisionRadius;
  s.v3LastPosition = m_v3Eye;

  Vector3d aVector3d;
  aVector3d.Set(m_v3Eye.x, m_v3Eye.y, m_v3Eye.z);

  // 뒤

  CCullingManager &rkCullingMgr = CCullingManager::Instance();

  {
    v3CheckVector = m_v3Eye - m_fObjectCollisionRadius * m_v3View;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      if (m_v3AngularVelocity.y > 0.0f) {
        m_v3AngularVelocity.y = 0.0f;
        m_v3AngularVelocity.z += fMoveAmountLarge;
      }
      //		m_v3AngularVelocity.y = fMAX(fMoveAmountSmall,
      // m_v3AngularVelocity.y);
      //// 		m_v3AngularVelocity.y += fMoveAmountSmall;

      if (kVct_kPosition.size() > 1) {
        //			m_v3AngularVelocity.z = fMAX(fMoveAmountSmall,
        // m_v3AngularVelocity.z);
        m_v3AngularVelocity.z += fMoveAmountSmall;
      } else {
        auto d3dd = (kVct_kPosition[0] - m_v3Eye);
        D3DXVec3Cross(&v3CheckVector, &d3dd, &m_v3View);
        float fDot = D3DXVec3Dot(&v3CheckVector, &m_v3Up);
        if (fDot < 0) {
          //				m_v3AngularVelocity.x =
          // fMIN(-fMoveAmountSmall, m_v3AngularVelocity.x);
          m_v3AngularVelocity.x -= fMoveAmountSmall;
        } else if (fDot > 0) {
          //				m_v3AngularVelocity.x =
          // fMAX(fMoveAmountSmall, m_v3AngularVelocity.x);
          m_v3AngularVelocity.x += fMoveAmountSmall;
        } else {
          //				m_v3AngularVelocity.z =
          // fMAX(fMoveAmountSmall, m_v3AngularVelocity.z);
          m_v3AngularVelocity.z += fMoveAmountSmall;
        }
      }
    }
  }

  // 위
  {
    v3CheckVector = m_v3Eye + 2.0f * m_fObjectCollisionRadius * m_v3Up;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      m_v3AngularVelocity.z = fMIN(-fMoveAmountSmall, m_v3AngularVelocity.y);
      // 		m_v3AngularVelocity.z -= 1.0f;
    }
  }

  // 좌
  {
    v3CheckVector = m_v3Eye + 3.0f * m_fObjectCollisionRadius * m_v3Cross;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      if (m_v3AngularVelocity.x > 0.0f) {
        m_v3AngularVelocity.x = 0.0f;
        m_v3AngularVelocity.y += fMoveAmountLarge;
      }
    }
  }

  // 우
  {
    v3CheckVector = m_v3Eye - 3.0f * m_fObjectCollisionRadius * m_v3Cross;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      if (m_v3AngularVelocity.x < 0.0f) {
        m_v3AngularVelocity.x = 0.0f;
        m_v3AngularVelocity.y += fMoveAmountLarge;
      }
    }
  }

  // 아래
  {
    v3CheckVector = m_v3Eye - 2.0f * m_fTerrainCollisionRadius * m_v3Up;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      if (m_v3AngularVelocity.z < 0.0f) {
        m_v3AngularVelocity.z = 0.0f;
        m_v3AngularVelocity.y += fMoveAmountLarge;
      }
    }
  }

  // 앞
  {
    v3CheckVector = m_v3Eye + 4.0f * m_fObjectCollisionRadius * m_v3View;
    s.v3Position = v3CheckVector;

    std::vector<D3DXVECTOR3> kVct_kPosition;
    CameraCollisionChecker kCameraCollisionChecker(&s, &kVct_kPosition);
    rkCullingMgr.ForInRange(aVector3d, m_fObjectCollisionRadius,
                            &kCameraCollisionChecker);
    bool bCollide = kCameraCollisionChecker.m_isBlocked;

    if (bCollide) {
      if (m_v3AngularVelocity.y < 0.0f) {
        m_v3AngularVelocity.y = 0.0f;
        m_v3AngularVelocity.z += fMoveAmountLarge;
      }

      if (kVct_kPosition.size() > 1) {
        //			m_v3AngularVelocity.z = fMAX(fMoveAmountLarge,
        // m_v3AngularVelocity.z);
        m_v3AngularVelocity.z += fMoveAmountLarge;
      } else {
        auto d3dd = (kVct_kPosition[0] - m_v3Eye);
        D3DXVec3Cross(&v3CheckVector, &d3dd, &m_v3View);
        float fDot = D3DXVec3Dot(&v3CheckVector, &m_v3Up);
        if (fDot < 0) {
          // 				m_v3AngularVelocity.x =
          // fMIN(-fMoveAmountSmall, m_v3AngularVelocity.x);
          m_v3AngularVelocity.x -= fMoveAmountSmall;
        } else if (fDot > 0) {
          // 				m_v3AngularVelocity.x =
          // fMAX(fMoveAmountSmall, m_v3AngularVelocity.x);
          m_v3AngularVelocity.x += fMoveAmountSmall;
        } else {
          //				m_v3AngularVelocity.z =
          // fMAX(fMoveAmountSmall, m_v3AngularVelocity.z);
          m_v3AngularVelocity.z += fMoveAmountSmall;
        }
      }
    }
  }
}

void CCamera::Update() {
  // 	ProcessBuildingCollision();

  RotateEyeAroundTarget(m_v3AngularVelocity.z, m_v3AngularVelocity.x);

  float fNewDistance =
      fMAX(CAMERA_MIN_DISTANCE,
           fMIN(CAMERA_MAX_DISTANCE, GetDistance() - m_v3AngularVelocity.y));
  SetDistance(fNewDistance);

  if (m_bProcessTerrainCollision)
    ProcessTerrainCollision();

  m_v3AngularVelocity *= 0.5f;
  if (fabs(m_v3AngularVelocity.x) < 1.0f)
    m_v3AngularVelocity.x = 0.0f;
  if (fabs(m_v3AngularVelocity.y) < 1.0f)
    m_v3AngularVelocity.y = 0.0f;
  if (fabs(m_v3AngularVelocity.z) < 1.0f)
    m_v3AngularVelocity.z = 0.0f;

  const float CAMERA_MOVABLE_DISTANCE =
      CAMERA_MAX_DISTANCE - CAMERA_MIN_DISTANCE;
  const float CAMERA_TARGET_DELTA = CAMERA_TARGET_FACE - CAMERA_TARGET_STANDARD;
  float fCameraCurMovableDistance = CAMERA_MAX_DISTANCE - GetDistance();
  float fNewTargetHeight =
      CAMERA_TARGET_STANDARD +
      CAMERA_TARGET_DELTA * fCameraCurMovableDistance / CAMERA_MOVABLE_DISTANCE;

  SetTargetHeight(fNewTargetHeight);

  // Cinematic
#ifdef __20040725_CAMERA_WORK__
  m_MovementPosition += m_MovementSpeed;

  if (0.0f != m_MovementPosition.m_fViewDir)
    MoveFront(m_MovementPosition.m_fViewDir);
  if (0.0f != m_MovementPosition.m_fCrossDir)
    MoveAlongCross(m_MovementPosition.m_fCrossDir);
  if (0.0f != m_MovementPosition.m_fUpDir)
    MoveVertical(m_MovementPosition.m_fUpDir);
#endif
}
