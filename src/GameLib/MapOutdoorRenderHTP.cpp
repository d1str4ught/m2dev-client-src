#include "StdAfx.h"
#include "AreaTerrain.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"

#include "EterLib/DX11TerrainShader.h"
#include "EterLib/StateManager.h"

// ============================================================================
// DX11 Helper: fill a CBTerrain constant buffer row-major from D3DXMATRIX
// ============================================================================
static void MatrixToArray(const D3DXMATRIX &m, float out[4][4]) {
  memcpy(out, &m, sizeof(float) * 16);
}

// ============================================================================
// __RenderTerrain_RenderHardwareTransformPatch
//
// HTP (Hardware Transform Patch) terrain renderer — DX11 path.
//
// DX9 equivalent:
//   Stage 0: tile color texture (wrap, camera-space uv from matTexTransform0)
//   Stage 1: splat alpha map    (clamp, camera-space uv from matSplatAlpha)
//   Blend:   SrcAlpha / InvSrcAlpha (alpha blending for splat coverage)
// ============================================================================
void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch() {
  // If DX11 terrain shader isn't ready, fall back to legacy DX9 path
  if (!m_pDX11TerrainShader || !m_pDX11TerrainShader->IsReady() ||
      !ms_pD3D11Context) {
    // ---- Legacy DX9 FFP path (kept for fallback) ----
    DWORD dwFogColor;
    float fFogFarDistance, fFogNearDistance;
    if (mc_pEnvironmentData) {
      dwFogColor = mc_pEnvironmentData->FogColor;
      fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
      fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
    } else {
      dwFogColor = 0xffffffff;
      fFogNearDistance = 5000.0f;
      fFogFarDistance = 10000.0f;
    }

    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX,
                                       D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
                                       D3DTTFF_COUNT2);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX,
                                       D3DTSS_TCI_CAMERASPACEPOSITION);
    STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,
                                       D3DTTFF_COUNT2);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);
    STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matWorldForCommonUse);
    STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matWorldForCommonUse);

    goto render_patches_legacy;
  }

  // =========================================================================
  // DX11 HTP Path
  // =========================================================================
  {
    m_iRenderedSplatNumSqSum = 0;
    m_iRenderedPatchNum = 0;
    m_iRenderedSplatNum = 0;
    m_RenderedTextureNumVector.clear();

    float fFogNearDistance = 5000.0f;
    float fFogFarDistance = 10000.0f;
    DWORD dwFogColor = 0xffffffff;
    if (mc_pEnvironmentData) {
      dwFogColor = mc_pEnvironmentData->FogColor;
      fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
      fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
    }

    // Bind terrain shader, input layout and topology
    m_pDX11TerrainShader->Bind();
    ms_pD3D11Context->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set up DX11 blend state: src_alpha / inv_src_alpha (alpha blending)
    // For DX9 compatibility we rely on the state set by STATEMANAGER —
    // the DX11 state cache already mirrors this from the render state bridge.

    // World offset reset
    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;

    std::pair<float, long> fog_far(fFogFarDistance + 1600.0f, 0);
    std::pair<float, long> fog_near(fFogNearDistance - 3200.0f, 0);
    if (mc_pEnvironmentData && mc_pEnvironmentData->bDensityFog)
      fog_far.first = 1e10f;

    auto far_it =
        std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
    auto near_it =
        std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

    WORD wPrimitiveCount;
    D3DPRIMITIVETYPE ePrimitiveType;
    BYTE byCurLOD = 0;
    float fLOD1 = __GetNoFogDistance();
    float fLOD2 = __GetFogDistance();

    SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

    // --- Patches within no-fog range ---
    for (auto it = m_PatchVector.begin(); it != near_it; ++it) {
      if (byCurLOD == 0 && fLOD1 <= it->first) {
        byCurLOD = 1;
        SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
      } else if (byCurLOD == 1 && fLOD2 <= it->first) {
        byCurLOD = 2;
        SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
      }
      __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount,
                                                ePrimitiveType);
      if (m_iRenderedSplatNum >= m_iSplatLimit)
        break;
    }

    // --- Patches in fog range ---
    if (m_iRenderedSplatNum < m_iSplatLimit) {
      for (auto it = near_it; it != far_it; ++it) {
        if (byCurLOD == 0 && fLOD1 <= it->first) {
          byCurLOD = 1;
          SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
        } else if (byCurLOD == 1 && fLOD2 <= it->first) {
          byCurLOD = 2;
          SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
        }
        __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount,
                                                  ePrimitiveType);
        if (m_iRenderedSplatNum >= m_iSplatLimit)
          break;
      }
    }

    // --- Far patches: render without textures (solid fog colour) ---
    if (m_iRenderedSplatNum < m_iSplatLimit) {
      for (auto it = far_it; it != m_PatchVector.end(); ++it) {
        if (byCurLOD == 0 && fLOD1 <= it->first) {
          byCurLOD = 1;
          SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
        } else if (byCurLOD == 1 && fLOD2 <= it->first) {
          byCurLOD = 2;
          SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
        }
        __HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount,
                                                 ePrimitiveType);
        if (m_iRenderedSplatNum >= m_iSplatLimit)
          break;
      }
    }

    std::sort(m_RenderedTextureNumVector.begin(),
              m_RenderedTextureNumVector.end());
    return;
  }

  // =========================================================================
  // Legacy DX9 FFP path (reached via goto when DX11 shader is unavailable)
  // =========================================================================
