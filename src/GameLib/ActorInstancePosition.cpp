#include "StdAfx.h"
#include "ActorInstance.h"

const TPixelPosition &CActorInstance::NEW_GetLastPixelPositionRef() {
  GetBlendingPosition(&m_kPPosLast);
  m_kPPosLast.y = -m_kPPosLast.y;

  return m_kPPosLast;
}

const D3DXVECTOR3 &CActorInstance::GetPositionVectorRef() {
  m_v3Pos.x = m_x;
  m_v3Pos.y = m_y;
  m_v3Pos.z = m_z;
  return m_v3Pos;
}

const D3DXVECTOR3 &CActorInstance::GetMovementVectorRef() {
  if (m_pkHorse)
    return m_pkHorse->GetMovementVectorRef();

  return m_v3Movement;
}

void CActorInstance::NEW_SetAtkPixelPosition(
    const TPixelPosition &c_rkPPosAtk) {
  m_kPPosAtk = c_rkPPosAtk;
}

void CActorInstance::SetCurPixelPosition(const TPixelPosition &c_rkPPosCur) {
  D3DXVECTOR3 v3PosCur;
  v3PosCur.x = +c_rkPPosCur.x;
  v3PosCur.y = -c_rkPPosCur.y;
  v3PosCur.z = +c_rkPPosCur.z;

  SetPixelPosition(v3PosCur);
}

void CActorInstance::NEW_SetSrcPixelPosition(
    const TPixelPosition &c_rkPPosSrc) {
  m_kPPosSrc = c_rkPPosSrc;
}

void CActorInstance::NEW_SetDstPixelPositionZ(float z) { m_kPPosDst.z = z; }

void CActorInstance::NEW_SetDstPixelPosition(
    const TPixelPosition &c_rkPPosDst) {
  m_kPPosDst = c_rkPPosDst;
}

const TPixelPosition &CActorInstance::NEW_GetAtkPixelPositionRef() {
  return m_kPPosAtk;
}

const TPixelPosition &CActorInstance::NEW_GetSrcPixelPositionRef() {
  return m_kPPosSrc;
}

const TPixelPosition &CActorInstance::NEW_GetDstPixelPositionRef() {
  return m_kPPosDst;
}

const TPixelPosition &CActorInstance::NEW_GetCurPixelPositionRef() {
  m_kPPosCur.x = +m_x;
  m_kPPosCur.y = -m_y;
  m_kPPosCur.z = +m_z;

  return m_kPPosCur;
}

void CActorInstance::GetPixelPosition(TPixelPosition *pPixelPosition) {
  pPixelPosition->x = m_x;
  pPixelPosition->y = m_y;
  pPixelPosition->z = m_z;
}

void CActorInstance::SetPixelPosition(const TPixelPosition &c_rPixelPos) {
  if (m_pkTree) {
    __SetTreePosition(c_rPixelPos.x, c_rPixelPos.y, c_rPixelPos.z);
  }

  if (m_pkHorse)
    m_pkHorse->SetPixelPosition(c_rPixelPos);

  m_x = c_rPixelPos.x;
  m_y = c_rPixelPos.y;
  m_z = c_rPixelPos.z;
  m_bNeedUpdateCollision = TRUE;
}

void CActorInstance::__InitializePositionData() {
  m_dwShakeTime = 0;

  m_x = 0.0f;
  m_y = 0.0f;
  m_z = 0.0f;
  m_bNeedUpdateCollision = FALSE;

  m_kPPosAtk = m_kPPosLast = m_kPPosDst = m_kPPosCur = m_kPPosSrc =
      TPixelPosition(0.0f, 0.0f, 0.0f);

  __InitializeMovement();
}
