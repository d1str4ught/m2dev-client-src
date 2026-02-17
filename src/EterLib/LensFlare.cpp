///////////////////////////////////////////////////////////////////////
//	CLensFlare Class
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com
//

///////////////////////////////////////////////////////////////////////
//	Preprocessor
#include "StdAfx.h"
#include "LensFlare.h"
#include "Camera.h"
#include "StateManager.h"
#include "ResourceManager.h"

#include <math.h>
using namespace std;

///////////////////////////////////////////////////////////////////////
//	Variables

static string g_strFiles[] = {"flare2.dds", "flare1.dds", "flare2.dds",
                              "flare1.dds", "flare6.dds", "flare4.dds",
                              "flare2.dds", "flare3.dds", ""};
static float g_fPosition[] = {-0.55f, -0.5f, -0.45f, 0.2f,
                              0.3f,   0.95f, 0.9f,   1.0f};
static float g_fWidth[] = {20.0f,  32.0f, 20.0f, 32.0f,
                           100.0f, 32.0f, 20.0f, 250.0f};

static float g_afColors[][4] = {
    {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.8f}, {0.3f, 0.5f, 1.0f, 0.9f},
    {0.3f, 0.5f, 1.0f, 0.6f}, {1.0f, 0.6f, 0.9f, 0.4f},
    {1.0f, 0.0f, 0.0f, 0.5f}, {1.0f, 0.6f, 0.3f, 0.4f}};

///////////////////////////////////////////////////////////////////////
//	CLensFlare::CLensFlare

