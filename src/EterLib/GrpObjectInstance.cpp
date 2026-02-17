#include "StdAfx.h"
#include "GrpObjectInstance.h"
#include "EterBase/Timer.h"

void CGraphicObjectInstance::OnInitialize() {
  ZeroMemory(m_abyPortalID, sizeof(m_abyPortalID));
}

void CGraphicObjectInstance::Clear() {
  if (m_CullingHandle) {
    CCullingManager::Instance().Unregister(m_CullingHandle);
    m_CullingHandle = NULL;
  }

  ClearHeightInstance();

  m_isVisible = TRUE;

  m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;
  m_v3Scale.x = m_v3Scale.y = m_v3Scale.z = 1.0f;
  // m_fRotation = 0.0f;
  m_fYaw = m_fPitch = m_fRoll = 0.0f;
  D3DXMatrixIdentity(&m_worldMatrix);

  ZeroMemory(m_abyPortalID, sizeof(m_abyPortalID));

  OnClear();
}

bool CGraphicObjectInstance::Render() {
  /*
  if (m_CullingHandle)
  {
          SpherePack * ps = m_CullingHandle->GetParent();
          CScreen s;
          s.SetColorOperation();
          //s.SetDiffuseColor(1,isShow()?1:0,0);
          //s.RenderCircle2d(m_CullingHandle->GetCenter().x,m_CullingHandle->GetCenter().y,m_CullingHandle->GetCenter().z,m_CullingHandle->GetRadius());
          s.SetDiffuseColor(1,isShow()?1:0,ps->HasSpherePackFlag(SPF_PARTIAL)?1:0);
          s.RenderCircle2d(ps->GetCenter().x,ps->GetCenter().y,ps->GetCenter().z,ps->GetRadius());
  }
  //*/
  if (!isShow())
    return false;

  OnRender();
  return true;
}

void CGraphicObjectInstance::BlendRender() {
  if (!isShow())
    return;

  OnBlendRender();
}

void CGraphicObjectInstance::RenderToShadowMap() {
  if (!isShow())
    return;

  OnRenderToShadowMap();
}

void CGraphicObjectInstance::RenderShadow() {
  if (!isShow())
    return;

  OnRenderShadow();
}

void CGraphicObjectInstance::RenderPCBlocker() {
  if (!isShow())
    return;

  OnRenderPCBlocker();
}

void CGraphicObjectInstance::Update() {
  OnUpdate();

  UpdateBoundingSphere();
}

void CGraphicObjectInstance::Deform() {
  if (!isShow())
    return;

  OnDeform();
}

void CGraphicObjectInstance::Transform() {
  m_worldMatrix = m_mRotation;

  m_worldMatrix._41 += m_v3Position.x;
  m_worldMatrix._42 += m_v3Position.y;
  m_worldMatrix._43 += m_v3Position.z;
}

const D3DXVECTOR3 &CGraphicObjectInstance::GetPosition() const {
  return m_v3Position;
}

const D3DXVECTOR3 &CGraphicObjectInstance::GetScale() const {
  return m_v3Scale;
}

float CGraphicObjectInstance::GetRotation() { return GetRoll(); }

float CGraphicObjectInstance::GetYaw() { return m_fYaw; }

float CGraphicObjectInstance::GetPitch() { return m_fPitch; }

float CGraphicObjectInstance::GetRoll() { return m_fRoll; }

D3DXMATRIX &CGraphicObjectInstance::GetTransform() { return m_worldMatrix; }

void CGraphicObjectInstance::SetRotationQuaternion(const D3DXQUATERNION &q) {
  D3DXMatrixRotationQuaternion(&m_mRotation, &q);
}

void CGraphicObjectInstance::SetRotationMatrix(const D3DXMATRIX &m) {
  m_mRotation = m;
}

void CGraphicObjectInstance::SetRotation(float fRotation) {
  m_fYaw = 0;
  m_fPitch = 0;
  m_fRoll = fRotation;

  D3DXMatrixRotationZ(&m_mRotation, D3DXToRadian(fRotation));
}

void CGraphicObjectInstance::SetRotation(float fYaw, float fPitch,
                                         float fRoll) {
  // m_fRotation = fRotation;
  m_fYaw = fYaw;
  m_fPitch = fPitch;
  m_fRoll = fRoll;

  D3DXMatrixRotationYawPitchRoll(&m_mRotation, D3DXToRadian(fYaw),
                                 D3DXToRadian(fPitch), D3DXToRadian(fRoll));
}

