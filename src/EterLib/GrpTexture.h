#pragma once

#include "GrpBase.h"

// Forward declaration for DX11
struct ID3D11ShaderResourceView;

class CGraphicTexture : public CGraphicBase {
public:
  virtual bool IsEmpty() const;

  int GetWidth() const;
  int GetHeight() const;

  void SetTextureStage(int stage) const;
  LPDIRECT3DTEXTURE9 GetD3DTexture() const;
  ID3D11ShaderResourceView *GetDX11SRV() const { return m_pDX11SRV; }

  void DestroyDeviceObjects();

protected:
  CGraphicTexture();
  virtual ~CGraphicTexture();

  void Destroy();
  void Initialize();

protected:
  bool m_bEmpty;

  int m_width;
  int m_height;

  LPDIRECT3DTEXTURE9 m_lpd3dTexture;
  ID3D11ShaderResourceView *m_pDX11SRV; // DX11 shader resource view (Phase 3)
};
