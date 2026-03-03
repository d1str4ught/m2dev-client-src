#pragma once

#include "GrpBase.h"

struct ID3D11PixelShader;

class CPixelShader : public CGraphicBase {
public:
  CPixelShader();
  virtual ~CPixelShader();

  void Destroy();
  bool CreateFromDiskFile(const char *c_szFileName);

  void Set();

protected:
  void Initialize();

protected:
  LPDIRECT3DPIXELSHADER9 m_handle;
  ID3D11PixelShader *m_pDX11Handle;
};
