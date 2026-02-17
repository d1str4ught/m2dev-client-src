#pragma once

#include "Eterlib/GrpObjectInstance.h"
#include "Eterlib/Pool.h"
#include "AudioLib/Type.h"

#include "StdAfx.h"
#include "EffectElementBaseInstance.h"
#include "EffectData.h"
#include "EffectMeshInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"

class CEffectInstance : public CGraphicObjectInstance {
public:
  typedef std::vector<CEffectElementBaseInstance *>
      TEffectElementInstanceVector;

  enum { ID = EFFECT_OBJECT };
  int GetType() const { return CEffectInstance::ID; }

  bool GetBoundingSphere(D3DXVECTOR3 &v3Center, float &fRadius);

  static void DestroySystem();

  static CEffectInstance *New();
  static void Delete(CEffectInstance *pkEftInst);

  static void ResetRenderingEffectCount();
  static int GetRenderingEffectCount();

public:
  CEffectInstance();
  virtual ~CEffectInstance();

  bool LessRenderOrder(CEffectInstance *pkEftInst);

  void SetEffectDataPointer(CEffectData *pEffectData);

  // Returns the pointer to the effect data associated with this instance.
  CEffectData *GetEffectDataPointer() const { return m_pkEftData; }

  void Clear();
  BOOL isAlive();
  void SetActive();
  void SetDeactive();
  void SetGlobalMatrix(const D3DXMATRIX &c_rmatGlobal);
  void UpdateSound();
  void OnUpdate();
  void OnRender();
  void OnBlendRender() {}       // Not used
  void OnRenderToShadowMap() {} // Not used
  void OnRenderShadow() {}      // Not used
  void OnRenderPCBlocker() {}   // Not used

protected:
  void __Initialize();

  void __SetParticleData(CParticleSystemData *pData);
  void __SetMeshData(CEffectMeshScript *pMesh);
  void __SetLightData(CLightData *pData);

  virtual void
  OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector) {
  } // Not used
  virtual void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance) {}
  virtual bool OnGetObjectHeight(float fX, float fY, float *pfHeight) {
    return false;
  }

protected:
  BOOL m_isAlive;
  DWORD m_dwFrame;
  D3DXMATRIX m_matGlobal;

  CEffectData *m_pkEftData;

  std::vector<CParticleSystemInstance *> m_ParticleInstanceVector;
  std::vector<CEffectMeshInstance *> m_MeshInstanceVector;
  std::vector<CLightInstance *> m_LightInstanceVector;

  NSound::TSoundInstanceVector *m_pSoundInstanceVector;

  float m_fBoundingSphereRadius;
  D3DXVECTOR3 m_v3BoundingSpherePosition;

  float m_fLastTime;

public:
  static CDynamicPool<CEffectInstance> ms_kPool;
  static int ms_iRenderingEffectCount;
};
