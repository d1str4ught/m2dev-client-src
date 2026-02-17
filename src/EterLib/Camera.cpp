// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EterBase/Utils.h"
#include "Camera.h"
#include <cmath>

const float c_fDefaultResistance = 0.3f;

CCameraManager aCameraManager; // CCameraManager Instance

void CCamera::SetCameraMaxDistance(float fMax) { CAMERA_MAX_DISTANCE = fMax; }

float CCamera::GetTargetHeight() { return m_fTarget_; }

void CCamera::SetTargetHeight(float fTarget) { m_fTarget_ = fTarget; }

//////////////////////////////////////////////////////////////////////////
// CCamera
//////////////////////////////////////////////////////////////////////////

CCamera::CCamera()
    : m_fEyeGroundHeightRatio(0.3f), m_fTargetHeightLimitRatio(2.0f),
      m_fResistance(c_fDefaultResistance), m_isLock(false) {
  m_fDistance = 1.0f;
  m_eCameraState = CAMERA_STATE_NORMAL;
  m_eCameraStatePrev = CAMERA_STATE_NORMAL;
  m_ulNumScreenBuilding = 0;

  m_fPitchSum = 0.0f;
  m_fRollSum = 0.0f;

  m_fTerrainCollisionRadius = 50.0f;
  m_fObjectCollisionRadius = 50.0f;

  m_bDrag = false;

  m_lMousePosX = -1;
  m_lMousePosY = -1;

  m_fTarget_ = CAMERA_TARGET_STANDARD;

  m_v3AngularAcceleration = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_v3AngularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

  m_bProcessTerrainCollision = true;

  SetViewParams(D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                D3DXVECTOR3(0.0f, 1.0f, 0.0f));
}

CCamera::~CCamera() {}

void CCamera::Lock() { m_isLock = true; }

void CCamera::Unlock() { m_isLock = false; }

bool CCamera::IsLock() { return m_isLock; }

void CCamera::SetResistance(float fResistance) {
  m_fResistance = c_fDefaultResistance * fResistance;
}

void CCamera::Wheel(int nLen) {
  if (IsLock())
    return;

  m_v3AngularVelocity.y = (float)(nLen)*m_fResistance;
}

void CCamera::BeginDrag(int nMouseX, int nMouseY) {
  if (IsLock())
    return;

  m_bDrag = true;
  m_lMousePosX = nMouseX;
  m_lMousePosY = nMouseY;
  m_fPitchSum = 0.0f;
  m_fRollSum = 0.0f;
}

bool CCamera::IsDraging() {
  if (IsLock())
    return false;

  return m_bDrag;
}

bool CCamera::EndDrag() {
  if (IsLock())
    return false;

  m_bDrag = false;

  float fSum = sqrt(m_fPitchSum * m_fPitchSum + m_fRollSum * m_fRollSum);

  m_fPitchSum = 0.0f;
  m_fRollSum = 0.0f;

  if (fSum < 1.0f)
    return false;

  return true;
}

bool CCamera::Drag(int nMouseX, int nMouseY, LPPOINT lpReturnPoint) {
  if (IsLock())
    return false;

  if (!m_bDrag) {
    m_lMousePosX = nMouseX;
    m_lMousePosY = nMouseY;
    lpReturnPoint->x = m_lMousePosX;
    lpReturnPoint->y = m_lMousePosY;
    return false;
  }

  long lMouseX = nMouseX;
  long lMouseY = nMouseY;

  float fNewPitchVelocity = (float)(lMouseY - m_lMousePosY) * m_fResistance;
  float fNewRotationVelocity = (float)(lMouseX - m_lMousePosX) * m_fResistance;

  m_fPitchSum += fNewPitchVelocity;
  m_fRollSum += fNewRotationVelocity;

  if (CAMERA_STATE_CANTGOLEFT == GetCameraState())
    fNewRotationVelocity = fMAX(0.0f, fNewRotationVelocity);
  if (CAMERA_STATE_CANTGORIGHT == GetCameraState())
    fNewRotationVelocity = fMIN(0.0f, fNewRotationVelocity);
  if (CAMERA_STATE_CANTGODOWN == GetCameraState())
    fNewPitchVelocity = fMAX(0.0f, fNewPitchVelocity);

  m_v3AngularVelocity.x = fNewRotationVelocity;
  m_v3AngularVelocity.z = fNewPitchVelocity;

  lpReturnPoint->x = m_lMousePosX;
  lpReturnPoint->y = m_lMousePosY;
  return true;
}

