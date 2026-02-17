#pragma once

class CGraphicColor {
public:
  CGraphicColor(const CGraphicColor &c_rSrcColor);
  CGraphicColor(float r, float g, float b, float a);
  CGraphicColor(DWORD color);

  CGraphicColor();
  ~CGraphicColor();

  void Clear();

  void Set(float r, float g, float b, float a);
  void Set(const CGraphicColor &c_rSrcColor);
  void Set(DWORD color);

  void Blend(float p, const CGraphicColor &c_rSrcColor,
             const CGraphicColor &c_rDstColor);

  DWORD GetPackValue() const;

protected:
  float m_r;
  float m_g;
  float m_b;
  float m_a;
};
