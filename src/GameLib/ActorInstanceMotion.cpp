#include "StdAfx.h"
#include "ActorInstance.h"
#include "RaceData.h"
#include "FlyHandler.h"

UINT CActorInstance::__GetMotionType() {
  if (!m_pkCurRaceMotionData)
    return CRaceMotionData::TYPE_NONE;

  return m_pkCurRaceMotionData->GetType();
}

void CActorInstance::__MotionEventProcess(BOOL isPC) {
  if (isAttacking()) {
    DWORD dwNextFrame = DWORD(GetAttackingElapsedTime() * g_fGameFPS);
    for (; m_kCurMotNode.dwcurFrame < dwNextFrame; ++m_kCurMotNode.dwcurFrame) {
      MotionEventProcess();
      SoundEventProcess(!isPC);
    }
  } else {
    MotionEventProcess();
    SoundEventProcess(!isPC);

    ++m_kCurMotNode.dwcurFrame;
  }
}

void CActorInstance::MotionProcess(BOOL isPC) {
  __MotionEventProcess(isPC);
  CurrentMotionProcess();
  ReservingMotionProcess();
}

void CActorInstance::HORSE_MotionProcess(BOOL isPC) {
  __MotionEventProcess(isPC);

  if (MOTION_TYPE_LOOP == m_kCurMotNode.iMotionType)
    if (m_kCurMotNode.dwcurFrame >= m_kCurMotNode.dwFrameCount)
      m_kCurMotNode.dwcurFrame = 0;
}

void CActorInstance::ReservingMotionProcess() {
  if (m_MotionDeque.empty())
    return;

  TReservingMotionNode &rReservingMotionNode = m_MotionDeque.front();

  float fCurrentTime = GetLocalTime();
  if (rReservingMotionNode.fStartTime > fCurrentTime)
    return;

  DWORD dwNextMotionIndex = GET_MOTION_INDEX(rReservingMotionNode.dwMotionKey);
  switch (dwNextMotionIndex) {
  case CRaceMotionData::NAME_STAND_UP:
  case CRaceMotionData::NAME_STAND_UP_BACK:
    if (IsFaint()) {
      // Tracenf("일어서려고 했으나 기절중");

      SetEndStopMotion();

      // 이후의 모션 전부 1초씩 딜레이
      TMotionDeque::iterator itor = m_MotionDeque.begin();
      for (; itor != m_MotionDeque.end(); ++itor) {
        TReservingMotionNode &rNode = *itor;
        rNode.fStartTime += 1.0f;
      }
      return;
    }
    break;
  }

  SCurrentMotionNode kPrevMotionNode = m_kCurMotNode;

  EMotionPushType iMotionType = rReservingMotionNode.iMotionType;
  float fSpeedRatio = rReservingMotionNode.fSpeedRatio;
  float fBlendTime = rReservingMotionNode.fBlendTime;

  DWORD dwMotionKey = rReservingMotionNode.dwMotionKey;

  m_MotionDeque.pop_front();

  DWORD dwCurrentMotionIndex = GET_MOTION_INDEX(dwMotionKey);
  switch (dwCurrentMotionIndex) {
  case CRaceMotionData::NAME_STAND_UP:
  case CRaceMotionData::NAME_STAND_UP_BACK:
    if (IsDead()) {
      // Tracenf("일어서려고 했으나 사망");
      //  예전 데이터로 복구
      m_kCurMotNode = kPrevMotionNode;
      __ClearMotion();

      // 이전 동작 마지막 상태 유지
      SetEndStopMotion();
      return;
    }
    break;
  }

  // Tracenf("MOTION %d", GET_MOTION_INDEX(dwMotionKey));

  int iLoopCount;
  if (MOTION_TYPE_ONCE == iMotionType)
    iLoopCount = 1;
  else
    iLoopCount = 0;

  SSetMotionData kSetMotData;
  kSetMotData.dwMotKey = dwMotionKey;
  kSetMotData.fBlendTime = fBlendTime;
  kSetMotData.fSpeedRatio = fSpeedRatio;
  kSetMotData.iLoopCount = iLoopCount;

  DWORD dwRealMotionKey = __SetMotion(kSetMotData);

  if (0 == dwRealMotionKey)
    return;

  // FIX: 위에서 호출한 __SetMotion 함수 안에서 랜덤으로 다른 모션을 재생할
  // 가능성도 있으므로 duration은 '현재 재생중인' 모션의 duration값을 사용해야
  // 함.
  // float fDurationTime=rReservingMotionNode.fDuration;
  float fDurationTime = GetMotionDuration(dwRealMotionKey) / fSpeedRatio;
  float fStartTime = rReservingMotionNode.fStartTime;
  float fEndTime = fStartTime + fDurationTime;

  if (dwRealMotionKey == 16777473) {
    int bp = 0;
    bp++;
  }

  m_kCurMotNode.uSkill = 0;
  m_kCurMotNode.iMotionType = iMotionType;
  m_kCurMotNode.fSpeedRatio = fSpeedRatio;
  m_kCurMotNode.fStartTime = fStartTime;
  m_kCurMotNode.fEndTime = fEndTime;
  m_kCurMotNode.dwMotionKey = dwRealMotionKey;
  m_kCurMotNode.dwcurFrame = 0;
  m_kCurMotNode.dwFrameCount = fDurationTime / (1.0f / g_fGameFPS);
}

