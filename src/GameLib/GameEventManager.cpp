#include "StdAfx.h"
#include "GameEventManager.h"

void CGameEventManager::SetCenterPosition(float fx, float fy, float fz) {
  m_CenterPosition.x = +fx;
  m_CenterPosition.y = -fy;
  m_CenterPosition.z = +fz;
}

void CGameEventManager::Update() {}

void CGameEventManager::ProcessEventScreenWaving(
    CActorInstance *pActorInstance,
    const CRaceMotionData::TScreenWavingEventData *c_pData) {
  TPixelPosition PixelPosition;
  pActorInstance->GetPixelPosition(&PixelPosition);

  float fdx = PixelPosition.x - m_CenterPosition.x;
  float fdy = PixelPosition.y - m_CenterPosition.y;
  float fdz = PixelPosition.z - m_CenterPosition.z;

  if (fdx * fdx + fdy * fdy + fdz * fdz >
      c_pData->iAffectingRange * c_pData->iAffectingRange)
    return;

  SetScreenEffectWaving(c_pData->fDurationTime, c_pData->iPower);
}

CGameEventManager::CGameEventManager() : m_CenterPosition(0.0f, 0.0f, 0.0f) {}

CGameEventManager::~CGameEventManager() {}