//////////////////////////////////////////////////////////////////////////
// Update

void CCamera::SetCameraState(eCameraState eNewCameraState) {
  if (eNewCameraState == m_eCameraState)
    return;

  m_eCameraStatePrev = m_eCameraState;
  m_eCameraState = eNewCameraState;

  /*
          if ((CAMERA_STATE_NORMAL == m_eCameraStatePrev))
          {
                  m_fDistanceBackup = m_fDistance;
                  m_fPitchBackup = m_fPitch;
                  m_fRollBackup = m_fRoll;
          }
          else if ((CAMERA_STATE_CANTGODOWN == m_eCameraStatePrev) &&
     (CAMERA_STATE_CANTGODOWN == m_eCameraState) )
          {
                  m_v3EyeBackup = m_v3Eye;
          }
  */
}

void CCamera::IncreaseNumSrcreenBuilding() { ++m_ulNumScreenBuilding; }

void CCamera::ResetNumScreenBuilding() { m_ulNumScreenBuilding = 0; }

//////////////////////////////////////////////////////////////////////////
// Property
void CCamera::SetViewParams(const D3DXVECTOR3 &v3Eye,
                            const D3DXVECTOR3 &v3Target,
                            const D3DXVECTOR3 &v3Up) {
  if (IsLock())
    return;

  // Set attributes for the view matrix
  m_v3Eye = v3Eye;
  m_v3Target = v3Target;
  m_v3Up = v3Up;

  // Avoid screen flipping with nan values in the view matrix
  if (!m_v3Eye.y)
    m_v3Eye.y = 0.1f;

  SetViewMatrix();
}

void CCamera::SetEye(const D3DXVECTOR3 &v3Eye) {
  if (IsLock())
    return;

  m_v3Eye = v3Eye;

  SetViewMatrix();
}

void CCamera::SetTarget(const D3DXVECTOR3 &v3Target) {
  if (IsLock())
    return;

  m_v3Target = v3Target;

  SetViewMatrix();
}

void CCamera::SetUp(const D3DXVECTOR3 &v3Up) {
  if (IsLock())
    return;

  m_v3Up = v3Up;

  SetViewMatrix();
}

bool IsNaN(const D3DXVECTOR3 &v) {
  return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);
}