void CActorInstance::CurrentMotionProcess() {
  if (MOTION_TYPE_LOOP ==
      m_kCurMotNode.iMotionType) // 임시다. 최종적인 목표는 Once도 절대로
                                 // 넘어가선 안된다. - [levites]
    if (m_kCurMotNode.dwcurFrame >= m_kCurMotNode.dwFrameCount)
      m_kCurMotNode.dwcurFrame = 0;

  if (IsDead())
    return;

  if (!m_MotionDeque.empty())
    return;

  float fCurrentTime = GetLocalTime();

  DWORD dwMotionIndex = GET_MOTION_INDEX(m_kCurMotNode.dwMotionKey);

  bool isLooping = false;

  // 끝났다면 Playing Flag를 끈다
  if (m_pkCurRaceMotionData && m_pkCurRaceMotionData->IsLoopMotion()) {
    if (m_kCurMotNode.iLoopCount > 1 || m_kCurMotNode.iLoopCount == -1) {
      if (fCurrentTime - m_kCurMotNode.fStartTime >
          m_pkCurRaceMotionData->GetLoopEndTime()) {
        m_kCurMotNode.dwcurFrame =
            DWORD(m_pkCurRaceMotionData->GetLoopStartTime() * g_fGameFPS);
        __SetLocalTime(m_kCurMotNode.fStartTime +
                       m_pkCurRaceMotionData->GetLoopStartTime());
        if (-1 != m_kCurMotNode.iLoopCount)
          --m_kCurMotNode.iLoopCount;

        isLooping = true;
      }
    } else if (!m_kQue_kFlyTarget.empty()) {
      if (!m_kBackupFlyTarget.IsObject()) {
        m_kBackupFlyTarget = m_kFlyTarget;
      }

      if (fCurrentTime - m_kCurMotNode.fStartTime >
          m_pkCurRaceMotionData->GetLoopEndTime()) {
        m_kCurMotNode.dwcurFrame =
            DWORD(m_pkCurRaceMotionData->GetLoopStartTime() * g_fGameFPS);
        __SetLocalTime(m_kCurMotNode.fStartTime +
                       m_pkCurRaceMotionData->GetLoopStartTime());

        SetFlyTarget(m_kQue_kFlyTarget.front());
        m_kQue_kFlyTarget.pop_front();

        isLooping = true;
      }
    }
  }

  if (!isLooping) {
    if (fCurrentTime > m_kCurMotNode.fEndTime) {
      if (m_kBackupFlyTarget.IsValidTarget()) {
        m_kFlyTarget = m_kBackupFlyTarget;
        m_kBackupFlyTarget.Clear();
      }

      ////////////////////////////////////////////

      if (MOTION_TYPE_ONCE == m_kCurMotNode.iMotionType) {
        switch (dwMotionIndex) {
        case CRaceMotionData::NAME_DAMAGE_FLYING:
        case CRaceMotionData::NAME_DAMAGE_FLYING_BACK:
        case CRaceMotionData::NAME_DEAD:
        case CRaceMotionData::NAME_INTRO_SELECTED:
        case CRaceMotionData::NAME_INTRO_NOT_SELECTED:
          m_kCurMotNode.fEndTime += 3.0f;
          SetEndStopMotion();
          break;
        default:
          InterceptLoopMotion(CRaceMotionData::NAME_WAIT);
          break;
        }
      } else if (MOTION_TYPE_LOOP == m_kCurMotNode.iMotionType) {
        if (CRaceMotionData::NAME_WAIT == dwMotionIndex) {
          PushLoopMotion(CRaceMotionData::NAME_WAIT, 0.5f);
        }
      }
    }
  }
}