render_patches_legacy: {
  DWORD dwFogColor;
  float fFogFarDistance, fFogNearDistance;
  if (mc_pEnvironmentData) {
    dwFogColor = mc_pEnvironmentData->FogColor;
    fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
    fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
  } else {
    dwFogColor = 0xffffffff;
    fFogNearDistance = 5000.0f;
    fFogFarDistance = 10000.0f;
  }

  CSpeedTreeWrapper::ms_bSelfShadowOn = true;
  STATEMANAGER.SetBestFiltering(0);
  STATEMANAGER.SetBestFiltering(1);

  m_matWorldForCommonUse._41 = 0.0f;
  m_matWorldForCommonUse._42 = 0.0f;
  STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

  m_iRenderedSplatNumSqSum = 0;
  m_iRenderedPatchNum = 0;
  m_iRenderedSplatNum = 0;
  m_RenderedTextureNumVector.clear();

  std::pair<float, long> fog_far(fFogFarDistance + 1600.0f, 0);
  std::pair<float, long> fog_near(fFogNearDistance - 3200.0f, 0);
  if (mc_pEnvironmentData && mc_pEnvironmentData->bDensityFog)
    fog_far.first = 1e10f;

  auto far_it =
      std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
  auto near_it =
      std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

  WORD wPrimitiveCount;
  D3DPRIMITIVETYPE ePrimitiveType;
  BYTE byCurLOD = 0;
  float fLOD1 = __GetNoFogDistance();
  float fLOD2 = __GetFogDistance();

  SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

  for (auto it = m_PatchVector.begin(); it != near_it; ++it) {
    if (byCurLOD == 0 && fLOD1 <= it->first) {
      byCurLOD = 1;
      SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
    } else if (byCurLOD == 1 && fLOD2 <= it->first) {
      byCurLOD = 2;
      SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
    }
    __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount,
                                              ePrimitiveType);
    if (m_iRenderedSplatNum >= m_iSplatLimit)
      break;
    if (m_bDrawWireFrame)
      DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
  }

  if (m_iRenderedSplatNum < m_iSplatLimit) {
    for (auto it = near_it; it != far_it; ++it) {
      if (byCurLOD == 0 && fLOD1 <= it->first) {
        byCurLOD = 1;
        SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
      } else if (byCurLOD == 1 && fLOD2 <= it->first) {
        byCurLOD = 2;
        SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
      }
      __HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount,
                                                ePrimitiveType);
      if (m_iRenderedSplatNum >= m_iSplatLimit)
        break;
      if (m_bDrawWireFrame)
        DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
    }
  }

  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
  STATEMANAGER.SetTexture(0, NULL);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);
  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  if (m_iRenderedSplatNum < m_iSplatLimit) {
    for (auto it = far_it; it != m_PatchVector.end(); ++it) {
      if (byCurLOD == 0 && fLOD1 <= it->first) {
        byCurLOD = 1;
        SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
      } else if (byCurLOD == 1 && fLOD2 <= it->first) {
        byCurLOD = 2;
        SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
      }
      __HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount,
                                               ePrimitiveType);
      if (m_iRenderedSplatNum >= m_iSplatLimit)
        break;
      if (m_bDrawWireFrame)
        DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
    }
  }

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

  std::sort(m_RenderedTextureNumVector.begin(),
            m_RenderedTextureNumVector.end());

  STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
  STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
  STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
  STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
}
}

