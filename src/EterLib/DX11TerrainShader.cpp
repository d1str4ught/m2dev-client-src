#include "DX11TerrainShader.h"
#include "StdAfx.h"

#include <d3d11.h>
#include <d3dcompiler.h>

// ============================================================================
// Terrain HLSL shader source
//
// Replicates the DX9 FFP terrain splat pipeline:
//   VS: Transforms position, generates tex-coords from camera-space position
//       using two texture transform matrices (tile tiling + splat alpha)
//   PS: Samples tile texture (slot 0) and splat alpha (slot 1)
//       Output = tile color with splat alpha controlling coverage
//   PS_Shadow: Shadow pass — multiplies shadow map onto terrain
// ============================================================================

static const char *s_szTerrainHLSL = R"(

// ---- Constant Buffers ----
cbuffer CBTerrain : register(b3)
{
    float4x4 matWorldViewProj;
    float4x4 matView;
    float4x4 matTexTransform0;  // Camera-space -> tile UV
    float4x4 matTexTransform1;  // Camera-space -> splat alpha UV
    float4   fogColor;
    float    fFogStart;
    float    fFogEnd;
    float    fAlphaTestEnable;
    float    fAlphaRef;
};

// ---- Textures and Samplers ----
Texture2D    texTile     : register(t0);  // Tile color texture
Texture2D    texSplat    : register(t1);  // Splat alpha map
SamplerState samTile     : register(s0);  // Wrap sampler
SamplerState samSplat    : register(s1);  // Clamp sampler

// ---- Vertex Structures ----
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
};

struct VS_OUTPUT
{
    float4 Position  : SV_POSITION;
    float2 TileUV    : TEXCOORD0;  // Tiled texture coordinate
    float2 SplatUV   : TEXCOORD1;  // Splat alpha coordinate
    float  FogFactor : TEXCOORD2;
};

// ---- Vertex Shader ----
VS_OUTPUT VS_Terrain(VS_INPUT input)
{
    VS_OUTPUT o;

    // Transform to clip space
    o.Position = mul(float4(input.Position, 1.0f), matWorldViewProj);

    // Camera-space position (for tex-coord generation, matching D3DTSS_TCI_CAMERASPACEPOSITION)
    float4 camPos = mul(float4(input.Position, 1.0f), matView);

    // Generate tile texture coordinates: camera-space * tile transform
    float4 tileUV = mul(camPos, matTexTransform0);
    o.TileUV = tileUV.xy;

    // Generate splat alpha texture coordinates: camera-space * splat transform
    float4 splatUV = mul(camPos, matTexTransform1);
    o.SplatUV = splatUV.xy;

    // Fog (linear distance-based)
    float dist = length(camPos.xyz);
    o.FogFactor = saturate((fFogEnd - dist) / max(fFogEnd - fFogStart, 0.001f));

    return o;
}

// ---- Pixel Shader: Terrain Splat ----
// Renders one splat layer: tile texture * splat alpha
float4 PS_TerrainSplat(VS_OUTPUT input) : SV_TARGET
{
    float4 tileColor = texTile.Sample(samTile, input.TileUV);
    float  splatAlpha = texSplat.Sample(samSplat, input.SplatUV).a;

    float4 finalColor = tileColor;
    finalColor.a = splatAlpha;

    // Alpha test
    if (fAlphaTestEnable > 0.5f)
    {
        if (finalColor.a <= fAlphaRef)
            discard;
    }

    // Apply fog
    finalColor.rgb = lerp(fogColor.rgb, finalColor.rgb, input.FogFactor);

    return finalColor;
}

// ---- Pixel Shader: Shadow Pass ----
// Multiplies shadow texture onto terrain (src=zero, dst=srccolor)
float4 PS_TerrainShadow(VS_OUTPUT input) : SV_TARGET
{
    float4 shadowColor = texTile.Sample(samTile, input.TileUV);
    return shadowColor;
}

)";

// ============================================================================
// CDX11TerrainShader Implementation
// ============================================================================

CDX11TerrainShader::CDX11TerrainShader()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pVS(nullptr), m_pPS(nullptr),
      m_pPSShadow(nullptr), m_pLayout(nullptr), m_pCBTerrain(nullptr) {}

CDX11TerrainShader::~CDX11TerrainShader() { Shutdown(); }