void CActorInstance::SetMotionMode(int iMotionMode) {
  if (IsPoly())
    iMotionMode = CRaceMotionData::MODE_GENERAL;

  m_wcurMotionMode = iMotionMode;
}

int CActorInstance::GetMotionMode() { return m_wcurMotionMode; }

void CActorInstance::SetMotionLoopCount(int iCount) {
  assert(iCount >= -1 && iCount < 100);
  m_kCurMotNode.iLoopCount = iCount;
}

void CActorInstance::PushMotion(EMotionPushType iMotionType, DWORD dwMotionKey,
                                float fBlendTime, float fSpeedRatio) {
  if (!CheckMotionThingIndex(dwMotionKey)) {
    Tracef(" Error - Not found want to using motion : %d\n",
           GET_MOTION_INDEX(dwMotionKey));
    return;
  }

  TReservingMotionNode MotionNode;

  MotionNode.iMotionType = iMotionType;
  MotionNode.dwMotionKey = dwMotionKey;
  MotionNode.fStartTime = GetLastMotionTime(fBlendTime);
  MotionNode.fBlendTime = fBlendTime;
  MotionNode.fSpeedRatio = fSpeedRatio;
  MotionNode.fDuration = GetMotionDuration(dwMotionKey);

  m_MotionDeque.push_back(MotionNode);
}

bool CActorInstance::InterceptOnceMotion(DWORD dwMotion, float fBlendTime,
                                         UINT uSkill, float fSpeedRatio) {
  return InterceptMotion(MOTION_TYPE_ONCE, dwMotion, fBlendTime, uSkill,
                         fSpeedRatio);
}

bool CActorInstance::InterceptLoopMotion(DWORD dwMotion, float fBlendTime) {
  return InterceptMotion(MOTION_TYPE_LOOP, dwMotion, fBlendTime);
}

void CActorInstance::SetLoopMotion(DWORD dwMotion, float fBlendTime,
                                   float fSpeedRatio) {
  if (!m_pkCurRaceData) {
    Tracenf("CActorInstance::SetLoopMotion(dwMotion=%d, fBlendTime=%f, "
            "fSpeedRatio=%f)",
            dwMotion, fBlendTime, fSpeedRatio);
    return;
  }

  MOTION_KEY dwMotionKey;
  if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion,
                                     &dwMotionKey)) {
    Tracenf("CActorInstance::SetLoopMotion(dwMotion=%d, fBlendTime=%f, "
            "fSpeedRatio=%f) - GetMotionKey(m_wcurMotionMode=%d, dwMotion=%d, "
            "&MotionKey) ERROR",
            dwMotion, fBlendTime, fSpeedRatio, m_wcurMotionMode, dwMotion);
    return;
  }

  __ClearMotion();

  SSetMotionData kSetMotData;
  kSetMotData.dwMotKey = dwMotionKey;
  kSetMotData.fBlendTime = fBlendTime;
  kSetMotData.fSpeedRatio = fSpeedRatio;

  DWORD dwRealMotionKey = __SetMotion(kSetMotData);

  if (0 == dwRealMotionKey)
    return;

  m_kCurMotNode.iMotionType = MOTION_TYPE_LOOP;
  m_kCurMotNode.fStartTime = GetLocalTime();
  m_kCurMotNode.dwMotionKey = dwRealMotionKey;
  m_kCurMotNode.fEndTime = 0.0f;
  m_kCurMotNode.fSpeedRatio = fSpeedRatio;
  m_kCurMotNode.dwcurFrame = 0;
  m_kCurMotNode.dwFrameCount =
      GetMotionDuration(dwRealMotionKey) / (1.0f / g_fGameFPS);
  m_kCurMotNode.uSkill = 0;
}

