#pragma once

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11Texture2D;
struct IDXGISwapChain;

// ============================================================================
// CDX11PostProcess
//
// Post-processing pipeline that takes the DX9 rendered frame (via shared
// surface) and applies DX11 effects before presenting through the DX11
// swap chain. Effects include:
//   - Bloom (bright-pass + gaussian blur + composite)
//   - Tone mapping (simple Reinhard)
//   - FXAA (future)
// ============================================================================

class CDX11PostProcess {
public:
  CDX11PostProcess();
  ~CDX11PostProcess();

  bool Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext,
                  IDXGISwapChain *pSwapChain, int width, int height);
  void Shutdown();

  // Resize intermediate buffers when window size changes
  void OnResize(int width, int height);

  // Apply post-processing and present to DX11 swap chain
  // Called after DX9 has rendered the frame
  void ApplyAndPresent(ID3D11ShaderResourceView *pSceneSRV);

  // Toggle effects on/off
  void SetBloomEnabled(bool bEnable) { m_bBloomEnabled = bEnable; }
  void SetBloomIntensity(float fIntensity) { m_fBloomIntensity = fIntensity; }
  void SetBloomThreshold(float fThreshold) { m_fBloomThreshold = fThreshold; }

  bool IsInitialized() const { return m_bInitialized; }

private:
  bool CreateShaders();
  bool CreateResources(int width, int height);
  void ReleaseResources();
  void DrawFullscreenQuad();

  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;
  IDXGISwapChain *m_pSwapChain;

  bool m_bInitialized;
  int m_iWidth;
  int m_iHeight;

  // Effect flags
  bool m_bBloomEnabled;
  float m_fBloomIntensity;
  float m_fBloomThreshold;

  // Shaders
  ID3D11VertexShader *m_pFullscreenVS;
  ID3D11PixelShader *m_pBloomExtractPS;
  ID3D11PixelShader *m_pBloomBlurPS;
  ID3D11PixelShader *m_pCompositePS;

  // Intermediate render targets for bloom
  ID3D11Texture2D *m_pBloomRT_Tex;
  ID3D11RenderTargetView *m_pBloomRT_RTV;
  ID3D11ShaderResourceView *m_pBloomRT_SRV;

  ID3D11Texture2D *m_pBloomBlurRT_Tex;
  ID3D11RenderTargetView *m_pBloomBlurRT_RTV;
  ID3D11ShaderResourceView *m_pBloomBlurRT_SRV;

  // Constant buffer for post-process params
  ID3D11Buffer *m_pCBPostProcess;
  ID3D11SamplerState *m_pSamplerLinear;
};
