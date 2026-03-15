#include "DX11StateCache.h"
#include "GrpBase.h"
#include "StdAfx.h"

#include <d3d11.h>

// ============================================================================
// CDX11StateCache Implementation
// ============================================================================

CDX11StateCache::CDX11StateCache()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pDepthStencilState(nullptr),
      m_pBlendState(nullptr), m_pRasterizerState(nullptr),
      m_bDepthStencilDirty(true), m_bBlendDirty(true), m_bRasterizerDirty(true),
      m_bRenderTargetBound(false) {
  memset(m_pSamplerStates, 0, sizeof(m_pSamplerStates));
  memset(m_bSamplerDirty, 1, sizeof(m_bSamplerDirty)); // all dirty initially

  // DX9 defaults — depth/stencil
  m_dwZEnable = D3DZB_TRUE;
  m_dwZWriteEnable = TRUE;
  m_dwZFunc = D3DCMP_LESSEQUAL;
  m_dwStencilEnable = FALSE;
  m_dwStencilFunc = D3DCMP_ALWAYS;
  m_dwStencilRef = 0;
  m_dwStencilMask = 0xFFFFFFFF;
  m_dwStencilWriteMask = 0xFFFFFFFF;
  m_dwStencilFail = D3DSTENCILOP_KEEP;
  m_dwStencilZFail = D3DSTENCILOP_KEEP;
  m_dwStencilPass = D3DSTENCILOP_KEEP;

  // DX9 defaults — blend
  m_dwAlphaBlendEnable = FALSE;
  m_dwSrcBlend = D3DBLEND_ONE;
  m_dwDestBlend = D3DBLEND_ZERO;
  m_dwBlendOp = D3DBLENDOP_ADD;
  m_dwSeparateAlphaBlendEnable = FALSE;
  m_dwSrcBlendAlpha = D3DBLEND_ONE;
  m_dwDestBlendAlpha = D3DBLEND_ZERO;
  m_dwBlendOpAlpha = D3DBLENDOP_ADD;
  m_dwAlphaTestEnable = FALSE;
  m_dwAlphaRef = 0;
  m_dwAlphaFunc = D3DCMP_ALWAYS;
  m_dwColorWriteEnable = 0x0F; // D3DCOLORWRITEENABLE_ALL

  // DX9 defaults — rasterizer
  m_dwFillMode = D3DFILL_SOLID;
  m_dwCullMode = D3DCULL_CCW;
  m_dwScissorTestEnable = FALSE;
  m_dwDepthBias = 0;
  m_dwSlopeScaleDepthBias = 0;
  m_dwMultiSampleAntiAlias = TRUE;
  m_dwAntiAliasedLineEnable = FALSE;

  // DX9 defaults — samplers
  for (int i = 0; i < DX11_MAX_SAMPLER_STAGES; i++) {
    m_SamplerMirror[i].MinFilter = D3DTEXF_POINT;
    m_SamplerMirror[i].MagFilter = D3DTEXF_POINT;
    m_SamplerMirror[i].MipFilter = D3DTEXF_NONE;
    m_SamplerMirror[i].AddressU = D3DTADDRESS_WRAP;
    m_SamplerMirror[i].AddressV = D3DTADDRESS_WRAP;
    m_SamplerMirror[i].AddressW = D3DTADDRESS_WRAP;
    m_SamplerMirror[i].MaxAnisotropy = 1;
    m_SamplerMirror[i].MipLODBias = 0;
  }
}

CDX11StateCache::~CDX11StateCache() { Shutdown(); }

void CDX11StateCache::Initialize(ID3D11Device *pDevice,
                                 ID3D11DeviceContext *pContext,
                                 int viewportWidth, int viewportHeight) {
  m_pDevice = pDevice;
  m_pContext = pContext;
  m_iViewportWidth = viewportWidth;
  m_iViewportHeight = viewportHeight;

  // Mark all dirty to force initial state creation
  m_bDepthStencilDirty = true;
  m_bBlendDirty = true;
  m_bRasterizerDirty = true;
  m_bRenderTargetBound = false;
  for (int i = 0; i < DX11_MAX_SAMPLER_STAGES; i++)
    m_bSamplerDirty[i] = true;
}