// 리턴값 == SetMotion의 리턴값 == 실제로 애니메이션 데이터를 플레이 했느냐?
bool CActorInstance::InterceptMotion(EMotionPushType iMotionType, WORD wMotion,
                                     float fBlendTime, UINT uSkill,
                                     float fSpeedRatio) {
  if (!m_pkCurRaceData) {
    Tracef("CActorInstance::InterceptMotion(iMotionType=%d, wMotion=%d, "
           "fBlendTime=%f) - m_pkCurRaceData=NULL",
           iMotionType, wMotion, fBlendTime);
    return false;
  }

  MOTION_KEY dwMotionKey;
  if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, wMotion, &dwMotionKey)) {
    Tracenf("CActorInstance::InterceptMotion(iLoopType=%d, wMotionMode=%d, "
            "wMotion=%d, fBlendTime=%f) - GetMotionKey(m_wcurMotionMode=%d, "
            "wMotion=%d, &MotionKey) ERROR",
            iMotionType, m_wcurMotionMode, wMotion, fBlendTime,
            m_wcurMotionMode, wMotion);
    return false;
  }

  __ClearMotion();

  int iLoopCount;
  if (MOTION_TYPE_ONCE == iMotionType)
    iLoopCount = 1;
  else
    iLoopCount = 0;

  SSetMotionData kSetMotData;
  kSetMotData.dwMotKey = dwMotionKey;
  kSetMotData.fBlendTime = fBlendTime;
  kSetMotData.iLoopCount = iLoopCount;
  kSetMotData.fSpeedRatio = fSpeedRatio;
  kSetMotData.uSkill = uSkill;

  DWORD dwRealMotionKey = __SetMotion(kSetMotData);

  if (0 == dwRealMotionKey)
    return false;

  if (m_pFlyEventHandler) {
    if (__IsNeedFlyTargetMotion()) {
      m_pFlyEventHandler->OnSetFlyTarget();
    }
  }

  assert(NULL != m_pkCurRaceMotionData);

  // FIX : 위에서 호출한 __SetMotion 함수 내에서 랜덤으로 다른 모션을 선택할
  // 수도 있기 때문에 dwMotionKey값은 유효하지 않고 따라서 해당 키로 산출한
  // duration은 유효하지 않음. 당연히 현재 play중인 모션의 시간을 구해야 함..
  // -_-;; float fDuration=GetMotionDuration(dwMotionKey)/fSpeedRatio;
  float fDuration = GetMotionDuration(dwRealMotionKey) / fSpeedRatio;

  m_kCurMotNode.iMotionType = iMotionType;
  m_kCurMotNode.fStartTime = GetLocalTime();
  m_kCurMotNode.fEndTime = m_kCurMotNode.fStartTime + fDuration;
  m_kCurMotNode.dwMotionKey = dwRealMotionKey;
  m_kCurMotNode.dwcurFrame = 0;
  m_kCurMotNode.dwFrameCount = fDuration / (1.0f / g_fGameFPS);
  m_kCurMotNode.uSkill = uSkill;
  m_kCurMotNode.fSpeedRatio = fSpeedRatio;

  return true;
}

bool CActorInstance::PushOnceMotion(DWORD dwMotion, float fBlendTime,
                                    float fSpeedRatio) {
  assert(m_pkCurRaceData);

  MOTION_KEY MotionKey;
  if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion, &MotionKey))
    return false;

  PushMotion(MOTION_TYPE_ONCE, MotionKey, fBlendTime, fSpeedRatio);
  return true;
}

bool CActorInstance::PushLoopMotion(DWORD dwMotion, float fBlendTime,
                                    float fSpeedRatio) {
  assert(m_pkCurRaceData);

  MOTION_KEY MotionKey;
  if (!m_pkCurRaceData->GetMotionKey(m_wcurMotionMode, dwMotion, &MotionKey))
    return false;

  PushMotion(MOTION_TYPE_LOOP, MotionKey, fBlendTime, fSpeedRatio);
  return true;
}

WORD CActorInstance::__GetCurrentMotionIndex() {
  return GET_MOTION_INDEX(m_kCurMotNode.dwMotionKey);
}

DWORD CActorInstance::__GetCurrentMotionKey() {
  return m_kCurMotNode.dwMotionKey;
}

BOOL CActorInstance::IsUsingSkill() {
  DWORD dwCurMotionIndex = __GetCurrentMotionIndex();

  if (dwCurMotionIndex >= CRaceMotionData::NAME_SKILL &&
      dwCurMotionIndex < CRaceMotionData::NAME_SKILL_END)
    return TRUE;

  switch (dwCurMotionIndex) {
  case CRaceMotionData::NAME_SPECIAL_1:
  case CRaceMotionData::NAME_SPECIAL_2:
  case CRaceMotionData::NAME_SPECIAL_3:
  case CRaceMotionData::NAME_SPECIAL_4:
  case CRaceMotionData::NAME_SPECIAL_5:
  case CRaceMotionData::NAME_SPECIAL_6:
    return TRUE;
  }

  return FALSE;
}