void CGraphicObjectInstance::SetPosition(float x, float y, float z) {
  m_v3Position.x = x;
  m_v3Position.y = y;
  m_v3Position.z = z;
}

void CGraphicObjectInstance::SetPosition(const D3DXVECTOR3 &newposition) {
  m_v3Position = newposition;
}

void CGraphicObjectInstance::SetScale(float x, float y, float z) {
  m_v3Scale.x = x;
  m_v3Scale.y = y;
  m_v3Scale.z = z;
}

void CGraphicObjectInstance::Show() { m_isVisible = true; }

void CGraphicObjectInstance::Hide() { m_isVisible = false; }

void CGraphicObjectInstance::ApplyAlwaysHidden() { m_isAlwaysHidden = true; }

void CGraphicObjectInstance::ReleaseAlwaysHidden() { m_isAlwaysHidden = false; }

bool CGraphicObjectInstance::isShow() { return m_isVisible; }

//

//////////////////////////////////////////////////////////////////////////

D3DXVECTOR4 &CGraphicObjectInstance::GetWTBBoxVertex(
    const unsigned char &c_rucNumTBBoxVertex) {
  return m_v4TBBox[c_rucNumTBBoxVertex];
}

bool CGraphicObjectInstance::isIntersect(const CRay &c_rRay, float *pu,
                                         float *pv, float *pt) {
  D3DXVECTOR3 v3Start, v3Dir;
  float fRayRange;
  c_rRay.GetStartPoint(&v3Start);
  c_rRay.GetDirection(&v3Dir, &fRayRange);

  TPosition posVertices[8];

  posVertices[0] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMin.y, m_v3TBBoxMin.z);
  posVertices[1] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMin.y, m_v3TBBoxMin.z);
  posVertices[2] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMax.y, m_v3TBBoxMin.z);
  posVertices[3] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMax.y, m_v3TBBoxMin.z);
  posVertices[4] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMin.y, m_v3TBBoxMax.z);
  posVertices[5] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMin.y, m_v3TBBoxMax.z);
  posVertices[6] = TPosition(m_v3TBBoxMin.x, m_v3TBBoxMax.y, m_v3TBBoxMax.z);
  posVertices[7] = TPosition(m_v3TBBoxMax.x, m_v3TBBoxMax.y, m_v3TBBoxMax.z);

  TIndex Indices[36] = {0, 1, 2, 1, 3, 2, 2, 0, 6, 0, 4, 6, 0, 1, 4, 1, 5, 4,
                        1, 3, 5, 3, 7, 5, 3, 2, 7, 2, 6, 7, 4, 5, 6, 5, 7, 6};

  int triCount = 12;
  WORD *pcurIdx = (WORD *)Indices;

  while (triCount--) {
    if (IntersectTriangle(v3Start, v3Dir, posVertices[pcurIdx[0]],
                          posVertices[pcurIdx[1]], posVertices[pcurIdx[2]], pu,
                          pv, pt)) {
      return true;
    }

    pcurIdx += 3;
  }

  return false;
}

CGraphicObjectInstance::CGraphicObjectInstance() {
  m_CullingHandle = 0;
  Initialize();
}

void CGraphicObjectInstance::Initialize() {
  if (m_CullingHandle)
    CCullingManager::Instance().Unregister(m_CullingHandle);
  m_CullingHandle = 0;

  m_pHeightAttributeInstance = NULL;

  m_isVisible = TRUE;

  m_BlockCamera = false;
  m_isAlwaysHidden = false;

  m_v3Position.x = m_v3Position.y = m_v3Position.z = 0.0f;
  m_v3Scale.x = m_v3Scale.y = m_v3Scale.z = 1.0f;
  m_fYaw = m_fPitch = m_fRoll = 0.0f;

  D3DXMatrixIdentity(&m_worldMatrix);
  D3DXMatrixIdentity(&m_mRotation);

  m_v3TBBoxMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_v3TBBoxMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_v3BBoxMin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
  m_v3BBoxMax = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

  for (int i = 0; i < 8; ++i)
    m_v4TBBox[i] = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

  memset(m_abyPortalID, 0, sizeof(m_abyPortalID));

  ClearCollision();
  OnInitialize();
}

CGraphicObjectInstance::~CGraphicObjectInstance() { Initialize(); }

