#include "GrpTexture.h"
#include "EterBase/Stl.h"
#include "StateManager.h"
#include "StdAfx.h"

#include <d3d11.h>

// Static texture registry: maps DX9 texture → DX11 SRV
static std::unordered_map<LPDIRECT3DBASETEXTURE9, ID3D11ShaderResourceView *>
    s_TextureRegistry;

void CGraphicTexture::RegisterDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex,
                                      ID3D11ShaderResourceView *pSRV) {
  if (pDX9Tex && pSRV)
    s_TextureRegistry[pDX9Tex] = pSRV;
}

void CGraphicTexture::UnregisterDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex) {
  if (pDX9Tex)
    s_TextureRegistry.erase(pDX9Tex);
}

ID3D11ShaderResourceView *
CGraphicTexture::LookupDX11SRV(LPDIRECT3DBASETEXTURE9 pDX9Tex) {
  if (!pDX9Tex)
    return nullptr;
  auto it = s_TextureRegistry.find(pDX9Tex);
  return (it != s_TextureRegistry.end()) ? it->second : nullptr;
}

void CGraphicTexture::DestroyDeviceObjects() {
  // Unregister from DX11 texture registry before releasing
  if (m_lpd3dTexture)
    UnregisterDX11SRV(m_lpd3dTexture);
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

  // DX11: Also bind SRV if available (direct, no registry lookup needed)
  if (ms_pD3D11Context && m_pDX11SRV)
    ms_pD3D11Context->PSSetShaderResources(stage, 1, &m_pDX11SRV);
}

LPDIRECT3DTEXTURE9 CGraphicTexture::GetD3DTexture() const {
  return m_lpd3dTexture;
}

int CGraphicTexture::GetWidth() const { return m_width; }

int CGraphicTexture::GetHeight() const { return m_height; }

CGraphicTexture::CGraphicTexture() { Initialize(); }

CGraphicTexture::~CGraphicTexture() {}
