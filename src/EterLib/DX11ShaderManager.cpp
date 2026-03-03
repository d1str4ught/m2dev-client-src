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

// ---- Shared Output ----
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float  FogFactor : TEXCOORD1;
};

float ComputeFog(float3 pos)
{
    float4 worldPos = mul(float4(pos, 1.0f), matWorld);
    float dist = length(worldPos.xyz);
    return saturate((fFogEnd - dist) / (fFogEnd - fFogStart + 0.0001f));
}

// ====== VS_FFP: Position + Diffuse + TexCoord (PDT) ======
struct VS_PDT_INPUT { float3 Position : POSITION; float4 Color : COLOR0; float2 TexCoord : TEXCOORD0; };
VS_OUTPUT VS_FFP(VS_PDT_INPUT input)
{
    VS_OUTPUT o;
    o.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);
    o.Color    = input.Color;
    o.TexCoord = input.TexCoord;
    o.FogFactor = ComputeFog(input.Position);
    return o;
}

// ====== VS_PT: Position + TexCoord (no diffuse — use white) ======
struct VS_PT_INPUT { float3 Position : POSITION; float2 TexCoord : TEXCOORD0; };
VS_OUTPUT VS_PT(VS_PT_INPUT input)
{
    VS_OUTPUT o;
    o.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);
    o.Color    = float4(1, 1, 1, 1);
    o.TexCoord = input.TexCoord;
    o.FogFactor = ComputeFog(input.Position);
    return o;
}

// ====== VS_PD: Position + Diffuse (no texture) ======
struct VS_PD_INPUT { float3 Position : POSITION; float4 Color : COLOR0; };
VS_OUTPUT VS_PD(VS_PD_INPUT input)
{
    VS_OUTPUT o;
    o.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);
    o.Color    = input.Color;
    o.TexCoord = float2(0, 0);
    o.FogFactor = ComputeFog(input.Position);
    return o;
}

// ====== VS_PNT: Position + Normal + TexCoord (basic directional light) ======
struct VS_PNT_INPUT { float3 Position : POSITION; float3 Normal : NORMAL; float2 TexCoord : TEXCOORD0; };
VS_OUTPUT VS_PNT(VS_PNT_INPUT input)
{
    VS_OUTPUT o;
    o.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);
    // Basic hemisphere lighting: dot(normal, up) * 0.5 + 0.5
    float3 worldNormal = mul(input.Normal, (float3x3)matWorld);
    float NdotL = dot(normalize(worldNormal), float3(0, 1, 0)) * 0.5f + 0.5f;
    o.Color    = float4(NdotL, NdotL, NdotL, 1.0f);
    o.TexCoord = input.TexCoord;
    o.FogFactor = ComputeFog(input.Position);
    return o;
}

// ====== VS_TL: Pre-transformed (XYZRHW) passthrough ======
struct VS_TL_INPUT { float4 Position : POSITION; float4 Color : COLOR0; float2 TexCoord : TEXCOORD0; };
VS_OUTPUT VS_TL(VS_TL_INPUT input)
{
    VS_OUTPUT o;
    o.Position = input.Position;  // Already in clip space
    o.Color    = input.Color;
    o.TexCoord = input.TexCoord;
    o.FogFactor = 1.0f;  // No fog for UI/2D
    return o;
}

// ---- Pixel Shader (shared by all variants) ----
float4 PS_FFP(VS_OUTPUT input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(samLinear, input.TexCoord);
    float4 finalColor = texColor * input.Color;

    if (fAlphaTestEnable > 0.5f)
    {
        if (finalColor.a < fAlphaRef)
            discard;
    }

    if (fFogEnable > 0.5f)
    {
        finalColor.rgb = lerp(vFogColor.rgb, finalColor.rgb, input.FogFactor);
    }

    return finalColor;
}

// PS for PD (no texture, just diffuse color)
float4 PS_PD(VS_OUTPUT input) : SV_TARGET
{
    float4 finalColor = input.Color;
    if (fAlphaTestEnable > 0.5f)
    {
        if (finalColor.a < fAlphaRef)
            discard;
    }
    if (fFogEnable > 0.5f)
    {
        finalColor.rgb = lerp(vFogColor.rgb, finalColor.rgb, input.FogFactor);
    }
    return finalColor;
}
)";

// Shader variant enum
enum EFFPVariant {
  FFP_PDT = 0, // Position + Diffuse + TexCoord
  FFP_PT,      // Position + TexCoord
  FFP_PD,      // Position + Diffuse
  FFP_PNT,     // Position + Normal + TexCoord
  FFP_TL,      // Pre-transformed (XYZRHW)
  FFP_COUNT
};

static const char *s_szVSEntryPoints[FFP_COUNT] = {"VS_FFP", "VS_PT", "VS_PD",
                                                   "VS_PNT", "VS_TL"};
static const char *s_szPSEntryPoints[FFP_COUNT] = {"PS_FFP", "PS_FFP", "PS_PD",
                                                   "PS_FFP", "PS_FFP"};

// ============================================================================
// CDX11ShaderManager Implementation
// ============================================================================

CDX11ShaderManager::CDX11ShaderManager()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pCBPerFrame(nullptr),
      m_pCBPerMaterial(nullptr), m_iCurrentVariant(-1) {
  memset(m_pFFP_VS_Variants, 0, sizeof(m_pFFP_VS_Variants));
  memset(m_pFFP_PS_Variants, 0, sizeof(m_pFFP_PS_Variants));
  memset(m_pFFP_InputLayouts, 0, sizeof(m_pFFP_InputLayouts));
}