void CCamera::SetViewMatrix() {
  // Calculate view direction
  m_v3View = m_v3Target - m_v3Eye;
  if (IsNaN(m_v3View)) {
    m_v3View = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  }

  // v3CenterRay is the reverse of the view vector
  D3DXVECTOR3 v3CenterRay = -m_v3View;

  // Calculate roll (if this function uses m_v3View, ensure it’s valid)
  CalculateRoll();

  // Compute the distance from eye to target
  m_fDistance = D3DXVec3Length(&m_v3View);
  if (std::isnan(m_fDistance)) {
    m_fDistance = 0.0f;
  }
  m_fDistance = std::fmax(0.0f, m_fDistance);
  assert(m_fDistance >= 0);

  // Normalize the view vector if possible
  if (m_fDistance > FLT_EPSILON) {
    D3DXVec3Normalize(&m_v3View, &m_v3View);
  } else {
    // Avoid dividing by zero; set to a default forward direction
    m_v3View = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  }

  // Compute the cross product for the right vector and normalize
  D3DXVec3Cross(&m_v3Cross, &m_v3Up, &m_v3View);
  float crossLength = D3DXVec3Length(&m_v3Cross);
  if (crossLength > FLT_EPSILON) {
    D3DXVec3Normalize(&m_v3Cross, &m_v3Cross);
  } else {
    // Use a default right vector if the cross product is near zero
    m_v3Cross = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
  }

  // Recompute the up vector and normalize
  D3DXVec3Cross(&m_v3Up, &m_v3View, &m_v3Cross);
  float upLength = D3DXVec3Length(&m_v3Up);
  if (upLength > FLT_EPSILON) {
    D3DXVec3Normalize(&m_v3Up, &m_v3Up);
  } else {
    // Use a default up vector.
    m_v3Up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
  }

  // Calculate the pitch angle from the up vector
  D3DXVECTOR3 val = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  m_fPitch = D3DXVec3Dot(&m_v3Up, &val);
  // Clamp the dot product so acosf is safe
  if (m_fPitch >= 1.0f)
    m_fPitch = 1.0f;
  else if (m_fPitch <= -1.0f)
    m_fPitch = -1.0f;
  m_fPitch = acosf(m_fPitch);
  if (std::isnan(m_fPitch)) {
    m_fPitch = 0.0f;
  }
  m_fPitch *= (180.0f / D3DX_PI);
  if (m_v3View.z > 0)
    m_fPitch = -m_fPitch;

  // Build the view matrix.
  D3DXMatrixLookAtRH(&m_matView, &m_v3Eye, &m_v3Target, &m_v3Up);

  // Compute the determinant and check it.
  float fDeterminantD3DMatView = D3DXMatrixDeterminant(&m_matView);
  if (std::isnan(fDeterminantD3DMatView) ||
      fabs(fDeterminantD3DMatView) < FLT_EPSILON) {
    D3DXMatrixIdentity(&m_matInverseView);
  } else {
    D3DXMatrixInverse(&m_matInverseView, &fDeterminantD3DMatView, &m_matView);
  }

  m_matBillboard = m_matInverseView;
  m_matBillboard._41 = 0.0f;
  m_matBillboard._42 = 0.0f;
  m_matBillboard._43 = 0.0f;

  m_ViewRay.SetStartPoint(m_v3Target);
  m_ViewRay.SetDirection(v3CenterRay, m_fDistance);

  m_kCameraBottomToTerrainRay.SetStartPoint(m_v3Eye);
  m_kCameraFrontToTerrainRay.SetStartPoint(m_v3Eye);
  m_kCameraBackToTerrainRay.SetStartPoint(m_v3Eye);
  m_kCameraLeftToTerrainRay.SetStartPoint(m_v3Eye);
  m_kCameraRightToTerrainRay.SetStartPoint(m_v3Eye);
  m_kTargetToCameraBottomRay.SetStartPoint(m_v3Target);

  m_kCameraBottomToTerrainRay.SetDirection(-m_v3Up,
                                           2.0f * m_fTerrainCollisionRadius);
  m_kCameraFrontToTerrainRay.SetDirection(m_v3View,
                                          4.0f * m_fTerrainCollisionRadius);
  m_kCameraBackToTerrainRay.SetDirection(-m_v3View, m_fTerrainCollisionRadius);
  m_kCameraLeftToTerrainRay.SetDirection(-m_v3Cross,
                                         3.0f * m_fTerrainCollisionRadius);
  m_kCameraRightToTerrainRay.SetDirection(m_v3Cross,
                                          3.0f * m_fTerrainCollisionRadius);

  D3DXVECTOR3 temp = (v3CenterRay - m_fTerrainCollisionRadius * m_v3Up);
  m_kTargetToCameraBottomRay.SetDirection(
      v3CenterRay - m_fTerrainCollisionRadius * m_v3Up, D3DXVec3Length(&temp));

  m_kLeftObjectCollisionRay.SetStartPoint(m_v3Target);
  m_kTopObjectCollisionRay.SetStartPoint(m_v3Target);
  m_kRightObjectCollisionRay.SetStartPoint(m_v3Target);
  m_kBottomObjectCollisionRay.SetStartPoint(m_v3Target);

  D3DXVECTOR3 val1 = (v3CenterRay + m_fObjectCollisionRadius * m_v3Cross);
  m_kLeftObjectCollisionRay.SetDirection(val1, D3DXVec3Length(&val1));

  D3DXVECTOR3 val2 = (v3CenterRay - m_fObjectCollisionRadius * m_v3Cross);
  m_kRightObjectCollisionRay.SetDirection(val2, D3DXVec3Length(&val2));

  D3DXVECTOR3 val3 = (v3CenterRay + m_fObjectCollisionRadius * m_v3Up);
  m_kTopObjectCollisionRay.SetDirection(val3, D3DXVec3Length(&val3));

  D3DXVECTOR3 val4 = (v3CenterRay - m_fObjectCollisionRadius * m_v3Up);
  m_kBottomObjectCollisionRay.SetDirection(val4, D3DXVec3Length(&val4));
}

void CCamera::Move(const D3DXVECTOR3 &v3Displacement) {
  if (IsLock())
    return;

  m_v3Eye += v3Displacement;
  m_v3Target += v3Displacement;

  SetViewMatrix();
}

void CCamera::Zoom(float fRatio) {
  if (IsLock())
    return;

  if (fRatio == 1.0f)
    return;

  D3DXVECTOR3 v3Temp = m_v3Eye - m_v3Target;
  v3Temp *= fRatio;
  m_v3Eye = v3Temp + m_v3Target;

  SetViewMatrix();
}