static bool CompileTerrainShader(const char *szSource, const char *szEntry,
                                 const char *szProfile, ID3D10Blob **ppBlob) {
  ID3D10Blob *pErrors = nullptr;
  HRESULT hr = D3DCompile(szSource, strlen(szSource), "TerrainShader", nullptr,
                          nullptr, szEntry, szProfile,
                          D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, ppBlob, &pErrors);
  if (FAILED(hr)) {
    if (pErrors) {
      OutputDebugStringA((const char *)pErrors->GetBufferPointer());
      pErrors->Release();
    }
    return false;
  }
  if (pErrors)
    pErrors->Release();
  return true;
}

bool CDX11TerrainShader::Initialize(ID3D11Device *pDevice,
                                    ID3D11DeviceContext *pContext) {
  m_pDevice = pDevice;
  m_pContext = pContext;

  // Compile vertex shader
  ID3D10Blob *pVSBlob = nullptr;
  if (!CompileTerrainShader(s_szTerrainHLSL, "VS_Terrain", "vs_5_0",
                            &pVSBlob)) {
    Tracenf("CDX11TerrainShader: VS compilation failed");
    return false;
  }
  m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),
                                pVSBlob->GetBufferSize(), nullptr, &m_pVS);

  // Input layout: Position (float3) + Normal (float3) = matches XYZ|NORMAL
  D3D11_INPUT_ELEMENT_DESC layout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  m_pDevice->CreateInputLayout(layout, 2, pVSBlob->GetBufferPointer(),
                               pVSBlob->GetBufferSize(), &m_pLayout);
  pVSBlob->Release();

  // Compile pixel shader (splat)
  ID3D10Blob *pPSBlob = nullptr;
  if (CompileTerrainShader(s_szTerrainHLSL, "PS_TerrainSplat", "ps_5_0",
                           &pPSBlob)) {
    m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                 pPSBlob->GetBufferSize(), nullptr, &m_pPS);
    pPSBlob->Release();
  }

  // Compile pixel shader (shadow pass)
  pPSBlob = nullptr;
  if (CompileTerrainShader(s_szTerrainHLSL, "PS_TerrainShadow", "ps_5_0",
                           &pPSBlob)) {
    m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                 pPSBlob->GetBufferSize(), nullptr,
                                 &m_pPSShadow);
    pPSBlob->Release();
  }

  // Create constant buffer
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = sizeof(CBTerrain);
  cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbDesc.Usage = D3D11_USAGE_DYNAMIC;
  cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  HRESULT hr = m_pDevice->CreateBuffer(&cbDesc, nullptr, &m_pCBTerrain);
  if (FAILED(hr))
    return false;

  Tracenf("CDX11TerrainShader: Initialized successfully");
  return true;
}

void CDX11TerrainShader::Shutdown() {
  if (m_pVS) {
    m_pVS->Release();
    m_pVS = nullptr;
  }
  if (m_pPS) {
    m_pPS->Release();
    m_pPS = nullptr;
  }
  if (m_pPSShadow) {
    m_pPSShadow->Release();
    m_pPSShadow = nullptr;
  }
  if (m_pLayout) {
    m_pLayout->Release();
    m_pLayout = nullptr;
  }
  if (m_pCBTerrain) {
    m_pCBTerrain->Release();
    m_pCBTerrain = nullptr;
  }
  m_pDevice = nullptr;
  m_pContext = nullptr;
}

void CDX11TerrainShader::Bind() {
  if (!m_pContext || !m_pVS || !m_pPS)
    return;

  m_pContext->VSSetShader(m_pVS, nullptr, 0);
  m_pContext->PSSetShader(m_pPS, nullptr, 0);
  m_pContext->IASetInputLayout(m_pLayout);

  // Bind terrain constant buffer to slot 3 (b3)
  m_pContext->VSSetConstantBuffers(3, 1, &m_pCBTerrain);
  m_pContext->PSSetConstantBuffers(3, 1, &m_pCBTerrain);
}

void CDX11TerrainShader::UpdateConstants(const CBTerrain &cb) {
  if (!m_pContext || !m_pCBTerrain)
    return;

  D3D11_MAPPED_SUBRESOURCE mapped;
  if (SUCCEEDED(m_pContext->Map(m_pCBTerrain, 0, D3D11_MAP_WRITE_DISCARD, 0,
                                &mapped))) {
    memcpy(mapped.pData, &cb, sizeof(cb));
    m_pContext->Unmap(m_pCBTerrain, 0);
  }
}

void CDX11TerrainShader::BindShadowPass() {
  if (!m_pContext || !m_pPSShadow)
    return;

  // Keep same VS and layout, switch to shadow pixel shader
  m_pContext->PSSetShader(m_pPSShadow, nullptr, 0);
}
