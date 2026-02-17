#include "StdAfx.h"
#include "InstanceBase.h"
#include "AbstractPlayer.h"
#include "GameLib/ActorInstance.h"

const int c_iFishingRotStep = 8;
const float c_fFishingDistance = 600.0f;

void CInstanceBase::SetMotionMode(int iMotionMode) {
  m_GraphicThingInstance.SetMotionMode(iMotionMode);
}

int CInstanceBase::GetMotionMode(DWORD dwMotionIndex) {
  return m_GraphicThingInstance.GetMotionMode();
}

void CInstanceBase::SetLoopMotion(WORD wMotion, float fBlendTime /* =0.1f */,
                                  float fSpeedRatio) {
  m_GraphicThingInstance.SetLoopMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::PushOnceMotion(WORD wMotion, float fBlendTime,
                                   float fSpeedRatio) {
  m_GraphicThingInstance.PushOnceMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::PushLoopMotion(WORD wMotion, float fBlendTime,
                                   float fSpeedRatio) {
  m_GraphicThingInstance.PushLoopMotion(wMotion, fBlendTime, fSpeedRatio);
}

void CInstanceBase::ResetLocalTime() {
  m_GraphicThingInstance.ResetLocalTime();
}

void CInstanceBase::SetEndStopMotion() {
  m_GraphicThingInstance.SetEndStopMotion();
}

BOOL CInstanceBase::isLock() { return m_GraphicThingInstance.isLock(); }

void CInstanceBase::StartFishing(float frot) {
  BlendRotation(frot);

  const TPixelPosition &c_rkPPosCur =
      m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
  float fRot = m_GraphicThingInstance.GetTargetRotation();
  // float
  // fPlainCoordRot=ELRightCoord_ConvertToPlainCoordDegree(fRightCoordRot);

  TPixelPosition kPPosFishing;
  ELPlainCoord_GetRotatedPixelPosition(c_rkPPosCur.x, c_rkPPosCur.y,
                                       c_fFishingDistance, fRot,
                                       &kPPosFishing.x, &kPPosFishing.y);
  if (!__Background_GetWaterHeight(kPPosFishing, &kPPosFishing.z))
    kPPosFishing.z = c_rkPPosCur.z;

  D3DXVECTOR3 v3Fishing;
  PixelPositionToD3DXVECTOR3(kPPosFishing, &v3Fishing);
  m_GraphicThingInstance.SetFishingPosition(v3Fishing);

  PushOnceMotion(CRaceMotionData::NAME_FISHING_THROW);
  PushLoopMotion(CRaceMotionData::NAME_FISHING_WAIT);
}
void CInstanceBase::StopFishing() {
  m_GraphicThingInstance.InterceptOnceMotion(
      CRaceMotionData::NAME_FISHING_STOP);
  PushLoopMotion(CRaceMotionData::NAME_WAIT);
}
void CInstanceBase::ReactFishing() {
  PushOnceMotion(CRaceMotionData::NAME_FISHING_REACT);
  PushLoopMotion(CRaceMotionData::NAME_FISHING_WAIT);
}
void CInstanceBase::CatchSuccess() {
  m_GraphicThingInstance.InterceptOnceMotion(
      CRaceMotionData::NAME_FISHING_CATCH);
  PushLoopMotion(CRaceMotionData::NAME_WAIT);
}
void CInstanceBase::CatchFail() {
  m_GraphicThingInstance.InterceptOnceMotion(
      CRaceMotionData::NAME_FISHING_FAIL);
  PushLoopMotion(CRaceMotionData::NAME_WAIT);
}

BOOL CInstanceBase::GetFishingRot(int *pirot) {
  const TPixelPosition &c_rkPPosCur =
      m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
  float fCharacterRot = m_GraphicThingInstance.GetRotation();

  // float frot = fCharacterRot;

  for (float fRot = 0.0f; fRot <= 180.0f; fRot += 10.0f) {
    TPixelPosition kPPosFishingRight;
    ELPlainCoord_GetRotatedPixelPosition(
        c_rkPPosCur.x, c_rkPPosCur.y, c_fFishingDistance, fCharacterRot + fRot,
        &kPPosFishingRight.x, &kPPosFishingRight.y);
    if (__Background_IsWaterPixelPosition(kPPosFishingRight)) {
      *pirot = fCharacterRot + fRot;
      return TRUE;
    }

    TPixelPosition kPPosFishingLeft;
    ELPlainCoord_GetRotatedPixelPosition(
        c_rkPPosCur.x, c_rkPPosCur.y, c_fFishingDistance, fCharacterRot - fRot,
        &kPPosFishingLeft.x, &kPPosFishingLeft.y);
    if (__Background_IsWaterPixelPosition(kPPosFishingLeft)) {
      *pirot = fCharacterRot - fRot;
      return TRUE;
    }
  }

  return FALSE;
}

void CInstanceBase::__EnableChangingTCPState() { m_bEnableTCPState = TRUE; }

void CInstanceBase::__DisableChangingTCPState() { m_bEnableTCPState = FALSE; }

void CInstanceBase::ActDualEmotion(CInstanceBase &rkDstInst,
                                   WORD wMotionNumber1, WORD wMotionNumber2) {
  if (!IsWaiting()) {
    m_GraphicThingInstance.SetLoopMotion(CRaceMotionData::NAME_WAIT, 0.05f);
  }
  if (!rkDstInst.IsWaiting()) {
    rkDstInst.m_GraphicThingInstance.SetLoopMotion(CRaceMotionData::NAME_WAIT,
                                                   0.05f);
  }

  const float c_fEmotionDistance = 100.0f;
  const TPixelPosition &c_rMainPosition = NEW_GetCurPixelPositionRef();
  const TPixelPosition &c_rTargetPosition =
      rkDstInst.NEW_GetCurPixelPositionRef();
  TPixelPosition kDirection = c_rMainPosition - c_rTargetPosition;
  float fDistance =
      sqrtf((kDirection.x * kDirection.x) + (kDirection.y * kDirection.y));
  TPixelPosition kDstPosition;
  kDstPosition.x =
      c_rTargetPosition.x + (kDirection.x / fDistance) * c_fEmotionDistance;
  kDstPosition.y =
      c_rTargetPosition.y + (kDirection.y / fDistance) * c_fEmotionDistance;

  DWORD dwCurTime = ELTimer_GetServerMSec() + 500;
  PushTCPStateExpanded(dwCurTime, kDstPosition, 0.0f, FUNC_EMOTION,
                       MAKELONG(wMotionNumber1, wMotionNumber2),
                       rkDstInst.GetVirtualID());

  __DisableChangingTCPState();
  rkDstInst.__DisableChangingTCPState();

  if (__IsMainInstance() || rkDstInst.__IsMainInstance()) {
    IAbstractPlayer &rPlayer = IAbstractPlayer::GetSingleton();
    rPlayer.StartEmotionProcess();
  }
}

void CInstanceBase::ActEmotion(DWORD dwMotionNumber) {
  PushOnceMotion(dwMotionNumber);
}