BOOL CActorInstance::IsFishing() {
  if (!m_pkCurRaceMotionData)
    return FALSE;

  if (__GetCurrentMotionIndex() == CRaceMotionData::NAME_FISHING_WAIT ||
      __GetCurrentMotionIndex() == CRaceMotionData::NAME_FISHING_REACT)
    return TRUE;

  return FALSE;
}

BOOL CActorInstance::CanCancelSkill() {
  assert(IsUsingSkill());
  return m_pkCurRaceMotionData->IsCancelEnableSkill();
}

BOOL CActorInstance::isLock() {
  DWORD dwCurMotionIndex = __GetCurrentMotionIndex();

  // Locked during attack
  switch (dwCurMotionIndex) {
  case CRaceMotionData::NAME_NORMAL_ATTACK:
  case CRaceMotionData::NAME_COMBO_ATTACK_1:
  case CRaceMotionData::NAME_COMBO_ATTACK_2:
  case CRaceMotionData::NAME_COMBO_ATTACK_3:
  case CRaceMotionData::NAME_COMBO_ATTACK_4:
  case CRaceMotionData::NAME_COMBO_ATTACK_5:
  case CRaceMotionData::NAME_COMBO_ATTACK_6:
  case CRaceMotionData::NAME_COMBO_ATTACK_7:
  case CRaceMotionData::NAME_COMBO_ATTACK_8:
  case CRaceMotionData::NAME_SPECIAL_1:
  case CRaceMotionData::NAME_SPECIAL_2:
  case CRaceMotionData::NAME_SPECIAL_3:
  case CRaceMotionData::NAME_SPECIAL_4:
  case CRaceMotionData::NAME_SPECIAL_5:
  case CRaceMotionData::NAME_SPECIAL_6:
  case CRaceMotionData::NAME_FISHING_THROW:
  case CRaceMotionData::NAME_FISHING_WAIT:
  case CRaceMotionData::NAME_FISHING_STOP:
  case CRaceMotionData::NAME_FISHING_REACT:
  case CRaceMotionData::NAME_FISHING_CATCH:
  case CRaceMotionData::NAME_FISHING_FAIL:
  case CRaceMotionData::NAME_CLAP:
  case CRaceMotionData::NAME_DANCE_1:
  case CRaceMotionData::NAME_DANCE_2:
  case CRaceMotionData::NAME_DANCE_3:
  case CRaceMotionData::NAME_DANCE_4:
  case CRaceMotionData::NAME_DANCE_5:
  case CRaceMotionData::NAME_DANCE_6:
  case CRaceMotionData::NAME_CONGRATULATION:
  case CRaceMotionData::NAME_FORGIVE:
  case CRaceMotionData::NAME_ANGRY:
  case CRaceMotionData::NAME_ATTRACTIVE:
  case CRaceMotionData::NAME_SAD:
  case CRaceMotionData::NAME_SHY:
  case CRaceMotionData::NAME_CHEERUP:
  case CRaceMotionData::NAME_BANTER:
  case CRaceMotionData::NAME_JOY:
  case CRaceMotionData::NAME_CHEERS_1:
  case CRaceMotionData::NAME_CHEERS_2:
  case CRaceMotionData::NAME_KISS_WITH_WARRIOR:
  case CRaceMotionData::NAME_KISS_WITH_ASSASSIN:
  case CRaceMotionData::NAME_KISS_WITH_SURA:
  case CRaceMotionData::NAME_KISS_WITH_SHAMAN:
  case CRaceMotionData::NAME_FRENCH_KISS_WITH_WARRIOR:
  case CRaceMotionData::NAME_FRENCH_KISS_WITH_ASSASSIN:
  case CRaceMotionData::NAME_FRENCH_KISS_WITH_SURA:
  case CRaceMotionData::NAME_FRENCH_KISS_WITH_SHAMAN:
  case CRaceMotionData::NAME_SLAP_HIT_WITH_WARRIOR:
  case CRaceMotionData::NAME_SLAP_HIT_WITH_ASSASSIN:
  case CRaceMotionData::NAME_SLAP_HIT_WITH_SURA:
  case CRaceMotionData::NAME_SLAP_HIT_WITH_SHAMAN:
  case CRaceMotionData::NAME_SLAP_HURT_WITH_WARRIOR:
  case CRaceMotionData::NAME_SLAP_HURT_WITH_ASSASSIN:
  case CRaceMotionData::NAME_SLAP_HURT_WITH_SURA:
  case CRaceMotionData::NAME_SLAP_HURT_WITH_SHAMAN:
    return TRUE;
    break;
  }

  // Locked during using skill
  if (IsUsingSkill()) {
    if (m_pkCurRaceMotionData->IsCancelEnableSkill())
      return FALSE;

    return TRUE;
  }

  return FALSE;
}

