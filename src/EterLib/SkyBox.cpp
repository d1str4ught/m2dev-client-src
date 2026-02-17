#include "stdafx.h"
#include "SkyBox.h"
#include "Camera.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include "EterBase/Timer.h"

//////////////////////////////////////////////////////////////////////////
// CSkyObjectQuad
//////////////////////////////////////////////////////////////////////////

CSkyObjectQuad::CSkyObjectQuad() {
  // Index buffer
  m_Indices[0] = 0;
  m_Indices[1] = 2;
  m_Indices[2] = 1;
  m_Indices[3] = 3;

  for (unsigned char uci = 0; uci < 4; ++uci) {
    memset(&m_Vertex[uci], 0, sizeof(TPDTVertex));
  }
}

CSkyObjectQuad::~CSkyObjectQuad() {}

void CSkyObjectQuad::Clear(const unsigned char &c_rucNumVertex,
                           const float &c_rfRed, const float &c_rfGreen,
                           const float &c_rfBlue, const float &c_rfAlpha) {
  if (c_rucNumVertex > 3)
    return;
  m_Helper[c_rucNumVertex].Clear(c_rfRed, c_rfGreen, c_rfBlue, c_rfAlpha);
}

void CSkyObjectQuad::SetSrcColor(const unsigned char &c_rucNumVertex,
                                 const float &c_rfRed, const float &c_rfGreen,
                                 const float &c_rfBlue,
                                 const float &c_rfAlpha) {
  if (c_rucNumVertex > 3)
    return;
  m_Helper[c_rucNumVertex].SetSrcColor(c_rfRed, c_rfGreen, c_rfBlue, c_rfAlpha);
}

void CSkyObjectQuad::SetTransition(const unsigned char &c_rucNumVertex,
                                   const float &c_rfRed, const float &c_rfGreen,
                                   const float &c_rfBlue,
                                   const float &c_rfAlpha, DWORD dwDuration) {
  if (c_rucNumVertex > 3)
    return;
  m_Helper[c_rucNumVertex].SetTransition(c_rfRed, c_rfGreen, c_rfBlue,
                                         c_rfAlpha, dwDuration);
}

void CSkyObjectQuad::SetVertex(const unsigned char &c_rucNumVertex,
                               const TPDTVertex &c_rPDTVertex) {
  if (c_rucNumVertex > 3)
    return;
  memcpy(&m_Vertex[m_Indices[c_rucNumVertex]], &c_rPDTVertex,
         sizeof(TPDTVertex));
}

void CSkyObjectQuad::StartTransition() {
  for (unsigned char uci = 0; uci < 4; ++uci) {
    m_Helper[uci].StartTransition();
  }
}

bool CSkyObjectQuad::Update() {
  bool bResult = false;
  for (unsigned char uci = 0; uci < 4; ++uci) {
    bResult = m_Helper[uci].Update() || bResult;
    m_Vertex[m_Indices[uci]].diffuse = m_Helper[uci].GetCurColor();
  }
  return bResult;
}

void CSkyObjectQuad::Render() {
  if (CGraphicBase::SetPDTStream(m_Vertex, 4))
    STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
  // STATEMANAGER.DrawIndexedPrimitiveUP(D3DPT_TRIANGLESTRIP, 0, 4, 2,
  // m_Indices, D3DFMT_INDEX16, &m_Vertex, sizeof(TPDTVertex));
}

//////////////////////////////////////////////////////////////////////////
// CSkyObject
/////////////////////////////////////////////////////////////////////////
CSkyObject::CSkyObject()
    : m_v3Position(0.0f, 0.0f, 0.0f), m_fScaleX(1.0f), m_fScaleY(1.0f),
      m_fScaleZ(1.0f) {
  D3DXMatrixIdentity(&m_matWorld);
  D3DXMatrixIdentity(&m_matTranslation);
  D3DXMatrixIdentity(&m_matTextureCloud);

  m_dwlastTime = CTimer::Instance().GetCurrentMillisecond();

  m_fCloudPositionU = 0.0f;
  m_fCloudPositionV = 0.0f;

  m_bTransitionStarted = false;
  m_bSkyMatrixUpdated = false;
}

