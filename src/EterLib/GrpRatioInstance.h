#pragma once

class CGraphicRatioInstance {
public:
  CGraphicRatioInstance();
  virtual ~CGraphicRatioInstance();

  void Clear();

  void SetRatioReference(const float &ratio);
  void BlendRatioReference(DWORD blendTime, const float &ratio);

  void Update();

  const float &GetCurrentRatioReference() const;

protected:
  DWORD GetTime();

protected:
  float m_curRatio;
  float m_srcRatio;
  float m_dstRatio;

  DWORD m_baseTime;
  DWORD m_blendTime;
};
