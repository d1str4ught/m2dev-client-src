#include "DX11PostProcess.h"
#include "StdAfx.h"

#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>

// ============================================================================
// Embedded HLSL: Fullscreen triangle + bloom shaders
// ============================================================================

static const char *s_szPostProcessHLSL = R"(
// ---- Constant Buffer ----
cbuffer CBPostProcess : register(b0)
{
    float fBloomThreshold;
    float fBloomIntensity;
    float2 vTexelSize;
};

Texture2D    texScene    : register(t0);
Texture2D    texBloom    : register(t1);
SamplerState samLinear   : register(s0);

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VS_OUTPUT VS_Fullscreen(uint vertexID : SV_VertexID)
{
    VS_OUTPUT output;
    output.TexCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(output.TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return output;
}

// ---- Bright-pass extract (soft knee) ----
float4 PS_BloomExtract(VS_OUTPUT input) : SV_TARGET
{
    float4 color = texScene.Sample(samLinear, input.TexCoord);
    float brightness = dot(color.rgb, float3(0.2126f, 0.7152f, 0.0722f));
    float knee = fBloomThreshold * 0.5f;
    float soft = brightness - fBloomThreshold + knee;
    soft = clamp(soft, 0.0f, 2.0f * knee);
    soft = soft * soft / (4.0f * knee + 0.00001f);
    float contribution = max(soft, brightness - fBloomThreshold);
    contribution = max(0.0f, contribution);
    return float4(color.rgb * (contribution / (brightness + 0.00001f)), 1.0f);
}

// ---- Gaussian blur (9-tap cross pattern) ----
float4 PS_BloomBlur(VS_OUTPUT input) : SV_TARGET
{
    static const float w0 = 0.227027f;
    static const float w1 = 0.1945946f;
    static const float w2 = 0.1216216f;
    static const float w3 = 0.054054f;
    static const float w4 = 0.016216f;

    float3 result = texScene.Sample(samLinear, input.TexCoord).rgb * w0;
    float2 t = vTexelSize * 2.0f;
    result += texScene.Sample(samLinear, input.TexCoord + float2(t.x, 0)).rgb * w1;
    result += texScene.Sample(samLinear, input.TexCoord - float2(t.x, 0)).rgb * w1;
    result += texScene.Sample(samLinear, input.TexCoord + float2(0, t.y)).rgb * w1;
    result += texScene.Sample(samLinear, input.TexCoord - float2(0, t.y)).rgb * w1;
    t *= 2.0f;
    result += texScene.Sample(samLinear, input.TexCoord + float2(t.x, 0)).rgb * w2;
    result += texScene.Sample(samLinear, input.TexCoord - float2(t.x, 0)).rgb * w2;
    result += texScene.Sample(samLinear, input.TexCoord + float2(0, t.y)).rgb * w2;
    result += texScene.Sample(samLinear, input.TexCoord - float2(0, t.y)).rgb * w2;
    t *= 1.5f;
    result += texScene.Sample(samLinear, input.TexCoord + float2(t.x, 0)).rgb * w3;
    result += texScene.Sample(samLinear, input.TexCoord - float2(t.x, 0)).rgb * w3;
    result += texScene.Sample(samLinear, input.TexCoord + float2(0, t.y)).rgb * w3;
    result += texScene.Sample(samLinear, input.TexCoord - float2(0, t.y)).rgb * w3;
    t *= 1.5f;
    result += texScene.Sample(samLinear, input.TexCoord + float2(t.x, 0)).rgb * w4;
    result += texScene.Sample(samLinear, input.TexCoord - float2(t.x, 0)).rgb * w4;
    result += texScene.Sample(samLinear, input.TexCoord + float2(0, t.y)).rgb * w4;
    result += texScene.Sample(samLinear, input.TexCoord - float2(0, t.y)).rgb * w4;
    return float4(result * 0.5f, 1.0f);
}

// ---- ACES Filmic Tonemapping ----
float3 ACESFilm(float3 x)
{
    float a = 2.51f; float b = 0.03f;
    float c = 2.43f; float d = 0.59f; float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// ---- Composite: ACES tonemap + bloom + vignette + color grading ----
float4 PS_Composite(VS_OUTPUT input) : SV_TARGET
{
    float3 scene = texScene.Sample(samLinear, input.TexCoord).rgb;
    float3 bloom = texBloom.Sample(samLinear, input.TexCoord).rgb;

    // Additive bloom
    float3 result = scene + bloom * fBloomIntensity;

    // ACES filmic tonemapping
    result = ACESFilm(result);

    // Contrast boost (smoothstep S-curve)
    result = result * result * (3.0f - 2.0f * result);

    // Color grading: warm shadows, cool highlights
    float lum = dot(result, float3(0.2126f, 0.7152f, 0.0722f));
    float3 warmTint = float3(1.03f, 0.98f, 0.93f);
    float3 coolTint = float3(0.96f, 0.99f, 1.04f);
    result *= lerp(warmTint, coolTint, saturate(lum * 1.5f));

    // Vignette
    float2 uv = input.TexCoord;
    float vig = uv.x * uv.y * (1.0f - uv.x) * (1.0f - uv.y);
    vig = saturate(pow(vig * 16.0f, 0.25f));
    result *= vig;

    return float4(saturate(result), 1.0f);
}
)";

// ============================================================================
// Post-process constant buffer structure (must match HLSL)
// ============================================================================
struct CBPostProcess {
  float fBloomThreshold;
  float fBloomIntensity;
  float vTexelSize[2]; // 1/width, 1/height
};

// ============================================================================
// Helper: Compile shader from string
// ============================================================================
static bool CompileShaderFromString(const char *szSource, const char *szEntry,
                                    const char *szProfile,
                                    ID3D10Blob **ppBlob) {
  ID3D10Blob *pErr = nullptr;
  DWORD flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
  HRESULT hr = D3DCompile(szSource, strlen(szSource), nullptr, nullptr, nullptr,
                          szEntry, szProfile, flags, 0, ppBlob, &pErr);
  if (FAILED(hr)) {
    if (pErr) {
      OutputDebugStringA((const char *)pErr->GetBufferPointer());
      pErr->Release();
    }
    return false;
  }
  if (pErr)
    pErr->Release();
  return true;
}

// ============================================================================
// CDX11PostProcess Implementation
// ============================================================================

CDX11PostProcess::CDX11PostProcess()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pSwapChain(nullptr),
      m_bInitialized(false), m_iWidth(0), m_iHeight(0), m_bBloomEnabled(true),
      m_fBloomIntensity(0.5f), m_fBloomThreshold(0.7f),
      m_pFullscreenVS(nullptr), m_pBloomExtractPS(nullptr),
      m_pBloomBlurPS(nullptr), m_pCompositePS(nullptr), m_pBloomRT_Tex(nullptr),
      m_pBloomRT_RTV(nullptr), m_pBloomRT_SRV(nullptr),
      m_pBloomBlurRT_Tex(nullptr), m_pBloomBlurRT_RTV(nullptr),
      m_pBloomBlurRT_SRV(nullptr), m_pCBPostProcess(nullptr),
      m_pSamplerLinear(nullptr) {}

CDX11PostProcess::~CDX11PostProcess() { Shutdown(); }

bool CDX11PostProcess::Initialize(ID3D11Device *pDevice,
                                  ID3D11DeviceContext *pContext,
                                  IDXGISwapChain *pSwapChain, int width,
                                  int height) {
  m_pDevice = pDevice;
  m_pContext = pContext;
  m_pSwapChain = pSwapChain;
  m_iWidth = width;
  m_iHeight = height;

  if (!CreateShaders()) {
    OutputDebugStringA("[DX11 PostProcess] CreateShaders FAILED\n");
    return false;
  }

  if (!CreateResources(width, height)) {
    OutputDebugStringA("[DX11 PostProcess] CreateResources FAILED\n");
    return false;
  }

  m_bInitialized = true;
  OutputDebugStringA("[DX11 PostProcess] Initialized OK — bloom enabled\n");
  return true;
}

void CDX11PostProcess::Shutdown() {
  ReleaseResources();

  if (m_pFullscreenVS) {
    m_pFullscreenVS->Release();
    m_pFullscreenVS = nullptr;
  }
  if (m_pBloomExtractPS) {
    m_pBloomExtractPS->Release();
    m_pBloomExtractPS = nullptr;
  }
  if (m_pBloomBlurPS) {
    m_pBloomBlurPS->Release();
    m_pBloomBlurPS = nullptr;
  }
  if (m_pCompositePS) {
    m_pCompositePS->Release();
    m_pCompositePS = nullptr;
  }
  if (m_pCBPostProcess) {
    m_pCBPostProcess->Release();
    m_pCBPostProcess = nullptr;
  }
  if (m_pSamplerLinear) {
    m_pSamplerLinear->Release();
    m_pSamplerLinear = nullptr;
  }

  m_bInitialized = false;
}

bool CDX11PostProcess::CreateShaders() {
  // Fullscreen vertex shader (no vertex buffer — uses SV_VertexID)
  ID3D10Blob *pBlob = nullptr;
  if (!CompileShaderFromString(s_szPostProcessHLSL, "VS_Fullscreen", "vs_5_0",
                               &pBlob))
    return false;
  m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
                                pBlob->GetBufferSize(), nullptr,
                                &m_pFullscreenVS);
  pBlob->Release();

  // Bloom extract pixel shader
  if (!CompileShaderFromString(s_szPostProcessHLSL, "PS_BloomExtract", "ps_5_0",
                               &pBlob))
    return false;
  m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(),
                               pBlob->GetBufferSize(), nullptr,
                               &m_pBloomExtractPS);
  pBlob->Release();

  // Bloom blur pixel shader
  if (!CompileShaderFromString(s_szPostProcessHLSL, "PS_BloomBlur", "ps_5_0",
                               &pBlob))
    return false;
  m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(),
                               pBlob->GetBufferSize(), nullptr,
                               &m_pBloomBlurPS);
  pBlob->Release();

  // Composite pixel shader
  if (!CompileShaderFromString(s_szPostProcessHLSL, "PS_Composite", "ps_5_0",
                               &pBlob))
    return false;
  m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(),
                               pBlob->GetBufferSize(), nullptr,
                               &m_pCompositePS);
  pBlob->Release();

  // Constant buffer
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = sizeof(CBPostProcess);
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pCBPostProcess);

  // Linear sampler with clamp
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  m_pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

  return true;
}

