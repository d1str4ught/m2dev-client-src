#include "StdAfx.h"
#include "InstanceBase.h"
#include "PythonBackground.h"

void CInstanceBase::SCRIPT_SetPixelPosition(float fx, float fy) {
  float fz = __GetBackgroundHeight(fx, fy);
  NEW_SetPixelPosition(TPixelPosition(fx, fy, fz));
}

void CInstanceBase::NEW_SetPixelPosition(
    const TPixelPosition &c_rPixelPosition) {
  m_GraphicThingInstance.SetCurPixelPosition(c_rPixelPosition);
}

void CInstanceBase::NEW_GetPixelPosition(TPixelPosition *pPixelPosition) {
  *pPixelPosition = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
}

void CInstanceBase::SetRotation(float fRotation) {
  m_GraphicThingInstance.SetRotation(fRotation);
}

void CInstanceBase::BlendRotation(float fRotation, float fBlendTime) {
  m_GraphicThingInstance.BlendRotation(fRotation, fBlendTime);
}

void CInstanceBase::NEW_LookAtFlyTarget() {
  m_GraphicThingInstance.LookAtFlyTarget();
}

void CInstanceBase::NEW_LookAtDestPixelPosition(
    const TPixelPosition &c_rkPPosDst) {
  m_GraphicThingInstance.LookAt(c_rkPPosDst.x, -c_rkPPosDst.y);
}

void CInstanceBase::NEW_LookAtDestInstance(CInstanceBase &rkInstDst) {
  m_GraphicThingInstance.LookAt(&rkInstDst.m_GraphicThingInstance);
  // 	Tracenf("LookAt %f", m_GraphicThingInstance.GetTargetRotation());
}

float CInstanceBase::GetRotation() {
  return m_GraphicThingInstance.GetRotation();
}

float CInstanceBase::GetAdvancingRotation() {
  return m_GraphicThingInstance.GetAdvancingRotation();
}

void CInstanceBase::SetDirection(int dir) {
  float fDegree = GetDegreeFromDirection(dir);
  SetRotation(fDegree);
  SetAdvancingRotation(fDegree);
}

void CInstanceBase::BlendDirection(int dir, float blendTime) {
  m_GraphicThingInstance.BlendRotation(GetDegreeFromDirection(dir), blendTime);
}

float CInstanceBase::GetDegreeFromDirection(int dir) {
  if (dir < 0)
    return 0.0f;

  if (dir >= DIR_MAX_NUM)
    return 0.0f;

  static float s_dirRot[DIR_MAX_NUM] = {
      +45.0f * 4, +45.0f * 3,     +45.0f * 2,         +45.0f,
      +0.0f,      360.0f - 45.0f, 360.0f - 45.0f * 2, 360.0f - 45.0f * 3,
  };

  return s_dirRot[dir];
}
