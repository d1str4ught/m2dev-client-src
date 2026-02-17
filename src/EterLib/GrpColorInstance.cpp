#include "StdAfx.h"
#include "GrpColorInstance.h"
#include "EterBase/Timer.h"

CGraphicColorInstance::CGraphicColorInstance() {
  m_baseTime = 0;
  m_blendTime = 0;
}

CGraphicColorInstance::~CGraphicColorInstance() {}

void CGraphicColorInstance::Clear() {
  m_srcColor.Clear();
  m_dstColor.Clear();
  m_curColor.Clear();

  m_baseTime = 0;
  m_blendTime = 0;
}

void CGraphicColorInstance::SetColorReference(
    const CGraphicColor &c_rSrcColor) {
  m_srcColor = c_rSrcColor;
  m_dstColor = c_rSrcColor;
  m_curColor = c_rSrcColor;
}

void CGraphicColorInstance::BlendColorReference(
    DWORD blendTime, const CGraphicColor &c_rDstColor) {
  m_baseTime = GetCurrentTime();
  m_blendTime = blendTime;

  m_srcColor = m_curColor;
  m_dstColor = c_rDstColor;
}

void CGraphicColorInstance::Update() {
  DWORD curTime = GetCurrentTime();
  DWORD elapsedTime = curTime - m_baseTime;

  if (elapsedTime < m_blendTime) {
    m_curColor.Blend(elapsedTime / float(m_blendTime), m_srcColor, m_dstColor);
  } else {
    m_curColor = m_dstColor;
  }
}

DWORD CGraphicColorInstance::GetCurrentTime() {
  return CTimer::Instance().GetCurrentMillisecond();
}

const CGraphicColor &CGraphicColorInstance::GetCurrentColorReference() const {
  return m_curColor;
}