void CDX11StateCache::Shutdown() {
  if (m_pDepthStencilState) {
    m_pDepthStencilState->Release();
    m_pDepthStencilState = nullptr;
  }
  if (m_pBlendState) {
    m_pBlendState->Release();
    m_pBlendState = nullptr;
  }
  if (m_pRasterizerState) {
    m_pRasterizerState->Release();
    m_pRasterizerState = nullptr;
  }

  for (int i = 0; i < DX11_MAX_SAMPLER_STAGES; i++) {
    if (m_pSamplerStates[i]) {
      m_pSamplerStates[i]->Release();
      m_pSamplerStates[i] = nullptr;
    }
  }

  m_pDevice = nullptr;
  m_pContext = nullptr;
}

// ============================================================================
// State Change Notifications
// ============================================================================

void CDX11StateCache::OnRenderStateChanged(D3DRENDERSTATETYPE type,
                                           DWORD value) {
  switch (type) {
  // --- Depth / Stencil ---
  case D3DRS_ZENABLE:
    m_dwZEnable = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_ZWRITEENABLE:
    m_dwZWriteEnable = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_ZFUNC:
    m_dwZFunc = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILENABLE:
    m_dwStencilEnable = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILFUNC:
    m_dwStencilFunc = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILREF:
    m_dwStencilRef = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILMASK:
    m_dwStencilMask = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILWRITEMASK:
    m_dwStencilWriteMask = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILFAIL:
    m_dwStencilFail = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILZFAIL:
    m_dwStencilZFail = value;
    m_bDepthStencilDirty = true;
    break;
  case D3DRS_STENCILPASS:
    m_dwStencilPass = value;
    m_bDepthStencilDirty = true;
    break;

  // --- Blend ---
  case D3DRS_ALPHABLENDENABLE:
    m_dwAlphaBlendEnable = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_SRCBLEND:
    m_dwSrcBlend = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_DESTBLEND:
    m_dwDestBlend = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_BLENDOP:
    m_dwBlendOp = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_SEPARATEALPHABLENDENABLE:
    m_dwSeparateAlphaBlendEnable = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_SRCBLENDALPHA:
    m_dwSrcBlendAlpha = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_DESTBLENDALPHA:
    m_dwDestBlendAlpha = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_BLENDOPALPHA:
    m_dwBlendOpAlpha = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_ALPHATESTENABLE:
    m_dwAlphaTestEnable = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_ALPHAREF:
    m_dwAlphaRef = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_ALPHAFUNC:
    m_dwAlphaFunc = value;
    m_bBlendDirty = true;
    break;
  case D3DRS_COLORWRITEENABLE:
    m_dwColorWriteEnable = value;
    m_bBlendDirty = true;
    break;

  // --- Rasterizer ---
  case D3DRS_FILLMODE:
    m_dwFillMode = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_CULLMODE:
    m_dwCullMode = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_SCISSORTESTENABLE:
    m_dwScissorTestEnable = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_DEPTHBIAS:
    m_dwDepthBias = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_SLOPESCALEDEPTHBIAS:
    m_dwSlopeScaleDepthBias = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_MULTISAMPLEANTIALIAS:
    m_dwMultiSampleAntiAlias = value;
    m_bRasterizerDirty = true;
    break;
  case D3DRS_ANTIALIASEDLINEENABLE:
    m_dwAntiAliasedLineEnable = value;
    m_bRasterizerDirty = true;
    break;

  default:
    break; // Unhandled states — no DX11 equivalent or handled elsewhere
  }
}

void CDX11StateCache::OnSamplerStateChanged(DWORD stage,
                                            D3DSAMPLERSTATETYPE type,
                                            DWORD value) {
  if (stage >= DX11_MAX_SAMPLER_STAGES)
    return;

  SamplerMirror &sm = m_SamplerMirror[stage];

  switch (type) {
  case D3DSAMP_MINFILTER:
    sm.MinFilter = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_MAGFILTER:
    sm.MagFilter = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_MIPFILTER:
    sm.MipFilter = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_ADDRESSU:
    sm.AddressU = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_ADDRESSV:
    sm.AddressV = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_ADDRESSW:
    sm.AddressW = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_MAXANISOTROPY:
    sm.MaxAnisotropy = value;
    m_bSamplerDirty[stage] = true;
    break;
  case D3DSAMP_MIPMAPLODBIAS:
    sm.MipLODBias = value;
    m_bSamplerDirty[stage] = true;
    break;
  default:
    break;
  }
}