void CCamera::MoveAlongView(float fDistance) {
  if (IsLock())
    return;

  D3DXVECTOR3 v3Temp;
  D3DXVec3Normalize(&v3Temp, &m_v3View);

  m_v3Eye += v3Temp * fDistance;
  m_v3Target += v3Temp * fDistance;

  SetViewMatrix();
}

void CCamera::MoveAlongCross(float fDistance) {
  if (IsLock())
    return;

  D3DXVECTOR3 v3Temp;
  D3DXVec3Normalize(&v3Temp, &m_v3Cross);

  m_v3Eye += v3Temp * fDistance;
  m_v3Target += v3Temp * fDistance;

  SetViewMatrix();
}

void CCamera::MoveAlongUp(FLOAT fDistance) {
  if (IsLock())
    return;

  D3DXVECTOR3 v3Temp;
  D3DXVec3Normalize(&v3Temp, &m_v3Up);
  m_v3Target += v3Temp * fDistance;
  m_v3Eye += v3Temp * fDistance;
  SetViewMatrix();
}

void CCamera::MoveLateral(float fDistance) {
  if (IsLock())
    return;

  MoveAlongCross(fDistance);
}

void CCamera::MoveFront(float fDistance) {
  if (IsLock())
    return;

  D3DXVECTOR3 v3Temp = D3DXVECTOR3(m_v3View.x, m_v3View.y, 0.0f);
  D3DXVec3Normalize(&v3Temp, &v3Temp);

  m_v3Eye += v3Temp * fDistance;
  m_v3Target += v3Temp * fDistance;

  SetViewMatrix();
}

void CCamera::MoveVertical(float fDistance) {
  if (IsLock())
    return;

  m_v3Eye.z += fDistance;
  m_v3Target.z += fDistance;

  SetViewMatrix();
}

// void CCamera::RotateUpper(float fDegree)
//{
//	D3DXMATRIX matRot;
//	D3DXMatrixRotationAxis(&matRot, &m_v3Cross, -D3DXToRadian(fDegree));
//	D3DXVec3TransformCoord(&m_v3View, &m_v3View, &matRot) ;
//     D3DXVec3Cross(&m_v3Up, &m_v3View, &m_v3Cross);
//
//	m_v3Target = m_v3Eye + m_v3View;
//
//	SetViewMatrix() ;
// }

void CCamera::RotateEyeAroundTarget(float fPitchDegree, float fRollDegree) {
  if (IsLock())
    return;

  D3DXMATRIX matRot, matRotPitch, matRotRoll;

  // 머리위로 넘어가기 막기...
  if (m_fPitch + fPitchDegree > 80.0f) {
    fPitchDegree = 80.0f - m_fPitch;
  } else if (m_fPitch + fPitchDegree < -80.0f) {
    fPitchDegree = -80.0f - m_fPitch;
  }

  D3DXMatrixRotationAxis(&matRotPitch, &m_v3Cross, D3DXToRadian(fPitchDegree));

  D3DXMatrixRotationZ(&matRotRoll, -D3DXToRadian(fRollDegree));
  matRot = matRotPitch * matRotRoll;

  D3DXVECTOR3 v3Temp = m_v3Eye - m_v3Target;
  D3DXVec3TransformCoord(&m_v3Eye, &v3Temp, &matRot);
  m_v3Eye += m_v3Target;

  SetUp(D3DXVECTOR3(0.0f, 0.0f, 1.0f));

  m_fRoll += fRollDegree;

  if (m_fRoll > 360.0f)
    m_fRoll -= 360.0f;
  else if (m_fRoll < -360.0f)
    m_fRoll += 360.0f;
}

void CCamera::RotateEyeAroundPoint(const D3DXVECTOR3 &v3Point,
                                   float fPitchDegree, float fRollDegree) {
  //	if (IsLock())
  //		return;

  D3DXMATRIX matRot, matRotPitch, matRotRoll;

  D3DXMatrixRotationAxis(&matRotPitch, &m_v3Cross, D3DXToRadian(fPitchDegree));

  D3DXMatrixRotationZ(&matRotRoll, -D3DXToRadian(fRollDegree));
  matRot = matRotPitch * matRotRoll;

  D3DXVECTOR3 v3Temp = m_v3Eye - v3Point;
  D3DXVec3TransformCoord(&m_v3Eye, &v3Temp, &matRot);
  m_v3Eye += v3Point;

  const auto vv2 = (v3Temp + m_v3Up);
  D3DXVec3TransformCoord(&m_v3Up, &vv2, &matRot);
  m_v3Up -= (m_v3Eye - v3Point);

  v3Temp = m_v3Target - v3Point;
  D3DXVec3TransformCoord(&m_v3Target, &v3Temp, &matRot);
  m_v3Target += v3Point;

  SetViewMatrix();
}

