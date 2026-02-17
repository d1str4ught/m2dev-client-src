#pragma once

#include "EterGrnLib/ThingInstance.h"

class IActorInstance : public CGraphicThingInstance {
public:
  enum { ID = ACTOR_OBJECT };
  int GetType() const { return ID; }

  IActorInstance() {}
  virtual ~IActorInstance() {}
  virtual bool
  TestCollisionWithDynamicSphere(const CDynamicSphereInstance &dsi) = 0;
  virtual DWORD GetVirtualID() = 0;
};