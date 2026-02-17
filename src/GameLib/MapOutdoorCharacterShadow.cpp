#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "EterLib/Camera.h"

#include "MapOutdoor.h"

static int recreate = false;

void CMapOutdoor::SetShadowTextureSize(WORD size) {
  if (m_wShadowMapSize != size) {
    recreate = true;
    Tracenf("ShadowTextureSize changed %d -> %d", m_wShadowMapSize, size);
  }

  m_wShadowMapSize = size;
}

void CMapOutdoor::CreateCharacterShadowTexture() {
  extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;

  if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
    return;

  ReleaseCharacterShadowTexture();

  if (IsLowTextureMemory())
    SetShadowTextureSize(128);

  m_ShadowMapViewport.X = 1;
  m_ShadowMapViewport.Y = 1;
  m_ShadowMapViewport.Width = m_wShadowMapSize - 2;
  m_ShadowMapViewport.Height = m_wShadowMapSize - 2;
  m_ShadowMapViewport.MinZ = 0.0f;
  m_ShadowMapViewport.MaxZ = 1.0f;

  if (FAILED(ms_lpd3dDevice->CreateTexture(
          m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET,
          D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture,
          nullptr))) {
    TraceError("CMapOutdoor Unable to create Character Shadow render target "
               "texture\n");
    return;
  }

  if (FAILED(m_lpCharacterShadowMapTexture->GetSurfaceLevel(
          0, &m_lpCharacterShadowMapRenderTargetSurface))) {
    TraceError("CMapOutdoor Unable to GetSurfaceLevel Character Shadow render "
               "target texture\n");
    return;
  }

  if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(
          m_wShadowMapSize, m_wShadowMapSize, D3DFMT_D16, D3DMULTISAMPLE_NONE,
          0, TRUE, &m_lpCharacterShadowMapDepthSurface, nullptr))) {
    TraceError("CMapOutdoor Unable to create Character Shadow depth Surface\n");
    return;
  }
}

void CMapOutdoor::ReleaseCharacterShadowTexture() {
  SAFE_RELEASE(m_lpCharacterShadowMapRenderTargetSurface);
  SAFE_RELEASE(m_lpCharacterShadowMapDepthSurface);
  SAFE_RELEASE(m_lpCharacterShadowMapTexture);
}

DWORD dwLightEnable = FALSE;

bool CMapOutdoor::BeginRenderCharacterShadowToTexture() {
  D3DXMATRIX matLightView, matLightProj;

  CCamera *pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

  if (!pCurrentCamera)
    return false;

  if (recreate) {
    CreateCharacterShadowTexture();
    recreate = false;
  }

  D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();

  D3DXVECTOR3 v3Eye(v3Target.x - 1.732f * 1250.0f, v3Target.y - 1250.0f,
                    v3Target.z + 2.0f * 1.732f * 1250.0f);

  const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
  D3DXMatrixLookAtRH(&matLightView, &v3Eye, &v3Target, &vv);

  D3DXMatrixOrthoRH(&matLightProj, 2550.0f, 2550.0f, 1.0f, 15000.0f);

  STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
  STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);

  dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

  STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

  ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface);
  ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface);

  ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurface);
  ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurface);

  ms_lpd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF,
                        1.0f, 0);

  ms_lpd3dDevice->GetViewport(&m_BackupViewport);
  ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport);

  return true;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture() {
  ms_lpd3dDevice->SetViewport(&m_BackupViewport);

  ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
  ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);

  SAFE_RELEASE(m_lpBackupRenderTargetSurface);
  SAFE_RELEASE(m_lpBackupDepthSurface);

  STATEMANAGER.RestoreTransform(D3DTS_VIEW);
  STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

  // Restore Device Context
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
  STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}
