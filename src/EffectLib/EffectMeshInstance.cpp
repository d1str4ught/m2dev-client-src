#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "EterLib/ResourceManager.h"
#include "EffectMeshInstance.h"
#include "Eterlib/GrpMath.h"

CDynamicPool<CEffectMeshInstance> CEffectMeshInstance::ms_kPool;

void CEffectMeshInstance::DestroySystem() { ms_kPool.Destroy(); }

CEffectMeshInstance *CEffectMeshInstance::New() { return ms_kPool.Alloc(); }

void CEffectMeshInstance::Delete(CEffectMeshInstance *pkMeshInstance) {
  pkMeshInstance->Destroy();
  ms_kPool.Free(pkMeshInstance);
}

BOOL CEffectMeshInstance::isActive() {
  if (!CEffectElementBaseInstance::isActive())
    return FALSE;

  if (!m_MeshFrameController.isActive())
    return FALSE;

  for (DWORD j = 0; j < m_TextureInstanceVector.size(); ++j) {
    int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
    if (m_TextureInstanceVector[j].TextureFrameController.isActive(
            iCurrentFrame))
      return TRUE;
  }

  return FALSE;
}

bool CEffectMeshInstance::OnUpdate(float fElapsedTime) {
  if (!isActive())
    return false;

  if (m_MeshFrameController.isActive())
    m_MeshFrameController.Update(fElapsedTime);

  for (DWORD j = 0; j < m_TextureInstanceVector.size(); ++j) {
    int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
    if (m_TextureInstanceVector[j].TextureFrameController.isActive(
            iCurrentFrame))
      m_TextureInstanceVector[j].TextureFrameController.Update(fElapsedTime);
  }

  return true;
}

void CEffectMeshInstance::OnRender() {
  if (!isActive())
    return;

  CEffectMesh *pEffectMesh = m_roMesh.GetPointer();

  for (DWORD i = 0; i < pEffectMesh->GetMeshCount(); ++i) {
    assert(i < m_TextureInstanceVector.size());

    CFrameController &rTextureFrameController =
        m_TextureInstanceVector[i].TextureFrameController;
    if (!rTextureFrameController.isActive(
            m_MeshFrameController.GetCurrentFrame()))
      continue;

    int iBillboardType = m_pMeshScript->GetBillboardType(i);

    D3DXMATRIX m_matWorld;
    D3DXMatrixIdentity(&m_matWorld);

    switch (iBillboardType) {
    case MESH_BILLBOARD_TYPE_ALL: {
      D3DXMATRIX matTemp;
      D3DXMatrixRotationX(&matTemp, 90.0f);
      D3DXMatrixInverse(&m_matWorld, NULL, &CScreen::GetViewMatrix());

      m_matWorld = matTemp * m_matWorld;
    } break;

    case MESH_BILLBOARD_TYPE_Y: {
      D3DXMATRIX matTemp;
      D3DXMatrixIdentity(&matTemp);

      D3DXMatrixInverse(&matTemp, NULL, &CScreen::GetViewMatrix());
      m_matWorld._11 = matTemp._11;
      m_matWorld._12 = matTemp._12;
      m_matWorld._21 = matTemp._21;
      m_matWorld._22 = matTemp._22;
    } break;

    case MESH_BILLBOARD_TYPE_MOVE: {
      D3DXVECTOR3 Position;
      m_pMeshScript->GetPosition(m_fLocalTime, Position);
      D3DXVECTOR3 LastPosition;
      m_pMeshScript->GetPosition(
          m_fLocalTime - CTimer::Instance().GetElapsedSecond(), LastPosition);
      Position -= LastPosition;
      if (D3DXVec3LengthSq(&Position) > 0.001f) {
        D3DXVec3Normalize(&Position, &Position);
        D3DXQUATERNION q =
            SafeRotationNormalizedArc(D3DXVECTOR3(0.0f, -1.0f, 0.0f), Position);
        D3DXMatrixRotationQuaternion(&m_matWorld, &q);
      }
    } break;
    }

    if (!m_pMeshScript->isBlendingEnable(i)) {
      STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    } else {
      int iBlendingSrcType = m_pMeshScript->GetBlendingSrcType(i);
      int iBlendingDestType = m_pMeshScript->GetBlendingDestType(i);
      STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
      STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, iBlendingSrcType);
      STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, iBlendingDestType);
    }

    D3DXVECTOR3 Position;
    m_pMeshScript->GetPosition(m_fLocalTime, Position);
    m_matWorld._41 = Position.x;
    m_matWorld._42 = Position.y;
    m_matWorld._43 = Position.z;
    m_matWorld = m_matWorld * *mc_pmatLocal;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

    BYTE byType;
    D3DXCOLOR Color(1.0f, 1.0f, 1.0f, 1.0f);
    if (m_pMeshScript->GetColorOperationType(i, &byType))
      STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, byType);
    m_pMeshScript->GetColorFactor(i, &Color);

    TTimeEventTableFloat *TableAlpha;

    float fAlpha = 1.0f;
    if (m_pMeshScript->GetTimeTableAlphaPointer(i, &TableAlpha) &&
        !TableAlpha->empty())
      fAlpha = GetTimeEventBlendValue(m_fLocalTime, *TableAlpha);

    // Render //
    CEffectMesh::TEffectMeshData *pMeshData =
        pEffectMesh->GetMeshDataPointer(i);

    assert(m_MeshFrameController.GetCurrentFrame() <
           pMeshData->EffectFrameDataVector.size());
    CEffectMesh::TEffectFrameData &rFrameData =
        pMeshData
            ->EffectFrameDataVector[m_MeshFrameController.GetCurrentFrame()];

    DWORD dwcurTextureFrame = rTextureFrameController.GetCurrentFrame();
    if (dwcurTextureFrame <
        m_TextureInstanceVector[i].TextureInstanceVector.size()) {
      CGraphicImageInstance *pImageInstance =
          m_TextureInstanceVector[i].TextureInstanceVector[dwcurTextureFrame];
      STATEMANAGER.SetTexture(
          0, pImageInstance->GetTexturePointer()->GetD3DTexture());
    }

    Color.a = fAlpha * rFrameData.fVisibility;
    STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, DWORD(Color));
    STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
    STATEMANAGER.DrawPrimitiveUP(
        D3DPT_TRIANGLELIST, rFrameData.dwIndexCount / 3,
        &rFrameData.PDTVertexVector[0], sizeof(TPTVertex));
    // Render //
  }
}