// ============================================================================
// Apply Dirty State Objects
// ============================================================================

void CDX11StateCache::ApplyState() {
  if (!m_pDevice || !m_pContext)
    return;

  // Safety fallback: bind scene render target if CScreen::Begin() hasn't yet.
  if (!m_bRenderTargetBound) {
    ID3D11RenderTargetView *pRTV = CGraphicBase::ms_pSceneRTV
                                       ? CGraphicBase::ms_pSceneRTV
                                       : CGraphicBase::ms_pRenderTargetView;

    if (pRTV) {
      m_pContext->OMSetRenderTargets(1, &pRTV,
                                     CGraphicBase::ms_pDepthStencilView);

      // Derive viewport from the actual render target dimensions
      ID3D11Resource *pRTResource = nullptr;
      pRTV->GetResource(&pRTResource);
      D3D11_VIEWPORT vp = {};
      if (pRTResource) {
        ID3D11Texture2D *pRTTex = nullptr;
        pRTResource->QueryInterface(__uuidof(ID3D11Texture2D),
                                    (void **)&pRTTex);
        if (pRTTex) {
          D3D11_TEXTURE2D_DESC texDesc;
          pRTTex->GetDesc(&texDesc);
          vp.Width = (float)texDesc.Width;
          vp.Height = (float)texDesc.Height;
          pRTTex->Release();
        }
        pRTResource->Release();
      }
      if (vp.Width < 2.0f) {
        vp.Width = 1024.0f;
        vp.Height = 768.0f;
      }
      vp.MinDepth = 0.0f;
      vp.MaxDepth = 1.0f;
      m_pContext->RSSetViewports(1, &vp);

      m_bRenderTargetBound = true;
    }
  }

  if (m_bDepthStencilDirty)
    ApplyDepthStencilState();
  if (m_bBlendDirty)
    ApplyBlendState();
  if (m_bRasterizerDirty)
    ApplyRasterizerState();
  for (int i = 0; i < DX11_MAX_SAMPLER_STAGES; i++) {
    if (m_bSamplerDirty[i])
      ApplySamplerState(i);
  }
}

// ============================================================================
// DepthStencil State
// ============================================================================

void CDX11StateCache::ApplyDepthStencilState() {
  m_bDepthStencilDirty = false;

  // Release old
  if (m_pDepthStencilState) {
    m_pDepthStencilState->Release();
    m_pDepthStencilState = nullptr;
  }

  D3D11_DEPTH_STENCIL_DESC desc = {};
  desc.DepthEnable = (m_dwZEnable != D3DZB_FALSE) ? TRUE : FALSE;
  desc.DepthWriteMask = m_dwZWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL
                                         : D3D11_DEPTH_WRITE_MASK_ZERO;
  desc.DepthFunc = (D3D11_COMPARISON_FUNC)TranslateCmpFunc(m_dwZFunc);

  desc.StencilEnable = m_dwStencilEnable ? TRUE : FALSE;
  desc.StencilReadMask = (UINT8)(m_dwStencilMask & 0xFF);
  desc.StencilWriteMask = (UINT8)(m_dwStencilWriteMask & 0xFF);

  desc.FrontFace.StencilFailOp =
      (D3D11_STENCIL_OP)TranslateStencilOp(m_dwStencilFail);
  desc.FrontFace.StencilDepthFailOp =
      (D3D11_STENCIL_OP)TranslateStencilOp(m_dwStencilZFail);
  desc.FrontFace.StencilPassOp =
      (D3D11_STENCIL_OP)TranslateStencilOp(m_dwStencilPass);
  desc.FrontFace.StencilFunc =
      (D3D11_COMPARISON_FUNC)TranslateCmpFunc(m_dwStencilFunc);

  // DX9 doesn't have separate back face stencil by default — mirror front
  desc.BackFace = desc.FrontFace;

  m_pDevice->CreateDepthStencilState(&desc, &m_pDepthStencilState);
  m_pContext->OMSetDepthStencilState(m_pDepthStencilState, m_dwStencilRef);
}

