#pragma once

#include "AudioLib/Type.h"

#include "ParticleSystemData.h"
#include "EffectMesh.h"
#include "SimpleLightData.h"

class CEffectData {
public:
  typedef std::vector<CParticleSystemData *> TParticleVector;
  typedef std::vector<CEffectMeshScript *> TMeshVector;
  typedef std::vector<CLightData *> TLightVector;

public:
  CEffectData();
  virtual ~CEffectData();

  void Clear();
  bool LoadScript(const char *c_szFileName);
  bool LoadSoundScriptData(const char *c_szFileName);

  DWORD GetParticleCount();
  CParticleSystemData *GetParticlePointer(DWORD dwPosition);

  DWORD GetMeshCount();
  CEffectMeshScript *GetMeshPointer(DWORD dwPosition);

  DWORD GetLightCount();
  CLightData *GetLightPointer(DWORD dwPosition);

  NSound::TSoundInstanceVector *GetSoundInstanceVector();

  float GetBoundingSphereRadius();
  D3DXVECTOR3 GetBoundingSpherePosition();

  const char *GetFileName() const;

protected:
  void __ClearParticleDataVector();
  void __ClearLightDataVector();
  void __ClearMeshDataVector();

  // FIXME : 이 부분은 그다지 맘에 들지 않는다. 좋은 아이디어를 찾아내어 고치자.
  //         상위가 (특화된) 상위의 인터페이스 때문에 모양이 바뀌어야 한다는
  //         것은 옳지 못하다. - [levites]
  virtual CParticleSystemData *AllocParticle();
  virtual CEffectMeshScript *AllocMesh();
  virtual CLightData *AllocLight();

protected:
  TParticleVector m_ParticleVector;
  TMeshVector m_MeshVector;
  TLightVector m_LightVector;
  NSound::TSoundInstanceVector m_SoundInstanceVector;

  float m_fBoundingSphereRadius;
  D3DXVECTOR3 m_v3BoundingSpherePosition;

  std::string m_strFileName;

public:
  static void DestroySystem();

  static CEffectData *New();
  static void Delete(CEffectData *pkData);

  static CDynamicPool<CEffectData> ms_kPool;
};