/******************************************************************************

  Copyright (C) 1999, 2000 NVIDIA Corporation

  This file is provided without support, instruction, or implied warranty of any
  kind.  NVIDIA makes no guarantee of its fitness for a particular purpose and
is not liable under any circumstances for any damages or loss whatsoever arising
  from the use or inability to use this file or items derived from it.

        Comments:

          A simple class to manage rendering state.  Created as a singleton.
          Create it as a static global, or with new.  It doesn't matter as long
as it is created before you use the CStateManager::GetSingleton() API to get a
reference to it.

          Call it with STATEMANAGER.SetRenderState(...)
          Call it with STATEMANAGER.SetTextureStageState(...), etc.

          Call the 'Save' versions of the function if you want to deviate from
the current state. Call the 'Restore' version to retrieve the last Save.

          There are two levels of caching:
          - All Sets/Saves/Restores are tracked for redundancy.  This reduces
the size of the batch to be flushed
          - The flush function is called before rendering, and only copies state
that is different from the current chip state.

  If you get an assert it is probably because an API call failed.

  See NVLink for a good example of how this class is used.

  Don't be afraid of the vector being used to track the flush batch.  It will
grow as big as it needs to be and then stop, so it shouldn't be reallocated.

  The state manager holds a reference to the d3d device.

  - cmaughan@nvidia.com

******************************************************************************/

#ifndef __CSTATEMANAGER_H
#define __CSTATEMANAGER_H

#include <d3d9.h>
#include <d3dx9.h>

#include <stack>
#include <vector>

#include "DX11ShaderManager.h"
#include "DX11StateCache.h"
#include "EterBase/Singleton.h"

