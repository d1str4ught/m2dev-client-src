#include "StdAfx.h"
#include "EffectLib/EffectManager.h"
#include "AudioLib/SoundEngine.h"

#include "ActorInstance.h"
#include "RaceData.h"

void CActorInstance::SetBattleHitEffect(DWORD dwID) {
  m_dwBattleHitEffectID = dwID;
}

void CActorInstance::SetBattleAttachEffect(DWORD dwID) {
  m_dwBattleAttachEffectID = dwID;
}

bool CActorInstance::CanAct() {
  if (IsDead())
    return false;

  if (IsStun())
    return false;

  if (IsParalysis())
    return false;

  if (IsFaint())
    return false;

  if (IsSleep())
    return false;

  return true;
}

bool CActorInstance::CanUseSkill() {
  if (!CanAct())
    return false;

  DWORD dwCurMotionIndex = __GetCurrentMotionIndex();

  // Locked during attack
  switch (dwCurMotionIndex) {
  case CRaceMotionData::NAME_FISHING_THROW:
  case CRaceMotionData::NAME_FISHING_WAIT:
  case CRaceMotionData::NAME_FISHING_STOP:
  case CRaceMotionData::NAME_FISHING_REACT:
  case CRaceMotionData::NAME_FISHING_CATCH:
  case CRaceMotionData::NAME_FISHING_FAIL:
    return TRUE;
    break;
  }

  // Locked during using skill
  if (IsUsingSkill()) {
    if (m_pkCurRaceMotionData->IsCancelEnableSkill())
      return TRUE;

    return FALSE;
  }

  return true;
}

bool CActorInstance::CanMove() {
  if (!CanAct())
    return false;

  if (isLock())
    return false;

  return true;
}

bool CActorInstance::CanAttack() {
  if (!CanAct())
    return false;

  if (IsUsingSkill()) {
    if (!CanCancelSkill())
      return false;
  }

  return true;
}

bool CActorInstance::CanFishing() {
  if (!CanAct())
    return false;

  if (IsUsingSkill())
    return false;

  switch (__GetCurrentMotionIndex()) {
  case CRaceMotionData::NAME_WAIT:
  case CRaceMotionData::NAME_WALK:
  case CRaceMotionData::NAME_RUN:
    break;
  default:
    return false;
    break;
  }

  return true;
}

BOOL CActorInstance::IsClickableDistanceDestInstance(CActorInstance &rkInstDst,
                                                     float fDistance) {
  TPixelPosition kPPosSrc;
  GetPixelPosition(&kPPosSrc);

  D3DXVECTOR3 kD3DVct3Src(kPPosSrc);

  TCollisionPointInstanceList &rkLstkDefPtInst =
      rkInstDst.m_DefendingPointInstanceList;
  TCollisionPointInstanceList::iterator i;

  for (i = rkLstkDefPtInst.begin(); i != rkLstkDefPtInst.end(); ++i) {
    CDynamicSphereInstanceVector &rkVctkDefSphere = (*i).SphereInstanceVector;

    CDynamicSphereInstanceVector::iterator j;
    for (j = rkVctkDefSphere.begin(); j != rkVctkDefSphere.end(); ++j) {
      CDynamicSphereInstance &rkSphere = (*j);

      const auto vv = D3DXVECTOR3(rkSphere.v3Position - kD3DVct3Src);
      float fMovDistance = D3DXVec3Length(&vv);
      float fAtkDistance = rkSphere.fRadius + fDistance;

      if (fAtkDistance > fMovDistance)
        return TRUE;
    }
  }

  return FALSE;
}

void CActorInstance::InputNormalAttackCommand(float fDirRot) {
  if (!__CanInputNormalAttackCommand())
    return;

  m_fAtkDirRot = fDirRot;
  NormalAttack(m_fAtkDirRot);
}

