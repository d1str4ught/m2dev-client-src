#include "StdAfx.h"
#include "InstanceBase.h"

CActorInstance::IEventHandler &CInstanceBase::GetEventHandlerRef() {
  return m_GraphicThingInstance.__GetEventHandlerRef();
}

CActorInstance::IEventHandler *CInstanceBase::GetEventHandlerPtr() {
  return m_GraphicThingInstance.__GetEventHandlerPtr();
}

void CInstanceBase::SetEventHandler(
    CActorInstance::IEventHandler *pkEventHandler) {
  m_GraphicThingInstance.SetEventHandler(pkEventHandler);
}
