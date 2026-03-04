#pragma once

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11VertexShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;

// ============================================================================
// CDX11ShadowMap
//
// Manages a 2048×2048 shadow depth map for directional sunlight.
// Provides Begin/End shadow pass to render scene depth from light's POV.
// ============================================================================

class CDX11ShadowMap {
public:
  CDX11ShadowMap();
  ~CDX11ShadowMap();

  bool Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext,
                  int iSize = 2048);
  void Shutdown();
  bool IsInitialized() const { return m_bInitialized; }

  // Call before rendering the scene from sunlight perspective
  void BeginShadowPass();

  // Call after shadow pass — restores previous render targets
  void EndShadowPass();

  // Get the shadow map SRV for sampling in main pass
  ID3D11ShaderResourceView *GetShadowSRV() const { return m_pShadowSRV; }

  // Get the shadow comparison sampler
  ID3D11SamplerState *GetShadowSampler() const { return m_pShadowSampler; }

  // Update the light-space VP matrix (call each frame)
  void UpdateLightMatrix(const float *pCameraPos, float fSceneRadius);

  // Get light VP matrix (for uploading to shader constant buffer)
  const float *GetLightVPMatrix() const { return m_matLightVP; }

  // Get the shadow constant buffer
  ID3D11Buffer *GetShadowCB() const { return m_pShadowCB; }

  // Get shadow depth-only vertex shader + layout
  ID3D11VertexShader *GetDepthVS() const { return m_pDepthVS; }
  ID3D11InputLayout *GetDepthLayout() const { return m_pDepthLayout; }

private:
  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;
  bool m_bInitialized;
  int m_iSize;

  // Shadow depth texture + views
  ID3D11Texture2D *m_pShadowTex;
  ID3D11DepthStencilView *m_pShadowDSV;
  ID3D11ShaderResourceView *m_pShadowSRV;

  // Shadow comparison sampler
  ID3D11SamplerState *m_pShadowSampler;

  // Depth bias rasterizer state
  ID3D11RasterizerState *m_pShadowRasterizer;

  // Depth-only shader for shadow pass
  ID3D11VertexShader *m_pDepthVS;
  ID3D11InputLayout *m_pDepthLayout;

  // Shadow constant buffer (light VP matrix)
  ID3D11Buffer *m_pShadowCB;
  float m_matLightVP[16];

  // Saved state for restoring after shadow pass
  ID3D11RenderTargetView *m_pSavedRTV;
  ID3D11DepthStencilView *m_pSavedDSV;
};
