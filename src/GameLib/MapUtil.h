#pragma once

void Environment_Init(SEnvironmentData &envData);
bool Environment_Load(SEnvironmentData &envData, const char *envFileName);

void GetInterpolatedPosition(float curPositionRate,
                             TPixelPosition *PixelPosition);
float GetLinearInterpolation(float begin, float end, float curRate);

void PixelPositionToAttributeCellPosition(TPixelPosition PixelPosition,
                                          TCellPosition *pAttrCellPosition);
void AttributeCellPositionToPixelPosition(TCellPosition AttrCellPosition,
                                          TPixelPosition *pPixelPosition);

float GetPixelPositionDistance(const TPixelPosition &c_rsrcPosition,
                               const TPixelPosition &c_rdstPosition);

class CEaseOutInterpolation {
public:
  CEaseOutInterpolation();
  virtual ~CEaseOutInterpolation();

  void Initialize();

  BOOL Setup(float fStart, float fEnd, float fTime);
  void Interpolate(float fElapsedTime);
  BOOL isPlaying();

  float GetValue();
  float GetChangingValue();

protected:
  float m_fRemainingTime;
  float m_fValue;
  float m_fSpeed;
  float m_fAcceleration;

  float m_fStartValue;
  float m_fLastValue;
};
