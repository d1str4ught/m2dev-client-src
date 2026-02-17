#include "StdAfx.h"
#include "ScreenFilter.h"
#include "StateManager.h"

void CScreenFilter::Render() {
  if (!m_bEnable)
    return;

  STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &ms_matIdentity);
  STATEMANAGER.SaveTransform(D3DTS_VIEW, &ms_matIdentity);
  STATEMANAGER.SetTransform(D3DTS_WORLD, &ms_matIdentity);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, m_bySrcType);
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, m_byDestType);

  SetOrtho2D(CScreen::ms_iWidth, CScreen::ms_iHeight, 400.0f);
  SetDiffuseColor(m_Color.r, m_Color.g, m_Color.b, m_Color.a);
  RenderBar2d(0, 0, CScreen::ms_iWidth, CScreen::ms_iHeight);

  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
  STATEMANAGER.RestoreTransform(D3DTS_VIEW);
  STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);
}

void CScreenFilter::SetEnable(BOOL /*bFlag*/) { m_bEnable = FALSE; }

void CScreenFilter::SetBlendType(BYTE bySrcType, BYTE byDestType) {
  m_bySrcType = bySrcType;
  m_byDestType = byDestType;
}
void CScreenFilter::SetColor(const D3DXCOLOR &c_rColor) { m_Color = c_rColor; }

CScreenFilter::CScreenFilter() {
  m_bEnable = FALSE;
  m_bySrcType = D3DBLEND_SRCALPHA;
  m_byDestType = D3DBLEND_INVSRCALPHA;
  m_Color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
}
CScreenFilter::~CScreenFilter() {}
