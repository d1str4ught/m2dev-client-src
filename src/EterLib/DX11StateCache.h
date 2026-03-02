#pragma once

// Forward declarations (full headers via StdAfx.h)
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11SamplerState;

#include <d3d9.h> // D3DRENDERSTATETYPE, D3DSAMPLERSTATETYPE, DWORD

// ============================================================================
// CDX11StateCache
//
// Translates DX9 render state calls into immutable DX11 state objects.
// The DX9 API continues to work — this builds equivalent DX11 state objects
// from the DX9 state cache and applies them lazily before draw calls.
//
// DX9 → DX11 mapping:
//   D3DRS_ZENABLE, ZWRITEENABLE, ZFUNC, STENCIL* → ID3D11DepthStencilState
//   D3DRS_ALPHABLENDENABLE, SRCBLEND, DESTBLEND  → ID3D11BlendState
//   D3DRS_FILLMODE, CULLMODE, SCISSORTESTENABLE   → ID3D11RasterizerState
//   D3DSAMP_MINFILTER, MAGFILTER, ADDRESSU/V/W   → ID3D11SamplerState
// ============================================================================

static const int DX11_MAX_SAMPLER_STAGES = 8;

class CDX11StateCache {
public:
  CDX11StateCache();
  ~CDX11StateCache();

  void Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext);
  void Shutdown();

  // ------ State change notifications (called from CStateManager) ------

  // Called when SetRenderState changes a DX9 render state
  void OnRenderStateChanged(D3DRENDERSTATETYPE type, DWORD value);

  // Called when SetSamplerState changes a DX9 sampler state
  void OnSamplerStateChanged(DWORD stage, D3DSAMPLERSTATETYPE type,
                             DWORD value);

  // ------ Apply dirty state before draw calls ------
  void ApplyState();

private:
  // Rebuild and apply individual state objects
  void ApplyDepthStencilState();
  void ApplyBlendState();
  void ApplyRasterizerState();
  void ApplySamplerState(DWORD stage);

  // Helper: convert DX9 blend mode to DX11
  static int TranslateBlendMode(DWORD d3d9Blend);
  static int TranslateBlendOp(DWORD d3d9BlendOp);
  static int TranslateCmpFunc(DWORD d3d9CmpFunc);
  static int TranslateStencilOp(DWORD d3d9StencilOp);
  static int TranslateFillMode(DWORD d3d9FillMode);
  static int TranslateCullMode(DWORD d3d9CullMode);
  static int TranslateFilter(DWORD d3d9MinMagFilter, DWORD d3d9MipFilter);
  static int TranslateAddressMode(DWORD d3d9AddressMode);

private:
  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;

  // ------ Cached DX11 state objects (released on change or shutdown) ------
  ID3D11DepthStencilState *m_pDepthStencilState;
  ID3D11BlendState *m_pBlendState;
  ID3D11RasterizerState *m_pRasterizerState;
  ID3D11SamplerState *m_pSamplerStates[DX11_MAX_SAMPLER_STAGES];

  // ------ Dirty flags ------
  bool m_bDepthStencilDirty;
  bool m_bBlendDirty;
  bool m_bRasterizerDirty;
  bool m_bSamplerDirty[DX11_MAX_SAMPLER_STAGES];

  // ------ DX9 state mirror (tracks current DX9 values for translation) ------

  // Depth/Stencil
  DWORD m_dwZEnable;
  DWORD m_dwZWriteEnable;
  DWORD m_dwZFunc;
  DWORD m_dwStencilEnable;
  DWORD m_dwStencilFunc;
  DWORD m_dwStencilRef;
  DWORD m_dwStencilMask;
  DWORD m_dwStencilWriteMask;
  DWORD m_dwStencilFail;
  DWORD m_dwStencilZFail;
  DWORD m_dwStencilPass;

  // Blend
  DWORD m_dwAlphaBlendEnable;
  DWORD m_dwSrcBlend;
  DWORD m_dwDestBlend;
  DWORD m_dwBlendOp;
  DWORD m_dwSeparateAlphaBlendEnable;
  DWORD m_dwSrcBlendAlpha;
  DWORD m_dwDestBlendAlpha;
  DWORD m_dwBlendOpAlpha;
  DWORD m_dwAlphaTestEnable; // emulated via discard in pixel shader
  DWORD m_dwAlphaRef;
  DWORD m_dwAlphaFunc;
  DWORD m_dwColorWriteEnable;

  // Rasterizer
  DWORD m_dwFillMode;
  DWORD m_dwCullMode;
  DWORD m_dwScissorTestEnable;
  DWORD m_dwDepthBias;
  DWORD m_dwSlopeScaleDepthBias;
  DWORD m_dwMultiSampleAntiAlias;
  DWORD m_dwAntiAliasedLineEnable;

  // Sampler (per stage)
  struct SamplerMirror {
    DWORD MinFilter;
    DWORD MagFilter;
    DWORD MipFilter;
    DWORD AddressU;
    DWORD AddressV;
    DWORD AddressW;
    DWORD MaxAnisotropy;
    DWORD MipLODBias;
  };
  SamplerMirror m_SamplerMirror[DX11_MAX_SAMPLER_STAGES];
};
