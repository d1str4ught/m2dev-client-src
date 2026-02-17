#pragma once

#include "Eterlib/GrpScreen.h"
#include "Eterlib/GrpImageInstance.h"
#include "EffectElementBaseInstance.h"
#include "FrameController.h"
#include "EffectMesh.h"

class CEffectMeshInstance : public CEffectElementBaseInstance {
public:
  // NOTE : Mesh 단위 텍스춰 데이타의 인스턴스이다.
  typedef struct STextureInstance {
    CFrameController TextureFrameController;
    std::vector<CGraphicImageInstance *> TextureInstanceVector;
  } TTextureInstance;

public:
  CEffectMeshInstance();
  virtual ~CEffectMeshInstance();

public:
  static void DestroySystem();

  static CEffectMeshInstance *New();
  static void Delete(CEffectMeshInstance *pkMeshInstance);

  static CDynamicPool<CEffectMeshInstance> ms_kPool;

protected:
  void OnSetDataPointer(CEffectElementBase *pElement);

  void OnInitialize();
  void OnDestroy();

  bool OnUpdate(float fElapsedTime);
  void OnRender();

  BOOL isActive();

protected:
  CEffectMeshScript *m_pMeshScript;
  CEffectMesh *m_pEffectMesh;

  CFrameController m_MeshFrameController;
  std::vector<TTextureInstance> m_TextureInstanceVector;

  CEffectMesh::TRef m_roMesh;
};