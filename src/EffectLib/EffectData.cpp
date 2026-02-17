#include "StdAfx.h"
#include "EffectData.h"

CDynamicPool<CEffectData> CEffectData::ms_kPool;

CEffectData *CEffectData::New() { return ms_kPool.Alloc(); }

void CEffectData::Delete(CEffectData *pkData) {
  pkData->Clear();
  ms_kPool.Free(pkData);
}

void CEffectData::DestroySystem() {
  ms_kPool.Destroy();

  CParticleSystemData::DestroySystem();
  CEffectMeshScript::DestroySystem();
  CLightData::DestroySystem();
}

bool CEffectData::LoadScript(const char *c_szFileName) {
  m_strFileName = c_szFileName;
  CFileNameHelper::StringPath(m_strFileName);

  CTextFileLoader TextFileLoader;
  if (!TextFileLoader.Load(c_szFileName))
    return false;

  TextFileLoader.SetTop();

  if (!TextFileLoader.GetTokenFloat("boundingsphereradius",
                                    &m_fBoundingSphereRadius)) {
    m_fBoundingSphereRadius = 0.0f;
  }

  if (!TextFileLoader.GetTokenVector3("boundingsphereposition",
                                      &m_v3BoundingSpherePosition)) {
    m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y =
        m_v3BoundingSpherePosition.z = 0.0f;
  }

  for (DWORD i = 0; i < TextFileLoader.GetChildNodeCount(); ++i) {
    if (!TextFileLoader.SetChildNode(i))
      continue;

    std::string strName;

    if (!TextFileLoader.GetCurrentNodeName(&strName))
      continue;

    if (0 == strName.compare("mesh")) {
      CEffectMeshScript *pMesh = AllocMesh();
      pMesh->Clear();
      pMesh->LoadScript(TextFileLoader);
    } else if (0 == strName.compare("particle")) {
      CParticleSystemData *pParticleSystemData = AllocParticle();
      pParticleSystemData->Clear();
      pParticleSystemData->LoadScript(TextFileLoader);
    } else if (0 == strName.compare("light")) {
      CLightData *pLightData = AllocLight();
      pLightData->Clear();
      pLightData->LoadScript(TextFileLoader);
    }

    TextFileLoader.SetParentNode();
  }

  // Load Sound
  std::string strPathHeader = "d:/ymir work/";
  std::string strNoExtensionName = CFileNameHelper::NoExtension(m_strFileName);
  int iPos = strNoExtensionName.find(strPathHeader.c_str());
  if (iPos >= 0)
    if (strNoExtensionName.size() > strPathHeader.size()) {
      std::string strSoundFileName;
      strSoundFileName = "sound/";
      strSoundFileName += &strNoExtensionName[strPathHeader.size()];
      strSoundFileName += ".mss";

      LoadSoundScriptData(strSoundFileName.c_str());
    }

  return true;
}

bool CEffectData::LoadSoundScriptData(const char *c_szFileName) {
  NSound::TSoundDataVector SoundDataVector;

  if (LoadSoundInformationPiece(c_szFileName, SoundDataVector)) {
    DataToInstance(SoundDataVector, &m_SoundInstanceVector);
    return false;
  }

  return true;
}

CParticleSystemData *CEffectData::AllocParticle() {
  CParticleSystemData *pParticle = CParticleSystemData::New();
  m_ParticleVector.push_back(pParticle);
  return pParticle;
}

CEffectMeshScript *CEffectData::AllocMesh() {
  CEffectMeshScript *pMesh = CEffectMeshScript::New();
  m_MeshVector.push_back(pMesh);
  return pMesh;
}

CLightData *CEffectData::AllocLight() {
  CLightData *pLight = CLightData::New();
  m_LightVector.push_back(pLight);
  return pLight;
}

DWORD CEffectData::GetLightCount() { return m_LightVector.size(); }

CLightData *CEffectData::GetLightPointer(DWORD dwPosition) {
  assert(dwPosition < m_LightVector.size());
  return m_LightVector[dwPosition];
}

DWORD CEffectData::GetParticleCount() { return m_ParticleVector.size(); }
CParticleSystemData *CEffectData::GetParticlePointer(DWORD dwPosition) {
  if (dwPosition < m_ParticleVector.size())
    return m_ParticleVector[dwPosition];
  else {
    assert(false);
    return NULL;
  }
}

DWORD CEffectData::GetMeshCount() { return m_MeshVector.size(); }
CEffectMeshScript *CEffectData::GetMeshPointer(DWORD dwPosition) {
  assert(dwPosition < m_MeshVector.size());
  return m_MeshVector[dwPosition];
}

NSound::TSoundInstanceVector *CEffectData::GetSoundInstanceVector() {
  return &m_SoundInstanceVector;
}

float CEffectData::GetBoundingSphereRadius() { return m_fBoundingSphereRadius; }

D3DXVECTOR3 CEffectData::GetBoundingSpherePosition() {
  return m_v3BoundingSpherePosition;
}

const char *CEffectData::GetFileName() const { return m_strFileName.c_str(); }

void CEffectData::__ClearParticleDataVector() {
  std::for_each(m_ParticleVector.begin(), m_ParticleVector.end(),
                CParticleSystemData::Delete);
  m_ParticleVector.clear();
}

void CEffectData::__ClearLightDataVector() {
  std::for_each(m_LightVector.begin(), m_LightVector.end(), CLightData::Delete);
  m_LightVector.clear();
}

void CEffectData::__ClearMeshDataVector() {
  std::for_each(m_MeshVector.begin(), m_MeshVector.end(),
                CEffectMeshScript::Delete);
  m_MeshVector.clear();
}

void CEffectData::Clear() {
  m_fBoundingSphereRadius = 0.0f;
  m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y =
      m_v3BoundingSpherePosition.z = 0.0f;
  __ClearParticleDataVector();
  __ClearLightDataVector();
  __ClearMeshDataVector();
}

CEffectData::CEffectData() {
  m_fBoundingSphereRadius = 0.0f;
  m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y =
      m_v3BoundingSpherePosition.z = 0.0f;
}
CEffectData::~CEffectData() {}
