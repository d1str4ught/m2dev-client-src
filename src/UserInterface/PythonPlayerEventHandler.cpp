#include "StdAfx.h"
#include "PythonPlayerEventHandler.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"

CPythonPlayerEventHandler &CPythonPlayerEventHandler::GetSingleton() {
  static CPythonPlayerEventHandler s_kPlayerEventHandler;
  return s_kPlayerEventHandler;
}

CPythonPlayerEventHandler::~CPythonPlayerEventHandler() {}

void CPythonPlayerEventHandler::OnClearAffects() {
  CPythonPlayer::Instance().ClearAffects();
}

void CPythonPlayerEventHandler::OnSetAffect(UINT uAffect) {
  CPythonPlayer::Instance().SetAffect(uAffect);
}

void CPythonPlayerEventHandler::OnResetAffect(UINT uAffect) {
  CPythonPlayer::Instance().ResetAffect(uAffect);
}

void CPythonPlayerEventHandler::OnSyncing(const SState &c_rkState) {
  const TPixelPosition &c_rkPPosCurSyncing = c_rkState.kPPosSelf;
  m_kPPosPrevWaiting = c_rkPPosCurSyncing;
}

void CPythonPlayerEventHandler::OnWaiting(const SState &c_rkState) {
  DWORD dwCurTime = ELTimer_GetMSec();
  if (m_dwNextWaitingNotifyTime > dwCurTime)
    return;

  m_dwNextWaitingNotifyTime = dwCurTime + 100;

  const TPixelPosition &c_rkPPosCurWaiting = c_rkState.kPPosSelf;
  float dx = m_kPPosPrevWaiting.x - c_rkPPosCurWaiting.x;
  float dy = m_kPPosPrevWaiting.y - c_rkPPosCurWaiting.y;
  float len = sqrt(dx * dx + dy * dy);

  if (len < 1.0f)
    return;

  m_kPPosPrevWaiting = c_rkPPosCurWaiting;

  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_WAIT, 0);

  // Trace("waiting\n");
}

void CPythonPlayerEventHandler::OnMoving(const SState &c_rkState) {
  DWORD dwCurTime = ELTimer_GetMSec();
  if (m_dwNextMovingNotifyTime > dwCurTime)
    return;

  m_dwNextMovingNotifyTime = dwCurTime + 300;

  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_MOVE, 0);

  //	Trace("moving\n");
}

void CPythonPlayerEventHandler::OnMove(const SState &c_rkState) {
  DWORD dwCurTime = ELTimer_GetMSec();
  m_dwNextWaitingNotifyTime = dwCurTime + 100;
  m_dwNextMovingNotifyTime = dwCurTime + 300;

  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_MOVE, 0);

  //	Trace("move\n");
}

void CPythonPlayerEventHandler::OnStop(const SState &c_rkState) {
  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_WAIT, 0);

  //	Trace("stop\n");
}

void CPythonPlayerEventHandler::OnWarp(const SState &c_rkState) {
  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf, CInstanceBase::FUNC_WAIT, 0);
}

void CPythonPlayerEventHandler::OnAttack(const SState &c_rkState,
                                         WORD wMotionIndex) {
  //	Tracef("CPythonPlayerEventHandler::OnAttack [%d]\n", wMotionIndex);
  assert(wMotionIndex < 255);

  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(c_rkState.kPPosSelf,
                                       c_rkState.fAdvRotSelf,
                                       CInstanceBase::FUNC_COMBO, wMotionIndex);

#ifdef __ATTACK_SPEED_CHECK__
  static DWORD s_dwLastTime = timeGetTime();

  DWORD dwCurTime = timeGetTime();
  Tracef("%d\n", dwCurTime - s_dwLastTime);
  s_dwLastTime = dwCurTime;
#endif
}

void CPythonPlayerEventHandler::OnUseSkill(const SState &c_rkState,
                                           UINT uMotSkill, UINT uArg) {
  CPythonNetworkStream &rkNetStream = CPythonNetworkStream::Instance();
  rkNetStream.SendCharacterStatePacket(
      c_rkState.kPPosSelf, c_rkState.fAdvRotSelf,
      CInstanceBase::FUNC_SKILL | uMotSkill, uArg);
}

void CPythonPlayerEventHandler::OnUpdate() {}

void CPythonPlayerEventHandler::OnChangeShape() {
  CPythonPlayer::Instance().NEW_Stop();
}

