#pragma once
#include "EffectElementBaseInstance.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "Eterlib/GrpScreen.h"
#include "Eterlib/StateManager.h"
#include "EterLib/GrpImageInstance.h"
#include "EmitterProperty.h"

class CParticleSystemInstance : public CEffectElementBaseInstance {
public:
  static void DestroySystem();

  static CParticleSystemInstance *New();
  static void Delete(CParticleSystemInstance *pkData);

  static CDynamicPool<CParticleSystemInstance> ms_kPool;

public:
  template <typename T> inline void ForEachParticleRendering(T &FunObj) {
    DWORD dwFrameIndex;
    for (dwFrameIndex = 0; dwFrameIndex < m_kVct_pkImgInst.size();
         dwFrameIndex++) {
      STATEMANAGER.SetTexture(0, m_kVct_pkImgInst[dwFrameIndex]
                                     ->GetTextureReference()
                                     .GetD3DTexture());
      TParticleInstanceList::iterator itor =
          m_ParticleInstanceListVector[dwFrameIndex].begin();
      for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end(); ++itor) {
        if (!InFrustum(*itor))
          continue;

        FunObj(*itor);
      }
    }
  }

  CParticleSystemInstance();
  virtual ~CParticleSystemInstance();

  void OnSetDataPointer(CEffectElementBase *pElement);

  void CreateParticles(float fElapsedTime);

  inline bool InFrustum(CParticleInstance *pInstance) {
    if (m_pParticleProperty->m_bAttachFlag)
      return CScreen::GetFrustum().ViewVolumeTest(
                 Vector3d(pInstance->m_v3Position.x + mc_pmatLocal->_41,
                          pInstance->m_v3Position.y + mc_pmatLocal->_42,
                          pInstance->m_v3Position.z + mc_pmatLocal->_43),
                 pInstance->GetRadiusApproximation()) != VS_OUTSIDE;
    else
      return CScreen::GetFrustum().ViewVolumeTest(
                 Vector3d(pInstance->m_v3Position.x, pInstance->m_v3Position.y,
                          pInstance->m_v3Position.z),
                 pInstance->GetRadiusApproximation()) != VS_OUTSIDE;
  }

  DWORD GetEmissionCount();

protected:
  void OnInitialize();
  void OnDestroy();

  bool OnUpdate(float fElapsedTime);
  void OnRender();

protected:
  float m_fEmissionResidue;

  DWORD m_dwCurrentEmissionCount;
  int m_iLoopCount;

  typedef std::list<CParticleInstance *> TParticleInstanceList;
  typedef std::vector<TParticleInstanceList> TParticleInstanceListVector;
  TParticleInstanceListVector m_ParticleInstanceListVector;

  typedef std::vector<CGraphicImageInstance *> TImageInstanceVector;
  TImageInstanceVector m_kVct_pkImgInst;

  CParticleSystemData *m_pData;

  CParticleProperty *m_pParticleProperty;
  CEmitterProperty *m_pEmitterProperty;
};
