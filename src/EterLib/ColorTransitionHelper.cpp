#include "StdAfx.h"
#include "ColorTransitionHelper.h"

void CColorTransitionHelper::Clear(const float &c_rfRed, const float &c_rfGreen,
                                   const float &c_rfBlue,
                                   const float &c_rfAlpha) {

  m_fSrcRed = c_rfRed;
  m_fSrcGreen = c_rfGreen;
  m_fSrcBlue = c_rfBlue;
  m_fSrcAlpha = c_rfAlpha;

  m_fDstRed = c_rfRed;
  m_fDstGreen = c_rfGreen;
  m_fDstBlue = c_rfBlue;
  m_fDstAlpha = c_rfAlpha;

  m_dwCurColor = 0x00000000;

  m_dwStartTime = m_dwDuration = 0;
}

void CColorTransitionHelper::SetSrcColor(const float &c_rfRed,
                                         const float &c_rfGreen,
                                         const float &c_rfBlue,
                                         const float &c_rfAlpha) {
  m_fSrcRed = c_rfRed;
  m_fSrcGreen = c_rfGreen;
  m_fSrcBlue = c_rfBlue;
  m_fSrcAlpha = c_rfAlpha;
}

void CColorTransitionHelper::SetTransition(const float &c_rfRed,
                                           const float &c_rfGreen,
                                           const float &c_rfBlue,
                                           const float &c_rfAlpha,
                                           const DWORD &dwDuration) {
  m_fDstRed = c_rfRed;
  m_fDstGreen = c_rfGreen;
  m_fDstBlue = c_rfBlue;
  m_fDstAlpha = c_rfAlpha;

  m_dwDuration = dwDuration;
}

void CColorTransitionHelper::StartTransition() {
  m_bTransitionStarted = true;
  m_dwStartTime = GetCurrentTime();
}

bool CColorTransitionHelper::Update() {
  //	if (!m_bTransitionStarted)
  //		return false;

  DWORD dwCurTime = GetCurrentTime();
  DWORD dwElapsedTime = dwCurTime - m_dwStartTime;

  float fpercent = (float)(dwElapsedTime) / (float)(m_dwDuration);
  if (fpercent <= 0.0f)
    fpercent = 0.0f;
  if (fpercent >= 1.0f)
    fpercent = 1.0f;
  float fCurRed, fCurGreen, fCurBlue, fCurAlpha;
  fCurRed = m_fSrcRed + (m_fDstRed - m_fSrcRed) * fpercent;
  fCurGreen = m_fSrcGreen + (m_fDstGreen - m_fSrcGreen) * fpercent;
  fCurBlue = m_fSrcBlue + (m_fDstBlue - m_fSrcBlue) * fpercent;
  fCurAlpha = m_fSrcAlpha + (m_fDstAlpha - m_fSrcAlpha) * fpercent;

  // 	Tracef("%f, %f, %f, %f\n", fCurRed, fCurGreen, fCurBlue, fCurAlpha);

  m_dwCurColor = (((DWORD)(fCurAlpha * 255.0f) & 0xff) << 24) |
                 (((DWORD)(fCurRed * 255.0f) & 0xff) << 16) |
                 (((DWORD)(fCurGreen * 255.0f) & 0xff) << 8) |
                 ((DWORD)(fCurBlue * 255.0f) & 0xff);

  if ((1.0f == fpercent) && (fCurAlpha == m_fDstAlpha) &&
      (fCurRed == m_fDstRed) && (fCurGreen == m_fDstGreen) &&
      (fCurBlue == m_fDstBlue)) {
    m_bTransitionStarted = false;
    return false;
  }

  return true;
}

const D3DCOLOR &CColorTransitionHelper::GetCurColor() { return m_dwCurColor; }

CColorTransitionHelper::CColorTransitionHelper() : m_bTransitionStarted(false) {
  Clear(0.0f, 0.0f, 0.0f, 0.0f);
}

CColorTransitionHelper::~CColorTransitionHelper() {
  Clear(0.0f, 0.0f, 0.0f, 0.0f);
}