bool CActorInstance::InputComboAttackCommand(float fDirRot) {
  m_fAtkDirRot = fDirRot;

  if (m_isPreInput)
    return false;

  /////////////////////////////////////////////////////////////////////////////////

  // Process Input
  if (0 == m_dwcurComboIndex) {
    __RunNextCombo();
    return true;
  } else if (m_pkCurRaceMotionData->IsComboInputTimeData()) {
    // 동작 경과 시간
    float fElapsedTime = GetAttackingElapsedTime();

    // 이미 입력 한계 시간이 지났다면..
    if (fElapsedTime > m_pkCurRaceMotionData->GetComboInputEndTime()) {
      // Tracen("입력 한계 시간 지남");
      if (IsBowMode())
        m_isNextPreInput = TRUE;
      return false;
    }

    if (fElapsedTime >
        m_pkCurRaceMotionData->GetNextComboTime()) // 콤보 발동 시간 이 후라면
    {
      // Tracen("다음 콤보 동작");
      //  args : BlendingTime
      __RunNextCombo();
      return true;
    } else if (fElapsedTime >
               m_pkCurRaceMotionData
                   ->GetComboInputStartTime()) // 선 입력 시간 범위 라면..
    {
      // Tracen("선 입력 설정");
      m_isPreInput = TRUE;
      return false;
    }
  } else {
    float fElapsedTime = GetAttackingElapsedTime();
    if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() *
                           0.9f) // 콤보 발동 시간 이 후라면
    {
      // Tracen("다음 콤보 동작");
      //  args : BlendingTime
      __RunNextCombo();
      return true;
    }
  }
  // Process Input

  return false;
}

void CActorInstance::ComboProcess() {
  // If combo is on action
  if (0 != m_dwcurComboIndex) {
    if (!m_pkCurRaceMotionData) {
      Tracef("Attacking motion data is NULL! : %d\n", m_dwcurComboIndex);
      __ClearCombo();
      return;
    }

    float fElapsedTime = GetAttackingElapsedTime();

    // Process PreInput
    if (m_isPreInput) {
      // Tracenf("선입력 %f 다음콤보시간 %f", fElapsedTime,
      // m_pkCurRaceMotionData->GetNextComboTime());
      if (fElapsedTime > m_pkCurRaceMotionData->GetNextComboTime()) {
        __RunNextCombo();
        m_isPreInput = FALSE;

        return;
      }
    }
  } else {
    m_isPreInput = FALSE;

    if (!IsUsingSkill())    // m_isNextPreInput는 활모드 일때만 사용하는 변수
      if (m_isNextPreInput) // 활일때만 스킬이 캔슬 되는건 이곳 때문임
      {
        __RunNextCombo();
        m_isNextPreInput = FALSE;
      }
  }
}

void CActorInstance::__RunNextCombo() {
  ++m_dwcurComboIndex;
  ///////////////////////////

  WORD wComboIndex = m_dwcurComboIndex;
  WORD wComboType = __GetCurrentComboType();

  if (wComboIndex == 0) {
    TraceError("CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d)",
               wComboType, wComboIndex);
    return;
  }

  DWORD dwComboArrayIndex = wComboIndex - 1;

  CRaceData::TComboData *pComboData;

  if (!m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode, wComboType,
                                            &pComboData)) {
    TraceError("CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d) "
               "- m_pkCurRaceData->GetComboDataPointer(m_wcurMotionMode=%d, "
               "&pComboData) == NULL",
               wComboType, wComboIndex, m_wcurMotionMode);
    return;
  }

  if (dwComboArrayIndex >= pComboData->ComboIndexVector.size()) {
    TraceError(
        "CActorInstance::__RunNextCombo(wComboType=%d, wComboIndex=%d) - "
        "(dwComboArrayIndex=%d) >= (pComboData->ComboIndexVector.size()=%d)",
        wComboType, wComboIndex, dwComboArrayIndex,
        pComboData->ComboIndexVector.size());
    return;
  }

  WORD wcurComboMotionIndex = pComboData->ComboIndexVector[dwComboArrayIndex];
  ComboAttack(wcurComboMotionIndex, m_fAtkDirRot, 0.1f);

  ////////////////////////////////
  // 콤보가 끝났다면
  if (m_dwcurComboIndex == pComboData->ComboIndexVector.size()) {
    __OnEndCombo();
  }
}

void CActorInstance::__OnEndCombo() {
  if (__IsMountingHorse()) {
    m_dwcurComboIndex = 1;
  }

  // 여기서 콤보를 초기화 해선 안된다.
  // 콤보가 초기화 되는 곳은 마지막 콤보가 끝나고 Motion 이 자동으로 Wait 으로
  // 돌아가는 시점이다.
}

