#include "StateManager.h"
#include "GrpLightManager.h"
#include "GrpTexture.h"
#include "StdAfx.h"

// #define StateManager_Assert(a) if (!(a)) puts("assert"#a)
#define StateManager_Assert(a) assert(a)

struct SLightData {
  D3DLIGHT9 m_akD3DLight[8];
} m_kLightData;

void CStateManager::SetLight(DWORD index, CONST D3DLIGHT9 *pLight) {
  assert(index < 8);
  m_kLightData.m_akD3DLight[index] = *pLight;

  m_lpD3DDev->SetLight(index, pLight);
}

void CStateManager::GetLight(DWORD index, D3DLIGHT9 *pLight) {
  assert(index < 8);
  *pLight = m_kLightData.m_akD3DLight[index];
}

void CStateManager::SetScissorRect(const RECT &c_rRect) {
  m_lpD3DDev->SetScissorRect(&c_rRect);
}

void CStateManager::GetScissorRect(RECT *pRect) {
  m_lpD3DDev->GetScissorRect(pRect);
}

bool CStateManager::BeginScene() {
  m_bScene = true;

  D3DXMATRIX m4Proj;
  D3DXMATRIX m4View;
  D3DXMATRIX m4World;
  GetTransform(D3DTS_WORLD, &m4World);
  GetTransform(D3DTS_PROJECTION, &m4Proj);
  GetTransform(D3DTS_VIEW, &m4View);
  SetTransform(D3DTS_WORLD, &m4World);
  SetTransform(D3DTS_PROJECTION, &m4Proj);
  SetTransform(D3DTS_VIEW, &m4View);

  if (FAILED(m_lpD3DDev->BeginScene()))
    return false;
  return true;
}

void CStateManager::EndScene() {
  m_lpD3DDev->EndScene();
  m_bScene = false;
}

CStateManager::CStateManager(LPDIRECT3DDEVICE9EX lpDevice) : m_lpD3DDev(NULL) {
  m_bScene = false;
  m_dwBestMinFilter = D3DTEXF_ANISOTROPIC;
  m_dwBestMagFilter = D3DTEXF_ANISOTROPIC;

  for (int i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
    lpDevice->GetRenderState((D3DRENDERSTATETYPE)i, &gs_DefaultRenderStates[i]);

  SetDevice(lpDevice);

  // DX11: Initialize state cache with DX11 device (from CGraphicBase statics)
  if (CGraphicBase::ms_pD3D11Device && CGraphicBase::ms_pD3D11Context) {
    m_DX11StateCache.Initialize(CGraphicBase::ms_pD3D11Device,
                                CGraphicBase::ms_pD3D11Context);
    m_DX11ShaderManager.Initialize(CGraphicBase::ms_pD3D11Device,
                                   CGraphicBase::ms_pD3D11Context);
  }
  m_bDX11TransformDirty = true;

#ifdef _DEBUG
  m_iDrawCallCount = 0;
  m_iLastDrawCallCount = 0;
#endif
}

CStateManager::~CStateManager() {
  if (m_lpD3DDev) {
    m_lpD3DDev->Release();
    m_lpD3DDev = NULL;
  }
}

void CStateManager::SetDevice(LPDIRECT3DDEVICE9EX lpDevice) {
  StateManager_Assert(lpDevice);
  lpDevice->AddRef();

  if (m_lpD3DDev) {
    m_lpD3DDev->Release();
    m_lpD3DDev = NULL;
  }

  m_lpD3DDev = lpDevice;

  SetDefaultState();
}

void CStateManager::SetBestFiltering(DWORD dwStage) {
  SetSamplerState(dwStage, D3DSAMP_MINFILTER, m_dwBestMinFilter);
  SetSamplerState(dwStage, D3DSAMP_MAGFILTER, m_dwBestMagFilter);
  SetSamplerState(dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
}

void CStateManager::Restore() {
  int i, j;

  m_bForce = true;

  for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i) {
    SetRenderState(D3DRENDERSTATETYPE(i), m_CurrentState.m_RenderStates[i]);
  }

  for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i) {
    for (j = 0; j < STATEMANAGER_MAX_TEXTURESTATES; ++j) {
      SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j),
                           m_CurrentState.m_TextureStates[i][j]);
      SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j),
                           m_CurrentState.m_SamplerStates[i][j]);
    }
  }

  for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i) {
    SetTexture(i, m_CurrentState.m_Textures[i]);
  }

  m_bForce = false;
}