// ============================================================================
// __HardwareTransformPatch_RenderPatchSplat
//
// Per-patch splat rendering.  On the DX11 path:
//   1. Build CBTerrain (WVP, View, two texture transforms, fog)
//   2. Upload + bind through CDX11TerrainShader::UpdateConstants()
//   3. Bind tile texture → slot 0, splat alpha → slot 1 via PS SRV
//   4. Bind vertex buffer + draw via STATEMANAGER (which already bridges to
//   DX11)
//   5. Shadow pass using the shadow pixel shader variant
// ============================================================================
void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplat(
    long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType) {
  assert(NULL != m_pTerrainPatchProxyList &&
         "__HardwareTransformPatch_RenderPatchSplat");
  CTerrainPatchProxy *pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

  if (!pTerrainPatchProxy->isUsed())
    return;

  long sPatchNum = pTerrainPatchProxy->GetPatchNum();
  if (sPatchNum < 0)
    return;

  BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
  if (0xFF == ucTerrainNum)
    return;

  CTerrain *pTerrain;
  if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
    return;

  DWORD dwFogColor = 0xffffffff;
  float fFogNear = 5000.0f;
  float fFogFar = 10000.0f;
  if (mc_pEnvironmentData) {
    dwFogColor = mc_pEnvironmentData->FogColor;
    fFogNear = mc_pEnvironmentData->GetFogNearDistance();
    fFogFar = mc_pEnvironmentData->GetFogFarDistance();
  }

  WORD wCoordX, wCoordY;
  pTerrain->GetCoordinate(&wCoordX, &wCoordY);

  TTerrainSplatPatch &rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

  // ---- Patch world offset (different per-terrain chunk) ----
  m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
  m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);

  CGraphicVertexBuffer *pkVB =
      pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
  if (!pkVB)
    return;

  // Bind vertex buffer (bridges to DX11 through STATEMANAGER)
  STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(),
                               m_iPatchTerrainVertexSize);

  // =====================================================================
  // DX11 path
  // =====================================================================
  if (m_pDX11TerrainShader && m_pDX11TerrainShader->IsReady() &&
      ms_pD3D11Context) {
    // Build combined WVP: World * View * Proj
    D3DXMATRIX matWVP;
    D3DXMatrixMultiply(&matWVP, &m_matWorldForCommonUse, &ms_matView);
    D3DXMatrixMultiply(&matWVP, &matWVP, &ms_matProj);

    // Common texture transform: camera-space = vertex_pos * View * World
    D3DXMATRIX matTexBase;
    D3DXMatrixMultiply(&matTexBase, &m_matViewInverse, &m_matWorldForCommonUse);

    // Splat alpha transform (Stage 1 equivalent)
    D3DXMATRIX matSplatAlpha;
    D3DXMatrixMultiply(&matSplatAlpha, &matTexBase, &m_matSplatAlpha);

    // Unpack fog color
    float fogR = ((dwFogColor >> 16) & 0xFF) / 255.0f;
    float fogG = ((dwFogColor >> 8) & 0xFF) / 255.0f;
    float fogB = ((dwFogColor >> 0) & 0xFF) / 255.0f;

    int iPrevSplatNum = m_iRenderedSplatNum;
    bool isFirst = true;

    for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j) {
      TTerainSplat &rSplat = rTerrainSplatPatch.Splats[j];
      if (!rSplat.Active)
        continue;
      if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
        continue;

      const TTerrainTexture &rTexture = m_TextureSet.GetTexture(j);

      // Tile UV transform (Stage 0 equivalent)
      D3DXMATRIX matTileTransform;
      D3DXMatrixMultiply(&matTileTransform, &m_matViewInverse,
                         &rTexture.m_matTransform);

      // Build and upload constant buffer
      CBTerrain cb = {};
      MatrixToArray(matWVP, cb.matWorldViewProj);
      MatrixToArray(ms_matView, cb.matView);
      MatrixToArray(matTileTransform, cb.matTexTransform0);
      MatrixToArray(matSplatAlpha, cb.matTexTransform1);
      cb.fogColor[0] = fogR;
      cb.fogColor[1] = fogG;
      cb.fogColor[2] = fogB;
      cb.fogColor[3] = 1.0f;
      cb.fFogStart = fFogNear;
      cb.fFogEnd = fFogFar;
      cb.fAlphaTestEnable =
          isFirst ? 0.0f : 1.0f; // No alpha test on first pass
      cb.fAlphaRef = 0.0f;
      m_pDX11TerrainShader->UpdateConstants(cb);

      // Bind tile texture (slot 0) and splat alpha (slot 1) as DX11
      ID3D11ShaderResourceView *pTileSRV =
          CGraphicTexture::LookupDX11SRV(rTexture.pd3dTexture);
      ID3D11ShaderResourceView *pSplatSRV =
          CGraphicTexture::LookupDX11SRV(rSplat.pd3dTexture);

      ms_pD3D11Context->PSSetShaderResources(0, 1, &pTileSRV);
      ms_pD3D11Context->PSSetShaderResources(1, 1, &pSplatSRV);

      // Draw via STATEMANAGER (routes to DX11 DrawIndexedPrimitive)
      STATEMANAGER.DrawIndexedPrimitive(
          ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);

      auto aIterator = std::find(m_RenderedTextureNumVector.begin(),
                                 m_RenderedTextureNumVector.end(), (int)j);
      if (aIterator == m_RenderedTextureNumVector.end())
        m_RenderedTextureNumVector.push_back(j);

      ++m_iRenderedSplatNum;
      isFirst = false;

      if (m_iRenderedSplatNum >= m_iSplatLimit)
        break;
    }

    // Shadow pass
    if (m_bDrawShadow) {
      m_pDX11TerrainShader->BindShadowPass();

      // Compute shadow texture transform
      D3DXMATRIX matShadowTex;
      D3DXMatrixMultiply(&matShadowTex, &matTexBase, &m_matStaticShadow);

      CBTerrain cbShadow = {};
      MatrixToArray(matWVP, cbShadow.matWorldViewProj);
      MatrixToArray(ms_matView, cbShadow.matView);
      MatrixToArray(matShadowTex, cbShadow.matTexTransform0);
      MatrixToArray(matShadowTex, cbShadow.matTexTransform1);
      cbShadow.fogColor[0] = fogR;
      cbShadow.fogColor[1] = fogG;
      cbShadow.fogColor[2] = fogB;
      cbShadow.fogColor[3] = 1.0f;
      cbShadow.fFogStart = fFogNear;
      cbShadow.fFogEnd = fFogFar;
      m_pDX11TerrainShader->UpdateConstants(cbShadow);

      // Bind static shadow texture
      ID3D11ShaderResourceView *pShadowSRV =
          CGraphicTexture::LookupDX11SRV(pTerrain->GetShadowTexture());
      ms_pD3D11Context->PSSetShaderResources(0, 1, &pShadowSRV);

      // Optional: character shadow in slot 1
      if (m_bDrawChrShadow && m_lpCharacterShadowMapTexture) {
        ID3D11ShaderResourceView *pChrSRV =
            CGraphicTexture::LookupDX11SRV(m_lpCharacterShadowMapTexture);
        ms_pD3D11Context->PSSetShaderResources(1, 1, &pChrSRV);
      }
    }

    STATEMANAGER.DrawIndexedPrimitive(
        ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
    ++m_iRenderedSplatNum;

    // Rebind main splat shader for next patch
    m_pDX11TerrainShader->Bind();

    ++m_iRenderedPatchNum;
    int iCurSplatNum = m_iRenderedSplatNum - iPrevSplatNum;
    m_iRenderedSplatNumSqSum += iCurSplatNum * iCurSplatNum;
    return;
  }