void CPythonPlayerEventHandler::OnHit(UINT uSkill,
                                      CActorInstance &rkActorVictim,
                                      BOOL isSendPacket) {
  DWORD dwVIDVictim = rkActorVictim.GetVirtualID();

  // Update Target
  CPythonPlayer::Instance().SetTarget(dwVIDVictim, FALSE);
  // Update Target

  if (isSendPacket) {
// #define ATTACK_TIME_LOG
#ifdef ATTACK_TIME_LOG
    static std::map<DWORD, float> s_prevTimed;
    float curTime = timeGetTime() / 1000.0f;
    bool isFirst = false;
    if (s_prevTimed.end() == s_prevTimed.find(dwVIDVictim)) {
      s_prevTimed[dwVIDVictim] = curTime;
      isFirst = true;
    }
    float diffTime = curTime - s_prevTimed[dwVIDVictim];
    if (diffTime < 0.1f && !isFirst) {
      TraceError("ATTACK(SPEED_HACK): %.4f(%.4f) %d", curTime, diffTime,
                 dwVIDVictim);
    } else {
      TraceError("ATTACK: %.4f(%.4f) %d", curTime, diffTime, dwVIDVictim);
    }

    s_prevTimed[dwVIDVictim] = curTime;
#endif
    CPythonNetworkStream &rkStream = CPythonNetworkStream::Instance();
    rkStream.SendAttackPacket(uSkill, dwVIDVictim);
  }

  if (!rkActorVictim.IsPushing())
    return;

  // 거대 몬스터 밀림 제외
  extern bool IS_HUGE_RACE(unsigned int vnum);
  if (IS_HUGE_RACE(rkActorVictim.GetRace()))
    return;

  CPythonCharacterManager::Instance().AdjustCollisionWithOtherObjects(
      &rkActorVictim);

  const TPixelPosition &kPPosLast = rkActorVictim.NEW_GetLastPixelPositionRef();

  SVictim kVictim;
  kVictim.m_dwVID = dwVIDVictim;
  kVictim.m_lPixelX = long(kPPosLast.x);
  kVictim.m_lPixelY = long(kPPosLast.y);

  rkActorVictim.TEMP_Push(kVictim.m_lPixelX, kVictim.m_lPixelY);

  m_kVctkVictim.push_back(kVictim);
}

void CPythonPlayerEventHandler::FlushVictimList() {
  if (m_kVctkVictim.empty())
    return;

  // #0000682: [M2EU] 대진각 스킬 사용시 튕김
  unsigned int SYNC_POSITION_COUNT_LIMIT = 16;
  unsigned int uiVictimCount = m_kVctkVictim.size();

  CPythonNetworkStream &rkStream = CPythonNetworkStream::Instance();

  TPacketCGSyncPosition kPacketSyncPos;
  kPacketSyncPos.header = CG::SYNC_POSITION;
  kPacketSyncPos.length = sizeof(kPacketSyncPos) +
                          sizeof(TPacketCGSyncPositionElement) * uiVictimCount;

  rkStream.Send(sizeof(kPacketSyncPos), &kPacketSyncPos);

  for (unsigned int i = 0; i < uiVictimCount; ++i) {
    const SVictim &rkVictim = m_kVctkVictim[i];
    rkStream.SendSyncPositionElementPacket(rkVictim.m_dwVID, rkVictim.m_lPixelX,
                                           rkVictim.m_lPixelY);
  }

  m_kVctkVictim.clear();
}

CPythonPlayerEventHandler::CPythonPlayerEventHandler() {
  m_dwPrevComboIndex = 0;
  m_dwNextMovingNotifyTime = 0;
  m_dwNextWaitingNotifyTime = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::
    OnSetFlyTarget() {
  SState s;
  m_pInstMain->NEW_GetPixelPosition(&s.kPPosSelf);
  s.fAdvRotSelf =
      m_pInstMain->GetGraphicThingInstancePtr()->GetTargetRotation();

  CPythonNetworkStream &rpns = CPythonNetworkStream::Instance();
  rpns.SendFlyTargetingPacket(
      m_pInstTarget->GetVirtualID(),
      m_pInstTarget->GetGraphicThingInstancePtr()->OnGetFlyTargetPosition());
}
void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::
    OnShoot(DWORD dwSkillIndex) {
  CPythonNetworkStream &rpns = CPythonNetworkStream::Instance();
  rpns.SendShootPacket(dwSkillIndex);
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::Set(
    CPythonPlayerEventHandler *pParent, CInstanceBase *pInstMain,
    CInstanceBase *pInstTarget) {
  m_pParent = (pParent);
  m_pInstMain = (pInstMain);
  m_pInstTarget = (pInstTarget);
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::
    SetTarget(CInstanceBase *pInstTarget) {
  m_pInstTarget = pInstTarget;
}

void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::
    OnExplodingAtAnotherTarget(DWORD dwSkillIndex, DWORD dwVID) {
  return;

  Tracef("Shoot : 다른 target에 맞았습니다 : %d, %d\n", dwSkillIndex, dwVID);

  CPythonNetworkStream &rkStream = CPythonNetworkStream::Instance();
  rkStream.SendAttackPacket(dwSkillIndex, dwVID);

  CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
  CInstanceBase *pInstance = rkChrMgr.GetInstancePtr(dwVID);
  if (pInstance) {
    pInstance->GetGraphicThingInstanceRef().OnShootDamage();
  }
}
void CPythonPlayerEventHandler::CNormalBowAttack_FlyEventHandler_AutoClear::
    OnExplodingAtTarget(DWORD dwSkillIndex) {
  //	Tracef("Shoot : 원하는 target에 맞았습니다 : %d, %d\n", dwSkillIndex,
  // m_pInstTarget->GetVirtualID()); 	CPythonNetworkStream&
  // rkStream=CPythonNetworkStream::Instance();
  //	rkStream.SendAttackPacket(dwSkillIndex, m_pInstTarget->GetVirtualID());
}
