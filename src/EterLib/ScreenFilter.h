#pragma once

#include "GrpScreen.h"

class CScreenFilter : public CScreen {
public:
  CScreenFilter();
  virtual ~CScreenFilter();

  void SetEnable(BOOL bFlag);
  void SetBlendType(BYTE bySrcType, BYTE byDestType);
  void SetColor(const D3DXCOLOR &c_rColor);

  void Render();

protected:
  BOOL m_bEnable;
  BYTE m_bySrcType;
  BYTE m_byDestType;
  D3DXCOLOR m_Color;
};