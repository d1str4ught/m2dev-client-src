#include "StdAfx.h"
#include "ActorInstance.h"

void CActorInstance::BlendAlphaValue(float fDstAlpha, float fDuration) {
  __BlendAlpha_Apply(fDstAlpha, fDuration);
}

void CActorInstance::SetBlendRenderMode() { m_iRenderMode = RENDER_MODE_BLEND; }

void CActorInstance::SetAlphaValue(float fAlpha) { m_fAlphaValue = fAlpha; }

float CActorInstance::GetAlphaValue() { return m_fAlphaValue; }

void CActorInstance::__BlendAlpha_Initialize() {
  m_kBlendAlpha.m_isBlending = false;
  m_kBlendAlpha.m_fBaseTime = 0.0f;
  m_kBlendAlpha.m_fDuration = 0.0f;
  m_kBlendAlpha.m_fBaseAlpha = 0.0f;
  m_kBlendAlpha.m_fDstAlpha = 0.0f;
  m_kBlendAlpha.m_iOldRenderMode = RENDER_MODE_NORMAL;
}

void CActorInstance::__BlendAlpha_Apply(float fDstAlpha, float fDuration) {
  m_kBlendAlpha.m_isBlending = true;
  m_kBlendAlpha.m_fBaseAlpha = GetAlphaValue();
  m_kBlendAlpha.m_fBaseTime = GetLocalTime();
  m_kBlendAlpha.m_fDuration = fDuration;
  m_kBlendAlpha.m_fDstAlpha = fDstAlpha;
  m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
}

void CActorInstance::__BlendAlpha_Update() {
  if (!m_kBlendAlpha.m_isBlending)
    return;

  float fElapsedTime = __BlendAlpha_GetElapsedTime();

  if (fElapsedTime < m_kBlendAlpha.m_fDuration) {
    float fCurAlpha = m_kBlendAlpha.m_fBaseAlpha +
                      (m_kBlendAlpha.m_fDstAlpha - m_kBlendAlpha.m_fBaseAlpha) *
                          fElapsedTime / m_kBlendAlpha.m_fDuration;
    SetBlendRenderMode();
    SetAlphaValue(fCurAlpha);
  } else {
    if (1.0f > m_kBlendAlpha.m_fDstAlpha) {
      SetBlendRenderMode();
    } else {
      m_iRenderMode = m_kBlendAlpha.m_iOldRenderMode;
    }

    SetAlphaValue(m_kBlendAlpha.m_fDstAlpha);

    __BlendAlpha_UpdateComplete();
  }
}

void CActorInstance::__BlendAlpha_UpdateComplete() {
  m_kBlendAlpha.m_isBlending = false;
}

float CActorInstance::__BlendAlpha_GetElapsedTime() {
  float fCurTime = GetLocalTime();
  return fCurTime - m_kBlendAlpha.m_fBaseTime;
}
