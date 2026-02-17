#pragma once

#include "AbstractSingleton.h"

class IAbstractApplication : public TAbstractSingleton<IAbstractApplication> {
public:
  struct SCameraPos {
    float m_fUpDir;
    float m_fViewDir;
    float m_fCrossDir;

    SCameraPos() : m_fUpDir(0.0f), m_fViewDir(0.0f), m_fCrossDir(0.0f) {}
  };

  struct SCameraSetting {
    D3DXVECTOR3 v3CenterPosition;
    SCameraPos kCmrPos;
    float fRotation;
    float fPitch;
    float fZoom;

    SCameraSetting()
        : v3CenterPosition(0.0f, 0.0f, 0.0f), fRotation(0.0f), fPitch(0.0f),
          fZoom(0.0f) {}
  };

public:
  IAbstractApplication() {}
  virtual ~IAbstractApplication() {}

  virtual void GetMousePosition(POINT *ppt) = 0;
  virtual float GetGlobalTime() = 0;
  virtual float GetGlobalElapsedTime() = 0;

  virtual void SetServerTime(time_t tTime) = 0;
  virtual void SetCenterPosition(float fx, float fy, float fz) = 0;

  virtual void SetEventCamera(const SCameraSetting &c_rCameraSetting) = 0;
  virtual void BlendEventCamera(const SCameraSetting &c_rCameraSetting,
                                float fBlendTime) = 0;
  virtual void SetDefaultCamera() = 0;

  virtual void RunIMEUpdate() = 0;
  virtual void RunIMETabEvent() = 0;
  virtual void RunIMEReturnEvent() = 0;

  virtual void RunIMEOpenCandidateListEvent() = 0;
  virtual void RunIMECloseCandidateListEvent() = 0;
  virtual void RunIMEOpenReadingWndEvent() = 0;
  virtual void RunIMECloseReadingWndEvent() = 0;
};