float CActorInstance::GetLastMotionTime(float fBlendTime) {
  if (m_MotionDeque.empty()) {
    if (MOTION_TYPE_ONCE == m_kCurMotNode.iMotionType)
      return (m_kCurMotNode.fEndTime - fBlendTime);

    return GetLocalTime();
  }

  TReservingMotionNode &rMotionNode = m_MotionDeque[m_MotionDeque.size() - 1];

  return rMotionNode.fStartTime + rMotionNode.fDuration - fBlendTime;
}

float CActorInstance::GetMotionDuration(DWORD dwMotionKey) {
  CGraphicThing *pMotion;

  if (!GetMotionThingPointer(dwMotionKey, &pMotion)) {
    Tracenf("CActorInstance::GetMotionDuration - Cannot get motion: %d / %d",
            GET_MOTION_MODE(dwMotionKey), GET_MOTION_INDEX(dwMotionKey));
    return 0.0f;
  }

  if (0 == pMotion->GetMotionCount()) {
#ifdef _DEBUG
    Tracenf(
        "CActorInstance::GetMotionDuration - Invalid Motion Key : %d, %d, %d",
        GET_MOTION_MODE(dwMotionKey), GET_MOTION_INDEX(dwMotionKey),
        GET_MOTION_SUB_INDEX(dwMotionKey));
#endif
    return 0.0f;
  }

  CGrannyMotion *pGrannyMotion = pMotion->GetMotionPointer(0);
  return pGrannyMotion->GetDuration();
}

MOTION_KEY CActorInstance::GetRandomMotionKey(MOTION_KEY dwMotionKey) {
  // NOTE : 자주 호출 되는 부분은 아니지만 어느 정도의 최적화 여지가 있음 -
  // [levites]
  // FIXME : 처음에 선택된 모션이 없는 것에 대한 처리가 되어 있지 않다.
  WORD wMode = GET_MOTION_MODE(dwMotionKey);
  WORD wIndex = GET_MOTION_INDEX(dwMotionKey);

  const CRaceData::TMotionVector *c_pMotionVector;
  if (m_pkCurRaceData->GetMotionVectorPointer(wMode, wIndex, &c_pMotionVector))
    if (c_pMotionVector->size() > 1) {
      int iPercentage = random() % 100;
      for (DWORD i = 0; i < c_pMotionVector->size(); ++i) {
        const CRaceData::TMotion &c_rMotion = c_pMotionVector->at(i);
        iPercentage -= c_rMotion.byPercentage;

        if (iPercentage < 0) {
          dwMotionKey = MAKE_RANDOM_MOTION_KEY(wMode, wIndex, i);

          // Temporary
          // NOTE: 현재로선 여기서 해봤자 의미없다. 전체적으로 확인결과 아래는
          // 씹히는 코드고 다른곳에서 해결해야 하므로 일단 주석처리함. 나중에
          // 통채로 지우자.. m_kCurMotNode.fEndTime = m_kCurMotNode.fStartTime +
          // GetMotionDuration(dwMotionKey); Temporary

          return dwMotionKey;
        }
      }
    }

  return dwMotionKey;
}

void CActorInstance::PreAttack() {}

void CActorInstance::__ClearMotion() {
  __HideWeaponTrace();

  m_MotionDeque.clear();
  m_kCurMotNode.dwcurFrame = 0;
  m_kCurMotNode.dwFrameCount = 0;
  m_kCurMotNode.uSkill = 0;
  m_kCurMotNode.iLoopCount = 0;
  m_kCurMotNode.iMotionType = MOTION_TYPE_NONE;
}

