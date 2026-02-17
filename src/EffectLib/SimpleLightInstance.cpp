#include "Stdafx.h"
#include "EterLib/GrpLightManager.h"

#include "SimpleLightInstance.h"

CDynamicPool<CLightInstance> CLightInstance::ms_kPool;

void CLightInstance::DestroySystem() { ms_kPool.Destroy(); }

CLightInstance *CLightInstance::New() { return ms_kPool.Alloc(); }

void CLightInstance::Delete(CLightInstance *pkData) {
  pkData->Destroy();
  ms_kPool.Free(pkData);
}

void CLightInstance::OnSetDataPointer(CEffectElementBase *pElement) {
  Destroy();

  m_pData = ((CLightData *)pElement);

  m_iLoopCount = m_pData->GetLoopCount();

  D3DLIGHT9 Light;
  m_pData->InitializeLight(Light);
  CLightManager::Instance().RegisterLight(LIGHT_TYPE_DYNAMIC, &m_LightID,
                                          Light);
}

bool CLightInstance::OnUpdate(float fElapsedTime) {
  if (!isActive()) {
    Destroy();
    return false;
  }

  if (m_fLocalTime >= m_pData->GetDuration()) {
    if (m_pData->isLoop() && --m_iLoopCount != 0) {
      if (m_iLoopCount < 0)
        m_iLoopCount = 0;
      m_fLocalTime -= m_pData->GetDuration();
    } else {
      Destroy();
      m_iLoopCount = 1;
      return false;
    }
    /*
    if (!m_pData->isLoop())
    {
            OnClear();
            return false;
    }
    m_fLocalTime -= m_pData->GetDuration();
    */
  }

  CLight *pLight = CLightManager::Instance().GetLight(m_LightID);

  if (pLight) {
    pLight->SetAmbientColor(m_pData->m_cAmbient.r, m_pData->m_cAmbient.g,
                            m_pData->m_cAmbient.b, m_pData->m_cAmbient.a);
    pLight->SetDiffuseColor(m_pData->m_cDiffuse.r, m_pData->m_cDiffuse.g,
                            m_pData->m_cDiffuse.b, m_pData->m_cDiffuse.a);

    /*if (m_pData->m_TimeEventTableRange.size()
            &&
    m_fLocalTime>=m_pData->GetDuration()*m_pData->m_TimeEventTableRange[m_dwRangeIndex].m_fTime)
    {
            while(m_dwRangeIndex<m_pData->m_TimeEventTableRange.size()
                    &&
    m_fLocalTime>=m_pData->GetDuration()*m_pData->m_TimeEventTableRange[m_dwRangeIndex].m_fTime)
                    m_dwRangeIndex++;
            float fLastTime;
            float
    fLastRange=m_pData->m_TimeEventTableRange[m_pData->m_TimeEventTableRange.size()-1].m_Value;
            if (m_dwRangeIndex == m_pData->m_TimeEventTableRange.size())
                    fLastTime = 1.0f;
            else
            {
                    fLastTime =
    m_pData->m_TimeEventTableRange[m_dwRangeIndex].m_fTime; fLastRange =
    m_pData->m_TimeEventTableRange[m_dwRangeIndex].m_Value;
            }
            m_dwRangeIndex--;
            pLight->BlendRange(fLastRange*m_pData->m_fMaxRange,
                    (fLastTime-m_pData->m_TimeEventTableRange[m_dwRangeIndex].m_fTime)*m_pData->GetDuration());
            m_dwRangeIndex++;
    }*/

    float fRange;
    m_pData->GetRange(m_fLocalTime, fRange);
    pLight->SetRange(fRange);

    D3DXVECTOR3 pos;
    m_pData->GetPosition(m_fLocalTime, pos);
    D3DXVec3TransformCoord(&pos, &pos, mc_pmatLocal);
    pLight->SetPosition(pos.x, pos.y, pos.z);
  }

  return true;
}

void CLightInstance::OnRender() {
  // OnUpdate(0);
}

void CLightInstance::OnInitialize() {
  m_LightID = 0;
  m_dwRangeIndex = 0;
}

void CLightInstance::OnDestroy() {
  if (m_LightID) {
    CLightManager::Instance().DeleteLight(m_LightID);
  }
}

CLightInstance::CLightInstance() { Initialize(); }

CLightInstance::~CLightInstance() { Destroy(); }
