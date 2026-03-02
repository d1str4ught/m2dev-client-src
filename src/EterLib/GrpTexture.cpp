#include "GrpTexture.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"


#include <d3d11.h>

void CGraphicTexture::DestroyDeviceObjects() {
  safe_release(m_lpd3dTexture);
  if (m_pDX11SRV) {
    m_pDX11SRV->Release();
    m_pDX11SRV = nullptr;
  }
}

void CGraphicTexture::Destroy() {
  DestroyDeviceObjects();

  Initialize();
}

void CGraphicTexture::Initialize() {
  m_lpd3dTexture = NULL;
  m_pDX11SRV = nullptr;
  m_width = 0;
  m_height = 0;
  m_bEmpty = true;
}

bool CGraphicTexture::IsEmpty() const { return m_bEmpty; }

void CGraphicTexture::SetTextureStage(int stage) const {
  assert(ms_lpd3dDevice != NULL);
  STATEMANAGER.SetTexture(stage, m_lpd3dTexture);
}

LPDIRECT3DTEXTURE9 CGraphicTexture::GetD3DTexture() const {
  return m_lpd3dTexture;
}

int CGraphicTexture::GetWidth() const { return m_width; }

int CGraphicTexture::GetHeight() const { return m_height; }

CGraphicTexture::CGraphicTexture() { Initialize(); }

CGraphicTexture::~CGraphicTexture() {}