CSkyObject::~CSkyObject() { Destroy(); }

void CSkyObject::Destroy() {}

void CSkyObject::Update() {
  D3DXVECTOR3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();

  if (m_v3Position == v3Eye)
    if (m_bSkyMatrixUpdated == false)
      return;

  m_v3Position = v3Eye;

  m_matWorld._41 = m_v3Position.x;
  m_matWorld._42 = m_v3Position.y;
  m_matWorld._43 = m_v3Position.z;

  m_matWorldCloud._41 = m_v3Position.x;
  m_matWorldCloud._42 = m_v3Position.y;
  m_matWorldCloud._43 = m_v3Position.z + m_fCloudHeight;

  if (m_bSkyMatrixUpdated)
    m_bSkyMatrixUpdated = false;
}

void CSkyObject::Render() {}

CGraphicImageInstance *CSkyObject::GenerateTexture(const char *szfilename) {
  assert(szfilename != NULL);

  if (strlen(szfilename) <= 0)
    assert(false);

  CResource *pResource =
      CResourceManager::Instance().GetResourcePointer(szfilename);

  if (!pResource->IsType(CGraphicImage::Type())) {
    assert(false);
    return NULL;
  }

  CGraphicImageInstance *pImageInstance = CGraphicImageInstance::New();
  pImageInstance->SetImagePointer(static_cast<CGraphicImage *>(pResource));
  return (pImageInstance);
}

void CSkyObject::DeleteTexture(CGraphicImageInstance *pImageInstance) {
  if (pImageInstance)
    CGraphicImageInstance::Delete(pImageInstance);
}

void CSkyObject::StartTransition() {}

//////////////////////////////////////////////////////////////////////////
// CSkyObject::TSkyObjectFace
//////////////////////////////////////////////////////////////////////////

void CSkyObject::TSkyObjectFace::StartTransition() {
  for (unsigned char uci = 0; uci < m_SkyObjectQuadVector.size(); ++uci) {
    m_SkyObjectQuadVector[uci].StartTransition();
  }
}

bool CSkyObject::TSkyObjectFace::Update() {
  bool bResult = false;
  for (DWORD dwi = 0; dwi < m_SkyObjectQuadVector.size(); ++dwi)
    bResult = m_SkyObjectQuadVector[dwi].Update() || bResult;
  return bResult;
}

void CSkyObject::TSkyObjectFace::Render() {
  for (unsigned char uci = 0; uci < m_SkyObjectQuadVector.size(); ++uci) {
    m_SkyObjectQuadVector[uci].Render();
  }
}

//////////////////////////////////////////////////////////////////////////
// CSkyBox
//////////////////////////////////////////////////////////////////////////

CSkyBox::CSkyBox() {
  m_ucVirticalGradientLevelUpper = 0;
  m_ucVirticalGradientLevelLower = 0;
}

CSkyBox::~CSkyBox() { Destroy(); }

void CSkyBox::Destroy() { Unload(); }

void CSkyBox::SetSkyBoxScale(const D3DXVECTOR3 &c_rv3Scale) {
  m_fScaleX = c_rv3Scale.x;
  m_fScaleY = c_rv3Scale.y;
  m_fScaleZ = c_rv3Scale.z;

  m_bSkyMatrixUpdated = true;
  D3DXMatrixScaling(&m_matWorld, m_fScaleX, m_fScaleY, m_fScaleZ);
}

void CSkyBox::SetGradientLevel(BYTE byUpper, BYTE byLower) {
  m_ucVirticalGradientLevelUpper = byUpper;
  m_ucVirticalGradientLevelLower = byLower;
}

void CSkyBox::SetFaceTexture(const char *c_szFileName, int iFaceIndex) {
  if (iFaceIndex < 0 || iFaceIndex > 5)
    return;

  TGraphicImageInstanceMap::iterator itor =
      m_GraphicImageInstanceMap.find(c_szFileName);
  if (m_GraphicImageInstanceMap.end() != itor)
    return;

  m_Faces[iFaceIndex].m_strFaceTextureFileName = c_szFileName;

  CGraphicImageInstance *pGraphicImageInstance = GenerateTexture(c_szFileName);
  m_GraphicImageInstanceMap.insert(TGraphicImageInstanceMap::value_type(
      c_szFileName, pGraphicImageInstance));
}

