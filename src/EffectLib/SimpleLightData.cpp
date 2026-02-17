#include "StdAfx.h"
#include "SimpleLightData.h"

CDynamicPool<CLightData> CLightData::ms_kPool;

void CLightData::DestroySystem() { ms_kPool.Destroy(); }

CLightData *CLightData::New() { return ms_kPool.Alloc(); }

void CLightData::Delete(CLightData *pkData) {
  pkData->Clear();
  ms_kPool.Free(pkData);
}

void CLightData::OnClear() {
  m_fMaxRange = 300.0f;

  m_TimeEventTableRange.clear();

  m_cAmbient.r = 0.5f;
  m_cAmbient.g = 0.5f;
  m_cAmbient.b = 0.5f;
  m_cAmbient.a = 1.0f;
  m_cDiffuse.r = 0.0f;
  m_cDiffuse.g = 0.0f;
  m_cDiffuse.b = 0.0f;
  m_cDiffuse.a = 1.0f;

  m_fDuration = 1.0f;

  m_fAttenuation0 = 0.0f;
  m_fAttenuation1 = 0.1f;
  m_fAttenuation2 = 0.0f;

  m_bLoopFlag = false;
  m_iLoopCount = 0;
}
void CLightData::GetRange(float fTime, float &rRange) {
  if (m_TimeEventTableRange.empty()) {
    rRange = 1.0f * m_fMaxRange;
    if (rRange < 0.0f)
      rRange = 0.0f;
    return;
  }

  rRange = GetTimeEventBlendValue(fTime, m_TimeEventTableRange);
  rRange *= m_fMaxRange;
  if (rRange < 0.0f)
    rRange = 0.0f;
  return;
}

bool CLightData::OnIsData() { return true; }

BOOL CLightData::OnLoadScript(CTextFileLoader &rTextFileLoader) {
  if (!rTextFileLoader.GetTokenFloat("duration", &m_fDuration))
    m_fDuration = 1.0f;

  if (!rTextFileLoader.GetTokenBoolean("loopflag", &m_bLoopFlag))
    m_bLoopFlag = false;

  if (!rTextFileLoader.GetTokenInteger("loopcount", &m_iLoopCount))
    m_iLoopCount = 0;

  if (!rTextFileLoader.GetTokenColor("ambientcolor", &m_cAmbient))
    return FALSE;

  if (!rTextFileLoader.GetTokenColor("diffusecolor", &m_cDiffuse))
    return FALSE;

  if (!rTextFileLoader.GetTokenFloat("maxrange", &m_fMaxRange))
    return FALSE;

  if (!rTextFileLoader.GetTokenFloat("attenuation0", &m_fAttenuation0))
    return FALSE;

  if (!rTextFileLoader.GetTokenFloat("attenuation1", &m_fAttenuation1))
    return FALSE;

  if (!rTextFileLoader.GetTokenFloat("attenuation2", &m_fAttenuation2))
    return FALSE;

  if (!GetTokenTimeEventFloat(rTextFileLoader, "timeeventrange",
                              &m_TimeEventTableRange)) {
    m_TimeEventTableRange.clear();
  }

  return true;
}

CLightData::CLightData() { Clear(); }

CLightData::~CLightData() {}

float CLightData::GetDuration() { return m_fDuration; }
void CLightData::InitializeLight(D3DLIGHT9 &light) {
  light.Type = D3DLIGHT_POINT;

  light.Ambient = m_cAmbient;
  light.Diffuse = m_cDiffuse;
  light.Attenuation0 = m_fAttenuation0;
  light.Attenuation1 = m_fAttenuation1;
  light.Attenuation2 = m_fAttenuation2;

  D3DXVECTOR3 position;
  GetPosition(0.0f, position);
  light.Position = position;

  GetRange(0.0f, light.Range);
}