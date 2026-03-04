#include "DX11ShadowMap.h"
#include "StdAfx.h"

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>

using namespace DirectX;

// ============================================================================
// Depth-only vertex shader for shadow pass
// ============================================================================
static const char *s_szDepthVS_HLSL = R"(
cbuffer CBShadow : register(b0)
{
    float4x4 matLightVP;
};

cbuffer CBWorld : register(b1)
{
    float4x4 matWorld;
};

struct VS_INPUT
{
    float3 Position : POSITION;
};

float4 VS_Depth(VS_INPUT input) : SV_POSITION
{
    float4 worldPos = mul(float4(input.Position, 1.0f), matWorld);
    return mul(worldPos, matLightVP);
}
)";

// ============================================================================
// CDX11ShadowMap Implementation
// ============================================================================

CDX11ShadowMap::CDX11ShadowMap()
    : m_pDevice(nullptr), m_pContext(nullptr), m_bInitialized(false),
      m_iSize(2048), m_pShadowTex(nullptr), m_pShadowDSV(nullptr),
      m_pShadowSRV(nullptr), m_pShadowSampler(nullptr),
      m_pShadowRasterizer(nullptr), m_pDepthVS(nullptr),
      m_pDepthLayout(nullptr), m_pShadowCB(nullptr), m_pSavedRTV(nullptr),
      m_pSavedDSV(nullptr) {
  memset(m_matLightVP, 0, sizeof(m_matLightVP));
}

CDX11ShadowMap::~CDX11ShadowMap() { Shutdown(); }

bool CDX11ShadowMap::Initialize(ID3D11Device *pDevice,
                                ID3D11DeviceContext *pContext, int iSize) {
  m_pDevice = pDevice;
  m_pContext = pContext;
  m_iSize = iSize;

  // 1) Create shadow depth texture (R32_TYPELESS for dual DSV/SRV binding)
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = iSize;
  texDesc.Height = iSize;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
  texDesc.SampleDesc.Count = 1;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

  HRESULT hr = m_pDevice->CreateTexture2D(&texDesc, nullptr, &m_pShadowTex);
  if (FAILED(hr))
    return false;

  // 2) DSV (D32_FLOAT format)
  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
  dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  hr = m_pDevice->CreateDepthStencilView(m_pShadowTex, &dsvDesc, &m_pShadowDSV);
  if (FAILED(hr))
    return false;

  // 3) SRV (R32_FLOAT format for sampling)
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  hr = m_pDevice->CreateShaderResourceView(m_pShadowTex, &srvDesc,
                                           &m_pShadowSRV);
  if (FAILED(hr))
    return false;

  // 4) Shadow comparison sampler (PCF hardware filtering)
  D3D11_SAMPLER_DESC sampDesc = {};
  sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
  sampDesc.BorderColor[0] = 1.0f; // Outside shadow = lit
  sampDesc.BorderColor[1] = 1.0f;
  sampDesc.BorderColor[2] = 1.0f;
  sampDesc.BorderColor[3] = 1.0f;
  hr = m_pDevice->CreateSamplerState(&sampDesc, &m_pShadowSampler);
  if (FAILED(hr))
    return false;

  // 5) Depth-bias rasterizer (reduce shadow acne)
  D3D11_RASTERIZER_DESC rsDesc = {};
  rsDesc.FillMode = D3D11_FILL_SOLID;
  rsDesc.CullMode = D3D11_CULL_BACK;
  rsDesc.DepthBias = 1000;
  rsDesc.DepthBiasClamp = 0.0f;
  rsDesc.SlopeScaledDepthBias = 2.0f;
  rsDesc.DepthClipEnable = TRUE;
  hr = m_pDevice->CreateRasterizerState(&rsDesc, &m_pShadowRasterizer);
  if (FAILED(hr))
    return false;

  // 6) Compile depth-only vertex shader
  ID3D10Blob *pVSBlob = nullptr;
  ID3D10Blob *pErrors = nullptr;
  hr = D3DCompile(s_szDepthVS_HLSL, strlen(s_szDepthVS_HLSL), "ShadowDepthVS",
                  nullptr, nullptr, "VS_Depth", "vs_5_0", 0, 0, &pVSBlob,
                  &pErrors);
  if (FAILED(hr)) {
    if (pErrors) {
      OutputDebugStringA((const char *)pErrors->GetBufferPointer());
      pErrors->Release();
    }
    return false;
  }

  hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
                                     pVSBlob->GetBufferSize(), nullptr,
                                     &m_pDepthVS);
  if (FAILED(hr)) {
    pVSBlob->Release();
    return false;
  }

  // Input layout: position only
  D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  hr = m_pDevice->CreateInputLayout(layoutDesc, 1, pVSBlob->GetBufferPointer(),
                                    pVSBlob->GetBufferSize(), &m_pDepthLayout);
  pVSBlob->Release();
  if (FAILED(hr))
    return false;

  // 7) Shadow constant buffer (light VP matrix)
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = sizeof(XMFLOAT4X4) * 2; // lightVP + world
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pShadowCB);
  if (FAILED(hr))
    return false;

  m_bInitialized = true;
  OutputDebugStringA("[DX11] Shadow map initialized (2048x2048)\n");
  return true;
}