void CSkyBox::SetCloudTexture(const char *c_szFileName) {
  TGraphicImageInstanceMap::iterator itor =
      m_GraphicImageInstanceMap.find(c_szFileName);
  if (m_GraphicImageInstanceMap.end() != itor)
    return;

  m_FaceCloud.m_strfacename = c_szFileName;
  CGraphicImageInstance *pGraphicImageInstance = GenerateTexture(c_szFileName);
  m_GraphicImageInstanceMap.insert(TGraphicImageInstanceMap::value_type(
      m_FaceCloud.m_strfacename, pGraphicImageInstance));

  // 이거 안쓰는거 같은데요? [cronan]
  //	CGraphicImage * pImage = (CGraphicImage *)
  // CResourceManager::Instance().GetResourcePointer("D:\\Ymir
  // Work\\special/cloudalpha.tga");
  //	m_CloudAlphaImageInstance.SetImagePointer(pImage);
}

void CSkyBox::SetCloudScale(const D3DXVECTOR2 &c_rv2CloudScale) {
  m_fCloudScaleX = c_rv2CloudScale.x;
  m_fCloudScaleY = c_rv2CloudScale.y;

  D3DXMatrixScaling(&m_matWorldCloud, m_fCloudScaleX, m_fCloudScaleY, 1.0f);
}

void CSkyBox::SetCloudHeight(float fHeight) { m_fCloudHeight = fHeight; }

void CSkyBox::SetCloudTextureScale(const D3DXVECTOR2 &c_rv2CloudTextureScale) {
  m_fCloudTextureScaleX = c_rv2CloudTextureScale.x;
  m_fCloudTextureScaleY = c_rv2CloudTextureScale.y;

  m_matTextureCloud._11 = m_fCloudTextureScaleX;
  m_matTextureCloud._22 = m_fCloudTextureScaleY;
}

void CSkyBox::SetCloudScrollSpeed(const D3DXVECTOR2 &c_rv2CloudScrollSpeed) {
  m_fCloudScrollSpeedU = c_rv2CloudScrollSpeed.x;
  m_fCloudScrollSpeedV = c_rv2CloudScrollSpeed.y;
}

void CSkyBox::Unload() {
  TGraphicImageInstanceMap::iterator itor = m_GraphicImageInstanceMap.begin();

  while (itor != m_GraphicImageInstanceMap.end()) {
    DeleteTexture(itor->second);
    ++itor;
  }

  m_GraphicImageInstanceMap.clear();
}