void CStateManager::SetDefaultState() {
  m_CurrentState.ResetState();
  m_CurrentState_Copy.ResetState();

  for (auto &stack : m_RenderStateStack)
    stack.clear();

  for (auto &stageStacks : m_SamplerStateStack)
    for (auto &stack : stageStacks)
      stack.clear();

  for (auto &stageStacks : m_TextureStageStateStack)
    for (auto &stack : stageStacks)
      stack.clear();

  for (auto &stack : m_TransformStack)
    stack.clear();

  for (auto &stack : m_TextureStack)
    stack.clear();

  for (auto &stack : m_StreamStack)
    stack.clear();

  m_MaterialStack.clear();
  m_FVFStack.clear();
  m_PixelShaderStack.clear();
  m_VertexShaderStack.clear();
  m_VertexDeclarationStack.clear();
  m_VertexProcessingStack.clear();
  m_IndexStack.clear();

  m_bScene = false;
  m_bForce = true;

  D3DXMATRIX matIdentity;
  D3DXMatrixIdentity(&matIdentity);

  SetTransform(D3DTS_WORLD, &matIdentity);
  SetTransform(D3DTS_VIEW, &matIdentity);
  SetTransform(D3DTS_PROJECTION, &matIdentity);

  D3DMATERIAL9 DefaultMat;
  ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL9));

  DefaultMat.Diffuse.r = 1.0f;
  DefaultMat.Diffuse.g = 1.0f;
  DefaultMat.Diffuse.b = 1.0f;
  DefaultMat.Diffuse.a = 1.0f;
  DefaultMat.Ambient.r = 1.0f;
  DefaultMat.Ambient.g = 1.0f;
  DefaultMat.Ambient.b = 1.0f;
  DefaultMat.Ambient.a = 1.0f;
  DefaultMat.Emissive.r = 0.0f;
  DefaultMat.Emissive.g = 0.0f;
  DefaultMat.Emissive.b = 0.0f;
  DefaultMat.Emissive.a = 0.0f;
  DefaultMat.Specular.r = 0.0f;
  DefaultMat.Specular.g = 0.0f;
  DefaultMat.Specular.b = 0.0f;
  DefaultMat.Specular.a = 0.0f;
  DefaultMat.Power = 0.0f;

  SetMaterial(&DefaultMat);

  SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
  SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
  SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
  SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

  // SetRenderState(D3DRS_LINEPATTERN, 0xFFFFFFFF);
  SetRenderState(D3DRS_LASTPIXEL, TRUE);
  SetRenderState(D3DRS_ALPHAREF, 1);
  SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
  // SetRenderState(D3DRS_ZVISIBLE, FALSE);
  SetRenderState(D3DRS_FOGSTART, 0);
  SetRenderState(D3DRS_FOGEND, 0);
  SetRenderState(D3DRS_FOGDENSITY, 0);
  // SetRenderState(D3DRS_EDGEANTIALIAS, TRUE);
  // SetRenderState(D3DRS_ZBIAS, 0);
  SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
  SetRenderState(D3DRS_AMBIENT, 0x00000000);
  SetRenderState(D3DRS_LOCALVIEWER, TRUE);
  SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
  SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
  SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
  SaveVertexProcessing(FALSE);
  SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
  SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
  SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
  SetRenderState(D3DRS_PATCHEDGESTYLE, D3DPATCHEDGE_CONTINUOUS);
  SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
  SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
  SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
  SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
  SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
  SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
  SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  SetRenderState(D3DRS_FOGENABLE, FALSE);
  SetRenderState(D3DRS_FOGCOLOR, 0xFF000000);
  // MR-14: Fog update by Alaric
  SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
  // MR-14: -- END OF -- Fog update by Alaric
  SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
  SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
  SetRenderState(D3DRS_ZENABLE, TRUE);
  SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  SetRenderState(D3DRS_DITHERENABLE, TRUE);
  SetRenderState(D3DRS_STENCILENABLE, FALSE);
  SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  SetRenderState(D3DRS_CLIPPING, TRUE);
  SetRenderState(D3DRS_LIGHTING, FALSE);
  SetRenderState(D3DRS_SPECULARENABLE, FALSE);
  SetRenderState(D3DRS_COLORVERTEX, FALSE);
  SetRenderState(D3DRS_WRAP0, 0);
  SetRenderState(D3DRS_WRAP1, 0);
  SetRenderState(D3DRS_WRAP2, 0);
  SetRenderState(D3DRS_WRAP3, 0);
  SetRenderState(D3DRS_WRAP4, 0);
  SetRenderState(D3DRS_WRAP5, 0);
  SetRenderState(D3DRS_WRAP6, 0);
  SetRenderState(D3DRS_WRAP7, 0);

  SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
  SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

  SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(3, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(4, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(4, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(4, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(4, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(5, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(5, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(5, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(5, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(5, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(5, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(6, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(6, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(6, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(6, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(6, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(6, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(7, D3DTSS_COLOROP, D3DTOP_DISABLE);
  SetTextureStageState(7, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  SetTextureStageState(7, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  SetTextureStageState(7, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  SetTextureStageState(7, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  SetTextureStageState(7, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

  SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
  SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
  SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2);
  SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 3);
  SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 4);
  SetTextureStageState(5, D3DTSS_TEXCOORDINDEX, 5);
  SetTextureStageState(6, D3DTSS_TEXCOORDINDEX, 6);
  SetTextureStageState(7, D3DTSS_TEXCOORDINDEX, 7);

  SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(5, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(5, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(5, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(6, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(6, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(6, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(7, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  SetSamplerState(7, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  SetSamplerState(7, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(4, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(5, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(5, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(6, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(6, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
  SetSamplerState(7, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  SetSamplerState(7, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

  SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(4, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(5, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(6, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
  SetTextureStageState(7, D3DTSS_TEXTURETRANSFORMFLAGS, 0);

  SetTexture(0, NULL);
  SetTexture(1, NULL);
  SetTexture(2, NULL);
  SetTexture(3, NULL);
  SetTexture(4, NULL);
  SetTexture(5, NULL);
  SetTexture(6, NULL);
  SetTexture(7, NULL);

  SetPixelShader(0);
  SetFVF(D3DFVF_XYZ);

  D3DXVECTOR4 av4Null[STATEMANAGER_MAX_VCONSTANTS];
  memset(av4Null, 0, sizeof(av4Null));
  SetVertexShaderConstant(0, av4Null, STATEMANAGER_MAX_VCONSTANTS);
  SetPixelShaderConstant(0, av4Null, STATEMANAGER_MAX_PCONSTANTS);

  m_bForce = false;
}

// Material
void CStateManager::SaveMaterial() {
  m_MaterialStack.push_back(m_CurrentState.m_D3DMaterial);
}

void CStateManager::SaveMaterial(const D3DMATERIAL9 *pMaterial) {
  m_MaterialStack.push_back(m_CurrentState.m_D3DMaterial);
  SetMaterial(pMaterial);
}

void CStateManager::RestoreMaterial() {
  SetMaterial(&m_MaterialStack.back());
  m_MaterialStack.pop_back();
}

void CStateManager::SetMaterial(const D3DMATERIAL9 *pMaterial) {
  m_CurrentState.m_D3DMaterial = *pMaterial;
  m_lpD3DDev->SetMaterial(&m_CurrentState.m_D3DMaterial);
}

void CStateManager::GetMaterial(D3DMATERIAL9 *pMaterial) {
  // Set the renderstate and remember it.
  *pMaterial = m_CurrentState.m_D3DMaterial;
}

// Renderstates
DWORD CStateManager::GetRenderState(D3DRENDERSTATETYPE Type) {
  return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::StateManager_Capture() {
  m_CurrentState_Copy = m_CurrentState;
}

void CStateManager::StateManager_Apply() {
  m_CurrentState = m_CurrentState_Copy;
}

LPDIRECT3DDEVICE9EX CStateManager::GetDevice() { return m_lpD3DDev; }

void CStateManager::SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue) {
  m_RenderStateStack[Type].push_back(m_CurrentState.m_RenderStates[Type]);
  SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3DRENDERSTATETYPE Type) {
#ifdef _DEBUG
  if (m_RenderStateStack[Type].empty()) {
    Tracef(" CStateManager::SaveRenderState - This render state was not saved "
           "[%d, %d]\n",
           Type);
    StateManager_Assert(!" This render state was not saved!");
  }
#endif _DEBUG

  SetRenderState(Type, m_RenderStateStack[Type].back());
  m_RenderStateStack[Type].pop_back();
}

void CStateManager::SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value) {
  if (m_CurrentState.m_RenderStates[Type] == Value)
    return;

  m_lpD3DDev->SetRenderState(Type, Value);
  m_CurrentState.m_RenderStates[Type] = Value;

  // DX11: Notify state cache
  m_DX11StateCache.OnRenderStateChanged(Type, Value);
}

void CStateManager::GetRenderState(D3DRENDERSTATETYPE Type, DWORD *pdwValue) {
  *pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage,
                                LPDIRECT3DBASETEXTURE9 pTexture) {
  m_TextureStack[dwStage].push_back(m_CurrentState.m_Textures[dwStage]);
  SetTexture(dwStage, pTexture);
}

void CStateManager::RestoreTexture(DWORD dwStage) {
  SetTexture(dwStage, m_TextureStack[dwStage].back());
  m_TextureStack[dwStage].pop_back();
}

void CStateManager::SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture) {
  if (pTexture == m_CurrentState.m_Textures[dwStage])
    return;

  m_lpD3DDev->SetTexture(dwStage, pTexture);
  m_CurrentState.m_Textures[dwStage] = pTexture;

  // DX11: Bind matching SRV via static texture registry
  if (CGraphicBase::ms_pD3D11Context) {
    ID3D11ShaderResourceView *pSRV = CGraphicTexture::LookupDX11SRV(pTexture);
    CGraphicBase::ms_pD3D11Context->PSSetShaderResources(
        dwStage, 1,
        &pSRV); // nullptr unbinds
  }
}

void CStateManager::GetTexture(DWORD dwStage,
                               LPDIRECT3DBASETEXTURE9 *ppTexture) {
  *ppTexture = m_CurrentState.m_Textures[dwStage];
}

// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage,
                                          D3DTEXTURESTAGESTATETYPE Type,
                                          DWORD dwValue) {
  m_TextureStageStateStack[dwStage][Type].push_back(
      m_CurrentState.m_TextureStates[dwStage][Type]);
  SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage,
                                             D3DTEXTURESTAGESTATETYPE Type) {
#ifdef _DEBUG
  if (m_TextureStageStateStack[dwStage][Type].empty()) {
    Tracef(" CStateManager::RestoreTextureStageState - This texture stage "
           "state was not saved [%d, %d]\n",
           dwStage, Type);
    StateManager_Assert(!" This texture stage state was not saved!");
  }
#endif _DEBUG
  SetTextureStageState(dwStage, Type,
                       m_TextureStageStateStack[dwStage][Type].back());
  m_TextureStageStateStack[dwStage][Type].pop_back();
}

void CStateManager::SetTextureStageState(DWORD dwStage,
                                         D3DTEXTURESTAGESTATETYPE Type,
                                         DWORD dwValue) {
  if (m_CurrentState.m_TextureStates[dwStage][Type] == dwValue)
    return;

  m_lpD3DDev->SetTextureStageState(dwStage, Type, dwValue);
  m_CurrentState.m_TextureStates[dwStage][Type] = dwValue;
}

void CStateManager::GetTextureStageState(DWORD dwStage,
                                         D3DTEXTURESTAGESTATETYPE Type,
                                         DWORD *pdwValue) {
  *pdwValue = m_CurrentState.m_TextureStates[dwStage][Type];
}

// Sampler states
void CStateManager::SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type,
                                     DWORD dwValue) {
  m_SamplerStateStack[dwStage][Type].push_back(
      m_CurrentState.m_SamplerStates[dwStage][Type]);
  SetSamplerState(dwStage, Type, dwValue);
}
void CStateManager::RestoreSamplerState(DWORD dwStage,
                                        D3DSAMPLERSTATETYPE Type) {
#ifdef _DEBUG
  if (m_SamplerStateStack[dwStage][Type].empty()) {
    Tracenf(" CStateManager::RestoreTextureStageState - This texture stage "
            "state was not saved [%d, %d]\n",
            dwStage, Type);
    StateManager_Assert(!" This texture stage state was not saved!");
  }
#endif _DEBUG
  SetSamplerState(dwStage, Type, m_SamplerStateStack[dwStage][Type].back());
  m_SamplerStateStack[dwStage][Type].pop_back();
}
void CStateManager::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type,
                                    DWORD dwValue) {
  if (m_CurrentState.m_SamplerStates[dwStage][Type] == dwValue)
    return;
  m_lpD3DDev->SetSamplerState(dwStage, Type, dwValue);
  m_CurrentState.m_SamplerStates[dwStage][Type] = dwValue;

  // DX11: Notify state cache
  m_DX11StateCache.OnSamplerStateChanged(dwStage, Type, dwValue);
}
void CStateManager::GetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type,
                                    DWORD *pdwValue) {
  *pdwValue = m_CurrentState.m_SamplerStates[dwStage][Type];
}

// Vertex Shader
void CStateManager::SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader) {
  m_VertexShaderStack.push_back(m_CurrentState.m_dwVertexShader);
  SetVertexShader(dwShader);
}

void CStateManager::RestoreVertexShader() {
  SetVertexShader(m_VertexShaderStack.back());
  m_VertexShaderStack.pop_back();
}

void CStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader) {
  if (m_CurrentState.m_dwVertexShader == dwShader)
    return;

  m_lpD3DDev->SetVertexShader(dwShader);
  m_CurrentState.m_dwVertexShader = dwShader;
}

void CStateManager::GetVertexShader(LPDIRECT3DVERTEXSHADER9 *pdwShader) {
  *pdwShader = m_CurrentState.m_dwVertexShader;
}

// Vertex Processing
void CStateManager::SaveVertexProcessing(BOOL IsON) {
  m_VertexProcessingStack.push_back(m_CurrentState.m_bVertexProcessing);
  m_lpD3DDev->SetSoftwareVertexProcessing(IsON);
  m_CurrentState.m_bVertexProcessing = IsON;
}
void CStateManager::RestoreVertexProcessing() {
  m_lpD3DDev->SetSoftwareVertexProcessing(m_VertexProcessingStack.back());
  m_VertexProcessingStack.pop_back();
}
// Vertex Declaration
void CStateManager::SaveVertexDeclaration(
    LPDIRECT3DVERTEXDECLARATION9 dwShader) {
  m_VertexDeclarationStack.push_back(m_CurrentState.m_dwVertexDeclaration);
  SetVertexDeclaration(dwShader);
}
void CStateManager::RestoreVertexDeclaration() {
  SetVertexDeclaration(m_VertexDeclarationStack.back());
  m_VertexDeclarationStack.pop_back();
}
void CStateManager::SetVertexDeclaration(
    LPDIRECT3DVERTEXDECLARATION9 dwShader) {
  m_lpD3DDev->SetVertexDeclaration(dwShader);
  m_CurrentState.m_dwVertexDeclaration = dwShader;
}
void CStateManager::GetVertexDeclaration(
    LPDIRECT3DVERTEXDECLARATION9 *pdwShader) {
  *pdwShader = m_CurrentState.m_dwVertexDeclaration;
}
// FVF
void CStateManager::SaveFVF(DWORD dwShader) {
  m_FVFStack.push_back(m_CurrentState.m_dwFVF);
  SetFVF(dwShader);
}
void CStateManager::RestoreFVF() {
  SetFVF(m_FVFStack.back());
  m_FVFStack.pop_back();
}
void CStateManager::SetFVF(DWORD dwShader) {
  // if (m_CurrentState.m_dwFVF == dwShader)
  //	return;
  m_lpD3DDev->SetFVF(dwShader);
  m_CurrentState.m_dwFVF = dwShader;

  // DX11: Auto-select FFP shader variant matching this vertex format
  m_DX11ShaderManager.BindForFVF(dwShader);
}
void CStateManager::GetFVF(DWORD *pdwShader) {
  *pdwShader = m_CurrentState.m_dwFVF;
}

// Pixel Shader
void CStateManager::SavePixelShader(LPDIRECT3DPIXELSHADER9 dwShader) {
  m_PixelShaderStack.push_back(m_CurrentState.m_dwPixelShader);
  SetPixelShader(dwShader);
}

void CStateManager::RestorePixelShader() {
  SetPixelShader(m_PixelShaderStack.back());
  m_PixelShaderStack.pop_back();
}

void CStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader) {
  if (m_CurrentState.m_dwPixelShader == dwShader)
    return;

  m_lpD3DDev->SetPixelShader(dwShader);
  m_CurrentState.m_dwPixelShader = dwShader;
}

void CStateManager::GetPixelShader(LPDIRECT3DPIXELSHADER9 *pdwShader) {
  *pdwShader = m_CurrentState.m_dwPixelShader;
}

// *** These states are cached, but not protected from multiple sends of the
// same value. Transform
void CStateManager::SaveTransform(D3DTRANSFORMSTATETYPE Type,
                                  const D3DXMATRIX *pMatrix) {
  m_TransformStack[Type].push_back(m_CurrentState.m_Matrices[Type]);
  SetTransform(Type, pMatrix);
}

void CStateManager::RestoreTransform(D3DTRANSFORMSTATETYPE Type) {
#ifdef _DEBUG
  if (m_TransformStack[Type].empty()) {
    Tracef(" CStateManager::RestoreTransform - This transform was not saved "
           "[%d]\n",
           Type);
    StateManager_Assert(!" This render state was not saved!");
  }
#endif _DEBUG

  SetTransform(Type, &m_TransformStack[Type].back());
  m_TransformStack[Type].pop_back();
}

// Don't cache-check the transform.  To much to do
void CStateManager::SetTransform(D3DTRANSFORMSTATETYPE Type,
                                 const D3DXMATRIX *pMatrix) {
  m_CurrentState.m_Matrices[Type] = *pMatrix;

  if (m_bScene)
    m_lpD3DDev->SetTransform(Type, &m_CurrentState.m_Matrices[Type]);
  else
    assert(D3DTS_VIEW == Type || D3DTS_PROJECTION == Type ||
           D3DTS_WORLD == Type);

  // DX11: Mark transforms dirty for constant buffer update before next draw
  if (Type == D3DTS_WORLD || Type == D3DTS_VIEW || Type == D3DTS_PROJECTION)
    m_bDX11TransformDirty = true;
}

void CStateManager::GetTransform(D3DTRANSFORMSTATETYPE Type,
                                 D3DXMATRIX *pMatrix) {
  *pMatrix = m_CurrentState.m_Matrices[Type];
}

void CStateManager::SetVertexShaderConstant(DWORD dwRegister,
                                            CONST void *pConstantData,
                                            DWORD dwConstantCount) {
  m_lpD3DDev->SetVertexShaderConstantF(dwRegister, (const float *)pConstantData,
                                       dwConstantCount);
}

void CStateManager::SetPixelShaderConstant(DWORD dwRegister,
                                           CONST void *pConstantData,
                                           DWORD dwConstantCount) {
  m_lpD3DDev->SetVertexShaderConstantF(dwRegister, (const float *)pConstantData,
                                       dwConstantCount);
}

void CStateManager::SaveStreamSource(UINT StreamNumber,
                                     LPDIRECT3DVERTEXBUFFER9 pStreamData,
                                     UINT Stride) {
  m_StreamStack[StreamNumber].push_back(
      m_CurrentState.m_StreamData[StreamNumber]);
  SetStreamSource(StreamNumber, pStreamData, Stride);
}

void CStateManager::RestoreStreamSource(UINT StreamNumber) {
  const auto &topStream = m_StreamStack[StreamNumber].back();
  SetStreamSource(StreamNumber, topStream.m_lpStreamData, topStream.m_Stride);
  m_StreamStack[StreamNumber].pop_back();
}

void CStateManager::SetStreamSource(UINT StreamNumber,
                                    LPDIRECT3DVERTEXBUFFER9 pStreamData,
                                    UINT Stride) {
  CStreamData kStreamData(pStreamData, Stride);
  if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
    return;

  m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, 0, Stride);
  m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}

void CStateManager::SaveIndices(LPDIRECT3DINDEXBUFFER9 pIndexData,
                                UINT BaseVertexIndex) {
  m_IndexStack.push_back(m_CurrentState.m_IndexData);
  SetIndices(pIndexData, BaseVertexIndex);
}

void CStateManager::RestoreIndices() {
  const auto &topIndex = m_IndexStack.back();
  SetIndices(topIndex.m_lpIndexData, topIndex.m_BaseVertexIndex);
  m_IndexStack.pop_back();
}

void CStateManager::SetIndices(LPDIRECT3DINDEXBUFFER9 pIndexData,
                               UINT BaseVertexIndex) {
  CIndexData kIndexData(pIndexData, BaseVertexIndex);

  if (m_CurrentState.m_IndexData == kIndexData)
    return;

  m_lpD3DDev->SetIndices(pIndexData);
  m_CurrentState.m_IndexData = kIndexData;
}

// Forward declarations for DX11 topology/primcount helpers (defined below)
static D3D11_PRIMITIVE_TOPOLOGY MapTopology(D3DPRIMITIVETYPE pt);
static UINT PrimCountToVertexCount(D3DPRIMITIVETYPE pt, UINT primCount);
static UINT PrimCountToIndexCount(D3DPRIMITIVETYPE pt, UINT primCount);

HRESULT CStateManager::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                     UINT StartVertex, UINT PrimitiveCount) {
#ifdef _DEBUG
  ++m_iDrawCallCount;
#endif

  // DX11: Apply any dirty state objects before draw
  m_DX11StateCache.ApplyState();

  // DX11: Update transforms (shader variant already selected by SetFVF)
  if (m_bDX11TransformDirty) {
    m_DX11ShaderManager.UpdateTransforms(
        (const float *)&m_CurrentState.m_Matrices[D3DTS_WORLD],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_VIEW],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_PROJECTION]);
    m_bDX11TransformDirty = false;
  }

  // DX11: Emit draw call alongside DX9
  if (CGraphicBase::ms_pD3D11Context) {
    CGraphicBase::ms_pD3D11Context->IASetPrimitiveTopology(
        MapTopology(PrimitiveType));
    UINT vertexCount = PrimCountToVertexCount(PrimitiveType, PrimitiveCount);
    CGraphicBase::ms_pD3D11Context->Draw(vertexCount, StartVertex);
  }

  return (
      m_lpD3DDev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

// Helper: map D3DPRIMITIVETYPE to D3D11_PRIMITIVE_TOPOLOGY
static D3D11_PRIMITIVE_TOPOLOGY MapTopology(D3DPRIMITIVETYPE pt) {
  switch (pt) {
  case D3DPT_POINTLIST:
    return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
  case D3DPT_LINELIST:
    return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
  case D3DPT_LINESTRIP:
    return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
  case D3DPT_TRIANGLELIST:
    return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  case D3DPT_TRIANGLESTRIP:
    return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
  case D3DPT_TRIANGLEFAN:
    return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; // DX11 has no fan
  default:
    return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
  }
}

static UINT PrimCountToVertexCount(D3DPRIMITIVETYPE pt, UINT primCount) {
  switch (pt) {
  case D3DPT_POINTLIST:
    return primCount;
  case D3DPT_LINELIST:
    return primCount * 2;
  case D3DPT_LINESTRIP:
    return primCount + 1;
  case D3DPT_TRIANGLELIST:
    return primCount * 3;
  case D3DPT_TRIANGLESTRIP:
    return primCount + 2;
  case D3DPT_TRIANGLEFAN:
    return primCount + 2; // approximate
  default:
    return primCount * 3;
  }
}

static UINT PrimCountToIndexCount(D3DPRIMITIVETYPE pt, UINT primCount) {
  switch (pt) {
  case D3DPT_TRIANGLELIST:
    return primCount * 3;
  case D3DPT_TRIANGLESTRIP:
    return primCount + 2;
  case D3DPT_LINELIST:
    return primCount * 2;
  case D3DPT_LINESTRIP:
    return primCount + 1;
  default:
    return primCount * 3;
  }
}

HRESULT CStateManager::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,
                                       UINT PrimitiveCount,
                                       const void *pVertexStreamZeroData,
                                       UINT VertexStreamZeroStride) {
#ifdef _DEBUG
  ++m_iDrawCallCount;
#endif

  // DX11: Apply any dirty state objects before draw
  m_DX11StateCache.ApplyState();

  m_CurrentState.m_StreamData[0] = NULL;
  return (m_lpD3DDev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount,
                                      pVertexStreamZeroData,
                                      VertexStreamZeroStride));
}

HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                            UINT minIndex, UINT NumVertices,
                                            UINT startIndex, UINT primCount) {
#ifdef _DEBUG
  ++m_iDrawCallCount;
#endif

  // DX11: Apply state + emit indexed draw
  m_DX11StateCache.ApplyState();

  // DX11: Update transforms if dirty
  if (m_bDX11TransformDirty) {
    m_DX11ShaderManager.UpdateTransforms(
        (const float *)&m_CurrentState.m_Matrices[D3DTS_WORLD],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_VIEW],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_PROJECTION]);
    m_bDX11TransformDirty = false;
  }

  // DX11: Emit indexed draw
  if (CGraphicBase::ms_pD3D11Context) {
    CGraphicBase::ms_pD3D11Context->IASetPrimitiveTopology(
        MapTopology(PrimitiveType));
    UINT indexCount = PrimCountToIndexCount(PrimitiveType, primCount);
    CGraphicBase::ms_pD3D11Context->DrawIndexed(indexCount, startIndex, 0);
  }

  return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, 0, minIndex,
                                           NumVertices, startIndex, primCount));
}

HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,
                                            INT baseVertexIndex, UINT minIndex,
                                            UINT NumVertices, UINT startIndex,
                                            UINT primCount) {
#ifdef _DEBUG
  ++m_iDrawCallCount;
#endif

  // DX11: Apply state + emit indexed draw with base vertex
  m_DX11StateCache.ApplyState();

  if (m_bDX11TransformDirty) {
    m_DX11ShaderManager.UpdateTransforms(
        (const float *)&m_CurrentState.m_Matrices[D3DTS_WORLD],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_VIEW],
        (const float *)&m_CurrentState.m_Matrices[D3DTS_PROJECTION]);
    m_bDX11TransformDirty = false;
  }

  if (CGraphicBase::ms_pD3D11Context) {
    CGraphicBase::ms_pD3D11Context->IASetPrimitiveTopology(
        MapTopology(PrimitiveType));
    UINT indexCount = PrimCountToIndexCount(PrimitiveType, primCount);
    CGraphicBase::ms_pD3D11Context->DrawIndexed(indexCount, startIndex,
                                                baseVertexIndex);
  }

  return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, baseVertexIndex,
                                           minIndex, NumVertices, startIndex,
                                           primCount));
}

HRESULT CStateManager::DrawIndexedPrimitiveUP(
    D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices,
    UINT PrimitiveCount, CONST void *pIndexData, D3DFORMAT IndexDataFormat,
    CONST void *pVertexStreamZeroData, UINT VertexStreamZeroStride) {
#ifdef _DEBUG
  ++m_iDrawCallCount;
#endif

  m_CurrentState.m_IndexData = NULL;
  m_CurrentState.m_StreamData[0] = NULL;
  return (m_lpD3DDev->DrawIndexedPrimitiveUP(
      PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount,
      pIndexData, IndexDataFormat, pVertexStreamZeroData,
      VertexStreamZeroStride));
}

#ifdef _DEBUG
void CStateManager::ResetDrawCallCounter() {
  m_iLastDrawCallCount = m_iDrawCallCount;
  m_iDrawCallCount = 0;
}

int CStateManager::GetDrawCallCount() const { return m_iLastDrawCallCount; }
#endif