CLensFlare::CLensFlare()
    : m_fSunSize(0), m_fBeforeBright(0.0f), m_fAfterBright(0.0f),
      m_bFlareVisible(false), m_bDrawFlare(true), m_bDrawBrightScreen(true),
      m_bEnabled(true), m_bShowMainFlare(true), m_fMaxBrightness(1.0f) {
  m_pControlPixels = new float[c_nDepthTestDimension * c_nDepthTestDimension];
  m_pTestPixels = new float[c_nDepthTestDimension * c_nDepthTestDimension];
  m_afColor[0] = m_afColor[1] = m_afColor[2] = 1.0f;
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::~CLensFlare

CLensFlare::~CLensFlare() {
  delete[] m_pControlPixels;
  delete[] m_pTestPixels;
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::Interpolate

float CLensFlare::Interpolate(float fStart, float fEnd, float fPercent) {
  return fStart + (fEnd - fStart) * fPercent;
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::DrawBeforeFlare

void CLensFlare::Compute(const D3DXVECTOR3 &c_rv3LightDirection) {
  float afSunPos[3];

  D3DXVECTOR3 v3Target =
      CCameraManager::Instance().GetCurrentCamera()->GetTarget();

  afSunPos[0] = v3Target.x - c_rv3LightDirection.x * 99999999.0f;
  afSunPos[1] = v3Target.y - c_rv3LightDirection.y * 99999999.0f;
  afSunPos[2] = v3Target.z - c_rv3LightDirection.z * 99999999.0f;

  float fX, fY;
  ProjectPosition(afSunPos[0], afSunPos[1], afSunPos[2], &fX, &fY);

  // set flare location
  SetFlareLocation(fX, fY);

  // determine visibility
  float fSunVectorMagnitude =
      sqrtf(afSunPos[0] * afSunPos[0] + afSunPos[1] * afSunPos[1] +
            afSunPos[2] * afSunPos[2]);
  float afSunVector[3];
  afSunVector[0] = -afSunPos[0] / fSunVectorMagnitude;
  afSunVector[1] = -afSunPos[1] / fSunVectorMagnitude;
  afSunVector[2] = -afSunPos[2] / fSunVectorMagnitude;

  float afCameraDirection[3];
  afCameraDirection[0] = ms_matView._13;
  afCameraDirection[1] = ms_matView._23;
  afCameraDirection[2] = ms_matView._33;

  float fDotProduct = (afSunVector[0] * afCameraDirection[0]) +
                      (afSunVector[1] * afCameraDirection[1]) +
                      (afSunVector[2] * afCameraDirection[2]);

  if (acosf(fDotProduct) < 0.5f * D3DX_PI)
    SetVisible(true);
  else
    SetVisible(false);

  // set flare brightness
  fX /= ms_Viewport.Width;
  fY /= ms_Viewport.Height;

  float fDistance =
      sqrtf(((0.5f - fX) * (0.5f - fX)) + ((0.5f - fY) * (0.5f - fY)));
  float fBeforeBright = Interpolate(0.0f, c_fHalfMaxBright,
                                    1.0f - (fDistance * c_fDistanceScale));
  float fAfterBright =
      Interpolate(0.0f, 1.0f, 1.0f - (fDistance * c_fDistanceScale));

  SetBrightnesses(fBeforeBright, fAfterBright);
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::DrawBeforeFlare

void CLensFlare::DrawBeforeFlare() {
  if (!m_bFlareVisible || !m_bEnabled || !m_bShowMainFlare)
    return;

  if (m_SunFlareImageInstance.IsEmpty())
    return;

  D3DXMATRIX matProj;
  D3DXMatrixOrthoOffCenterRH(&matProj, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
  STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProj);
  STATEMANAGER.SaveTransform(D3DTS_VIEW, &ms_matIdentity);

  D3DXMATRIX matWorld;
  D3DXMatrixTranslation(&matWorld, m_afFlarePos[0], m_afFlarePos[1], 0.0f);
  STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorld);

  STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,
                               FALSE); // glDisable(GL_DEPTH_TEST);
  STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_CULLMODE,
                               D3DCULL_NONE); // glDisable(GL_CULL_FACE);
  STATEMANAGER.SaveRenderState(D3DRS_SHADEMODE,
                               D3DSHADE_FLAT); // glShadeModel(GL_FLAT);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE,
                               FALSE); // glDisable(GL_ALPHA_TEST);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE,
                               TRUE); // glEnable(GL_BLEND);
  STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
  /*
  if (m_fBeforeBright != 0.0f && m_bDrawFlare && m_bDrawBrightScreen && false)
  // ¿Ø false?
  {
          glColor4f(1.0f, 1.0f, 1.0f, m_fBeforeBright);
          glDisable(GL_TEXTURE_2D);
          glBegin(GL_TRIANGLE_STRIP);
                  glVertex2f(0.0f, 0.0f);
                  glVertex2f(0.0f, 1.0f);
                  glVertex2f(1.0f, 0.0f);
                  glVertex2f(1.0f, 1.0f);
          glEnd();
  }
  */
  float fAspectRatio = ms_Viewport.Width / float(ms_Viewport.Height);
  float fHeight = m_fSunSize * fAspectRatio;
  D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);

  SVertex vertices[4];
  vertices[0].x = -m_fSunSize;
  vertices[0].y = -fHeight;
  vertices[0].z = 0.0f;
  vertices[0].color = color;
  vertices[0].u = 0.0f;
  vertices[0].v = 0.0f;

  vertices[1].x = -m_fSunSize;
  vertices[1].y = fHeight;
  vertices[1].z = 0.0f;
  vertices[1].color = color;
  vertices[1].u = 0.0f;
  vertices[1].v = 1.0f;

  vertices[2].x = m_fSunSize;
  vertices[2].y = -fHeight;
  vertices[2].z = 0.0f;
  vertices[2].color = color;
  vertices[2].u = 1.0f;
  vertices[2].v = 0.0f;

  vertices[3].x = m_fSunSize;
  vertices[3].y = fHeight;
  vertices[3].z = 0.0f;
  vertices[3].color = color;
  vertices[3].u = 1.0f;
  vertices[3].v = 1.0f;

  STATEMANAGER.SetTexture(
      0, m_SunFlareImageInstance.GetTexturePointer()->GetD3DTexture());
  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

  STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
  STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices,
                               sizeof(SVertex));

  STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
  STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE); // glDisable(GL_DEPTH_TEST);
  STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);  // glDisable(GL_CULL_FACE);
  STATEMANAGER.RestoreRenderState(D3DRS_SHADEMODE); // glShadeModel(GL_FLAT);
  STATEMANAGER.RestoreRenderState(
      D3DRS_ALPHATESTENABLE); // glDisable(GL_ALPHA_TEST);
  STATEMANAGER.RestoreRenderState(
      D3DRS_ALPHABLENDENABLE); // glEnable(GL_BLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

  STATEMANAGER.RestoreTransform(D3DTS_VIEW);
  STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::DrawAfterFlare

