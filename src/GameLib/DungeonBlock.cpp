#include "StdAfx.h"
#include "DungeonBlock.h"

#include "EterLib/StateManager.h"

class CDungeonModelInstance : public CGrannyModelInstance {
public:
  CDungeonModelInstance() {}
  virtual ~CDungeonModelInstance() {}

  void RenderDungeonBlock() {
    if (IsEmpty())
      return;

    STATEMANAGER.SetVertexDeclaration(ms_pnt2VS);
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf =
        m_pModel->GetPNTD3DVertexBuffer();
    if (lpd3dRigidPNTVtxBuf) {
      STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNT2Vertex));
      RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID,
                                       CGrannyMaterial::TYPE_BLEND_PNT);
    }
  }

  void RenderDungeonBlockShadow() {
    if (IsEmpty())
      return;

    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
    STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

    STATEMANAGER.SetVertexDeclaration(ms_pnt2VS);
    LPDIRECT3DVERTEXBUFFER9 lpd3dRigidPNTVtxBuf =
        m_pModel->GetPNTD3DVertexBuffer();
    if (lpd3dRigidPNTVtxBuf) {
      STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNT2Vertex));
      RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID,
                                       CGrannyMaterial::TYPE_BLEND_PNT);
    }

    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
    STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
    STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
  }
};

struct FUpdate {
  float fElapsedTime;
  D3DXMATRIX *pmatWorld;
  void operator()(CGrannyModelInstance *pInstance) {
    pInstance->Update(CGrannyModelInstance::ANIFPS_MIN);
    pInstance->UpdateLocalTime(fElapsedTime);
    pInstance->Deform(pmatWorld);
  }
};

void CDungeonBlock::Update() {
  Transform();

  FUpdate Update;
  Update.fElapsedTime = 0.0f;
  Update.pmatWorld = &m_worldMatrix;
  for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(),
           Update);
}

struct FRender {
  void operator()(CDungeonModelInstance *pInstance) {
    pInstance->RenderDungeonBlock();
  }
};

void CDungeonBlock::Render() {
  //	if (!isShow())
  //		return;

  for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(),
           FRender());
}

struct FRenderShadow {
  void operator()(CDungeonModelInstance *pInstance) {
    pInstance->RenderDungeonBlockShadow();
  }
};

void CDungeonBlock::OnRenderShadow() {
  for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(),
           FRenderShadow());
}

struct FBoundBox {
  D3DXVECTOR3 *m_pv3Min;
  D3DXVECTOR3 *m_pv3Max;

  FBoundBox(D3DXVECTOR3 *pv3Min, D3DXVECTOR3 *pv3Max) {
    m_pv3Min = pv3Min;
    m_pv3Max = pv3Max;
  }
  void operator()(CGrannyModelInstance *pInstance) {
    pInstance->GetBoundBox(m_pv3Min, m_pv3Max);
  }
};

bool CDungeonBlock::GetBoundingSphere(D3DXVECTOR3 &v3Center, float &fRadius) {
  v3Center = m_v3Center;
  fRadius = m_fRadius;
  D3DXVec3TransformCoord(&v3Center, &v3Center, &GetTransform());
  return true;
}

void CDungeonBlock::OnUpdateCollisionData(
    const CStaticCollisionDataVector *pscdVector) {
  assert(pscdVector);
  CStaticCollisionDataVector::const_iterator it;
  for (it = pscdVector->begin(); it != pscdVector->end(); ++it) {
    AddCollision(&(*it), &GetTransform());
  }
}

void CDungeonBlock::OnUpdateHeighInstance(
    CAttributeInstance *pAttributeInstance) {
  assert(pAttributeInstance);
  SetHeightInstance(pAttributeInstance);
}

bool CDungeonBlock::OnGetObjectHeight(float fX, float fY, float *pfHeight) {
  if (m_pHeightAttributeInstance &&
      m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight))
    return true;
  return false;
}

