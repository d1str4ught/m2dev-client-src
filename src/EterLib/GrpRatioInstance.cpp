#include "StdAfx.h"
#include "GrpRatioInstance.h"
#include "EterBase/Timer.h"

CGraphicRatioInstance::CGraphicRatioInstance() {
  m_baseTime = 0;
  m_blendTime = 0;
}

CGraphicRatioInstance::~CGraphicRatioInstance() {}

void CGraphicRatioInstance::Clear() {
  m_curRatio = 0.0f;
  m_srcRatio = 0.0f;
  m_dstRatio = 0.0f;

  m_baseTime = 0;
  m_blendTime = 0;
}

void CGraphicRatioInstance::SetRatioReference(const float &c_rRatio) {
  m_curRatio = c_rRatio;
  m_srcRatio = m_curRatio;
  m_dstRatio = m_curRatio;
}

void CGraphicRatioInstance::BlendRatioReference(DWORD blendTime,
                                                const float &c_rDstRatio) {
  m_baseTime = GetTime();
  m_blendTime = blendTime;

  m_srcRatio = m_curRatio;
  m_dstRatio = c_rDstRatio;
}

void CGraphicRatioInstance::Update() {
  DWORD curTime = GetTime();
  DWORD elapsedTime = curTime - m_baseTime;

  if (elapsedTime < m_blendTime) {
    float diff = m_dstRatio - m_srcRatio;
    float rate = elapsedTime / float(m_blendTime);
    m_curRatio = diff * rate + m_srcRatio;
  } else {
    m_curRatio = m_dstRatio;
  }
}

DWORD CGraphicRatioInstance::GetTime() {
  return CTimer::Instance().GetCurrentMillisecond();
}

const float &CGraphicRatioInstance::GetCurrentRatioReference() const {
  return m_curRatio;
}
