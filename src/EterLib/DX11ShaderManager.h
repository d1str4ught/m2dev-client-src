#pragma once

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct ID3D10Blob; // ID3DBlob = ID3D10Blob

#include <d3d9.h> // D3DFVF_*, D3DTRANSFORMSTATETYPE

// ============================================================================
// Constant Buffer Structures (must be 16-byte aligned)
// ============================================================================

// Per-frame constants: World-View-Projection matrix
struct CBPerFrame {
  float matWorldViewProj[4][4]; // 64 bytes — combined WVP matrix
  float matWorld[4][4];         // 64 bytes — world matrix (for lighting)
};

// Per-material constants: alpha test, fog, diffuse multiply
struct CBPerMaterial {
  float fAlphaRef;        // alpha test reference (0.0-1.0)
  float fAlphaTestEnable; // 1.0 = alpha test on, 0.0 = off
  float fFogEnable;       // 1.0 = fog on
  float fPad0;            // padding to 16-byte boundary
  float vFogColor[4];     // fog color RGBA
  float fFogStart;        // fog start distance
  float fFogEnd;          // fog end distance
  float fPad1;
  float fPad2;
};

// ============================================================================
// CDX11ShaderManager
//
// Manages compiled DX11 shaders and input layouts for FFP emulation.
// Provides built-in shader programs that replicate DX9 fixed-function:
//   - FFP_PDT: Position + Diffuse + TexCoord (the most common vertex format)
//   - FFP_POS_ONLY: Position only (debug/wireframe)
// ============================================================================

class CDX11ShaderManager {
public:
  CDX11ShaderManager();
  ~CDX11ShaderManager();

  bool Initialize(ID3D11Device *pDevice, ID3D11DeviceContext *pContext);
  void Shutdown();

  // ------ Built-in FFP shaders ------

  // Bind the PDT (position + diffuse + texcoord) shader
  void BindFFP_PDT();

  // Auto-select and bind the right shader for a given FVF
  void BindForFVF(DWORD dwFVF);

  // ------ Constant buffer updates ------

  // Update WVP matrix (called when transforms change)
  void UpdateTransforms(const float *pWorld4x4, const float *pView4x4,
                        const float *pProj4x4);

  // Update material/alpha test params
  void UpdateMaterialParams(float alphaRef, bool alphaTestEnable,
                            bool fogEnable, const float *fogColor,
                            float fogStart, float fogEnd);

  // ------ Accessors (backwards compat) ------
  ID3D11VertexShader *GetFFP_VS() const { return m_pFFP_VS; }
  ID3D11PixelShader *GetFFP_PS() const { return m_pFFP_PS; }
  ID3D11InputLayout *GetFFP_InputLayout() const { return m_pFFP_InputLayout; }

private:
  bool CompileShader(const char *szSource, const char *szEntryPoint,
                     const char *szProfile, ID3D10Blob **ppBlob);
  void BindForVariant(int variant);

  ID3D11Device *m_pDevice;
  ID3D11DeviceContext *m_pContext;

  // Variant shader arrays (indexed by EFFPVariant enum in .cpp)
  static const int MAX_VARIANTS = 5;
  ID3D11VertexShader *m_pFFP_VS_Variants[MAX_VARIANTS];
  ID3D11PixelShader *m_pFFP_PS_Variants[MAX_VARIANTS];
  ID3D11InputLayout *m_pFFP_InputLayouts[MAX_VARIANTS];
  int m_iCurrentVariant;

  // Backwards-compat aliases (point to variant[0] = PDT)
  ID3D11VertexShader *m_pFFP_VS;
  ID3D11PixelShader *m_pFFP_PS;
  ID3D11InputLayout *m_pFFP_InputLayout;

  // Constant buffers
  ID3D11Buffer *m_pCBPerFrame;
  ID3D11Buffer *m_pCBPerMaterial;
};