void CDungeonBlock::BuildBoundingSphere() {
  D3DXVECTOR3 v3Min, v3Max;
  for_each(m_ModelInstanceContainer.begin(), m_ModelInstanceContainer.end(),
           FBoundBox(&v3Min, &v3Max));

  m_v3Center = (v3Min + v3Max) * 0.5f;
  const auto vv = (v3Max - v3Min);
  m_fRadius =
      D3DXVec3Length(&vv) * 0.5f + 150.0f; // extra length for attached objects
}

bool CDungeonBlock::Intersect(float *pfu, float *pfv, float *pft) {
  TModelInstanceContainer::iterator itor = m_ModelInstanceContainer.begin();
  for (; itor != m_ModelInstanceContainer.end(); ++itor) {
    CDungeonModelInstance *pInstance = *itor;
    if (pInstance->Intersect(&CGraphicObjectInstance::GetTransform(), pfu, pfv,
                             pft))
      return true;
  }

  return false;
}

void CDungeonBlock::GetBoundBox(D3DXVECTOR3 *pv3Min, D3DXVECTOR3 *pv3Max) {
  pv3Min->x = +10000000.0f;
  pv3Min->y = +10000000.0f;
  pv3Min->z = +10000000.0f;
  pv3Max->x = -10000000.0f;
  pv3Max->y = -10000000.0f;
  pv3Max->z = -10000000.0f;

  TModelInstanceContainer::iterator itor = m_ModelInstanceContainer.begin();
  for (; itor != m_ModelInstanceContainer.end(); ++itor) {
    CDungeonModelInstance *pInstance = *itor;

    D3DXVECTOR3 v3Min;
    D3DXVECTOR3 v3Max;
    pInstance->GetBoundBox(&v3Min, &v3Max);

    pv3Min->x = std::min(v3Min.x, pv3Min->x);
    pv3Min->y = std::min(v3Min.x, pv3Min->y);
    pv3Min->z = std::min(v3Min.x, pv3Min->z);
    pv3Max->x = std::max(v3Max.x, pv3Max->x);
    pv3Max->y = std::max(v3Max.x, pv3Max->y);
    pv3Max->z = std::max(v3Max.x, pv3Max->z);
  }
}

bool CDungeonBlock::Load(const char *c_szFileName) {
  Destroy();

  m_pThing = (CGraphicThing *)CResourceManager::Instance().GetResourcePointer(
      c_szFileName);

  m_pThing->AddReference();
  if (m_pThing->GetModelCount() <= 0) {
    TraceError("CDungeonBlock::Load(filename=%s) - model count is %d\n",
               c_szFileName, m_pThing->GetModelCount());
    return false;
  }

  m_ModelInstanceContainer.reserve(m_pThing->GetModelCount());

  for (int i = 0; i < m_pThing->GetModelCount(); ++i) {
    CDungeonModelInstance *pModelInstance = new CDungeonModelInstance;
    pModelInstance->SetMainModelPointer(m_pThing->GetModelPointer(i),
                                        &m_kDeformableVertexBuffer);
    DWORD dwVertexCount = pModelInstance->GetVertexCount();
    m_kDeformableVertexBuffer.Destroy();
    m_kDeformableVertexBuffer.Create(dwVertexCount,
                                     D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,
                                     D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT);
    m_ModelInstanceContainer.push_back(pModelInstance);
  }

  return true;
}

void CDungeonBlock::__Initialize() {
  m_v3Center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_fRadius = 0.0f;

  m_pThing = NULL;
}

void CDungeonBlock::Destroy() {
  if (m_pThing) {
    m_pThing->Release();
    m_pThing = NULL;
  }

  stl_wipe(m_ModelInstanceContainer);

  __Initialize();
}

CDungeonBlock::CDungeonBlock() { __Initialize(); }
CDungeonBlock::~CDungeonBlock() { Destroy(); }