void CEffectMeshInstance::OnSetDataPointer(CEffectElementBase *pElement) {
  CEffectMeshScript *pMesh = (CEffectMeshScript *)pElement;
  m_pMeshScript = pMesh;

  const char *c_szMeshFileName = pMesh->GetMeshFileName();

  m_pEffectMesh =
      (CEffectMesh *)CResourceManager::Instance().GetResourcePointer(
          c_szMeshFileName);

  if (!m_pEffectMesh)
    return;

  m_roMesh.SetPointer(m_pEffectMesh);

  m_MeshFrameController.Clear();
  m_MeshFrameController.SetMaxFrame(m_roMesh.GetPointer()->GetFrameCount());
  m_MeshFrameController.SetFrameTime(pMesh->GetMeshAnimationFrameDelay());
  m_MeshFrameController.SetLoopFlag(pMesh->isMeshAnimationLoop());
  m_MeshFrameController.SetLoopCount(pMesh->GetMeshAnimationLoopCount());
  m_MeshFrameController.SetStartFrame(0);

  m_TextureInstanceVector.clear();
  m_TextureInstanceVector.resize(m_pEffectMesh->GetMeshCount());
  for (DWORD j = 0; j < m_TextureInstanceVector.size(); ++j) {
    CEffectMeshScript::TMeshData *pMeshData;
    if (!m_pMeshScript->GetMeshDataPointer(j, &pMeshData))
      continue;

    CEffectMesh *pkEftMesh = m_roMesh.GetPointer();

    if (!pkEftMesh)
      continue;

    std::vector<CGraphicImage *> *pTextureVector =
        pkEftMesh->GetTextureVectorPointer(j);
    if (!pTextureVector)
      continue;

    std::vector<CGraphicImage *> &rTextureVector = *pTextureVector;

    CFrameController &rFrameController =
        m_TextureInstanceVector[j].TextureFrameController;
    rFrameController.Clear();
    rFrameController.SetMaxFrame(rTextureVector.size());
    rFrameController.SetFrameTime(pMeshData->fTextureAnimationFrameDelay);
    rFrameController.SetLoopFlag(pMeshData->bTextureAnimationLoopEnable);
    rFrameController.SetStartFrame(pMeshData->dwTextureAnimationStartFrame);

    std::vector<CGraphicImageInstance *> &rImageInstanceVector =
        m_TextureInstanceVector[j].TextureInstanceVector;
    rImageInstanceVector.clear();
    rImageInstanceVector.reserve(rTextureVector.size());
    for (std::vector<CGraphicImage *>::iterator itor = rTextureVector.begin();
         itor != rTextureVector.end(); ++itor) {
      CGraphicImage *pImage = *itor;
      CGraphicImageInstance *pImageInstance =
          CGraphicImageInstance::ms_kPool.Alloc();
      pImageInstance->SetImagePointer(pImage);
      rImageInstanceVector.push_back(pImageInstance);
    }
  }
}

void CEffectMeshInstance_DeleteImageInstance(
    CGraphicImageInstance *pkInstance) {
  CGraphicImageInstance::ms_kPool.Free(pkInstance);
}

void CEffectMeshInstance_DeleteTextureInstance(
    CEffectMeshInstance::TTextureInstance &rkInstance) {
  std::vector<CGraphicImageInstance *> &rVector =
      rkInstance.TextureInstanceVector;
  for_each(rVector.begin(), rVector.end(),
           CEffectMeshInstance_DeleteImageInstance);
  rVector.clear();
}

void CEffectMeshInstance::OnInitialize() {}

void CEffectMeshInstance::OnDestroy() {
  for_each(m_TextureInstanceVector.begin(), m_TextureInstanceVector.end(),
           CEffectMeshInstance_DeleteTextureInstance);
  m_TextureInstanceVector.clear();
  m_roMesh.SetPointer(NULL);
}

CEffectMeshInstance::CEffectMeshInstance() { Initialize(); }

CEffectMeshInstance::~CEffectMeshInstance() { Destroy(); }