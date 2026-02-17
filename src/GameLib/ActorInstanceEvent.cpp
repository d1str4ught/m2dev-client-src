#include "StdAfx.h"
#include "ActorInstance.h"

void CActorInstance::__OnSyncing() {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnSyncing(kState);
}

void CActorInstance::__OnWaiting() {
  assert(!IsPushing());

  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnWaiting(kState);
}

void CActorInstance::__OnMoving() {
  assert(!IsPushing());

  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  const TPixelPosition &c_rkPPosCur = NEW_GetCurPixelPositionRef();
  const TPixelPosition &c_rkPPosDst = NEW_GetDstPixelPositionRef();

  TPixelPosition kPPosDir = c_rkPPosDst - c_rkPPosCur;
  float distance = sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

  IEventHandler::SState kState;

  if (distance > 1000.0f) {
    D3DXVec3Normalize(&kPPosDir, &kPPosDir);
    D3DXVec3Scale(&kPPosDir, &kPPosDir, 1000.0f);
    D3DXVec3Add(&kState.kPPosSelf, &kPPosDir, &c_rkPPosCur);
  } else {
    kState.kPPosSelf = c_rkPPosDst;
  }
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnMoving(kState);
}

void CActorInstance::__OnMove() {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnMove(kState);
}

void CActorInstance::__OnStop() {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnStop(kState);
}

void CActorInstance::__OnWarp() {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();
  rkEventHandler.OnWarp(kState);
}

void CActorInstance::__OnAttack(WORD wMotionIndex) {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetTargetRotation();
  rkEventHandler.OnAttack(kState, wMotionIndex);
}

void CActorInstance::__OnUseSkill(UINT uMotSkill, UINT uLoopCount,
                                  bool isMovingSkill) {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();

  IEventHandler::SState kState;
  kState.kPPosSelf = NEW_GetCurPixelPositionRef();
  kState.fAdvRotSelf = GetAdvancingRotation();

  UINT uArg = uLoopCount;
  if (isMovingSkill)
    uArg |= 1 << 4;

  rkEventHandler.OnUseSkill(kState, uMotSkill, uArg);
}

void CActorInstance::__OnHit(UINT uSkill, CActorInstance &rkActorVictm,
                             BOOL isSendPacket) {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();
  rkEventHandler.OnHit(uSkill, rkActorVictm, isSendPacket);
}

void CActorInstance::__OnClearAffects() {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();
  rkEventHandler.OnClearAffects();
}

void CActorInstance::__OnSetAffect(UINT uAffect) {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();
  rkEventHandler.OnSetAffect(uAffect);
}

void CActorInstance::__OnResetAffect(UINT uAffect) {
  IEventHandler &rkEventHandler = __GetEventHandlerRef();
  rkEventHandler.OnResetAffect(uAffect);
}

CActorInstance::IEventHandler &CActorInstance::__GetEventHandlerRef() {
  assert(m_pkEventHandler != NULL && "CActorInstance::GetEventHandlerRef");
  return *m_pkEventHandler;
}

CActorInstance::IEventHandler *CActorInstance::__GetEventHandlerPtr() {
  return m_pkEventHandler;
}

void CActorInstance::SetEventHandler(IEventHandler *pkEventHandler) {
  m_pkEventHandler = pkEventHandler;
}

CActorInstance::IEventHandler *CActorInstance::IEventHandler::GetEmptyPtr() {
  static class CEmptyEventHandler : public IEventHandler {
  public:
    CEmptyEventHandler() {}
    virtual ~CEmptyEventHandler() {}

    virtual void OnSyncing(const SState &c_rkState) {}
    virtual void OnWaiting(const SState &c_rkState) {}
    virtual void OnMoving(const SState &c_rkState) {}
    virtual void OnMove(const SState &c_rkState) {}
    virtual void OnStop(const SState &c_rkState) {}
    virtual void OnWarp(const SState &c_rkState) {}

    virtual void OnClearAffects() {}
    virtual void OnSetAffect(UINT uAffect) {}
    virtual void OnResetAffect(UINT uAffect) {}

    virtual void OnAttack(const SState &c_rkState, WORD wMotionIndex) {}
    virtual void OnUseSkill(const SState &c_rkState, UINT uMotSkill,
                            UINT uMotLoopCount) {}

    virtual void OnHit(UINT uMotAttack, CActorInstance &rkActorVictim,
                       BOOL isSendPacket) {}

    virtual void OnChangeShape() {}

  } s_kEmptyEventHandler;

  return &s_kEmptyEventHandler;
}
