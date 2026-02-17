#include "StdAfx.h"
#include "GrpColor.h"

CGraphicColor::CGraphicColor() { Clear(); }

CGraphicColor::CGraphicColor(const CGraphicColor &c_rSrcColor) {
  Set(c_rSrcColor);
}

CGraphicColor::CGraphicColor(float r, float g, float b, float a) {
  Set(r, g, b, a);
}

CGraphicColor::CGraphicColor(DWORD color) { Set(color); }

CGraphicColor::~CGraphicColor() {}

void CGraphicColor::Clear() { Set(1.0f, 1.0f, 1.0f, 1.0f); }

void CGraphicColor::Set(float r, float g, float b, float a) {
  m_r = r;
  m_g = g;
  m_b = b;
  m_a = a;
}

void CGraphicColor::Set(const CGraphicColor &c_rSrcColor) {
  m_r = c_rSrcColor.m_r;
  m_g = c_rSrcColor.m_g;
  m_b = c_rSrcColor.m_b;
  m_a = c_rSrcColor.m_a;
}

void CGraphicColor::Blend(float p, const CGraphicColor &c_rSrcColor,
                          const CGraphicColor &c_rDstColor) {
  float q = 1.0f - p;
  m_r = c_rSrcColor.m_r * q + c_rDstColor.m_r * p;
  m_g = c_rSrcColor.m_g * q + c_rDstColor.m_g * p;
  m_b = c_rSrcColor.m_b * q + c_rDstColor.m_b * p;
  m_a = c_rSrcColor.m_a * q + c_rDstColor.m_a * p;
}

void CGraphicColor::Set(DWORD pack) {
  m_b = (pack & 0xff) / 255.0f;
  pack >>= 8;
  m_g = (pack & 0xff) / 255.0f;
  pack >>= 8;
  m_r = (pack & 0xff) / 255.0f;
  pack >>= 8;
  m_a = (pack) / 255.0f;
}

DWORD CGraphicColor::GetPackValue() const {
  DWORD packValue = 0;

  packValue = int(255.0f * m_a);
  packValue <<= 8;
  packValue |= int(255.0f * m_r);
  packValue <<= 8;
  packValue |= int(255.0f * m_g);
  packValue <<= 8;
  packValue |= int(255.0f * m_b);

  return packValue;
}
