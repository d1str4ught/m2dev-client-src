#include "StdAfx.h"
#include "GrpShadowTexture.h"
#include "StateManager.h"

//////////////////////////////////////////////////////////////////////////
void CGraphicShadowTexture::Destroy() {
  CGraphicTexture::Destroy();

  if (m_lpd3dShadowSurface) {
    m_lpd3dShadowSurface->Release();
    m_lpd3dShadowSurface = NULL;
  }

  if (m_lpd3dDepthSurface) {
    m_lpd3dDepthSurface->Release();
    m_lpd3dDepthSurface = NULL;
  }

  if (m_lpd3dShadowTexture) {
    m_lpd3dShadowTexture->Release();
    m_lpd3dShadowTexture = NULL;
  }

  Initialize();
}

bool CGraphicShadowTexture::Create(int width, int height) {
  Destroy();

  m_width = width;
  m_height = height;

  if (FAILED(ms_lpd3dDevice->CreateTexture(
          m_width, m_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
          D3DPOOL_DEFAULT, &m_lpd3dShadowTexture, nullptr)))
    return false;

  if (FAILED(m_lpd3dShadowTexture->GetSurfaceLevel(0, &m_lpd3dShadowSurface)))
    return false;

  if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(
          m_width, m_height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE,
          &m_lpd3dDepthSurface, nullptr)))
    return false;

  return true;
}

void CGraphicShadowTexture::Set(int stage) const {
  STATEMANAGER.SetTexture(stage, m_lpd3dShadowTexture);
}

const D3DXMATRIX &CGraphicShadowTexture::GetLightVPMatrixReference() const {
  return m_d3dLightVPMatrix;
}

LPDIRECT3DTEXTURE9 CGraphicShadowTexture::GetD3DTexture() const {
  return m_lpd3dShadowTexture;
}

void CGraphicShadowTexture::Begin() {
  D3DXMatrixMultiply(&m_d3dLightVPMatrix, &ms_matView, &ms_matProj);

  ms_lpd3dDevice->GetRenderTarget(0, &m_lpd3dOldBackBufferSurface);
  ms_lpd3dDevice->GetDepthStencilSurface(&m_lpd3dOldDepthBufferSurface);
  ms_lpd3dDevice->GetViewport(&m_d3dOldViewport);

  ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dDepthSurface);
  ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dShadowSurface);

  D3DVIEWPORT9 d3dViewport;
  d3dViewport.MinZ = 0.0f;
  d3dViewport.MaxZ = 1.0f;
  d3dViewport.X = 0;
  d3dViewport.Y = 0;
  d3dViewport.Width = m_width;
  d3dViewport.Height = m_height;

  ms_lpd3dDevice->SetViewport(&d3dViewport);
  ms_lpd3dDevice->BeginScene();

  ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                        0x00000000, 1.0f, 0L);

  STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  STATEMANAGER.SaveRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, true);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, true);
  STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xbb000000);

  STATEMANAGER.SetTexture(0, NULL);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

  STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
  STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

void CGraphicShadowTexture::End() {
  assert(m_lpd3dOldBackBufferSurface != NULL);
  assert(m_lpd3dOldDepthBufferSurface != NULL);

  ms_lpd3dDevice->EndScene();

  ms_lpd3dDevice->SetDepthStencilSurface(m_lpd3dOldDepthBufferSurface);
  ms_lpd3dDevice->SetRenderTarget(0, m_lpd3dOldBackBufferSurface);
  ms_lpd3dDevice->SetViewport(&m_d3dOldViewport);

  m_lpd3dOldBackBufferSurface->Release();
  m_lpd3dOldDepthBufferSurface->Release();

  m_lpd3dOldBackBufferSurface = NULL;
  m_lpd3dOldDepthBufferSurface = NULL;

  STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
  STATEMANAGER.RestoreRenderState(D3DRS_ZFUNC);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);

  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);

  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG1);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG2);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLOROP);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG1);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG2);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAOP);

  STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
  STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
  STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
  STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
  STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);
}

void CGraphicShadowTexture::Initialize() {
  CGraphicTexture::Initialize();

  m_lpd3dShadowSurface = NULL;
  m_lpd3dDepthSurface = NULL;
  m_lpd3dOldBackBufferSurface = NULL;
  m_lpd3dOldDepthBufferSurface = NULL;
  m_lpd3dShadowTexture = NULL;
}

CGraphicShadowTexture::CGraphicShadowTexture() { Initialize(); }

CGraphicShadowTexture::~CGraphicShadowTexture() { Destroy(); }