void CActorInstance::__ClearCombo() {
  m_dwcurComboIndex = 0;
  m_isPreInput = FALSE;
  m_pkCurRaceMotionData = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CActorInstance::isAttacking() {
  if (isNormalAttacking())
    return TRUE;

  if (isComboAttacking())
    return TRUE;

  if (IsSplashAttacking())
    return TRUE;

  return FALSE;
}

BOOL CActorInstance::isValidAttacking() {
  if (!m_pkCurRaceMotionData)
    return FALSE;

  if (!m_pkCurRaceMotionData->isAttackingMotion())
    return FALSE;

  const NRaceData::TMotionAttackData *c_pData =
      m_pkCurRaceMotionData->GetMotionAttackDataPointer();
  float fElapsedTime = GetAttackingElapsedTime();
  NRaceData::THitDataContainer::const_iterator itor =
      c_pData->HitDataContainer.begin();
  for (; itor != c_pData->HitDataContainer.end(); ++itor) {
    const NRaceData::THitData &c_rHitData = *itor;
    if (fElapsedTime > c_rHitData.fAttackStartTime &&
        fElapsedTime < c_rHitData.fAttackEndTime)
      return TRUE;
  }

  return TRUE;
}

BOOL CActorInstance::CanCheckAttacking() {
  if (isAttacking())
    return true;

  return false;
}

bool CActorInstance::__IsInSplashTime() {
  if (m_kSplashArea.fDisappearingTime > GetLocalTime())
    return true;

  return false;
}

BOOL CActorInstance::isNormalAttacking() {
  if (!m_pkCurRaceMotionData)
    return FALSE;

  if (!m_pkCurRaceMotionData->isAttackingMotion())
    return FALSE;

  const NRaceData::TMotionAttackData *c_pData =
      m_pkCurRaceMotionData->GetMotionAttackDataPointer();
  if (NRaceData::MOTION_TYPE_NORMAL != c_pData->iMotionType)
    return FALSE;

  return TRUE;
}

BOOL CActorInstance::isComboAttacking() {
  if (!m_pkCurRaceMotionData)
    return FALSE;

  if (!m_pkCurRaceMotionData->isAttackingMotion())
    return FALSE;

  const NRaceData::TMotionAttackData *c_pData =
      m_pkCurRaceMotionData->GetMotionAttackDataPointer();
  if (NRaceData::MOTION_TYPE_COMBO != c_pData->iMotionType)
    return FALSE;

  return TRUE;
}

BOOL CActorInstance::IsSplashAttacking() {
  if (!m_pkCurRaceMotionData)
    return FALSE;

  if (m_pkCurRaceMotionData->HasSplashMotionEvent())
    return TRUE;

  return FALSE;
}

BOOL CActorInstance::__IsMovingSkill(WORD wSkillNumber) {
  enum {
    HORSE_DASH_SKILL_NUMBER = 137,
  };

  return HORSE_DASH_SKILL_NUMBER == wSkillNumber;
}

BOOL CActorInstance::IsActEmotion() {
  DWORD dwCurMotionIndex = __GetCurrentMotionIndex();
  switch (dwCurMotionIndex) {
  case CRaceMotionData::NAME_FRENCH_KISS_START + 0:
  case CRaceMotionData::NAME_FRENCH_KISS_START + 1:
  case CRaceMotionData::NAME_FRENCH_KISS_START + 2:
  case CRaceMotionData::NAME_FRENCH_KISS_START + 3:
  case CRaceMotionData::NAME_KISS_START + 0:
  case CRaceMotionData::NAME_KISS_START + 1:
  case CRaceMotionData::NAME_KISS_START + 2:
  case CRaceMotionData::NAME_KISS_START + 3:
    return TRUE;
    break;
  }

  return FALSE;
}

BOOL CActorInstance::IsUsingMovingSkill() {
  return __IsMovingSkill(m_kCurMotNode.uSkill);
}

DWORD CActorInstance::GetComboIndex() { return m_dwcurComboIndex; }

float CActorInstance::GetAttackingElapsedTime() {
  return (GetLocalTime() - m_kCurMotNode.fStartTime) *
         m_kCurMotNode.fSpeedRatio;
  //	return (GetLocalTime() - m_kCurMotNode.fStartTime) * __GetAttackSpeed();
}

bool CActorInstance::__CanInputNormalAttackCommand() {
  if (IsWaiting())
    return true;

  if (isNormalAttacking()) {
    float fElapsedTime = GetAttackingElapsedTime();

    if (fElapsedTime > m_pkCurRaceMotionData->GetMotionDuration() * 0.9f)
      return true;
  }

  return false;
}

BOOL CActorInstance::NormalAttack(float fDirRot, float fBlendTime) {
  WORD wMotionIndex;
  if (!m_pkCurRaceData->GetNormalAttackIndex(m_wcurMotionMode, &wMotionIndex))
    return FALSE;

  BlendRotation(fDirRot, fBlendTime);
  SetAdvancingRotation(fDirRot);
  InterceptOnceMotion(wMotionIndex, 0.1f, 0, __GetAttackSpeed());

  __OnAttack(wMotionIndex);

  NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

  return TRUE;
}

BOOL CActorInstance::ComboAttack(DWORD dwMotionIndex, float fDirRot,
                                 float fBlendTime) {
  BlendRotation(fDirRot, fBlendTime);
  SetAdvancingRotation(fDirRot);

  InterceptOnceMotion(dwMotionIndex, fBlendTime, 0, __GetAttackSpeed());

  __OnAttack(dwMotionIndex);

  NEW_SetAtkPixelPosition(NEW_GetCurPixelPositionRef());

  return TRUE;
}

void CActorInstance::__ProcessMotionEventAttackSuccess(
    DWORD dwMotionKey, BYTE byEventIndex, CActorInstance &rVictim) {
  CRaceMotionData *pMotionData;

  if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &pMotionData))
    return;

  if (byEventIndex >= pMotionData->GetMotionEventDataCount())
    return;

  const CRaceMotionData::TMotionAttackingEventData *pMotionEventData;
  if (!pMotionData->GetMotionAttackingEventDataPointer(byEventIndex,
                                                       &pMotionEventData))
    return;

  const D3DXVECTOR3 &c_rv3VictimPos = rVictim.GetPositionVectorRef();
  __ProcessDataAttackSuccess(pMotionEventData->AttackData, rVictim,
                             c_rv3VictimPos);
}

