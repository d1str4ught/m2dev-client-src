#pragma once

#include "GameLib/ActorInstance.h"
#include "GameLib/FlyHandler.h"

#include "PythonNetworkStream.h"
#include "InstanceBase.h"

class CPythonPlayerEventHandler : public CActorInstance::IEventHandler {
public:
  static CPythonPlayerEventHandler &GetSingleton();

public:
  virtual ~CPythonPlayerEventHandler();

  virtual void OnSyncing(const SState &c_rkState);
  virtual void OnWaiting(const SState &c_rkState);
  virtual void OnMoving(const SState &c_rkState);
  virtual void OnMove(const SState &c_rkState);
  virtual void OnStop(const SState &c_rkState);
  virtual void OnWarp(const SState &c_rkState);
  virtual void OnClearAffects();
  virtual void OnSetAffect(UINT uAffect);
  virtual void OnResetAffect(UINT uAffect);
  virtual void OnAttack(const SState &c_rkState, WORD wMotionIndex);
  virtual void OnUseSkill(const SState &c_rkState, UINT uMotSkill, UINT uArg);
  virtual void OnUpdate();
  virtual void OnChangeShape();
  virtual void OnHit(UINT uSkill, CActorInstance &rkActorVictim,
                     BOOL isSendPacket);

  void FlushVictimList();

protected:
  CPythonPlayerEventHandler();

protected:
  struct SVictim {
    DWORD m_dwVID;
    long m_lPixelX;
    long m_lPixelY;
  };

protected:
  std::vector<SVictim> m_kVctkVictim;

  DWORD m_dwPrevComboIndex;
  DWORD m_dwNextWaitingNotifyTime;
  DWORD m_dwNextMovingNotifyTime;
  TPixelPosition m_kPPosPrevWaiting;

private:
  class CNormalBowAttack_FlyEventHandler_AutoClear : public IFlyEventHandler {
  public:
    CNormalBowAttack_FlyEventHandler_AutoClear() {}
    virtual ~CNormalBowAttack_FlyEventHandler_AutoClear() {}

    void Set(CPythonPlayerEventHandler *pParent, CInstanceBase *pInstMain,
             CInstanceBase *pInstTarget);
    void SetTarget(CInstanceBase *pInstTarget);

    virtual void OnSetFlyTarget();
    virtual void OnShoot(DWORD dwSkillIndex);

    virtual void OnNoTarget() { /*Tracenf("Shoot : target이 없습니다.");*/ }
    virtual void
    OnExplodingOutOfRange() { /*Tracenf("Shoot : 사정거리가 끝났습니다.");*/ }
    virtual void
    OnExplodingAtBackground() { /*Tracenf("Shoot : 배경에 맞았습니다.");*/ }
    virtual void OnExplodingAtAnotherTarget(DWORD dwSkillIndex, DWORD dwVID);
    virtual void OnExplodingAtTarget(DWORD dwSkillIndex);

  protected:
    CPythonPlayerEventHandler *m_pParent;
    CInstanceBase *m_pInstMain;
    CInstanceBase *m_pInstTarget;
  } m_NormalBowAttack_FlyEventHandler_AutoClear;

public:
  IFlyEventHandler *
  GetNormalBowAttackFlyEventHandler(CInstanceBase *pInstMain,
                                    CInstanceBase *pInstTarget) {
    m_NormalBowAttack_FlyEventHandler_AutoClear.Set(this, pInstMain,
                                                    pInstTarget);
    return &m_NormalBowAttack_FlyEventHandler_AutoClear;
  }

  void ChangeFlyTarget(CInstanceBase *pInstTarget) {
    m_NormalBowAttack_FlyEventHandler_AutoClear.SetTarget(pInstTarget);
  }
};