#include "StdAfx.h"
#include "ActorInstance.h"

void CActorInstance::SetXYRotation(float fRotX, float fRotY) {
  m_rotX = fRotX;
  m_rotY = fRotY;
}

void CActorInstance::SetRotation(float fRot) {
  if (m_pkHorse)
    m_pkHorse->SetRotation(fRot);

  m_fcurRotation = fRot;
  m_rotBegin = m_fcurRotation;
  m_rotEnd = m_fcurRotation;

  m_rotBlendTime = 0.0f;
  m_rotBeginTime = 0.0f;
  m_rotEndTime = 0.0f;

  m_bNeedUpdateCollision = TRUE;
}

void CActorInstance::BlendRotation(float fRot, float fBlendTime) {
  if (m_pkHorse)
    m_pkHorse->BlendRotation(fRot, fBlendTime);

  if (m_fcurRotation == fRot)
    return;

  m_rotBegin = fmod(m_fcurRotation, 360.0f);
  m_rotEnd = fRot;

  m_rotBlendTime = fBlendTime;
  m_rotBeginTime = GetLocalTime();
  m_rotEndTime = m_rotBeginTime + m_rotBlendTime;
}

void CActorInstance::SetAdvancingRotation(float fRot) {
  if (m_pkHorse)
    m_pkHorse->SetAdvancingRotation(fRot);

  m_fAdvancingRotation = fRot;
}

void CActorInstance::RotationProcess() {
  if (m_pkHorse)
    m_pkHorse->RotationProcess();

  if (GetLocalTime() < m_rotEndTime) {
    m_fcurRotation = GetInterpolatedRotation(m_rotBegin, m_rotEnd,
                                             (GetLocalTime() - m_rotBeginTime) /
                                                 m_rotBlendTime);
    SetAdvancingRotation(m_fcurRotation);
  } else {
    m_fcurRotation = m_rotEnd;
  }

  // FIXME : "건물일때만 체크"로 바꾼다. - [levites]
  if (0.0f != m_rotX || 0.0f != m_rotY) {
    CGraphicObjectInstance::SetRotation(m_rotX, m_rotY, m_fcurRotation);
  } else {
    CGraphicObjectInstance::SetRotation(m_fcurRotation);
  }
}

void CActorInstance::LookAtFromXY(float x, float y,
                                  CActorInstance *pDestInstance) {
  float rot =
      GetDegreeFromPosition2(pDestInstance->m_x, pDestInstance->m_y, x, y);

  LookAt(rot);
}

void CActorInstance::LookAt(float fDirRot) { BlendRotation(fDirRot, 0.3f); }

void CActorInstance::LookAt(float fx, float fy) {
  float rot = GetDegreeFromPosition2(m_x, m_y, fx, fy);

  LookAt(rot);
}

void CActorInstance::LookAt(CActorInstance *pInstance) {
  TPixelPosition PixelPosition;
  pInstance->GetPixelPosition(&PixelPosition);
  LookAt(PixelPosition.x, PixelPosition.y);
}

void CActorInstance::LookWith(CActorInstance *pInstance) {
  BlendRotation(pInstance->m_rotEnd, 0.3f);
}

float CActorInstance::GetRotation() { return m_fcurRotation; }

float CActorInstance::GetTargetRotation() { return m_rotEnd; }

float CActorInstance::GetRotatingTime() { return m_rotEndTime; }

float CActorInstance::GetAdvancingRotation() { return m_fAdvancingRotation; }
