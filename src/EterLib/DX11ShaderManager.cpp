#include "DX11ShaderManager.h"
#include "StdAfx.h"


#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>


// ============================================================================
// Embedded HLSL: FFP emulation for Position + Diffuse + TexCoord vertices
// This replicates the DX9 fixed-function pipeline for the most common
// vertex format: D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
// ============================================================================

static const char *s_szFFP_HLSL = R"(
// ---- Constant Buffers ----
cbuffer CBPerFrame : register(b0)
{
    float4x4 matWorldViewProj;
    float4x4 matWorld;
};

cbuffer CBPerMaterial : register(b1)
{
    float fAlphaRef;
    float fAlphaTestEnable;
    float fFogEnable;
    float fPad0;
    float4 vFogColor;
    float fFogStart;
    float fFogEnd;
    float fPad1;
    float fPad2;
};

// ---- Textures and Samplers ----
Texture2D    texDiffuse : register(t0);
SamplerState samLinear  : register(s0);

// ---- Vertex Shader ----
struct VS_INPUT
{
    float3 Position : POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float  FogFactor : TEXCOORD1;
};

VS_OUTPUT VS_FFP(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);
    output.Color    = input.Color;
    output.TexCoord = input.TexCoord;

    // Linear fog based on view-space Z
    float4 worldPos = mul(float4(input.Position, 1.0f), matWorld);
    float dist = length(worldPos.xyz);
    output.FogFactor = saturate((fFogEnd - dist) / (fFogEnd - fFogStart + 0.0001f));

    return output;
}

// ---- Pixel Shader ----
float4 PS_FFP(VS_OUTPUT input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(samLinear, input.TexCoord);
    float4 finalColor = texColor * input.Color;

    // Alpha test (discard if below reference)
    if (fAlphaTestEnable > 0.5f)
    {
        if (finalColor.a < fAlphaRef)
            discard;
    }

    // Fog
    if (fFogEnable > 0.5f)
    {
        finalColor.rgb = lerp(vFogColor.rgb, finalColor.rgb, input.FogFactor);
    }

    return finalColor;
}
)";

// ============================================================================
// CDX11ShaderManager Implementation
// ============================================================================

CDX11ShaderManager::CDX11ShaderManager()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pFFP_VS(nullptr),
      m_pFFP_PS(nullptr), m_pFFP_InputLayout(nullptr), m_pCBPerFrame(nullptr),
      m_pCBPerMaterial(nullptr) {}

CDX11ShaderManager::~CDX11ShaderManager() { Shutdown(); }

bool CDX11ShaderManager::Initialize(ID3D11Device *pDevice,
                                    ID3D11DeviceContext *pContext) {
  m_pDevice = pDevice;
  m_pContext = pContext;

  // ------ Compile vertex shader ------
  ID3D10Blob *pVSBlob = nullptr;
  if (!CompileShader(s_szFFP_HLSL, "VS_FFP", "vs_5_0", &pVSBlob))
    return false;

  HRESULT hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
                                             pVSBlob->GetBufferSize(), nullptr,
                                             &m_pFFP_VS);
  if (FAILED(hr)) {
    pVSBlob->Release();
    return false;
  }

  // ------ Create input layout matching TPDTVertex (Position + Diffuse +
  // TexCoord) ------
  D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  hr = m_pDevice->CreateInputLayout(
      layoutDesc, ARRAYSIZE(layoutDesc), pVSBlob->GetBufferPointer(),
      pVSBlob->GetBufferSize(), &m_pFFP_InputLayout);
  pVSBlob->Release();

  if (FAILED(hr))
    return false;

  // ------ Compile pixel shader ------
  ID3D10Blob *pPSBlob = nullptr;
  if (!CompileShader(s_szFFP_HLSL, "PS_FFP", "ps_5_0", &pPSBlob))
    return false;

  hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                    pPSBlob->GetBufferSize(), nullptr,
                                    &m_pFFP_PS);
  pPSBlob->Release();

  if (FAILED(hr))
    return false;

  // ------ Create constant buffers ------
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

  // CBPerFrame (128 bytes: 2x float4x4)
  cbDesc.ByteWidth = sizeof(CBPerFrame);
  hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pCBPerFrame);
  if (FAILED(hr))
    return false;

  // CBPerMaterial (48 bytes, padded to 16-byte alignment)
  cbDesc.ByteWidth = sizeof(CBPerMaterial);
  hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pCBPerMaterial);
  if (FAILED(hr))
    return false;

  // Set initial material params (alpha test off, fog off)
  CBPerMaterial mat = {};
  mat.fAlphaRef = 0.0f;
  mat.fAlphaTestEnable = 0.0f;
  mat.fFogEnable = 0.0f;
  D3D11_MAPPED_SUBRESOURCE mapped;
  m_pContext->Map(m_pCBPerMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  memcpy(mapped.pData, &mat, sizeof(mat));
  m_pContext->Unmap(m_pCBPerMaterial, 0);

  return true;
}