DWORD CActorInstance::__SetMotion(const SSetMotionData &c_rkSetMotData,
                                  DWORD dwRandMotKey) {
  DWORD dwMotKey = dwRandMotKey;

  if (dwMotKey == 0)
    dwMotKey = GetRandomMotionKey(c_rkSetMotData.dwMotKey);

  UINT uNextMot = GET_MOTION_INDEX(c_rkSetMotData.dwMotKey);

  if (IsDead()) {
    if (uNextMot != CRaceMotionData::NAME_DAMAGE_FLYING &&
        uNextMot != CRaceMotionData::NAME_DAMAGE_FLYING_BACK &&
        uNextMot != CRaceMotionData::NAME_DEAD &&
        uNextMot != CRaceMotionData::NAME_DEAD_BACK)
      return 0;
  }
  if (IsUsingSkill()) {
    __OnStop();
  }

  if (__IsStandUpMotion()) {
    __OnStop();
  }

  if (__IsMoveMotion()) {
    if (uNextMot == CRaceMotionData::NAME_DAMAGE ||
        uNextMot == CRaceMotionData::NAME_DAMAGE_BACK ||
        uNextMot == CRaceMotionData::NAME_DAMAGE_FLYING ||
        uNextMot == CRaceMotionData::NAME_DAMAGE_FLYING_BACK) {
      if (!m_isMain) {
        Logn(0, "주인공이 아니라면 이동중이라 데미지 동작을 취하지 않음");
        return false;
      }
    }

    if (uNextMot != CRaceMotionData::NAME_RUN &&
        uNextMot != CRaceMotionData::NAME_WALK &&
        !__IsMovingSkill(c_rkSetMotData.uSkill)) {
      __OnStop();
    }
  } else {
    if (uNextMot == CRaceMotionData::NAME_RUN ||
        __IsMovingSkill(c_rkSetMotData.uSkill)) {
      __OnMove();
    }
  }

  // NOTE : 스킬 사용중 사라지는 문제를 위한 안전 장치 - [levites]
  if (__IsHiding()) {
    __ShowEvent();
  }

  if (-1 != m_iFishingEffectID) {
    CEffectManager &rkEftMgr = CEffectManager::Instance();
    rkEftMgr.DeactiveEffectInstance(m_iFishingEffectID);

    m_iFishingEffectID = -1;
  }

  if (m_pkHorse) {
    WORD wMotionIndex = GET_MOTION_INDEX(dwMotKey);
    WORD wMotionSubIndex = GET_MOTION_SUB_INDEX(dwMotKey);
    DWORD dwChildMotKey = MAKE_RANDOM_MOTION_KEY(m_pkHorse->m_wcurMotionMode,
                                                 wMotionIndex, wMotionSubIndex);

    if (CRaceMotionData::NAME_DEAD == wMotionIndex)
      CGraphicThingInstance::ChangeMotion(dwMotKey, c_rkSetMotData.iLoopCount,
                                          c_rkSetMotData.fSpeedRatio);
    else
      CGraphicThingInstance::SetMotion(dwMotKey, c_rkSetMotData.fBlendTime,
                                       c_rkSetMotData.iLoopCount,
                                       c_rkSetMotData.fSpeedRatio);

    m_pkHorse->SetMotion(dwChildMotKey, c_rkSetMotData.fBlendTime,
                         c_rkSetMotData.iLoopCount, c_rkSetMotData.fSpeedRatio);
    m_pkHorse->__BindMotionData(dwChildMotKey);

    if (c_rkSetMotData.iLoopCount)
      m_pkHorse->m_kCurMotNode.iMotionType =
          MOTION_TYPE_ONCE; // 무조건 이전 모션 타입으로 설정되고 있었음
    else
      m_pkHorse->m_kCurMotNode.iMotionType = MOTION_TYPE_LOOP;

    m_pkHorse->m_kCurMotNode.dwFrameCount =
        m_pkHorse->GetMotionDuration(dwChildMotKey) / (1.0f / g_fGameFPS);
    m_pkHorse->m_kCurMotNode.dwcurFrame = 0;
    m_pkHorse->m_kCurMotNode.dwMotionKey = dwChildMotKey;
  } else {
    CGraphicThingInstance::SetMotion(dwMotKey, c_rkSetMotData.fBlendTime,
                                     c_rkSetMotData.iLoopCount,
                                     c_rkSetMotData.fSpeedRatio);
  }

  __HideWeaponTrace();

  if (__BindMotionData(dwMotKey)) {
    int iLoopCount = __GetLoopCount();
    SetMotionLoopCount(iLoopCount);

    if (__CanAttack()) {
      // 여기서 공격 모션일 경우의 처리를 합니다 - [levites]
      __ShowWeaponTrace();

      m_HitDataMap.clear();
      // PreAttack();
    }

    if (__IsComboAttacking()) {
      if (!__CanNextComboAttack()) {
        // 2004.11.19.myevan.동물 변신시 이부분에서 바로 리셋되어 다음동작
        // 안나온다
        m_dwcurComboIndex = 0; // 콤보 리셋 - [levites]

        // NOTE : ClearCombo() 를 수행해서는 안된다.
        //        콤보 다음에 스킬을 이어서 사용할 경우
        //        m_pkCurRaceMotionData까지 초기화 되어 버린다.
        // Tracef("MotionData에 콤보 데이타가 들어 있지 않습니다.\n");
      }
    }
  }

  return dwMotKey;
}