void CCamera::Pitch(const float fPitchDelta) {
  //	if (IsLock())
  //		return;

  RotateEyeAroundTarget(fPitchDelta, 0.0f);
}

void CCamera::Roll(const float fRollDelta) {
  //	if (IsLock())
  //		return;

  RotateEyeAroundTarget(0.0f, fRollDelta);
}

void CCamera::SetDistance(const float fdistance) {
  //	if (IsLock())
  //		return;

  Zoom(fdistance / m_fDistance);
}

void CCamera::CalculateRoll() {
  D3DXVECTOR2 v2ViewXY;
  v2ViewXY.x = m_v3View.x;
  v2ViewXY.y = m_v3View.y;
  D3DXVec2Normalize(&v2ViewXY, &v2ViewXY);
  const auto vv = D3DXVECTOR2(0.0f, 1.0f);
  float fDot = D3DXVec2Dot(&v2ViewXY, &vv);
  if (fDot >= 1)
    fDot = 1;
  else if (fDot <= -1)
    fDot = -1;
  fDot = acosf(fDot);
  fDot *= (180.0f / D3DX_PI);
  float fCross = D3DXVec2CCW(&v2ViewXY, &vv);
  if (0 > fCross) {
    fDot = -fDot;
  }

  m_fRoll = fDot;
}

//////////////////////////////////////////////////////////////////////////
// CCameraMananger
//////////////////////////////////////////////////////////////////////////

CCameraManager::CCameraManager()
    : m_pCurrentCamera(NULL), m_pPreviousCamera(NULL) {
  AddCamera(DEFAULT_PERSPECTIVE_CAMERA);
  AddCamera(DEFAULT_ORTHO_CAMERA);

  SetCurrentCamera(DEFAULT_PERSPECTIVE_CAMERA);
}

CCameraManager::~CCameraManager() {
  for (TCameraMap::iterator itor = m_CameraMap.begin();
       itor != m_CameraMap.end(); ++itor) {
    delete (*itor).second;
  }
  m_CameraMap.clear();
}

CCamera *CCameraManager::GetCurrentCamera() {
  if (!m_pCurrentCamera)
    assert(false);
  return m_pCurrentCamera;
}

void CCameraManager::SetCurrentCamera(unsigned char ucCameraNum) {
  if (m_pCurrentCamera != m_CameraMap[ucCameraNum])
    m_pPreviousCamera = m_pCurrentCamera;

  m_pCurrentCamera = m_CameraMap[ucCameraNum];
}

void CCameraManager::ResetToPreviousCamera() {
  if (!m_pPreviousCamera)
    assert(false);
  m_pCurrentCamera = m_pPreviousCamera;
  m_pPreviousCamera = NULL;
}

bool CCameraManager::isCurrentCamera(unsigned char ucCameraNum) {
  if (m_CameraMap[ucCameraNum] == m_pCurrentCamera)
    return true;
  return false;
}

// 잡스러운 함수들...
bool CCameraManager::AddCamera(unsigned char ucCameraNum) {
  if (m_CameraMap.end() != m_CameraMap.find(ucCameraNum))
    return false;
  m_CameraMap.insert(TCameraMap::value_type(ucCameraNum, new CCamera));
  return true;
}

bool CCameraManager::RemoveCamera(unsigned char ucCameraNum) {
  TCameraMap::iterator itor = m_CameraMap.find(ucCameraNum);
  if (m_CameraMap.end() == itor)
    return false;
  m_CameraMap.erase(itor);
  return true;
}

unsigned char CCameraManager::GetCurrentCameraNum() {
  if (!m_pCurrentCamera)
    return NO_CURRENT_CAMERA;
  for (TCameraMap::iterator itor = m_CameraMap.begin();
       itor != m_CameraMap.end(); ++itor)
    if (m_pCurrentCamera == (*itor).second)
      return (*itor).first;
  return NO_CURRENT_CAMERA;
}

bool CCameraManager::isTerrainCollisionEnable() {
  return m_pCurrentCamera->isTerrainCollisionEnable();
}

void CCameraManager::SetTerrainCollision(bool bEnable) {
  m_pCurrentCamera->SetTerrainCollision(bEnable);
}