void CLensFlare::DrawAfterFlare() {
  if (m_bEnabled && m_fAfterBright != 0.0f && m_bDrawBrightScreen) {
    SetDiffuseColor(m_afColor[0], m_afColor[1], m_afColor[2], m_fAfterBright);
    RenderBar2d(0.0f, 0.0f, 1024.0f, 1024.0f);
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::SetMainFlare

void CLensFlare::SetMainFlare(string strSunFile, float fSunSize) {
  if (m_bEnabled && m_bShowMainFlare) {
    m_fSunSize = fSunSize;
    CResource *pResource =
        CResourceManager::Instance().GetResourcePointer(strSunFile.c_str());

    if (!pResource->IsType(CGraphicImage::Type()))
      assert(false);

    m_SunFlareImageInstance.SetImagePointer(
        static_cast<CGraphicImage *>(pResource));
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::DrawFlare

void CLensFlare::DrawFlare() {
  if (m_bEnabled && m_bFlareVisible && m_bDrawFlare && m_fAfterBright != 0.0f) {
    // glPushAttrib(GL_ENABLE_BIT);
    STATEMANAGER.SaveRenderState(D3DRS_LIGHTING,
                                 FALSE); // glDisable(GL_LIGHTING);
    STATEMANAGER.SaveRenderState(D3DRS_ZENABLE,
                                 FALSE); // glDisable(GL_DEPTH_TEST);
    STATEMANAGER.SaveRenderState(D3DRS_CULLMODE,
                                 D3DCULL_NONE); // glDisable(GL_CULL_FACE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE,
                                 FALSE); // glDisable(GL_ALPHA_TEST);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE,
                                 TRUE); // glEnable(GL_BLEND);

    D3DXMATRIX matProj;
    D3DXMatrixOrthoOffCenterRH(&matProj, 0.0f, ms_Viewport.Width,
                               ms_Viewport.Height, 0.0f, -1.0f, 1.0f);
    STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matProj);
    STATEMANAGER.SaveTransform(D3DTS_VIEW, &ms_matIdentity);

    STATEMANAGER.SetTransform(D3DTS_WORLD, &ms_matIdentity);
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();

    // glDisable(GL_TEXTURE_2D);
    DrawAfterFlare();

    // glEnable(GL_TEXTURE_2D);
    m_cFlare.Draw(m_fAfterBright, ms_Viewport.Width, ms_Viewport.Height,
                  static_cast<int>(m_afFlareWinPos[0]),
                  static_cast<int>(m_afFlareWinPos[1]));

    STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING); // glDisable(GL_LIGHTING);
    STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE); // glDisable(GL_DEPTH_TEST);
    STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE); // glDisable(GL_CULL_FACE);
    STATEMANAGER.RestoreRenderState(
        D3DRS_ALPHABLENDENABLE); // glEnable(GL_BLEND);
    STATEMANAGER.RestoreRenderState(
        D3DRS_ALPHATESTENABLE); // glDisable(GL_ALPHA_TEST);

    STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
    STATEMANAGER.RestoreTransform(D3DTS_VIEW);
    // glDisable(GL_TEXTURE_2D);
    // glPopAttrib();
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::CharacterizeFlare
void CLensFlare::CharacterizeFlare(bool bEnabled, bool bShowMainFlare,
                                   float fMaxBrightness,
                                   const D3DXCOLOR &c_rColor) {
  m_bEnabled = bEnabled;
  m_bShowMainFlare = bShowMainFlare;
  m_fMaxBrightness = fMaxBrightness;

  m_afColor[0] = c_rColor.r;
  m_afColor[1] = c_rColor.g;
  m_afColor[2] = c_rColor.b;
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::Initialize
void CLensFlare::Initialize(std::string strPath) {
  if (m_bEnabled)
    m_cFlare.Init(strPath);
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::SetFlareLocation
void CLensFlare::SetFlareLocation(double dX, double dY) {
  if (m_bEnabled) {
    m_afFlareWinPos[0] = float(dX);
    m_afFlareWinPos[1] = float(dY);

    m_afFlarePos[0] = float(dX) / ms_Viewport.Width;
    m_afFlarePos[1] = float(dY) / ms_Viewport.Height;
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::SetBrightnesses

void CLensFlare::SetBrightnesses(float fBeforeBright, float fAfterBright) {
  if (m_bEnabled) {
    m_fBeforeBright = fBeforeBright;
    m_fAfterBright = fAfterBright;

    ClampBrightness();
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::ReadControlPixels

void CLensFlare::ReadControlPixels() {
  if (m_bEnabled)
    ReadDepthPixels(m_pControlPixels);
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::AdjustBrightness

void CLensFlare::AdjustBrightness() {
  if (m_bEnabled) {
    ReadDepthPixels(m_pTestPixels);

    int nDifferent = 0;

    for (int i = 0; i < c_nDepthTestDimension * c_nDepthTestDimension; ++i)
      if (m_pTestPixels[i] != m_pControlPixels[i])
        ++nDifferent;

    float fAdjust = (static_cast<float>(nDifferent) /
                     (c_nDepthTestDimension * c_nDepthTestDimension));
    fAdjust = sqrtf(fAdjust) * 0.85f;
    m_fAfterBright *= 1.0f - fAdjust;
  }
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::ReadDepthPixels

void CLensFlare::ReadDepthPixels(float * /*pPixels*/) {
  /*
  LPDIRECT3DSURFACE9 lpSurface;
  if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&lpSurface)))
          assert(false);

  D3DLOCKED_RECT rect;
  lpSurface->LockRect(&rect, NULL, D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE);

  lpSurface->UnlockRect();
  */
  /*
  glReadPixels(GLint(m_afFlareWinPos[0] - c_nDepthTestDimension / 2),
                           GLint(m_afFlareWinPos[1] - c_nDepthTestDimension /
  2), c_nDepthTestDimension, c_nDepthTestDimension, GL_DEPTH_COMPONENT,
  GL_FLOAT, pPixels);
  */
}

///////////////////////////////////////////////////////////////////////
//	CLensFlare::ClampBrightness

void CLensFlare::ClampBrightness() {
  // before
  if (m_fBeforeBright < 0.0f)
    m_fBeforeBright = 0.0f;
  else if (m_fBeforeBright > 1.0f)
    m_fBeforeBright = 1.0f;

  m_fBeforeBright *= m_fMaxBrightness;

  if (m_fAfterBright < 0.0f)
    m_fAfterBright = 0.0f;
  else if (m_fAfterBright > 1.0f)
    m_fAfterBright = 1.0f;

  m_fAfterBright *= m_fMaxBrightness;
}

///////////////////////////////////////////////////////////////////////
//	CFlare implementation
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//	CFlare::CFlare

CFlare::CFlare() {}

///////////////////////////////////////////////////////////////////////
//	CFlare::~CFlare

CFlare::~CFlare() {}

///////////////////////////////////////////////////////////////////////
//	CFlare::Init

void CFlare::Init(std::string strPath) {
  int i = 0;

  while (g_strFiles[i] != "") {
    CResource *pResource = CResourceManager::Instance().GetResourcePointer(
        (strPath + "/" + string(g_strFiles[i])).c_str());

    if (!pResource->IsType(CGraphicImage::Type()))
      assert(false);

    SFlarePiece *pPiece = new SFlarePiece;

    pPiece->m_imageInstance.SetImagePointer(
        static_cast<CGraphicImage *>(pResource));
    pPiece->m_fPosition = g_fPosition[i];
    pPiece->m_fWidth = g_fWidth[i];
    pPiece->m_pColor = g_afColors[i];

    m_vFlares.push_back(pPiece);
    i++;
  }
}

///////////////////////////////////////////////////////////////////////
//	CFlare::Draw
void CFlare::Draw(float fBrightScale, int nWidth, int nHeight, int nX, int nY) {
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

  float fDX = float(nX) - float(nWidth) / 2.0f;
  float fDY = float(nY) - float(nHeight) / 2.0f;

  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  for (unsigned int i = 0; i < m_vFlares.size(); i++) {
    float fCenterX = float(nX) - (m_vFlares[i]->m_fPosition + 1.0f) * fDX;
    float fCenterY = float(nY) - (m_vFlares[i]->m_fPosition + 1.0f) * fDY;
    float fW = m_vFlares[i]->m_fWidth;

    D3DXCOLOR d3dColor(m_vFlares[i]->m_pColor[0] * fBrightScale,
                       m_vFlares[i]->m_pColor[1] * fBrightScale,
                       m_vFlares[i]->m_pColor[2] * fBrightScale,
                       m_vFlares[i]->m_pColor[3] * fBrightScale);

    STATEMANAGER.SetTexture(
        0, m_vFlares[i]->m_imageInstance.GetTexturePointer()->GetD3DTexture());

    TVertex vertices[4];

    vertices[0].u = 0.0f;
    vertices[0].v = 0.0f;
    vertices[0].x = fCenterX - fW;
    vertices[0].y = fCenterY - fW;
    vertices[0].z = 0.0f;
    vertices[0].color = d3dColor;

    vertices[1].u = 0.0f;
    vertices[1].v = 1.0f;
    vertices[1].x = fCenterX - fW;
    vertices[1].y = fCenterY + fW;
    vertices[1].z = 0.0f;
    vertices[1].color = d3dColor;

    vertices[2].u = 1.0f;
    vertices[2].v = 0.0f;
    vertices[2].x = fCenterX + fW;
    vertices[2].y = fCenterY - fW;
    vertices[2].z = 0.0f;
    vertices[2].color = d3dColor;

    vertices[3].u = 1.0f;
    vertices[3].v = 1.0f;
    vertices[3].x = fCenterX + fW;
    vertices[3].y = fCenterY + fW;
    vertices[3].z = 0.0f;
    vertices[3].color = d3dColor;

    STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices,
                                 sizeof(TVertex));
  }

  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}
