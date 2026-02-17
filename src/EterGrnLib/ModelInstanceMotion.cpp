#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::CopyMotion(CGrannyModelInstance *pModelInstance,
                                      bool bIsFreeSourceControl) {
  if (!pModelInstance->IsMotionPlaying())
    return;

  if (m_pgrnCtrl)
    GrannyFreeControl(m_pgrnCtrl);

  float localTime = GetLocalTime();
  m_pgrnAni = pModelInstance->m_pgrnAni;
  m_pgrnCtrl =
      GrannyPlayControlledAnimation(localTime, m_pgrnAni, m_pgrnModelInstance);

  if (!m_pgrnCtrl)
    return;

  GrannySetControlSpeed(m_pgrnCtrl,
                        GrannyGetControlSpeed(pModelInstance->m_pgrnCtrl));
  GrannySetControlLoopCount(
      m_pgrnCtrl, GrannyGetControlLoopCount(pModelInstance->m_pgrnCtrl));

  GrannySetControlEaseIn(m_pgrnCtrl, true);
  GrannySetControlEaseOut(m_pgrnCtrl, false);

  GrannySetControlRawLocalClock(
      m_pgrnCtrl, GrannyGetControlRawLocalClock(pModelInstance->m_pgrnCtrl));

  GrannyFreeControlOnceUnused(m_pgrnCtrl);

  if (bIsFreeSourceControl) {
    GrannyFreeControl(pModelInstance->m_pgrnCtrl);
    pModelInstance->m_pgrnCtrl = NULL;
  }
}

bool CGrannyModelInstance::IsMotionPlaying() {
  if (!m_pgrnCtrl)
    return false;

  if (GrannyControlIsComplete(m_pgrnCtrl))
    return false;

  return true;
}

void CGrannyModelInstance::SetMotionPointer(const CGrannyMotion *pMotion,
                                            float blendTime, int loopCount,
                                            float speedRatio) {
  // TEST
  if (!m_pgrnWorldPoseReal)
    return;
  // END_OF_TEST

  granny_model_instance *pgrnModelInstance = m_pgrnModelInstance;
  if (!pgrnModelInstance)
    return;

  float localTime = GetLocalTime();

  bool isFirst = false;
  if (m_pgrnCtrl) {
    // float durationLeft = GrannyGetControlDurationLeft(m_pgrnCtrl);
    // float easeOutTime = (blendTime < durationLeft) ? blendTime :
    // durationLeft; float oldCtrlFinishTime = GrannyEaseControlOut(m_pgrnCtrl,
    // blendTime); //easeOutTime);
    GrannySetControlEaseOutCurve(m_pgrnCtrl, localTime, localTime + blendTime,
                                 1.0f, 1.0f, 0.0f, 0.0f);

    GrannySetControlEaseIn(m_pgrnCtrl, false);
    GrannySetControlEaseOut(m_pgrnCtrl, true);

    // Tracef("easeOut %f\n", easeOutTime);
    GrannyCompleteControlAt(m_pgrnCtrl, localTime + blendTime);
    // GrannyCompleteControlAt(m_pgrnCtrl, oldCtrlFinishTime);
    // GrannyCompleteControlAt(m_pgrnCtrl, localTime);
    GrannyFreeControlIfComplete(m_pgrnCtrl);
  } else {
    isFirst = true;
  }

  m_pgrnAni = pMotion->GetGrannyAnimationPointer();
  m_pgrnCtrl =
      GrannyPlayControlledAnimation(localTime, m_pgrnAni, pgrnModelInstance);
  if (!m_pgrnCtrl)
    return;

  GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
  GrannySetControlLoopCount(m_pgrnCtrl, loopCount);

  if (isFirst) {
    GrannySetControlEaseIn(m_pgrnCtrl, false);
    GrannySetControlEaseOut(m_pgrnCtrl, false);
  } else {
    GrannySetControlEaseIn(m_pgrnCtrl, true);
    GrannySetControlEaseOut(m_pgrnCtrl, false);
    if (blendTime > 0.0f)
      GrannySetControlEaseInCurve(m_pgrnCtrl, localTime, localTime + blendTime,
                                  0.0f, 0.0f, 1.0f, 1.0f);
  }

  // GrannyEaseControlIn(m_pgrnCtrl, blendTime, false);
  GrannyFreeControlOnceUnused(m_pgrnCtrl);
  // Tracef("easeIn %f\n", blendTime);
}

void CGrannyModelInstance::ChangeMotionPointer(const CGrannyMotion *pMotion,
                                               int loopCount,
                                               float speedRatio) {
  granny_model_instance *pgrnModelInstance = m_pgrnModelInstance;
  if (!pgrnModelInstance)
    return;

  // 보간 되는 앞부분을 스킵 하기 위해 LocalTime 을 어느 정도 무시한다. -
  // [levites]
  float fSkipTime = 0.3f;
  float localTime = GetLocalTime() - fSkipTime;

  if (m_pgrnCtrl) {
    GrannySetControlEaseIn(m_pgrnCtrl, false);
    GrannySetControlEaseOut(m_pgrnCtrl, false);
    GrannyCompleteControlAt(m_pgrnCtrl, localTime);
    GrannyFreeControlIfComplete(m_pgrnCtrl);
  }

  m_pgrnAni = pMotion->GetGrannyAnimationPointer();
  m_pgrnCtrl =
      GrannyPlayControlledAnimation(localTime, m_pgrnAni, pgrnModelInstance);
  if (!m_pgrnCtrl)
    return;

  GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
  GrannySetControlLoopCount(m_pgrnCtrl, loopCount);
  GrannySetControlEaseIn(m_pgrnCtrl, false);
  GrannySetControlEaseOut(m_pgrnCtrl, false);

  GrannyFreeControlOnceUnused(m_pgrnCtrl);
}

void CGrannyModelInstance::SetMotionAtEnd() {
  if (!m_pgrnCtrl)
    return;

  // Tracef("%f\n", endingTime);
  float endingTime = GrannyGetControlLocalDuration(m_pgrnCtrl);
  GrannySetControlRawLocalClock(m_pgrnCtrl, endingTime);
}