// =====================================================================
// Legacy DX9 FFP path
// =====================================================================
D3DXMATRIX matTexTransform, matSplatAlphaTexTransform,
    matSplatColorTexTransform;

D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse,
                   &m_matWorldForCommonUse);
D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform,
                   &m_matSplatAlpha);
STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

D3DXMATRIX matTiling;
D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
matTiling._41 = 0.0f;
matTiling._42 = 0.0f;
D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

int iPrevRenderedSplatNum = m_iRenderedSplatNum;
bool isFirst = true;
for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j) {
  TTerainSplat &rSplat = rTerrainSplatPatch.Splats[j];
  if (!rSplat.Active)
    continue;
  if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
    continue;

  const TTerrainTexture &rTexture = m_TextureSet.GetTexture(j);

  D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse,
                     &rTexture.m_matTransform);
  STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

  if (isFirst) {
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
    STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
    STATEMANAGER.DrawIndexedPrimitive(
        ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    isFirst = false;
  } else {
    STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
    STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
    STATEMANAGER.DrawIndexedPrimitive(
        ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
  }

  auto aIt = std::find(m_RenderedTextureNumVector.begin(),
                       m_RenderedTextureNumVector.end(), (int)j);
  if (aIt == m_RenderedTextureNumVector.end())
    m_RenderedTextureNumVector.push_back(j);
  ++m_iRenderedSplatNum;
  if (m_iRenderedSplatNum >= m_iSplatLimit)
    break;
}

// Shadow pass (DX9 path)
if (m_bDrawShadow) {
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
  STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, 0xFFFFFFFF);
  STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
  STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

  D3DXMATRIX matShadowTexTransform;
  D3DXMatrixMultiply(&matShadowTexTransform, &matTexTransform,
                     &m_matStaticShadow);
  STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matShadowTexTransform);
  STATEMANAGER.SetTexture(0, pTerrain->GetShadowTexture());

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
  STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

  if (m_bDrawChrShadow) {
    STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);
    STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
  } else {
    STATEMANAGER.SetTexture(1, NULL);
  }

  ms_faceCount += wPrimitiveCount;
  STATEMANAGER.DrawIndexedPrimitive(
      ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
  ++m_iRenderedSplatNum;

  if (m_bDrawChrShadow) {
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  }
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, dwFogColor);
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
}

++m_iRenderedPatchNum;
int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;
m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchNone(
    long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType) {
  assert(NULL != m_pTerrainPatchProxyList &&
         "__HardwareTransformPatch_RenderPatchNone");
  CTerrainPatchProxy *pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

  if (!pTerrainPatchProxy->isUsed())
    return;

  CGraphicVertexBuffer *pkVB =
      pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
  if (!pkVB)
    return;

  STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(),
                               m_iPatchTerrainVertexSize);
  STATEMANAGER.DrawIndexedPrimitive(
      ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
}