// ============================================================================
// Blend State
// ============================================================================

void CDX11StateCache::ApplyBlendState() {
  m_bBlendDirty = false;

  if (m_pBlendState) {
    m_pBlendState->Release();
    m_pBlendState = nullptr;
  }

  D3D11_BLEND_DESC desc = {};
  desc.AlphaToCoverageEnable = FALSE;
  desc.IndependentBlendEnable = FALSE;

  D3D11_RENDER_TARGET_BLEND_DESC &rt = desc.RenderTarget[0];
  rt.BlendEnable = m_dwAlphaBlendEnable ? TRUE : FALSE;

  rt.SrcBlend = (D3D11_BLEND)TranslateBlendMode(m_dwSrcBlend);
  rt.DestBlend = (D3D11_BLEND)TranslateBlendMode(m_dwDestBlend);
  rt.BlendOp = (D3D11_BLEND_OP)TranslateBlendOp(m_dwBlendOp);

  if (m_dwSeparateAlphaBlendEnable) {
    rt.SrcBlendAlpha = (D3D11_BLEND)TranslateBlendMode(m_dwSrcBlendAlpha);
    rt.DestBlendAlpha = (D3D11_BLEND)TranslateBlendMode(m_dwDestBlendAlpha);
    rt.BlendOpAlpha = (D3D11_BLEND_OP)TranslateBlendOp(m_dwBlendOpAlpha);
  } else {
    rt.SrcBlendAlpha = rt.SrcBlend;
    rt.DestBlendAlpha = rt.DestBlend;
    rt.BlendOpAlpha = rt.BlendOp;
  }

  rt.RenderTargetWriteMask = 0;
  if (m_dwColorWriteEnable & D3DCOLORWRITEENABLE_RED)
    rt.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;
  if (m_dwColorWriteEnable & D3DCOLORWRITEENABLE_GREEN)
    rt.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
  if (m_dwColorWriteEnable & D3DCOLORWRITEENABLE_BLUE)
    rt.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
  if (m_dwColorWriteEnable & D3DCOLORWRITEENABLE_ALPHA)
    rt.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

  m_pDevice->CreateBlendState(&desc, &m_pBlendState);

  float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  m_pContext->OMSetBlendState(m_pBlendState, blendFactor, 0xFFFFFFFF);
}

// ============================================================================
// Rasterizer State
// ============================================================================

void CDX11StateCache::ApplyRasterizerState() {
  m_bRasterizerDirty = false;

  if (m_pRasterizerState) {
    m_pRasterizerState->Release();
    m_pRasterizerState = nullptr;
  }

  D3D11_RASTERIZER_DESC desc = {};
  desc.FillMode = (D3D11_FILL_MODE)TranslateFillMode(m_dwFillMode);
  desc.CullMode = (D3D11_CULL_MODE)TranslateCullMode(m_dwCullMode);
  desc.FrontCounterClockwise = FALSE; // DX9 default winding
  desc.DepthBias = (INT)(*(float *)&m_dwDepthBias *
                         16777216.0f); // DX9 float bias → DX11 int bias
  desc.SlopeScaledDepthBias = *(float *)&m_dwSlopeScaleDepthBias;
  desc.DepthBiasClamp = 0.0f;
  desc.DepthClipEnable = TRUE;
  desc.ScissorEnable = m_dwScissorTestEnable ? TRUE : FALSE;
  desc.MultisampleEnable = m_dwMultiSampleAntiAlias ? TRUE : FALSE;
  desc.AntialiasedLineEnable = m_dwAntiAliasedLineEnable ? TRUE : FALSE;

  m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState);
  m_pContext->RSSetState(m_pRasterizerState);
}

// ============================================================================
// Sampler State
// ============================================================================