bool CDX11PostProcess::CreateResources(int width, int height) {
  // Bloom render targets at half resolution for performance
  int bloomW = width / 2;
  int bloomH = height / 2;

  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = bloomW;
  texDesc.Height = bloomH;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  texDesc.SampleDesc.Count = 1;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

  // Bloom extract RT
  HRESULT hr = m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pBloomRT_Tex);
  if (FAILED(hr))
    return false;
  m_pDevice->CreateRenderTargetView(m_pBloomRT_Tex, nullptr, &m_pBloomRT_RTV);
  m_pDevice->CreateShaderResourceView(m_pBloomRT_Tex, nullptr, &m_pBloomRT_SRV);

  // Bloom blur RT
  hr = m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pBloomBlurRT_Tex);
  if (FAILED(hr))
    return false;
  m_pDevice->CreateRenderTargetView(m_pBloomBlurRT_Tex, nullptr,
                                    &m_pBloomBlurRT_RTV);
  m_pDevice->CreateShaderResourceView(m_pBloomBlurRT_Tex, nullptr,
                                      &m_pBloomBlurRT_SRV);

  return true;
}

void CDX11PostProcess::ReleaseResources() {
  if (m_pBloomRT_SRV) {
    m_pBloomRT_SRV->Release();
    m_pBloomRT_SRV = nullptr;
  }
  if (m_pBloomRT_RTV) {
    m_pBloomRT_RTV->Release();
    m_pBloomRT_RTV = nullptr;
  }
  if (m_pBloomRT_Tex) {
    m_pBloomRT_Tex->Release();
    m_pBloomRT_Tex = nullptr;
  }
  if (m_pBloomBlurRT_SRV) {
    m_pBloomBlurRT_SRV->Release();
    m_pBloomBlurRT_SRV = nullptr;
  }
  if (m_pBloomBlurRT_RTV) {
    m_pBloomBlurRT_RTV->Release();
    m_pBloomBlurRT_RTV = nullptr;
  }
  if (m_pBloomBlurRT_Tex) {
    m_pBloomBlurRT_Tex->Release();
    m_pBloomBlurRT_Tex = nullptr;
  }
}