void CActorInstance::__ProcessMotionAttackSuccess(DWORD dwMotionKey,
                                                  CActorInstance &rVictim) {
  CRaceMotionData *c_pMotionData;

  if (!m_pkCurRaceData->GetMotionDataPointer(dwMotionKey, &c_pMotionData))
    return;

  const D3DXVECTOR3 &c_rv3VictimPos = rVictim.GetPositionVectorRef();
  __ProcessDataAttackSuccess(c_pMotionData->GetMotionAttackDataReference(),
                             rVictim, c_rv3VictimPos);
}

DWORD CActorInstance::__GetOwnerVID() { return m_dwOwnerVID; }

float CActorInstance::__GetOwnerTime() {
  return GetLocalTime() - m_fOwnerBaseTime;
}

bool IS_HUGE_RACE(unsigned int vnum) {
  switch (vnum) {
  case 2493:
    return true;
  }
  return false;
}

bool CActorInstance::__CanPushDestActor(CActorInstance &rkActorDst) {
  if (rkActorDst.IsBuilding())
    return false;

  if (rkActorDst.IsDoor())
    return false;

  if (rkActorDst.IsStone())
    return false;

  if (rkActorDst.IsNPC())
    return false;

  // 거대 몬스터 밀림 제외
  extern bool IS_HUGE_RACE(unsigned int vnum);
  if (IS_HUGE_RACE(rkActorDst.GetRace()))
    return false;

  if (rkActorDst.IsStun())
    return true;

  if (rkActorDst.__GetOwnerVID() != GetVirtualID())
    return false;

  if (rkActorDst.__GetOwnerTime() > 3.0f)
    return false;

  return true;
}

bool IS_PARTY_HUNTING_RACE(unsigned int vnum) {
  return true;

  // 모든 몬스터 파티 사냥 적용
  /*
  if (vnum < 8) // 플레이어
          return true;

  if (vnum >= 8000 && vnum <= 8112) // 메틴석
          return true;

  if (vnum >= 2400 && vnum <  5000) // 천의 동굴 이후 몬스터
          return true;

  return false;
  */
}