void CDX11StateCache::ApplySamplerState(DWORD stage) {
  m_bSamplerDirty[stage] = false;

  if (m_pSamplerStates[stage]) {
    m_pSamplerStates[stage]->Release();
    m_pSamplerStates[stage] = nullptr;
  }

  const SamplerMirror &sm = m_SamplerMirror[stage];

  D3D11_SAMPLER_DESC desc = {};
  desc.Filter = (D3D11_FILTER)TranslateFilter(sm.MinFilter, sm.MipFilter);
  desc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)TranslateAddressMode(sm.AddressU);
  desc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)TranslateAddressMode(sm.AddressV);
  desc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)TranslateAddressMode(sm.AddressW);
  desc.MipLODBias = *(float *)&sm.MipLODBias;
  desc.MaxAnisotropy = sm.MaxAnisotropy;
  desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  desc.MinLOD = 0;
  desc.MaxLOD = D3D11_FLOAT32_MAX;
  desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] =
      desc.BorderColor[3] = 0.0f;

  // Override filter for anisotropic
  if (sm.MinFilter == D3DTEXF_ANISOTROPIC ||
      sm.MagFilter == D3DTEXF_ANISOTROPIC)
    desc.Filter = D3D11_FILTER_ANISOTROPIC;

  m_pDevice->CreateSamplerState(&desc, &m_pSamplerStates[stage]);
  m_pContext->PSSetSamplers(stage, 1, &m_pSamplerStates[stage]);
}

// ============================================================================
// Translation Helpers: DX9 → DX11 enums
// ============================================================================

int CDX11StateCache::TranslateBlendMode(DWORD d3d9Blend) {
  switch (d3d9Blend) {
  case D3DBLEND_ZERO:
    return D3D11_BLEND_ZERO;
  case D3DBLEND_ONE:
    return D3D11_BLEND_ONE;
  case D3DBLEND_SRCCOLOR:
    return D3D11_BLEND_SRC_COLOR;
  case D3DBLEND_INVSRCCOLOR:
    return D3D11_BLEND_INV_SRC_COLOR;
  case D3DBLEND_SRCALPHA:
    return D3D11_BLEND_SRC_ALPHA;
  case D3DBLEND_INVSRCALPHA:
    return D3D11_BLEND_INV_SRC_ALPHA;
  case D3DBLEND_DESTALPHA:
    return D3D11_BLEND_DEST_ALPHA;
  case D3DBLEND_INVDESTALPHA:
    return D3D11_BLEND_INV_DEST_ALPHA;
  case D3DBLEND_DESTCOLOR:
    return D3D11_BLEND_DEST_COLOR;
  case D3DBLEND_INVDESTCOLOR:
    return D3D11_BLEND_INV_DEST_COLOR;
  case D3DBLEND_SRCALPHASAT:
    return D3D11_BLEND_SRC_ALPHA_SAT;
  case D3DBLEND_BLENDFACTOR:
    return D3D11_BLEND_BLEND_FACTOR;
  case D3DBLEND_INVBLENDFACTOR:
    return D3D11_BLEND_INV_BLEND_FACTOR;
  default:
    return D3D11_BLEND_ONE;
  }
}

int CDX11StateCache::TranslateBlendOp(DWORD d3d9BlendOp) {
  switch (d3d9BlendOp) {
  case D3DBLENDOP_ADD:
    return D3D11_BLEND_OP_ADD;
  case D3DBLENDOP_SUBTRACT:
    return D3D11_BLEND_OP_SUBTRACT;
  case D3DBLENDOP_REVSUBTRACT:
    return D3D11_BLEND_OP_REV_SUBTRACT;
  case D3DBLENDOP_MIN:
    return D3D11_BLEND_OP_MIN;
  case D3DBLENDOP_MAX:
    return D3D11_BLEND_OP_MAX;
  default:
    return D3D11_BLEND_OP_ADD;
  }
}

