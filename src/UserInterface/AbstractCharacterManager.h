#pragma once

#include "AbstractSingleton.h"

class CInstanceBase;

class IAbstractCharacterManager
    : public TAbstractSingleton<IAbstractCharacterManager> {
public:
  IAbstractCharacterManager() {}
  virtual ~IAbstractCharacterManager() {}

  virtual void Destroy() = 0;
  virtual CInstanceBase *GetInstancePtr(DWORD dwVID) = 0;
};