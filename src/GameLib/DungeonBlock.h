#pragma once

#include "EterLib/ResourceManager.h"
#include "EterLib/GrpObjectInstance.h"
#include "EterGrnLib/ModelInstance.h"
#include "EterGrnLib/Thing.h"

class CDungeonModelInstance;

class CDungeonBlock : public CGraphicObjectInstance {
public:
  typedef std::vector<CDungeonModelInstance *> TModelInstanceContainer;
  enum { ID = THING_OBJECT };
  int GetType() const { return ID; }

public:
  CDungeonBlock();
  virtual ~CDungeonBlock();

  void Destroy();

  void BuildBoundingSphere();
  bool Load(const char *c_szFileName);

  bool Intersect(float *pfu, float *pfv, float *pft);
  void GetBoundBox(D3DXVECTOR3 *pv3Min, D3DXVECTOR3 *pv3Max);

  void Update();
  void Render();

  bool GetBoundingSphere(D3DXVECTOR3 &v3Center, float &fRadius);
  void OnUpdateCollisionData(const CStaticCollisionDataVector *pscdVector);
  void OnUpdateHeighInstance(CAttributeInstance *pAttributeInstance);
  bool OnGetObjectHeight(float fX, float fY, float *pfHeight);

  void OnRender() {}
  void OnBlendRender() {}
  void OnRenderToShadowMap() {}
  void OnRenderShadow();
  void OnRenderPCBlocker() {}

protected:
  void __Initialize();

protected:
  D3DXVECTOR3 m_v3Center;
  float m_fRadius;

  CGraphicThing *m_pThing;
  TModelInstanceContainer m_ModelInstanceContainer;
  CGraphicVertexBuffer m_kDeformableVertexBuffer;
};