void CDX11PostProcess::OnResize(int width, int height) {
  if (width == m_iWidth && height == m_iHeight)
    return;
  m_iWidth = width;
  m_iHeight = height;
  ReleaseResources();
  CreateResources(width, height);
}

void CDX11PostProcess::DrawFullscreenQuad() {
  // Draw 3 vertices — the fullscreen VS generates a triangle from SV_VertexID
  m_pContext->IASetInputLayout(nullptr);
  m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  m_pContext->Draw(3, 0);
}

void CDX11PostProcess::ApplyAndPresent(ID3D11ShaderResourceView *pSceneSRV) {
  if (!m_bInitialized || !pSceneSRV)
    return;

  // Update constant buffer
  CBPostProcess cb;
  cb.fBloomThreshold = m_fBloomThreshold;
  cb.fBloomIntensity = m_fBloomIntensity;
  cb.vTexelSize[0] = 2.0f / m_iWidth; // bloom is at half res
  cb.vTexelSize[1] = 2.0f / m_iHeight;

  D3D11_MAPPED_SUBRESOURCE mapped;
  m_pContext->Map(m_pCBPostProcess, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  memcpy(mapped.pData, &cb, sizeof(cb));
  m_pContext->Unmap(m_pCBPostProcess, 0);

  // Bind shared state
  m_pContext->VSSetShader(m_pFullscreenVS, nullptr, 0);
  m_pContext->PSSetConstantBuffers(0, 1, &m_pCBPostProcess);
  m_pContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

  if (m_bBloomEnabled) {
    int bloomW = m_iWidth / 2;
    int bloomH = m_iHeight / 2;

    D3D11_VIEWPORT bloomVP = {0, 0, (float)bloomW, (float)bloomH, 0.0f, 1.0f};

    // Pass 1: Bloom Extract — bright pixels from scene
    m_pContext->OMSetRenderTargets(1, &m_pBloomRT_RTV, nullptr);
    m_pContext->RSSetViewports(1, &bloomVP);
    m_pContext->PSSetShader(m_pBloomExtractPS, nullptr, 0);
    m_pContext->PSSetShaderResources(0, 1, &pSceneSRV);
    DrawFullscreenQuad();

    // Unbind scene SRV from slot 0 before using bloom RT as input
    ID3D11ShaderResourceView *nullSRV = nullptr;
    m_pContext->PSSetShaderResources(0, 1, &nullSRV);

    // Pass 2: Blur the extracted bloom
    m_pContext->OMSetRenderTargets(1, &m_pBloomBlurRT_RTV, nullptr);
    m_pContext->PSSetShader(m_pBloomBlurPS, nullptr, 0);
    m_pContext->PSSetShaderResources(0, 1, &m_pBloomRT_SRV);
    DrawFullscreenQuad();

    // Unbind
    m_pContext->PSSetShaderResources(0, 1, &nullSRV);
  }

  // Pass 3: Composite — render to swap chain back buffer
  // Get the swap chain's back buffer RTV
  ID3D11Texture2D *pBackBuffer = nullptr;
  m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&pBackBuffer);
  ID3D11RenderTargetView *pBackBufferRTV = nullptr;
  m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRTV);
  pBackBuffer->Release();

  D3D11_VIEWPORT fullVP = {0, 0, (float)m_iWidth, (float)m_iHeight, 0.0f, 1.0f};
  m_pContext->OMSetRenderTargets(1, &pBackBufferRTV, nullptr);
  m_pContext->RSSetViewports(1, &fullVP);

  if (m_bBloomEnabled) {
    // Composite scene + bloom
    m_pContext->PSSetShader(m_pCompositePS, nullptr, 0);
    ID3D11ShaderResourceView *srvs[2] = {pSceneSRV, m_pBloomBlurRT_SRV};
    m_pContext->PSSetShaderResources(0, 2, srvs);
  } else {
    // Just tone-map the scene (composite with zero bloom)
    m_pContext->PSSetShader(m_pCompositePS, nullptr, 0);
    ID3D11ShaderResourceView *srvs[2] = {pSceneSRV, nullptr};
    m_pContext->PSSetShaderResources(0, 2, srvs);
  }
  DrawFullscreenQuad();

  // Cleanup
  ID3D11ShaderResourceView *nullSRVs[2] = {nullptr, nullptr};
  m_pContext->PSSetShaderResources(0, 2, nullSRVs);
  pBackBufferRTV->Release();

  // Present via DX11 swap chain
  m_pSwapChain->Present(0, 0);
}
