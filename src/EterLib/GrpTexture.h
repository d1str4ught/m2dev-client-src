#pragma once

#include "GrpBase.h"
#include <unordered_map>

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

  // ------ DX11 Texture Registry (static) ------
  // Maps DX9 texture pointers to their DX11 SRV counterparts
  static void RegisterDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex,
                              ID3D11ShaderResourceView *pSRV);
  static void UnregisterDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex);
  static ID3D11ShaderResourceView *
  LookupDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex);

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