CDX11ShaderManager::~CDX11ShaderManager() { Shutdown(); }

bool CDX11ShaderManager::Initialize(ID3D11Device *pDevice,
                                    ID3D11DeviceContext *pContext) {
  m_pDevice = pDevice;
  m_pContext = pContext;

  // ------ Compile all shader variants ------
  struct LayoutEntry {
    const char *semantic;
    UINT semanticIndex;
    DXGI_FORMAT format;
    UINT offset;
  };

  // PDT layout
  LayoutEntry layoutPDT[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
      {"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 12},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 16},
  };
  // PT layout
  LayoutEntry layoutPT[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 12},
  };
  // PD layout
  LayoutEntry layoutPD[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
      {"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 12},
  };
  // PNT layout
  LayoutEntry layoutPNT[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 24},
  };
  // TL layout (XYZRHW + diffuse + texcoord)
  LayoutEntry layoutTL[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0},
      {"COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 16},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 20},
  };

  struct VariantDef {
    LayoutEntry *layout;
    UINT layoutCount;
  };
  VariantDef variants[FFP_COUNT] = {
      {layoutPDT, 3}, {layoutPT, 2}, {layoutPD, 2},
      {layoutPNT, 3}, {layoutTL, 3},
  };

  for (int i = 0; i < FFP_COUNT; i++) {
    // Compile VS
    ID3D10Blob *pVSBlob = nullptr;
    if (!CompileShader(s_szFFP_HLSL, s_szVSEntryPoints[i], "vs_5_0", &pVSBlob))
      continue;
    m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
                                  pVSBlob->GetBufferSize(), nullptr,
                                  &m_pFFP_VS_Variants[i]);

    // Create input layout
    D3D11_INPUT_ELEMENT_DESC desc[4] = {};
    for (UINT j = 0; j < variants[i].layoutCount; j++) {
      desc[j].SemanticName = variants[i].layout[j].semantic;
      desc[j].SemanticIndex = variants[i].layout[j].semanticIndex;
      desc[j].Format = variants[i].layout[j].format;
      desc[j].AlignedByteOffset = variants[i].layout[j].offset;
      desc[j].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    }
    m_pDevice->CreateInputLayout(
        desc, variants[i].layoutCount, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &m_pFFP_InputLayouts[i]);
    pVSBlob->Release();

    // Compile PS
    ID3D10Blob *pPSBlob = nullptr;
    if (CompileShader(s_szFFP_HLSL, s_szPSEntryPoints[i], "ps_5_0", &pPSBlob)) {
      m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                   pPSBlob->GetBufferSize(), nullptr,
                                   &m_pFFP_PS_Variants[i]);
      pPSBlob->Release();
    }
  }

  // Backwards compat aliases
  m_pFFP_VS = m_pFFP_VS_Variants[FFP_PDT];
  m_pFFP_PS = m_pFFP_PS_Variants[FFP_PDT];
  m_pFFP_InputLayout = m_pFFP_InputLayouts[FFP_PDT];

  // ------ Create constant buffers ------
  HRESULT hr;
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
  for (int i = 0; i < FFP_COUNT; i++) {
    if (m_pFFP_VS_Variants[i]) {
      m_pFFP_VS_Variants[i]->Release();
      m_pFFP_VS_Variants[i] = nullptr;
    }
    if (m_pFFP_PS_Variants[i]) {
      m_pFFP_PS_Variants[i]->Release();
      m_pFFP_PS_Variants[i] = nullptr;
    }
    if (m_pFFP_InputLayouts[i]) {
      m_pFFP_InputLayouts[i]->Release();
      m_pFFP_InputLayouts[i] = nullptr;
    }
  }
  m_pFFP_VS = nullptr;
  m_pFFP_PS = nullptr;
  m_pFFP_InputLayout = nullptr;
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
  m_iCurrentVariant = -1;
}

// ============================================================================
// Bind the FFP PDT shader pipeline
// ============================================================================

void CDX11ShaderManager::BindFFP_PDT() { BindForVariant(FFP_PDT); }

void CDX11ShaderManager::BindForFVF(DWORD dwFVF) {
  if (dwFVF & D3DFVF_XYZRHW) {
    BindForVariant(FFP_TL);
  } else if ((dwFVF & D3DFVF_NORMAL) && (dwFVF & D3DFVF_TEX1)) {
    BindForVariant(FFP_PNT);
  } else if ((dwFVF & D3DFVF_DIFFUSE) && (dwFVF & D3DFVF_TEX1)) {
    BindForVariant(FFP_PDT);
  } else if (dwFVF & D3DFVF_TEX1) {
    BindForVariant(FFP_PT);
  } else if (dwFVF & D3DFVF_DIFFUSE) {
    BindForVariant(FFP_PD);
  } else {
    BindForVariant(FFP_PDT); // fallback
  }
}

void CDX11ShaderManager::BindForVariant(int variant) {
  if (!m_pContext || variant < 0 || variant >= FFP_COUNT)
    return;
  if (variant == m_iCurrentVariant)
    return; // already bound

  m_pContext->VSSetShader(m_pFFP_VS_Variants[variant], nullptr, 0);
  m_pContext->PSSetShader(m_pFFP_PS_Variants[variant], nullptr, 0);
  m_pContext->IASetInputLayout(m_pFFP_InputLayouts[variant]);

  ID3D11Buffer *cbs[] = {m_pCBPerFrame, m_pCBPerMaterial};
  m_pContext->VSSetConstantBuffers(0, 2, cbs);
  m_pContext->PSSetConstantBuffers(0, 2, cbs);
  m_iCurrentVariant = variant;
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
