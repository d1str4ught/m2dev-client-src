#include "StdAfx.h"
#include "FlyTarget.h"

CFlyTarget::CFlyTarget() { __Initialize(); }

CFlyTarget::CFlyTarget(IFlyTargetableObject *pFlyTarget) {
  __Initialize();

  m_eType = TYPE_OBJECT;
  m_pFlyTarget = pFlyTarget;
  m_pFlyTarget->AddFlyTargeter(this);
}
CFlyTarget::CFlyTarget(const D3DXVECTOR3 &v3FlyTargetPosition) {
  __Initialize();

  m_eType = TYPE_POSITION;
  m_v3FlyTargetPosition = v3FlyTargetPosition;
}

CFlyTarget::CFlyTarget(const CFlyTarget &rhs) {
  __Initialize();

  *this = rhs;
}

CFlyTarget::~CFlyTarget() {
  if (m_pFlyTarget)
    m_pFlyTarget->RemoveFlyTargeter(this);
}

void CFlyTarget::__Initialize() {
  m_v3FlyTargetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_pFlyTarget = NULL;
  m_eType = TYPE_NONE;
}

void CFlyTarget::Clear() {
  m_eType = TYPE_NONE;
  m_pFlyTarget = 0;
}

bool CFlyTarget::IsObject() { return (TYPE_OBJECT == GetType()); }

bool CFlyTarget::IsPosition() { return (TYPE_POSITION == GetType()); }

bool CFlyTarget::IsValidTarget() { return (TYPE_NONE != GetType()); }

void CFlyTarget::NotifyTargetClear() {
  // if (m_eType == FLY_TARGET_TYPE_OBJECT)
  m_eType = TYPE_POSITION;
  m_pFlyTarget = 0;
}

CFlyTarget::EType CFlyTarget::GetType() { return m_eType; }

IFlyTargetableObject *CFlyTarget::GetFlyTarget() {
  assert(TYPE_OBJECT == GetType() && "CFly::GetFlyTarget");
  return m_pFlyTarget;
}

const D3DXVECTOR3 &CFlyTarget::GetFlyTargetPosition() const {
  if (m_eType == TYPE_OBJECT) {
    m_v3FlyTargetPosition = m_pFlyTarget->OnGetFlyTargetPosition();
  }
  return m_v3FlyTargetPosition;
}

CFlyTarget &CFlyTarget::operator=(const CFlyTarget &rhs) {
  if (m_pFlyTarget) {
    m_pFlyTarget->RemoveFlyTargeter(this);
  }
  m_eType = rhs.m_eType;
  m_v3FlyTargetPosition = rhs.m_v3FlyTargetPosition;
  m_pFlyTarget = rhs.m_pFlyTarget;
  if (m_pFlyTarget) {
    m_pFlyTarget->AddFlyTargeter(this);
  }
  return *this;
}

void CFlyTarget::GetFlyTargetData(CFlyTarget *pFlyTarget) {
  pFlyTarget->m_eType = m_eType;
  pFlyTarget->m_v3FlyTargetPosition = m_v3FlyTargetPosition;
  pFlyTarget->m_pFlyTarget = m_pFlyTarget;
}