#define CHECK_D3DAPI(a)                                                        \
  {                                                                            \
    HRESULT hr = (a);                                                          \
                                                                               \
    if (hr != S_OK)                                                            \
      assert(!#a);                                                             \
  }

static const DWORD STATEMANAGER_MAX_RENDERSTATES = 256;
static const DWORD STATEMANAGER_MAX_TEXTURESTATES = 128;
static const DWORD STATEMANAGER_MAX_STAGES = 8;
static const DWORD STATEMANAGER_MAX_VCONSTANTS = 96;
static const DWORD STATEMANAGER_MAX_PCONSTANTS = 8;
static const DWORD STATEMANAGER_MAX_TRANSFORMSTATES =
    300; // World1 lives way up there...
static const DWORD STATEMANAGER_MAX_STREAMS = 16;

static DWORD gs_DefaultRenderStates[STATEMANAGER_MAX_RENDERSTATES];

class CStreamData {
public:
  CStreamData(LPDIRECT3DVERTEXBUFFER9 pStreamData = NULL, UINT Stride = 0)
      : m_lpStreamData(pStreamData), m_Stride(Stride) {}

  bool operator==(const CStreamData &rhs) const {
    return ((m_lpStreamData == rhs.m_lpStreamData) &&
            (m_Stride == rhs.m_Stride));
  }

  LPDIRECT3DVERTEXBUFFER9 m_lpStreamData;
  UINT m_Stride;
};

class CIndexData {
public:
  CIndexData(LPDIRECT3DINDEXBUFFER9 pIndexData = NULL, UINT BaseVertexIndex = 0)
      : m_lpIndexData(pIndexData), m_BaseVertexIndex(BaseVertexIndex) {}

  bool operator==(const CIndexData &rhs) const {
    return ((m_lpIndexData == rhs.m_lpIndexData) &&
            (m_BaseVertexIndex == rhs.m_BaseVertexIndex));
  }

  LPDIRECT3DINDEXBUFFER9 m_lpIndexData;
  UINT m_BaseVertexIndex;
};

// State types managed by the class
typedef enum eStateType {
  STATE_MATERIAL = 0,
  STATE_RENDER,
  STATE_TEXTURE,
  STATE_TEXTURESTAGE,
  STATE_VSHADER,
  STATE_PSHADER,
  STATE_TRANSFORM,
  STATE_VCONSTANT,
  STATE_PCONSTANT,
  STATE_STREAM,
  STATE_INDEX
} eStateType;

class CStateID {
public:
  CStateID(eStateType Type, DWORD dwValue0 = 0, DWORD dwValue1 = 0)
      : m_Type(Type), m_dwValue0(dwValue0), m_dwValue1(dwValue1) {}

  CStateID(eStateType Type, DWORD dwStage, D3DTEXTURESTAGESTATETYPE StageType)
      : m_Type(Type), m_dwStage(dwStage), m_TextureStageStateType(StageType) {}

  CStateID(eStateType Type, D3DRENDERSTATETYPE RenderType)
      : m_Type(Type), m_RenderStateType(RenderType) {}

  eStateType m_Type;

  union {
    DWORD m_dwValue0;
    DWORD m_dwStage;
    D3DRENDERSTATETYPE m_RenderStateType;
    D3DTRANSFORMSTATETYPE m_TransformStateType;
  };

  union {
    DWORD m_dwValue1;
    D3DTEXTURESTAGESTATETYPE m_TextureStageStateType;
  };
};

typedef std::vector<CStateID> TStateID;

class CStateManagerState {
public:
  CStateManagerState() {}

  void ResetState() {
    DWORD i, y;

    for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
      m_RenderStates[i] = gs_DefaultRenderStates[i];

    for (i = 0; i < STATEMANAGER_MAX_STAGES; i++)
      for (y = 0; y < STATEMANAGER_MAX_TEXTURESTATES; y++)
        m_TextureStates[i][y] = 0x7FFFFFFF;

    for (i = 0; i < STATEMANAGER_MAX_STREAMS; i++)
      m_StreamData[i] = CStreamData();

    m_IndexData = CIndexData();

    for (i = 0; i < STATEMANAGER_MAX_STAGES; i++)
      m_Textures[i] = NULL;

    // Matrices and constants are not cached, just restored.  It's silly to
    // check all the data elements (by which time the driver could have been
    // sent them).
    for (i = 0; i < STATEMANAGER_MAX_TRANSFORMSTATES; i++)
      D3DXMatrixIdentity(&m_Matrices[i]);

    m_dwPixelShader = 0;
    m_dwVertexShader = 0;
    m_dwVertexDeclaration = 0;
    m_dwFVF = D3DFVF_XYZ;
    m_bVertexProcessing = FALSE;
  }

  // Renderstates
  DWORD m_RenderStates[STATEMANAGER_MAX_RENDERSTATES];

  // Texture stage states
  DWORD m_TextureStates[STATEMANAGER_MAX_STAGES]
                       [STATEMANAGER_MAX_TEXTURESTATES];
  DWORD m_SamplerStates[STATEMANAGER_MAX_STAGES]
                       [STATEMANAGER_MAX_TEXTURESTATES];

  // Textures
  LPDIRECT3DBASETEXTURE9 m_Textures[STATEMANAGER_MAX_STAGES];

  // Shaders
  LPDIRECT3DPIXELSHADER9 m_dwPixelShader;
  LPDIRECT3DVERTEXSHADER9 m_dwVertexShader;
  LPDIRECT3DVERTEXDECLARATION9 m_dwVertexDeclaration;
  DWORD m_dwFVF;

  D3DXMATRIX m_Matrices[STATEMANAGER_MAX_TRANSFORMSTATES];

  D3DMATERIAL9 m_D3DMaterial;

  CStreamData m_StreamData[STATEMANAGER_MAX_STREAMS];
  CIndexData m_IndexData;

  BOOL m_bVertexProcessing;
};

class CStateManager : public CSingleton<CStateManager> {
public:
  CStateManager(LPDIRECT3DDEVICE9EX lpDevice);
  virtual ~CStateManager();

  void SetDefaultState();
  void Restore();

  bool BeginScene();
  void EndScene();

  // Material
  void SaveMaterial();
  void SaveMaterial(const D3DMATERIAL9 *pMaterial);
  void RestoreMaterial();
  void SetMaterial(const D3DMATERIAL9 *pMaterial);
  void GetMaterial(D3DMATERIAL9 *pMaterial);

  void SetLight(DWORD index, CONST D3DLIGHT9 *pLight);
  void GetLight(DWORD index, D3DLIGHT9 *pLight);

  // Scissor Rect
  void SetScissorRect(const RECT &c_rRect);
  void GetScissorRect(RECT *pRect);

  // Renderstates
  void SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue);
  void RestoreRenderState(D3DRENDERSTATETYPE Type);
  void SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value);
  void GetRenderState(D3DRENDERSTATETYPE Type, DWORD *pdwValue);

  // Textures
  void SaveTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
  void RestoreTexture(DWORD dwStage);
  void SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture);
  void GetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 *ppTexture);

  // Texture stage states
  void SaveTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type,
                             DWORD dwValue);
  void RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type);
  void SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type,
                            DWORD dwValue);
  void GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type,
                            DWORD *pdwValue);
  void SetBestFiltering(
      DWORD dwStage); // if possible set anisotropy filtering, or use trilinear

  // Sampler states
  void SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue);
  void RestoreSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type);
  void SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue);
  void GetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type,
                       DWORD *pdwValue);

  // Vertex Shader
  void SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
  void RestoreVertexShader();
  void SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader);
  void GetVertexShader(LPDIRECT3DVERTEXSHADER9 *pdwShader);

  // Vertex Declaration
  void SaveVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
  void RestoreVertexDeclaration();
  void SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader);
  void GetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 *pdwShader);

  // FVF
  void SaveFVF(DWORD dwShader);
  void RestoreFVF();
  void SetFVF(DWORD dwShader);
  void GetFVF(DWORD *pdwShader);

  // Pixel Shader
  void SavePixelShader(LPDIRECT3DPIXELSHADER9 dwShader);
  void RestorePixelShader();
  void SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader);
  void GetPixelShader(LPDIRECT3DPIXELSHADER9 *pdwShader);

  // *** These states are cached, but not protected from multiple sends of the
  // same value. Transform
  void SaveTransform(D3DTRANSFORMSTATETYPE Transform,
                     const D3DXMATRIX *pMatrix);
  void RestoreTransform(D3DTRANSFORMSTATETYPE Transform);

  // VertexProcessing
  void SaveVertexProcessing(BOOL IsON);
  void RestoreVertexProcessing();

  // Don't cache-check the transform.  To much to do
  void SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DXMATRIX *pMatrix);
  void GetTransform(D3DTRANSFORMSTATETYPE Type, D3DXMATRIX *pMatrix);

  // SetVertexShaderConstant
  void SetVertexShaderConstant(DWORD dwRegister, CONST void *pConstantData,
                               DWORD dwConstantCount);

  // SetPixelShaderConstant
  void SetPixelShaderConstant(DWORD dwRegister, CONST void *pConstantData,
                              DWORD dwConstantCount);

  void SaveStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData,
                        UINT Stride);
  void RestoreStreamSource(UINT StreamNumber);
  void SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER9 pStreamData,
                       UINT Stride);

  void SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex);
  void RestoreIndices();
  void SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData, UINT BaseVertexIndex);

  HRESULT DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex,
                        UINT PrimitiveCount);
  HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount,
                          const void *pVertexStreamZeroData,
                          UINT VertexStreamZeroStride);
  HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex,
                               UINT NumVertices, UINT startIndex,
                               UINT primCount);
  HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                               INT baseVertexIndex, UINT minIndex,
                               UINT NumVertices, UINT startIndex,
                               UINT primCount);
  HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                 UINT MinVertexIndex, UINT NumVertexIndices,
                                 UINT PrimitiveCount, CONST void *pIndexData,
                                 D3DFORMAT IndexDataFormat,
                                 CONST void *pVertexStreamZeroData,
                                 UINT VertexStreamZeroStride);

  // Codes For Debug
  DWORD GetRenderState(D3DRENDERSTATETYPE Type);

  void StateManager_Capture();
  void StateManager_Apply();

  LPDIRECT3DDEVICE9EX GetDevice();

