#include "StdAfx.h"
#include "ParticleProperty.h"
#include "Eterlib/ResourceManager.h"

void CParticleProperty::InsertTexture(const char *c_szFileName) {
  CGraphicImage *pImage =
      (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
          c_szFileName);
  m_ImageVector.push_back(pImage);
}

bool CParticleProperty::SetTexture(const char *c_szFileName) {
  if (m_ImageVector.size() > 1) {
    assert(false);
    return false;
  }
  m_ImageVector.clear();
  InsertTexture(c_szFileName);
  return true;
}

void CParticleProperty::Clear() {
  m_byRotationType = 0;
  m_fRotationSpeed = 0.0f;
  m_wRotationRandomStartingBegin = 0;
  m_wRotationRandomStartingEnd = 0;

  m_bAttachFlag = FALSE;
  m_bStretchFlag = FALSE;

  m_bySrcBlendType = D3DBLEND_SRCALPHA;
  m_byDestBlendType = D3DBLEND_ONE;
  m_byColorOperationType = D3DTOP_MODULATE;

  m_byBillboardType = BILLBOARD_TYPE_NONE;

  m_byTexAniType = TEXTURE_ANIMATION_TYPE_NONE;
  m_fTexAniDelay = 0.05f;
  m_bTexAniRandomStartFrameFlag = FALSE;

  // m_fGravity = 0.0f;
  // m_fAirResistance = 0.0f;

  m_TimeEventGravity.clear();
  m_TimeEventAirResistance.clear();

  m_TimeEventScaleX.clear();
  m_TimeEventScaleY.clear();
  // m_TimeEventScaleXY.clear();
  m_TimeEventColor.clear();
  m_TimeEventRotation.clear();

  m_ImageVector.clear();
}

CParticleProperty::CParticleProperty() {}
CParticleProperty::~CParticleProperty() {}

CParticleProperty &
CParticleProperty::operator=(const CParticleProperty &c_ParticleProperty) {
  m_byTexAniType = c_ParticleProperty.m_byTexAniType;
  m_fTexAniDelay = c_ParticleProperty.m_fTexAniDelay;
  m_bTexAniRandomStartFrameFlag =
      c_ParticleProperty.m_bTexAniRandomStartFrameFlag;

  m_bySrcBlendType = c_ParticleProperty.m_bySrcBlendType;
  m_byDestBlendType = c_ParticleProperty.m_byDestBlendType;
  m_byColorOperationType = c_ParticleProperty.m_byColorOperationType;

  m_byBillboardType = c_ParticleProperty.m_byBillboardType;

  m_byRotationType = c_ParticleProperty.m_byRotationType;
  m_fRotationSpeed = c_ParticleProperty.m_fRotationSpeed;
  m_wRotationRandomStartingBegin =
      c_ParticleProperty.m_wRotationRandomStartingBegin;
  m_wRotationRandomStartingEnd =
      c_ParticleProperty.m_wRotationRandomStartingEnd;

  m_bAttachFlag = c_ParticleProperty.m_bAttachFlag;
  m_bStretchFlag = c_ParticleProperty.m_bStretchFlag;

  m_TimeEventGravity = c_ParticleProperty.m_TimeEventGravity;
  m_TimeEventAirResistance = c_ParticleProperty.m_TimeEventAirResistance;

  m_TimeEventScaleX = c_ParticleProperty.m_TimeEventScaleX;
  m_TimeEventScaleY = c_ParticleProperty.m_TimeEventScaleY;

  m_TimeEventColor = c_ParticleProperty.m_TimeEventColor;
  m_TimeEventRotation = c_ParticleProperty.m_TimeEventRotation;

  m_ImageVector = c_ParticleProperty.m_ImageVector;

  return *this;
}