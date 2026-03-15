#pragma once

// Forward declarations for DX11
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;

// ============================================================================
// CBTerrain — Constant buffer for terrain rendering (register b3)
//
// Contains the 2 texture transform matrices used by the terrain splat system:
//   matTexTransform0: Camera-space → tiled texture UV (tile color texture)
//   matTexTransform1: Camera-space → splat alpha UV (splat coverage map)
//   matWorldViewProj: Combined WVP for this patch
//   matView:          View matrix (camera space conversion)
//   fogColor:         Fog color for far patches
// ============================================================================

struct CBTerrain {
  float matWorldViewProj[4][4]; // 64 bytes
  float matView[4][4];          // 64 bytes
  float matTexTransform0[4][4]; // 64 bytes — tile texture coord transform
  float matTexTransform1[4][4]; // 64 bytes — splat alpha coord transform
  float fogColor[4];            // 16 bytes
  float fFogStart;              // 4 bytes
  float fFogEnd;                // 4 bytes
  float fAlphaTestEnable;       // 4 bytes
  float fAlphaRef;              // 4 bytes
}; // Total: 288 bytes

// ============================================================================
// CDX11TerrainShader
//
// Manages the terrain-specific HLSL shader for DX11 HTP rendering.
// Replaces the DX9 FFP multi-texture-stage terrain splat pipeline.
//
// Usage:
//   1. Call Bind() before rendering terrain patches
//   2. Call UpdateConstants() before each patch (different transforms per
//   patch)
//   3. Bind tile texture to slot 0, splat alpha to slot 1
//   4. DrawIndexedPrimitive as normal
// ============================================================================

class CDX11TerrainShader {
public:
  CDX11TerrainShader();
  ~CDX11TerrainShader();

  bool Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext);
  void Shutdown();

  // Bind terrain shader pipeline (VS + PS + input layout)
  void Bind();

  // Update per-patch constant buffer
  void UpdateConstants(const CBTerrain &cb);

  // Shadow pass: bind shadow variant (multiplies shadow texture onto terrain)
  void BindShadowPass();

  // Check if shader is ready
  bool IsReady() const { return m_pVS != nullptr && m_pPS != nullptr; }

private:
  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;

  ID3D11VertexShader *m_pVS;      // Terrain vertex shader
  ID3D11PixelShader *m_pPS;       // Terrain pixel shader (splat blend)
  ID3D11PixelShader *m_pPSShadow; // Terrain pixel shader (shadow pass)
  ID3D11InputLayout *m_pLayout;   // Input layout for XYZ + Normal
  ID3D11Buffer *m_pCBTerrain;     // Constant buffer (register b3)
};