#ifdef _DEBUG
  void ResetDrawCallCounter();
  int GetDrawCallCount() const;
#endif

private:
  void SetDevice(LPDIRECT3DDEVICE9EX lpDevice);

private:
  CStateManagerState m_CurrentState;
  CStateManagerState m_CurrentState_Copy;

  bool m_bForce;
  bool m_bScene;
  DWORD m_dwBestMinFilter;
  DWORD m_dwBestMagFilter;
  LPDIRECT3DDEVICE9EX m_lpD3DDev;

  // DX11 state cache (Phase 2)
  CDX11StateCache m_DX11StateCache;

  // DX11 shader manager (Phase 4/5 wiring)
  CDX11ShaderManager m_DX11ShaderManager;
  bool m_bDX11TransformDirty;

  std::vector<DWORD> m_RenderStateStack[STATEMANAGER_MAX_RENDERSTATES];
  std::vector<DWORD> m_SamplerStateStack[STATEMANAGER_MAX_STAGES]
                                        [STATEMANAGER_MAX_TEXTURESTATES];
  std::vector<DWORD> m_TextureStageStateStack[STATEMANAGER_MAX_STAGES]
                                             [STATEMANAGER_MAX_TEXTURESTATES];
  std::vector<D3DXMATRIX> m_TransformStack[STATEMANAGER_MAX_TRANSFORMSTATES];
  std::vector<LPDIRECT3DBASETEXTURE9> m_TextureStack[STATEMANAGER_MAX_STAGES];
  std::vector<D3DMATERIAL9> m_MaterialStack;
  std::vector<DWORD> m_FVFStack;
  std::vector<LPDIRECT3DPIXELSHADER9> m_PixelShaderStack;
  std::vector<LPDIRECT3DVERTEXSHADER9> m_VertexShaderStack;
  std::vector<LPDIRECT3DVERTEXDECLARATION9> m_VertexDeclarationStack;
  std::vector<BOOL> m_VertexProcessingStack;
  std::vector<CStreamData> m_StreamStack[STATEMANAGER_MAX_STREAMS];
  std::vector<CIndexData> m_IndexStack;

#ifdef _DEBUG
  // Saving Flag
  int m_iDrawCallCount;
  int m_iLastDrawCallCount;
#endif _DEBUG
};

#define STATEMANAGER (CStateManager::Instance())

#endif __CSTATEMANAGER_H
