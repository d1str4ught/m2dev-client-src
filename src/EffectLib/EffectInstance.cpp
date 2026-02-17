#include "StdAfx.h"
#include "EffectInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"

#include "EterBase/Stl.h"
#include "EterLib/StateManager.h"
#include "AudioLib/SoundEngine.h"

CDynamicPool<CEffectInstance> CEffectInstance::ms_kPool;
int CEffectInstance::ms_iRenderingEffectCount = 0;

bool CEffectInstance::LessRenderOrder(CEffectInstance *pkEftInst) {
  return (m_pkEftData < pkEftInst->m_pkEftData);
}

void CEffectInstance::ResetRenderingEffectCount() {
  ms_iRenderingEffectCount = 0;
}

int CEffectInstance::GetRenderingEffectCount() {
  return ms_iRenderingEffectCount;
}

CEffectInstance *CEffectInstance::New() {
  CEffectInstance *pkEftInst = ms_kPool.Alloc();
  return pkEftInst;
}

void CEffectInstance::Delete(CEffectInstance *pkEftInst) {
  pkEftInst->Clear();
  ms_kPool.Free(pkEftInst);
}

void CEffectInstance::DestroySystem() {
  ms_kPool.Destroy();

  CParticleSystemInstance::DestroySystem();
  CEffectMeshInstance::DestroySystem();
  CLightInstance::DestroySystem();
}

void CEffectInstance::UpdateSound() {
  if (m_pSoundInstanceVector) {
    SoundEngine::Instance().UpdateSoundInstance(
        m_matGlobal._41, m_matGlobal._42, m_matGlobal._43, m_dwFrame,
        m_pSoundInstanceVector, false);
    // NOTE : 매트릭스에서 위치를 직접 얻어온다 - [levites]
  }
  ++m_dwFrame;
}

struct FEffectUpdator {
  BOOL isAlive;
  float fElapsedTime;
  FEffectUpdator(float fElapsedTime)
      : isAlive(FALSE), fElapsedTime(fElapsedTime) {}
  void operator()(CEffectElementBaseInstance *pInstance) {
    if (pInstance->Update(fElapsedTime)) [[likely]]
      isAlive = TRUE;
  }
};

void CEffectInstance::OnUpdate() {
  Transform();

  FEffectUpdator f(CTimer::Instance().GetCurrentSecond() - m_fLastTime);
  f = std::for_each(m_ParticleInstanceVector.begin(),
                    m_ParticleInstanceVector.end(), f);
  f = std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(),
                    f);
  f = std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(),
                    f);
  m_isAlive = f.isAlive;

  m_fLastTime = CTimer::Instance().GetCurrentSecond();
}

void CEffectInstance::OnRender() {
  STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

  STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
  STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
  /////

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  std::for_each(m_ParticleInstanceVector.begin(),
                m_ParticleInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::Render));
  std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::Render));

  /////
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
  STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
  STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);

  ++ms_iRenderingEffectCount;
}

void CEffectInstance::SetGlobalMatrix(const D3DXMATRIX &c_rmatGlobal) {
  m_matGlobal = c_rmatGlobal;
}

BOOL CEffectInstance::isAlive() { return m_isAlive; }

void CEffectInstance::SetActive() {
  std::for_each(m_ParticleInstanceVector.begin(),
                m_ParticleInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetActive));
  std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetActive));
  std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetActive));
}

void CEffectInstance::SetDeactive() {
  std::for_each(m_ParticleInstanceVector.begin(),
                m_ParticleInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetDeactive));
  std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetDeactive));
  std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(),
                std::mem_fn(&CEffectElementBaseInstance::SetDeactive));
}

void CEffectInstance::__SetParticleData(CParticleSystemData *pData) {
  CParticleSystemInstance *pInstance = CParticleSystemInstance::New();
  pInstance->SetDataPointer(pData);
  pInstance->SetLocalMatrixPointer(&m_matGlobal);

  m_ParticleInstanceVector.push_back(pInstance);
}
void CEffectInstance::__SetMeshData(CEffectMeshScript *pMesh) {
  CEffectMeshInstance *pMeshInstance = CEffectMeshInstance::New();
  pMeshInstance->SetDataPointer(pMesh);
  pMeshInstance->SetLocalMatrixPointer(&m_matGlobal);

  m_MeshInstanceVector.push_back(pMeshInstance);
}

void CEffectInstance::__SetLightData(CLightData *pData) {
  CLightInstance *pInstance = CLightInstance::New();
  pInstance->SetDataPointer(pData);
  pInstance->SetLocalMatrixPointer(&m_matGlobal);

  m_LightInstanceVector.push_back(pInstance);
}

void CEffectInstance::SetEffectDataPointer(CEffectData *pEffectData) {
  m_isAlive = true;

  m_pkEftData = pEffectData;

  m_fLastTime = CTimer::Instance().GetCurrentSecond();
  m_fBoundingSphereRadius = pEffectData->GetBoundingSphereRadius();
  m_v3BoundingSpherePosition = pEffectData->GetBoundingSpherePosition();

  if (m_fBoundingSphereRadius > 0.0f)
    CGraphicObjectInstance::RegisterBoundingSphere();

  DWORD i;

  for (i = 0; i < pEffectData->GetParticleCount(); ++i) {
    CParticleSystemData *pParticle = pEffectData->GetParticlePointer(i);

    __SetParticleData(pParticle);
  }

  for (i = 0; i < pEffectData->GetMeshCount(); ++i) {
    CEffectMeshScript *pMesh = pEffectData->GetMeshPointer(i);

    __SetMeshData(pMesh);
  }

  for (i = 0; i < pEffectData->GetLightCount(); ++i) {
    CLightData *pLight = pEffectData->GetLightPointer(i);

    __SetLightData(pLight);
  }

  m_pSoundInstanceVector = pEffectData->GetSoundInstanceVector();
}

bool CEffectInstance::GetBoundingSphere(D3DXVECTOR3 &v3Center, float &fRadius) {
  v3Center.x = m_matGlobal._41 + m_v3BoundingSpherePosition.x;
  v3Center.y = m_matGlobal._42 + m_v3BoundingSpherePosition.y;
  v3Center.z = m_matGlobal._43 + m_v3BoundingSpherePosition.z;
  fRadius = m_fBoundingSphereRadius;
  return true;
}

void CEffectInstance::Clear() {
  if (!m_ParticleInstanceVector.empty()) {
    std::for_each(m_ParticleInstanceVector.begin(),
                  m_ParticleInstanceVector.end(),
                  CParticleSystemInstance::Delete);
    m_ParticleInstanceVector.clear();
  }

  if (!m_MeshInstanceVector.empty()) {
    std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(),
                  CEffectMeshInstance::Delete);
    m_MeshInstanceVector.clear();
  }

  if (!m_LightInstanceVector.empty()) {
    std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(),
                  CLightInstance::Delete);
    m_LightInstanceVector.clear();
  }

  __Initialize();
}

void CEffectInstance::__Initialize() {
  ReleaseAlwaysHidden();

  m_isAlive = FALSE;
  m_dwFrame = 0;
  m_pSoundInstanceVector = NULL;
  m_fBoundingSphereRadius = 0.0f;
  m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y =
      m_v3BoundingSpherePosition.z = 0.0f;

  m_pkEftData = NULL;

  D3DXMatrixIdentity(&m_matGlobal);
}

CEffectInstance::CEffectInstance() { __Initialize(); }
CEffectInstance::~CEffectInstance() {
  assert(m_ParticleInstanceVector.empty());
  assert(m_MeshInstanceVector.empty());
  assert(m_LightInstanceVector.empty());
}