void CActorInstance::__ProcessDataAttackSuccess(
    const NRaceData::TAttackData &c_rAttackData, CActorInstance &rVictim,
    const D3DXVECTOR3 &c_rv3Position, UINT uiSkill, BOOL isSendPacket) {
  if (NRaceData::HIT_TYPE_NONE == c_rAttackData.iHittingType)
    return;

  InsertDelay(c_rAttackData.fStiffenTime);

  if (__CanPushDestActor(rVictim) && c_rAttackData.fExternalForce > 0.0f) {
    __PushCircle(rVictim);

    // VICTIM_COLLISION_TEST
    const D3DXVECTOR3 &kVictimPos = rVictim.GetPosition();
    rVictim.m_PhysicsObject.IncreaseExternalForce(
        kVictimPos, c_rAttackData.fExternalForce); //*nForceRatio/100.0f);
                                                   // VICTIM_COLLISION_TEST_END
  }

  // MR-3: Shaman on-mount hitting fix
  // Invisible Time
  if (IS_PARTY_HUNTING_RACE(rVictim.GetRace())) {
    if (uiSkill) // 파티 사냥 몬스터라도 스킬이면 무적시간 적용
      rVictim.m_fInvisibleTime =
          CTimer::Instance().GetCurrentSecond() +
          (c_rAttackData.fInvisibleTime -
           __GetInvisibleTimeAdjust(uiSkill, c_rAttackData));

    if (m_isMain) // #0000794: [M2KR] 폴리모프 - 밸런싱 문제 타인 공격에 의한
                  // 무적 타임은 고려하지 않고 자신 공격에 의한것만 체크한다
      rVictim.m_fInvisibleTime =
          CTimer::Instance().GetCurrentSecond() +
          (c_rAttackData.fInvisibleTime -
           __GetInvisibleTimeAdjust(uiSkill, c_rAttackData));
  } else // 파티 사냥 몬스터가 아닐 경우만 적용
  {
    rVictim.m_fInvisibleTime =
        CTimer::Instance().GetCurrentSecond() +
        (c_rAttackData.fInvisibleTime -
         __GetInvisibleTimeAdjust(uiSkill, c_rAttackData));
  }
  // MR-3: -- END OF -- Shaman on-mount hitting fix

  // Stiffen Time
  rVictim.InsertDelay(c_rAttackData.fStiffenTime);

  // Hit Effect
  D3DXVECTOR3 vec3Effect(rVictim.m_x, rVictim.m_y, rVictim.m_z);

  // #0000780: [M2KR] 수룡 타격구 문제
  extern bool IS_HUGE_RACE(unsigned int vnum);
  if (IS_HUGE_RACE(rVictim.GetRace())) {
    vec3Effect = c_rv3Position;
  }

  const D3DXVECTOR3 &v3Pos = GetPosition();

  float fHeight =
      D3DXToDegree(atan2(-vec3Effect.x + v3Pos.x, +vec3Effect.y - v3Pos.y));

  // 2004.08.03.myevan.빌딩이나 문의 경우 타격 효과가 보이지 않는다
  if (rVictim.IsBuilding() || rVictim.IsDoor()) {
    D3DXVECTOR3 vec3Delta = vec3Effect - v3Pos;
    D3DXVec3Normalize(&vec3Delta, &vec3Delta);
    vec3Delta *= 30.0f;

    CEffectManager &rkEftMgr = CEffectManager::Instance();
    if (m_dwBattleHitEffectID)
      rkEftMgr.CreateEffect(m_dwBattleHitEffectID, v3Pos + vec3Delta,
                            D3DXVECTOR3(0.0f, 0.0f, 0.0f));
  } else {
    CEffectManager &rkEftMgr = CEffectManager::Instance();
    if (m_dwBattleHitEffectID)
      rkEftMgr.CreateEffect(m_dwBattleHitEffectID, vec3Effect,
                            D3DXVECTOR3(0.0f, 0.0f, fHeight));
    if (m_dwBattleAttachEffectID)
      rVictim.AttachEffectByID(0, NULL, m_dwBattleAttachEffectID);
  }

  if (rVictim.IsBuilding()) {
    // 2004.08.03.빌딩의 경우 흔들리면 이상하다
  } else if (rVictim.IsStone() || rVictim.IsDoor()) {
    __HitStone(rVictim);
  } else {
    ///////////
    // Motion
    if (NRaceData::HIT_TYPE_GOOD == c_rAttackData.iHittingType ||
        rVictim.IsResistFallen()) {
      __HitGood(rVictim);
    } else if (NRaceData::HIT_TYPE_GREAT == c_rAttackData.iHittingType) {
      __HitGreate(rVictim);
    } else {
      TraceError(
          "ProcessSucceedingAttacking: Unknown AttackingData.iHittingType %d",
          c_rAttackData.iHittingType);
    }
  }

  __OnHit(uiSkill, rVictim, isSendPacket);
}