void CSkyBox::SetSkyObjectQuadVertical(
    TSkyObjectQuadVector *pSkyObjectQuadVector,
    const D3DXVECTOR2 *c_pv2QuadPoints) {
  TPDTVertex aPDTVertex;

  DWORD dwIndex = 0;

  pSkyObjectQuadVector->clear();
  pSkyObjectQuadVector->resize(m_ucVirticalGradientLevelUpper +
                               m_ucVirticalGradientLevelLower);

  unsigned char ucY;
  for (ucY = 0; ucY < m_ucVirticalGradientLevelUpper; ++ucY) {
    CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

    aPDTVertex.position.x = c_pv2QuadPoints[0].x;
    aPDTVertex.position.y = c_pv2QuadPoints[0].y;
    aPDTVertex.position.z =
        1.0f - (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper);
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y =
        (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(0, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[0].x;
    aPDTVertex.position.y = c_pv2QuadPoints[0].y;
    aPDTVertex.position.z =
        1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper);
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y =
        (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(1, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[1].x;
    aPDTVertex.position.y = c_pv2QuadPoints[1].y;
    aPDTVertex.position.z =
        1.0f - (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper);
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y =
        (float)(ucY + 1) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(2, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[1].x;
    aPDTVertex.position.y = c_pv2QuadPoints[1].y;
    aPDTVertex.position.z =
        1.0f - (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper);
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y =
        (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(3, aPDTVertex);
  }
  for (ucY = 0; ucY < m_ucVirticalGradientLevelLower; ++ucY) {
    CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(dwIndex++);

    aPDTVertex.position.x = c_pv2QuadPoints[0].x;
    aPDTVertex.position.y = c_pv2QuadPoints[0].y;
    aPDTVertex.position.z =
        -(float)(ucY + 1) / (float)(m_ucVirticalGradientLevelLower);
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1) /
                                       (float)(m_ucVirticalGradientLevelUpper) *
                                       0.5f;
    rSkyObjectQuad.SetVertex(0, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[0].x;
    aPDTVertex.position.y = c_pv2QuadPoints[0].y;
    aPDTVertex.position.z =
        -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower);
    aPDTVertex.texCoord.x = 0.0f;
    aPDTVertex.texCoord.y =
        0.5f + (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(1, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[1].x;
    aPDTVertex.position.y = c_pv2QuadPoints[1].y;
    aPDTVertex.position.z =
        -(float)(ucY + 1) / (float)(m_ucVirticalGradientLevelLower);
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y = 0.5f + (float)(ucY + 1) /
                                       (float)(m_ucVirticalGradientLevelUpper) *
                                       0.5f;
    rSkyObjectQuad.SetVertex(2, aPDTVertex);
    aPDTVertex.position.x = c_pv2QuadPoints[1].x;
    aPDTVertex.position.y = c_pv2QuadPoints[1].y;
    aPDTVertex.position.z =
        -(float)(ucY) / (float)(m_ucVirticalGradientLevelLower);
    aPDTVertex.texCoord.x = 1.0f;
    aPDTVertex.texCoord.y =
        0.5f + (float)(ucY) / (float)(m_ucVirticalGradientLevelUpper) * 0.5f;
    rSkyObjectQuad.SetVertex(3, aPDTVertex);
  }
}

// void CSkyBox::UpdateSkyFaceQuadTransform(D3DXVECTOR3 * c_pv3QuadPoints)
//{
//	for( int i = 0; i < 4; ++i )
//	{
//		c_pv3QuadPoints[i].x *= m_fScaleX;
//		c_pv3QuadPoints[i].y *= m_fScaleY;
//		c_pv3QuadPoints[i].z *= m_fScaleZ;
//
//		c_pv3QuadPoints[i] += m_v3Position;
//	}
// }

void CSkyBox::SetSkyObjectQuadHorizon(
    TSkyObjectQuadVector *pSkyObjectQuadVector,
    const D3DXVECTOR3 *c_pv3QuadPoints) {
  pSkyObjectQuadVector->clear();
  pSkyObjectQuadVector->resize(1);
  CSkyObjectQuad &rSkyObjectQuad = pSkyObjectQuadVector->at(0);

  TPDTVertex aPDTVertex;
  aPDTVertex.position = c_pv3QuadPoints[0];
  aPDTVertex.texCoord.x = 0.0f;
  aPDTVertex.texCoord.y = 1.0f;
  rSkyObjectQuad.SetVertex(0, aPDTVertex);

  aPDTVertex.position = c_pv3QuadPoints[1];
  aPDTVertex.texCoord.x = 0.0f;
  aPDTVertex.texCoord.y = 0.0f;
  rSkyObjectQuad.SetVertex(1, aPDTVertex);

  aPDTVertex.position = c_pv3QuadPoints[2];
  aPDTVertex.texCoord.x = 1.0f;
  aPDTVertex.texCoord.y = 1.0f;
  rSkyObjectQuad.SetVertex(2, aPDTVertex);

  aPDTVertex.position = c_pv3QuadPoints[3];
  aPDTVertex.texCoord.x = 1.0f;
  aPDTVertex.texCoord.y = 0.0f;
  rSkyObjectQuad.SetVertex(3, aPDTVertex);
}

void CSkyBox::Refresh() {
  D3DXVECTOR3 v3QuadPoints[4];

  if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DEFAULT ||
      m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_DIFFUSE) {
    if (m_ucVirticalGradientLevelUpper + m_ucVirticalGradientLevelLower <= 0)
      return;

    D3DXVECTOR2 v2QuadPoints[2];

    //// Face 0: FRONT
    v2QuadPoints[0] = D3DXVECTOR2(1.0f, -1.0f);
    v2QuadPoints[1] = D3DXVECTOR2(-1.0f, -1.0f);
    SetSkyObjectQuadVertical(&m_Faces[0].m_SkyObjectQuadVector, v2QuadPoints);
    m_Faces[0].m_strfacename = "front";

    //// Face 1: BACK
    v2QuadPoints[0] = D3DXVECTOR2(-1.0f, 1.0f);
    v2QuadPoints[1] = D3DXVECTOR2(1.0f, 1.0f);
    SetSkyObjectQuadVertical(&m_Faces[1].m_SkyObjectQuadVector, v2QuadPoints);
    m_Faces[1].m_strfacename = "back";

    //// Face 2: LEFT
    v2QuadPoints[0] = D3DXVECTOR2(-1.0f, -1.0f);
    v2QuadPoints[1] = D3DXVECTOR2(-1.0f, 1.0f);
    SetSkyObjectQuadVertical(&m_Faces[2].m_SkyObjectQuadVector, v2QuadPoints);
    m_Faces[2].m_strfacename = "left";

    //// Face 3: RIGHT
    v2QuadPoints[0] = D3DXVECTOR2(1.0f, 1.0f);
    v2QuadPoints[1] = D3DXVECTOR2(1.0f, -1.0f);
    SetSkyObjectQuadVertical(&m_Faces[3].m_SkyObjectQuadVector, v2QuadPoints);
    m_Faces[3].m_strfacename = "right";

    //// Face 4: TOP
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[4].m_strfacename = "top";

    //// Face 5: BOTTOM
    v3QuadPoints[0] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[5].m_strfacename = "bottom";

  } else if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE) {
    // Face 0: FRONT
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[0].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[0].m_strfacename = "front";

    //// Face 1: BACK
    v3QuadPoints[0] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[1].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[1].m_strfacename = "back";

    // Face 2: LEFT
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[2].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[2].m_strfacename = "left";

    // Face 3: RIGHT
    v3QuadPoints[0] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[3].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[3].m_strfacename = "right";

    // Face 4: TOP
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, 1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[4].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[4].m_strfacename = "top";

    ////// Face 5: BOTTOM
    v3QuadPoints[0] = D3DXVECTOR3(1.0f, -1.0f, -1.0f);
    v3QuadPoints[1] = D3DXVECTOR3(1.0f, 1.0f, -1.0f);
    v3QuadPoints[2] = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
    v3QuadPoints[3] = D3DXVECTOR3(-1.0f, 1.0f, -1.0f);

    // UpdateSkyFaceQuadTransform(v3QuadPoints);

    SetSkyObjectQuadHorizon(&m_Faces[5].m_SkyObjectQuadVector, v3QuadPoints);
    m_Faces[5].m_strfacename = "bottom";
  }

  //// Clouds..
  v3QuadPoints[0] = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
  v3QuadPoints[1] = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
  v3QuadPoints[2] = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
  v3QuadPoints[3] = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
  SetSkyObjectQuadHorizon(&m_FaceCloud.m_SkyObjectQuadVector, v3QuadPoints);
}

void CSkyBox::SetCloudColor(const TGradientColor &c_rColor,
                            const TGradientColor &c_rNextColor,
                            const DWORD &dwTransitionTime) {
  TSkyObjectFace &aFaceCloud = m_FaceCloud;
  for (DWORD dwk = 0; dwk < aFaceCloud.m_SkyObjectQuadVector.size(); ++dwk) {
    CSkyObjectQuad &aSkyObjectQuad = aFaceCloud.m_SkyObjectQuadVector[dwk];

    aSkyObjectQuad.SetSrcColor(0, c_rColor.m_FirstColor.r,
                               c_rColor.m_FirstColor.g, c_rColor.m_FirstColor.b,
                               c_rColor.m_FirstColor.a);
    aSkyObjectQuad.SetTransition(0, c_rNextColor.m_FirstColor.r,
                                 c_rNextColor.m_FirstColor.g,
                                 c_rNextColor.m_FirstColor.b,
                                 c_rNextColor.m_FirstColor.a, dwTransitionTime);
    aSkyObjectQuad.SetSrcColor(1, c_rColor.m_FirstColor.r,
                               c_rColor.m_FirstColor.g, c_rColor.m_FirstColor.b,
                               c_rColor.m_FirstColor.a);
    aSkyObjectQuad.SetTransition(1, c_rNextColor.m_FirstColor.r,
                                 c_rNextColor.m_FirstColor.g,
                                 c_rNextColor.m_FirstColor.b,
                                 c_rNextColor.m_FirstColor.a, dwTransitionTime);
    aSkyObjectQuad.SetSrcColor(2, c_rColor.m_FirstColor.r,
                               c_rColor.m_FirstColor.g, c_rColor.m_FirstColor.b,
                               c_rColor.m_FirstColor.a);
    aSkyObjectQuad.SetTransition(2, c_rNextColor.m_FirstColor.r,
                                 c_rNextColor.m_FirstColor.g,
                                 c_rNextColor.m_FirstColor.b,
                                 c_rNextColor.m_FirstColor.a, dwTransitionTime);
    aSkyObjectQuad.SetSrcColor(3, c_rColor.m_FirstColor.r,
                               c_rColor.m_FirstColor.g, c_rColor.m_FirstColor.b,
                               c_rColor.m_FirstColor.a);
    aSkyObjectQuad.SetTransition(3, c_rNextColor.m_FirstColor.r,
                                 c_rNextColor.m_FirstColor.g,
                                 c_rNextColor.m_FirstColor.b,
                                 c_rNextColor.m_FirstColor.a, dwTransitionTime);
  }
}

void CSkyBox::SetSkyColor(const TVectorGradientColor &c_rColorVector,
                          const TVectorGradientColor &c_rNextColorVector,
                          long lTransitionTime) {
  unsigned long ulVectorGradientColornum = 0;
  unsigned long uck;
  for (unsigned char ucj = 0; ucj < 4; ++ucj) {
    TSkyObjectFace &aFace = m_Faces[ucj];
    ulVectorGradientColornum = 0;
    for (uck = 0; uck < aFace.m_SkyObjectQuadVector.size(); ++uck) {
      CSkyObjectQuad &aSkyObjectQuad = aFace.m_SkyObjectQuadVector[uck];

      aSkyObjectQuad.SetSrcColor(
          0, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
      aSkyObjectQuad.SetTransition(
          0, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
          lTransitionTime);
      aSkyObjectQuad.SetSrcColor(
          1, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
      aSkyObjectQuad.SetTransition(
          1, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
          lTransitionTime);
      aSkyObjectQuad.SetSrcColor(
          2, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
          c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
      aSkyObjectQuad.SetTransition(
          2, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
          c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
          lTransitionTime);
      aSkyObjectQuad.SetSrcColor(
          3, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
          c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
      aSkyObjectQuad.SetTransition(
          3, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
          c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
          lTransitionTime);

      ulVectorGradientColornum++;
    }
  }

  /////

  TSkyObjectFace &aFaceTop = m_Faces[4];
  ulVectorGradientColornum = 0;
  for (uck = 0; uck < aFaceTop.m_SkyObjectQuadVector.size(); ++uck) {
    CSkyObjectQuad &aSkyObjectQuad = aFaceTop.m_SkyObjectQuadVector[uck];

    aSkyObjectQuad.SetSrcColor(
        0, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
    aSkyObjectQuad.SetTransition(
        0, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        1, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
    aSkyObjectQuad.SetTransition(
        1, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        2, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
    aSkyObjectQuad.SetTransition(
        2, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        3, c_rColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rColorVector[ulVectorGradientColornum].m_FirstColor.a);
    aSkyObjectQuad.SetTransition(
        3, c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_FirstColor.a,
        lTransitionTime);
  }
  TSkyObjectFace &aFaceBottom = m_Faces[5];
  ulVectorGradientColornum = c_rColorVector.size() - 1;
  for (uck = 0; uck < aFaceBottom.m_SkyObjectQuadVector.size(); ++uck) {
    CSkyObjectQuad &aSkyObjectQuad = aFaceBottom.m_SkyObjectQuadVector[uck];

    aSkyObjectQuad.SetSrcColor(
        0, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
    aSkyObjectQuad.SetTransition(
        0, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        1, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
    aSkyObjectQuad.SetTransition(
        1, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        2, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
    aSkyObjectQuad.SetTransition(
        2, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
        lTransitionTime);
    aSkyObjectQuad.SetSrcColor(
        3, c_rColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rColorVector[ulVectorGradientColornum].m_SecondColor.a);
    aSkyObjectQuad.SetTransition(
        3, c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.r,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.g,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.b,
        c_rNextColorVector[ulVectorGradientColornum].m_SecondColor.a,
        lTransitionTime);
  }
}

void CSkyBox::StartTransition() {
  m_bTransitionStarted = true;
  for (unsigned char ucj = 0; ucj < 6; ++ucj)
    m_Faces[ucj].StartTransition();
  m_FaceCloud.StartTransition();
}

void CSkyBox::Update() {
  CSkyObject::Update();

  if (!m_bTransitionStarted)
    return;

  bool bResult = false;
  for (unsigned char uci = 0; uci < 6; ++uci)
    bResult = m_Faces[uci].Update() || bResult;
  bResult = m_FaceCloud.Update() || bResult;

  m_bTransitionStarted = bResult;
}

void CSkyBox::Render() {
  // 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리
  STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

  STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

  // Render Face
  if (m_ucRenderMode == CSkyObject::SKY_RENDER_MODE_TEXTURE) {
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    for (unsigned int i = 0; i < 6; ++i) {
      CGraphicImageInstance *pFaceImageInstance =
          m_GraphicImageInstanceMap[m_Faces[i].m_strFaceTextureFileName];
      if (!pFaceImageInstance)
        break;

      STATEMANAGER.SetTexture(
          0, pFaceImageInstance->GetTextureReference().GetD3DTexture());

      m_Faces[i].Render();
    }

    // STATEMANAGER.SetTexture( 0, NULL );

    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
    STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
  } else {
    for (unsigned int i = 0; i < 6; ++i) {
      m_Faces[i].Render();
    }
  }

  STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
  STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);

  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
}

void CSkyBox::RenderCloud() {
  CGraphicImageInstance *pCloudGraphicImageInstance =
      m_GraphicImageInstanceMap[m_FaceCloud.m_strfacename];
  if (!pCloudGraphicImageInstance)
    return;

  // 2004.01.25 myevan 처리를 렌더링 후반으로 옮기고, DepthTest 처리
  STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

  STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
                                     D3DTTFF_COUNT2);

  m_matTextureCloud._31 = m_fCloudPositionU;
  m_matTextureCloud._32 = m_fCloudPositionV;

  DWORD dwCurTime = CTimer::Instance().GetCurrentMillisecond();

  m_fCloudPositionU +=
      m_fCloudScrollSpeedU * (float)(dwCurTime - m_dwlastTime) * 0.001f;
  if (m_fCloudPositionU >= 1.0f)
    m_fCloudPositionU = 0.0f;

  m_fCloudPositionV +=
      m_fCloudScrollSpeedV * (float)(dwCurTime - m_dwlastTime) * 0.001f;
  if (m_fCloudPositionV >= 1.0f)
    m_fCloudPositionV = 0.0f;

  m_dwlastTime = dwCurTime;

  STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matTextureCloud);

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,
                                    D3DTOP_MODULATEINVALPHA_ADDCOLOR);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

  D3DXMATRIX matProjCloud;
  D3DXMatrixPerspectiveFovRH(&matProjCloud, D3DX_PI * 0.25f, 1.33333f, 50.0f,
                             999999.0f);
  STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldCloud);
  STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProjCloud);
  STATEMANAGER.SetTexture(
      0, pCloudGraphicImageInstance->GetTexturePointer()->GetD3DTexture());
  m_FaceCloud.Render();
  STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

  STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
  STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);

  STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
  STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}