void CGraphicObjectInstance::UpdateBoundingSphere() {
  if (m_CullingHandle) {
    Vector3d center;
    float radius;
    GetBoundingSphere(center, radius);
    if (radius != m_CullingHandle->GetRadius())
      m_CullingHandle->NewPosRadius(center, radius);
    else
      m_CullingHandle->NewPos(center);
  }
}

void CGraphicObjectInstance::RegisterBoundingSphere() {
  if (m_CullingHandle)
    CCullingManager::Instance().Unregister(m_CullingHandle);

  m_CullingHandle = CCullingManager::Instance().Register(this);
}

void CGraphicObjectInstance::AddCollision(const CStaticCollisionData *pscd,
                                          const D3DXMATRIX *pMat) {
  m_StaticCollisionInstanceVector.push_back(
      CBaseCollisionInstance::BuildCollisionInstance(pscd, pMat));
}

void CGraphicObjectInstance::ClearCollision() {
  CCollisionInstanceVector::iterator it;
  for (it = m_StaticCollisionInstanceVector.begin();
       it != m_StaticCollisionInstanceVector.end(); ++it) {
    (*it)->Destroy();
  }
  m_StaticCollisionInstanceVector.clear();
}

bool CGraphicObjectInstance::CollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  CCollisionInstanceVector::const_iterator it;
  for (it = m_StaticCollisionInstanceVector.begin();
       it != m_StaticCollisionInstanceVector.end(); ++it) {
    if ((*it)->CollisionDynamicSphere(s))
      return true;
  }
  return false;
}

bool CGraphicObjectInstance::MovementCollisionDynamicSphere(
    const CDynamicSphereInstance &s) const {
  CCollisionInstanceVector::const_iterator it;
  for (it = m_StaticCollisionInstanceVector.begin();
       it != m_StaticCollisionInstanceVector.end(); ++it) {
    if ((*it)->MovementCollisionDynamicSphere(s))
      return true;
  }
  return false;
}

D3DXVECTOR3 CGraphicObjectInstance::GetCollisionMovementAdjust(
    const CDynamicSphereInstance &s) const {
  CCollisionInstanceVector::const_iterator it;
  for (it = m_StaticCollisionInstanceVector.begin();
       it != m_StaticCollisionInstanceVector.end(); ++it) {
    if ((*it)->MovementCollisionDynamicSphere(s))
      return (*it)->GetCollisionMovementAdjust(s);
  }

  return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void CGraphicObjectInstance::UpdateCollisionData(
    const CStaticCollisionDataVector *pscdVector) {
  ClearCollision();
  OnUpdateCollisionData(pscdVector);
}

DWORD CGraphicObjectInstance::GetCollisionInstanceCount() {
  return m_StaticCollisionInstanceVector.size();
}

CBaseCollisionInstance *
CGraphicObjectInstance::GetCollisionInstanceData(DWORD dwIndex) {
  if (dwIndex > m_StaticCollisionInstanceVector.size()) {
    return 0;
  }
  return m_StaticCollisionInstanceVector[dwIndex];
}

//////////////////////////////////////////////////////////////////////////
// Height

void CGraphicObjectInstance::SetHeightInstance(
    CAttributeInstance *pAttributeInstance) {
  m_pHeightAttributeInstance = pAttributeInstance;
}

void CGraphicObjectInstance::ClearHeightInstance() {
  m_pHeightAttributeInstance = NULL;
}

void CGraphicObjectInstance::UpdateHeightInstance(
    CAttributeInstance *pAttributeInstance) {
  ClearHeightInstance();
  OnUpdateHeighInstance(pAttributeInstance);
}

bool CGraphicObjectInstance::IsObjectHeight() {
  if (m_pHeightAttributeInstance)
    return true;

  return false;
}

bool CGraphicObjectInstance::GetObjectHeight(float fX, float fY,
                                             float *pfHeight) {
  if (!m_pHeightAttributeInstance)
    return false;

  return OnGetObjectHeight(fX, fY, pfHeight);
}

void CGraphicObjectInstance::SetPortal(DWORD dwIndex, int iID) {
  if (dwIndex >= PORTAL_ID_MAX_NUM) {
    assert(dwIndex < PORTAL_ID_MAX_NUM);
    return;
  }

  m_abyPortalID[dwIndex] = iID;
}

int CGraphicObjectInstance::GetPortal(DWORD dwIndex) {
  if (dwIndex >= PORTAL_ID_MAX_NUM) {
    assert(dwIndex < PORTAL_ID_MAX_NUM);
    return 0;
  }

  return m_abyPortalID[dwIndex];
}