void CActorInstance::OnShootDamage() {
  if (IsStun()) {
    Die();
  } else {
    __Shake(100);

    if (!isLock() && !__IsKnockDownMotion() && !__IsStandUpMotion()) {
      if (InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
        PushLoopMotion(CRaceMotionData::NAME_WAIT);
    }
  }
}

void CActorInstance::__Shake(DWORD dwDuration) {
  DWORD dwCurTime = ELTimer_GetMSec();
  m_dwShakeTime = dwCurTime + dwDuration;
}

void CActorInstance::ShakeProcess() {
  if (m_dwShakeTime) {
    D3DXVECTOR3 v3Pos(0.0f, 0.0f, 0.0f);

    DWORD dwCurTime = ELTimer_GetMSec();

    if (m_dwShakeTime < dwCurTime) {
      m_dwShakeTime = 0;
    } else {
      int nShakeSize = 10;

      switch (rand() % 2) {
      case 0:
        v3Pos.x += rand() % nShakeSize;
        break;
      case 1:
        v3Pos.x -= rand() % nShakeSize;
        break;
      }

      switch (rand() % 2) {
      case 0:
        v3Pos.y += rand() % nShakeSize;
        break;
      case 1:
        v3Pos.y -= rand() % nShakeSize;
        break;
      }

      switch (rand() % 2) {
      case 0:
        v3Pos.z += rand() % nShakeSize;
        break;
      case 1:
        v3Pos.z -= rand() % nShakeSize;
        break;
      }
    }

    m_worldMatrix._41 += v3Pos.x;
    m_worldMatrix._42 += v3Pos.y;
    m_worldMatrix._43 += v3Pos.z;
  }
}

void CActorInstance::__HitStone(CActorInstance &rVictim) {
  if (rVictim.IsStun()) {
    rVictim.Die();
  } else {
    rVictim.__Shake(100);
  }
}

void CActorInstance::__HitGood(CActorInstance &rVictim) {
  if (rVictim.IsKnockDown())
    return;

  if (rVictim.IsStun()) {
    rVictim.Die();
  } else {
    rVictim.__Shake(100);

    if (!rVictim.isLock()) {
      float fRotRad = D3DXToRadian(GetRotation());
      float fVictimRotRad = D3DXToRadian(rVictim.GetRotation());

      D3DXVECTOR2 v2Normal(sin(fRotRad), cos(fRotRad));
      D3DXVECTOR2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

      D3DXVec2Normalize(&v2Normal, &v2Normal);
      D3DXVec2Normalize(&v2VictimNormal, &v2VictimNormal);

      float fScalar = D3DXVec2Dot(&v2Normal, &v2VictimNormal);

      if (fScalar < 0.0f) {
        if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
          rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
      } else {
        if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_BACK))
          rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
        else if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE))
          rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
      }
    }
  }
}