void CDX11ShadowMap::Shutdown() {
  if (m_pShadowCB) {
    m_pShadowCB->Release();
    m_pShadowCB = nullptr;
  }
  if (m_pDepthLayout) {
    m_pDepthLayout->Release();
    m_pDepthLayout = nullptr;
  }
  if (m_pDepthVS) {
    m_pDepthVS->Release();
    m_pDepthVS = nullptr;
  }
  if (m_pShadowRasterizer) {
    m_pShadowRasterizer->Release();
    m_pShadowRasterizer = nullptr;
  }
  if (m_pShadowSampler) {
    m_pShadowSampler->Release();
    m_pShadowSampler = nullptr;
  }
  if (m_pShadowSRV) {
    m_pShadowSRV->Release();
    m_pShadowSRV = nullptr;
  }
  if (m_pShadowDSV) {
    m_pShadowDSV->Release();
    m_pShadowDSV = nullptr;
  }
  if (m_pShadowTex) {
    m_pShadowTex->Release();
    m_pShadowTex = nullptr;
  }
  m_bInitialized = false;
}

void CDX11ShadowMap::UpdateLightMatrix(const float *pCameraPos,
                                       float fSceneRadius) {
  // Sun direction matches the FFP shader constant
  XMVECTOR sunDir = XMVector3Normalize(XMVectorSet(-0.5f, -0.8f, 0.3f, 0.0f));
  XMVECTOR camPos =
      XMVectorSet(pCameraPos[0], pCameraPos[1], pCameraPos[2], 1.0f);

  // Light position: offset from camera along inverse sun direction
  XMVECTOR lightPos =
      XMVectorSubtract(camPos, XMVectorScale(sunDir, fSceneRadius));

  // Light view matrix
  XMMATRIX lightView =
      XMMatrixLookAtLH(lightPos, camPos, XMVectorSet(0, 1, 0, 0));

  // Orthographic projection covering scene radius
  float halfSize = fSceneRadius * 1.2f;
  XMMATRIX lightProj = XMMatrixOrthographicLH(halfSize * 2.0f, halfSize * 2.0f,
                                              0.1f, fSceneRadius * 3.0f);

  XMMATRIX lightVP = XMMatrixMultiply(lightView, lightProj);

  // Store row-major for HLSL (transpose from column-major)
  XMMATRIX lightVPT = XMMatrixTranspose(lightVP);
  XMStoreFloat4x4((XMFLOAT4X4 *)m_matLightVP, lightVPT);

  // Update constant buffer with light VP
  if (m_pShadowCB) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(m_pContext->Map(m_pShadowCB, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                  &mapped))) {
      memcpy(mapped.pData, m_matLightVP, sizeof(m_matLightVP));
      m_pContext->Unmap(m_pShadowCB, 0);
    }
  }
}

void CDX11ShadowMap::BeginShadowPass() {
  if (!m_bInitialized)
    return;

  // Save current render targets
  m_pSavedRTV = nullptr;
  m_pSavedDSV = nullptr;
  m_pContext->OMGetRenderTargets(1, &m_pSavedRTV, &m_pSavedDSV);

  // Set shadow depth as render target (no color RT)
  ID3D11RenderTargetView *nullRTV = nullptr;
  m_pContext->OMSetRenderTargets(1, &nullRTV, m_pShadowDSV);

  // Clear shadow depth
  m_pContext->ClearDepthStencilView(m_pShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

  // Set shadow viewport
  D3D11_VIEWPORT vp = {};
  vp.Width = (float)m_iSize;
  vp.Height = (float)m_iSize;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  m_pContext->RSSetViewports(1, &vp);

  // Set depth bias rasterizer to reduce shadow acne
  m_pContext->RSSetState(m_pShadowRasterizer);

  // Bind depth-only shader
  m_pContext->VSSetShader(m_pDepthVS, nullptr, 0);
  m_pContext->PSSetShader(nullptr, nullptr, 0); // No pixel shader
  m_pContext->IASetInputLayout(m_pDepthLayout);

  // Bind shadow CB (light VP) at slot b0
  m_pContext->VSSetConstantBuffers(0, 1, &m_pShadowCB);
}

void CDX11ShadowMap::EndShadowPass() {
  if (!m_bInitialized)
    return;

  // Restore previous render targets
  m_pContext->OMSetRenderTargets(1, &m_pSavedRTV, m_pSavedDSV);
  if (m_pSavedRTV) {
    m_pSavedRTV->Release();
    m_pSavedRTV = nullptr;
  }
  if (m_pSavedDSV) {
    m_pSavedDSV->Release();
    m_pSavedDSV = nullptr;
  }

  // Reset rasterizer state
  m_pContext->RSSetState(nullptr);
}