void CDX11ShaderManager::Shutdown() {
  if (m_pFFP_VS) {
    m_pFFP_VS->Release();
    m_pFFP_VS = nullptr;
  }
  if (m_pFFP_PS) {
    m_pFFP_PS->Release();
    m_pFFP_PS = nullptr;
  }
  if (m_pFFP_InputLayout) {
    m_pFFP_InputLayout->Release();
    m_pFFP_InputLayout = nullptr;
  }
  if (m_pCBPerFrame) {
    m_pCBPerFrame->Release();
    m_pCBPerFrame = nullptr;
  }
  if (m_pCBPerMaterial) {
    m_pCBPerMaterial->Release();
    m_pCBPerMaterial = nullptr;
  }

  m_pDevice = nullptr;
  m_pContext = nullptr;
}

// ============================================================================
// Bind the FFP PDT shader pipeline
// ============================================================================

void CDX11ShaderManager::BindFFP_PDT() {
  if (!m_pContext)
    return;

  m_pContext->VSSetShader(m_pFFP_VS, nullptr, 0);
  m_pContext->PSSetShader(m_pFFP_PS, nullptr, 0);
  m_pContext->IASetInputLayout(m_pFFP_InputLayout);

  // Bind constant buffers to both VS and PS
  ID3D11Buffer *cbs[] = {m_pCBPerFrame, m_pCBPerMaterial};
  m_pContext->VSSetConstantBuffers(0, 2, cbs);
  m_pContext->PSSetConstantBuffers(0, 2, cbs);
}

// ============================================================================
// Update transforms — multiplies World * View * Proj and uploads to GPU
// ============================================================================

void CDX11ShaderManager::UpdateTransforms(const float *pWorld4x4,
                                          const float *pView4x4,
                                          const float *pProj4x4) {
  if (!m_pContext || !m_pCBPerFrame)
    return;

  using namespace DirectX;

  XMMATRIX matWorld = XMLoadFloat4x4((const XMFLOAT4X4 *)pWorld4x4);
  XMMATRIX matView = XMLoadFloat4x4((const XMFLOAT4X4 *)pView4x4);
  XMMATRIX matProj = XMLoadFloat4x4((const XMFLOAT4X4 *)pProj4x4);
  XMMATRIX matWVP = matWorld * matView * matProj;

  // Transpose for HLSL (column-major)
  XMMATRIX matWVPT = XMMatrixTranspose(matWVP);
  XMMATRIX matWorldT = XMMatrixTranspose(matWorld);

  CBPerFrame cb;
  XMStoreFloat4x4((XMFLOAT4X4 *)&cb.matWorldViewProj, matWVPT);
  XMStoreFloat4x4((XMFLOAT4X4 *)&cb.matWorld, matWorldT);

  D3D11_MAPPED_SUBRESOURCE mapped;
  m_pContext->Map(m_pCBPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  memcpy(mapped.pData, &cb, sizeof(cb));
  m_pContext->Unmap(m_pCBPerFrame, 0);
}

// ============================================================================
// Update material parameters (alpha test, fog)
// ============================================================================

void CDX11ShaderManager::UpdateMaterialParams(float alphaRef,
                                              bool alphaTestEnable,
                                              bool fogEnable,
                                              const float *fogColor,
                                              float fogStart, float fogEnd) {
  if (!m_pContext || !m_pCBPerMaterial)
    return;

  CBPerMaterial cb = {};
  cb.fAlphaRef = alphaRef;
  cb.fAlphaTestEnable = alphaTestEnable ? 1.0f : 0.0f;
  cb.fFogEnable = fogEnable ? 1.0f : 0.0f;
  if (fogColor) {
    cb.vFogColor[0] = fogColor[0];
    cb.vFogColor[1] = fogColor[1];
    cb.vFogColor[2] = fogColor[2];
    cb.vFogColor[3] = fogColor[3];
  }
  cb.fFogStart = fogStart;
  cb.fFogEnd = fogEnd;

  D3D11_MAPPED_SUBRESOURCE mapped;
  m_pContext->Map(m_pCBPerMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
  memcpy(mapped.pData, &cb, sizeof(cb));
  m_pContext->Unmap(m_pCBPerMaterial, 0);
}

// ============================================================================
// Compile HLSL shader from source string
// ============================================================================

bool CDX11ShaderManager::CompileShader(const char *szSource,
                                       const char *szEntryPoint,
                                       const char *szProfile,
                                       ID3D10Blob **ppBlob) {
  ID3D10Blob *pErrorBlob = nullptr;

  DWORD dwFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  dwFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
  dwFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

  HRESULT hr =
      D3DCompile(szSource, strlen(szSource), nullptr, nullptr, nullptr,
                 szEntryPoint, szProfile, dwFlags, 0, ppBlob, &pErrorBlob);

  if (FAILED(hr)) {
    if (pErrorBlob) {
      OutputDebugStringA((const char *)pErrorBlob->GetBufferPointer());
      pErrorBlob->Release();
    }
    return false;
  }

  if (pErrorBlob)
    pErrorBlob->Release();

  return true;
}