void CActorInstance::__HitGreate(CActorInstance &rVictim) {
  // DISABLE_KNOCKDOWN_ATTACK
  if (rVictim.IsKnockDown())
    return;
  if (rVictim.__IsStandUpMotion())
    return;
  // END_OF_DISABLE_KNOCKDOWN_ATTACK

  float fRotRad = D3DXToRadian(GetRotation());
  float fVictimRotRad = D3DXToRadian(rVictim.GetRotation());

  D3DXVECTOR2 v2Normal(sin(fRotRad), cos(fRotRad));
  D3DXVECTOR2 v2VictimNormal(sin(fVictimRotRad), cos(fVictimRotRad));

  D3DXVec2Normalize(&v2Normal, &v2Normal);
  D3DXVec2Normalize(&v2VictimNormal, &v2VictimNormal);

  float fScalar = D3DXVec2Dot(&v2Normal, &v2VictimNormal);

  rVictim.__Shake(100);

  if (rVictim.IsUsingSkill())
    return;

  if (rVictim.IsStun()) {
    if (fScalar < 0.0f)
      rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING);
    else {
      if (!rVictim.InterceptOnceMotion(
              CRaceMotionData::NAME_DAMAGE_FLYING_BACK))
        rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING);
    }

    rVictim.m_isRealDead = true;
  } else {
    if (fScalar < 0.0f) {
      if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING)) {
        rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP);
        rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
      }
    } else {
      if (!rVictim.InterceptOnceMotion(
              CRaceMotionData::NAME_DAMAGE_FLYING_BACK)) {
        if (rVictim.InterceptOnceMotion(CRaceMotionData::NAME_DAMAGE_FLYING)) {
          rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP);
          rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
        }
      } else {
        rVictim.PushOnceMotion(CRaceMotionData::NAME_STAND_UP_BACK);
        rVictim.PushLoopMotion(CRaceMotionData::NAME_WAIT);
      }
    }
  }
}

void CActorInstance::SetBlendingPosition(const TPixelPosition &c_rPosition,
                                         float fBlendingTime) {
  // return;
  TPixelPosition Position;

  Position.x = c_rPosition.x - m_x;
  Position.y = c_rPosition.y - m_y;
  Position.z = 0;

  m_PhysicsObject.SetLastPosition(Position, fBlendingTime);
}

void CActorInstance::ResetBlendingPosition() { m_PhysicsObject.Initialize(); }

void CActorInstance::GetBlendingPosition(TPixelPosition *pPosition) {
  if (m_PhysicsObject.isBlending()) {
    m_PhysicsObject.GetLastPosition(pPosition);
    pPosition->x += m_x;
    pPosition->y += m_y;
    pPosition->z += m_z;
  } else {
    pPosition->x = m_x;
    pPosition->y = m_y;
    pPosition->z = m_z;
  }
}

void CActorInstance::__PushCircle(CActorInstance &rVictim) {
  const TPixelPosition &c_rkPPosAtk = NEW_GetAtkPixelPositionRef();

  D3DXVECTOR3 v3SrcPos(c_rkPPosAtk.x, -c_rkPPosAtk.y, c_rkPPosAtk.z);

  const D3DXVECTOR3 &c_rv3SrcPos = v3SrcPos;
  const D3DXVECTOR3 &c_rv3DstPos = rVictim.GetPosition();

  D3DXVECTOR3 v3Direction;
  v3Direction.x = c_rv3DstPos.x - c_rv3SrcPos.x;
  v3Direction.y = c_rv3DstPos.y - c_rv3SrcPos.y;
  v3Direction.z = 0.0f;
  D3DXVec3Normalize(&v3Direction, &v3Direction);

  rVictim.__SetFallingDirection(v3Direction.x, v3Direction.y);
}

void CActorInstance::__PushDirect(CActorInstance &rVictim) {
  D3DXVECTOR3 v3Direction;
  v3Direction.x = cosf(D3DXToRadian(m_fcurRotation + 270.0f));
  v3Direction.y = sinf(D3DXToRadian(m_fcurRotation + 270.0f));
  v3Direction.z = 0.0f;

  rVictim.__SetFallingDirection(v3Direction.x, v3Direction.y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CActorInstance::__isInvisible() {
  if (IsDead())
    return true;

  if (CTimer::Instance().GetCurrentSecond() >= m_fInvisibleTime)
    return false;

  return true;
}

void CActorInstance::__SetFallingDirection(float fx, float fy) {
  m_PhysicsObject.SetDirection(D3DXVECTOR3(fx, fy, 0.0f));
}

// MR-3: Shaman on-mount hitting fix
float CActorInstance::__GetInvisibleTimeAdjust(
    const UINT uiSkill, const NRaceData::TAttackData &c_rAttackData) {

  static const int shamanw = 3, shamanm = 7;

  if ((GetRace() != shamanw && GetRace() != shamanm) || uiSkill != 0 ||
      m_fAtkSpd < 1.3)
    return 0.0f;

  const auto scale = (m_fAtkSpd - 1.3) / 1.3;
  const auto inv = c_rAttackData.fInvisibleTime * 0.5;
  return inv * scale;
}
// MR-3: -- END OF -- Shaman on-mount hitting fix
