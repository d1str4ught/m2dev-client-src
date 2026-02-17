#pragma once

#include "EterLib/GrpScreen.h"

class CSnowParticle;

class CSnowEnvironment : public CScreen {
public:
  CSnowEnvironment();
  virtual ~CSnowEnvironment();

  bool Create();
  void Destroy();

  void Enable();
  void Disable();

  void Update(const D3DXVECTOR3 &c_rv3Pos);
  void Deform();
  void Render();

protected:
  void __Initialize();
  bool __CreateBlurTexture();
  bool __CreateGeometry();
  void __BeginBlur();
  void __ApplyBlur();

protected:
  LPDIRECT3DSURFACE9 m_lpOldSurface;
  LPDIRECT3DSURFACE9 m_lpOldDepthStencilSurface;

  LPDIRECT3DTEXTURE9 m_lpSnowTexture;
  LPDIRECT3DSURFACE9 m_lpSnowRenderTargetSurface;
  LPDIRECT3DSURFACE9 m_lpSnowDepthSurface;

  LPDIRECT3DTEXTURE9 m_lpAccumTexture;
  LPDIRECT3DSURFACE9 m_lpAccumRenderTargetSurface;
  LPDIRECT3DSURFACE9 m_lpAccumDepthSurface;

  LPDIRECT3DVERTEXBUFFER9 m_pVB;
  LPDIRECT3DINDEXBUFFER9 m_pIB;

  D3DXVECTOR3 m_v3Center;

  WORD m_wBlurTextureSize;
  CGraphicImageInstance *m_pImageInstance;
  std::vector<CSnowParticle *> m_kVct_pkParticleSnow;

  DWORD m_dwParticleMaxNum;
  BOOL m_bBlurEnable;

  BOOL m_bSnowEnable;
};
