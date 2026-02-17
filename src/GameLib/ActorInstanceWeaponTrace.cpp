#include "StdAfx.h"
#include "ActorInstance.h"
#include "WeaponTrace.h"

void CActorInstance::TraceProcess() {
  if (!m_WeaponTraceVector.empty()) {
    std::vector<CWeaponTrace *>::iterator it;
    for (it = m_WeaponTraceVector.begin(); it != m_WeaponTraceVector.end();
         ++it) {
      CWeaponTrace *pWeaponTrace = (*it);
      pWeaponTrace->SetPosition(m_x, m_y, m_z);
      pWeaponTrace->SetRotation(m_fcurRotation);
      pWeaponTrace->Update(__GetReachScale());
    }
  }
}

void CActorInstance::RenderTrace() {
  for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
           std::mem_fn(&CWeaponTrace::Render));
}

void CActorInstance::__DestroyWeaponTrace() {
  std::for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
                CWeaponTrace::Delete);
  m_WeaponTraceVector.clear();
}

void CActorInstance::__ShowWeaponTrace() {
  for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
           std::mem_fn(&CWeaponTrace::TurnOn));
}

void CActorInstance::__HideWeaponTrace() {
  for_each(m_WeaponTraceVector.begin(), m_WeaponTraceVector.end(),
           std::mem_fn(&CWeaponTrace::TurnOff));
}