bool CActorInstance::__BindMotionData(DWORD dwMotionKey) {
  if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey,
                                             &m_pkCurRaceMotionData)) {
    Tracen("Failed to bind motion.");
    m_pkCurRaceMotionData = NULL;
    m_dwcurComboIndex = 0;
    return false;
  }

  return true;
}

int CActorInstance::__GetLoopCount() {
  if (!m_pkCurRaceMotionData) {
    TraceError(
        "CActorInstance::__GetLoopCount() - m_pkCurRaceMotionData==NULL");
    return -1;
  }

  return m_pkCurRaceMotionData->GetLoopCount();
}

bool CActorInstance::__CanAttack() {
  if (!m_pkCurRaceMotionData) {
    TraceError("CActorInstance::__CanAttack() - m_pkCurRaceMotionData==NULL");
    return false;
  }

  if (!m_pkCurRaceMotionData->isAttackingMotion())
    return false;

  return true;
}

bool CActorInstance::__CanNextComboAttack() {
  if (!m_pkCurRaceMotionData) {
    TraceError(
        "CActorInstance::__CanNextComboAttack() - m_pkCurRaceMotionData==NULL");
    return false;
  }

  if (!m_pkCurRaceMotionData->IsComboInputTimeData())
    return false;

  return true;
}

bool CActorInstance::__IsComboAttacking() {
  if (0 == m_dwcurComboIndex)
    return false;

  return true;
}

bool CActorInstance::__IsNeedFlyTargetMotion() {
  if (!m_pkCurRaceMotionData)
    return true;

  for (DWORD i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i) {
    const CRaceMotionData::TMotionEventData *c_pData;
    if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
      continue;

    if (c_pData->iType == CRaceMotionData::MOTION_EVENT_TYPE_WARP)
      return true;

    if (c_pData->iType == CRaceMotionData::MOTION_EVENT_TYPE_FLY)
      return true;

    if (c_pData->iType == CRaceMotionData::MOTION_EVENT_TYPE_EFFECT_TO_TARGET)
      return true;
  }

  return false;
}

bool CActorInstance::__HasMotionFlyEvent() {
  if (!m_pkCurRaceMotionData)
    return true;

  for (DWORD i = 0; i < m_pkCurRaceMotionData->GetMotionEventDataCount(); ++i) {
    const CRaceMotionData::TMotionEventData *c_pData;
    if (!m_pkCurRaceMotionData->GetMotionEventDataPointer(i, &c_pData))
      continue;

    if (c_pData->iType == CRaceMotionData::MOTION_EVENT_TYPE_FLY)
      return true;
  }
  return false;
}

bool CActorInstance::__IsWaitMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_WAIT);
}

bool CActorInstance::__IsMoveMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_MOVE);
}

bool CActorInstance::__IsAttackMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_ATTACK);
}

bool CActorInstance::__IsComboAttackMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_COMBO);
}

bool CActorInstance::__IsDamageMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_DAMAGE);
}

bool CActorInstance::__IsKnockDownMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_KNOCKDOWN);
}

bool CActorInstance::__IsDieMotion() {
  if (__IsKnockDownMotion())
    return true;

  return (__GetMotionType() == CRaceMotionData::TYPE_DIE);
}

bool CActorInstance::__IsStandUpMotion() {
  return (__GetMotionType() == CRaceMotionData::TYPE_STANDUP);
}