int CDX11StateCache::TranslateCmpFunc(DWORD d3d9CmpFunc) {
  switch (d3d9CmpFunc) {
  case D3DCMP_NEVER:
    return D3D11_COMPARISON_NEVER;
  case D3DCMP_LESS:
    return D3D11_COMPARISON_LESS;
  case D3DCMP_EQUAL:
    return D3D11_COMPARISON_EQUAL;
  case D3DCMP_LESSEQUAL:
    return D3D11_COMPARISON_LESS_EQUAL;
  case D3DCMP_GREATER:
    return D3D11_COMPARISON_GREATER;
  case D3DCMP_NOTEQUAL:
    return D3D11_COMPARISON_NOT_EQUAL;
  case D3DCMP_GREATEREQUAL:
    return D3D11_COMPARISON_GREATER_EQUAL;
  case D3DCMP_ALWAYS:
    return D3D11_COMPARISON_ALWAYS;
  default:
    return D3D11_COMPARISON_LESS_EQUAL;
  }
}

int CDX11StateCache::TranslateStencilOp(DWORD d3d9StencilOp) {
  switch (d3d9StencilOp) {
  case D3DSTENCILOP_KEEP:
    return D3D11_STENCIL_OP_KEEP;
  case D3DSTENCILOP_ZERO:
    return D3D11_STENCIL_OP_ZERO;
  case D3DSTENCILOP_REPLACE:
    return D3D11_STENCIL_OP_REPLACE;
  case D3DSTENCILOP_INCRSAT:
    return D3D11_STENCIL_OP_INCR_SAT;
  case D3DSTENCILOP_DECRSAT:
    return D3D11_STENCIL_OP_DECR_SAT;
  case D3DSTENCILOP_INVERT:
    return D3D11_STENCIL_OP_INVERT;
  case D3DSTENCILOP_INCR:
    return D3D11_STENCIL_OP_INCR;
  case D3DSTENCILOP_DECR:
    return D3D11_STENCIL_OP_DECR;
  default:
    return D3D11_STENCIL_OP_KEEP;
  }
}

int CDX11StateCache::TranslateFillMode(DWORD d3d9FillMode) {
  switch (d3d9FillMode) {
  case D3DFILL_WIREFRAME:
    return D3D11_FILL_WIREFRAME;
  case D3DFILL_SOLID:
    return D3D11_FILL_SOLID;
  default:
    return D3D11_FILL_SOLID;
  }
}

int CDX11StateCache::TranslateCullMode(DWORD d3d9CullMode) {
  switch (d3d9CullMode) {
  case D3DCULL_NONE:
    return D3D11_CULL_NONE;
  case D3DCULL_CW:
    return D3D11_CULL_FRONT; // DX9 CW = DX11 FRONT (same winding sense)
  case D3DCULL_CCW:
    return D3D11_CULL_BACK; // DX9 CCW = DX11 BACK
  default:
    return D3D11_CULL_BACK;
  }
}

int CDX11StateCache::TranslateFilter(DWORD d3d9MinMagFilter,
                                     DWORD d3d9MipFilter) {
  // DX11 combines min/mag/mip into a single filter enum
  bool bLinearMin = (d3d9MinMagFilter == D3DTEXF_LINEAR ||
                     d3d9MinMagFilter == D3DTEXF_ANISOTROPIC);
  bool bLinearMip = (d3d9MipFilter == D3DTEXF_LINEAR);

  if (d3d9MinMagFilter == D3DTEXF_ANISOTROPIC)
    return D3D11_FILTER_ANISOTROPIC;

  if (bLinearMin && bLinearMip)
    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  if (bLinearMin && !bLinearMip)
    return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
  if (!bLinearMin && bLinearMip)
    return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
  return D3D11_FILTER_MIN_MAG_MIP_POINT;
}

int CDX11StateCache::TranslateAddressMode(DWORD d3d9AddressMode) {
  switch (d3d9AddressMode) {
  case D3DTADDRESS_WRAP:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  case D3DTADDRESS_MIRROR:
    return D3D11_TEXTURE_ADDRESS_MIRROR;
  case D3DTADDRESS_CLAMP:
    return D3D11_TEXTURE_ADDRESS_CLAMP;
  case D3DTADDRESS_BORDER:
    return D3D11_TEXTURE_ADDRESS_BORDER;
  case D3DTADDRESS_MIRRORONCE:
    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
  default:
    return D3D11_TEXTURE_ADDRESS_WRAP;
  }
}
