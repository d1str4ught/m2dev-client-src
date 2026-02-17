#include "StdAfx.h"
#include "EterLib/ResourceManager.h"
#include "EterLib/StateManager.h"

#include "WeaponTrace.h"

CDynamicPool<CWeaponTrace> CWeaponTrace::ms_kPool;

void CWeaponTrace::DestroySystem() { ms_kPool.Destroy(); }

void CWeaponTrace::Delete(CWeaponTrace *pkWTDel) {
  assert(pkWTDel != NULL && "CWeaponTrace::Delete");

  pkWTDel->Clear();
  ms_kPool.Free(pkWTDel);
}

CWeaponTrace *CWeaponTrace::New() { return ms_kPool.Alloc(); }

void CWeaponTrace::Update(float fReachScale) {
  float fElapsedTime = CTimer::Instance().GetCurrentSecond() - m_fLastUpdate;
  m_fLastUpdate = CTimer::Instance().GetCurrentSecond();

  if (!m_pInstance)
    return;
  {
    // 잔상을 남기는 시간 범위 내의 점들만 유지합니다.
    TTimePointList::iterator it;
    for (it = m_ShortTimePointList.begin(); it != m_ShortTimePointList.end();
         ++it) {
      it->first += fElapsedTime;
      if (it->first > m_fLifeTime) {
        it++;
        break;
      }
    }
    if (it != m_ShortTimePointList.end())
      m_ShortTimePointList.erase(it, m_ShortTimePointList.end());
    for (it = m_LongTimePointList.begin(); it != m_LongTimePointList.end();
         ++it) {
      it->first += fElapsedTime;
      if (it->first > m_fLifeTime) {
        it++;
        break;
      }
    }
    if (it != m_LongTimePointList.end())
      m_LongTimePointList.erase(it, m_LongTimePointList.end());
  }

  if (m_isPlaying && m_fz >= 0.0001f) {
    D3DXMATRIX *pMatrix;
    if (m_pInstance->GetCompositeBoneMatrix(m_dwModelInstanceIndex,
                                            m_iBoneIndex, &pMatrix)) {
      D3DXMATRIX *pBoneMat;
      m_pInstance->GetBoneMatrix(m_dwModelInstanceIndex, m_iBoneIndex,
                                 &pBoneMat);
      D3DXMATRIX mat = *pMatrix;
      mat._41 = pBoneMat->_41;
      mat._42 = pBoneMat->_42;
      mat._43 = pBoneMat->_43;
      // 현재 위치를 추가합니다.
      D3DXMATRIX matPoint;
      D3DXMATRIX matTranslation;
      D3DXMATRIX matRotation;

      // D3DXMatrixTranslation(&matTranslation, 0.0f, m_fLength, 0.0f);
      D3DXMatrixTranslation(&matTranslation, 0.0f, 0.0f,
                            m_fLength * fReachScale);
      D3DXMatrixRotationZ(&matRotation, D3DXToRadian(m_fRotation));

      matPoint = /**pMatrix*/ mat * matRotation;
      /*TPDTVertex PDTVertex;
      PDTVertex.position.x = m_fx + matPoint._41;
      PDTVertex.position.y = m_fy + matPoint._42;
      PDTVertex.position.z = m_fz + matPoint._43;
      PDTVertex.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.1f);
      m_PDTVertexVector.push_back(PDTVertex);*/
      m_ShortTimePointList.push_front(
          TTimePoint(0.0f, D3DXVECTOR3(m_fx + matPoint._41, m_fy + matPoint._42,
                                       m_fz + matPoint._43)));

      matPoint = matTranslation * matPoint;
      /*PDTVertex.position.x = m_fx + matPoint._41;
      PDTVertex.position.y = m_fy + matPoint._42;
      PDTVertex.position.z = m_fz + matPoint._43;
      PDTVertex.diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.1f);
      m_PDTVertexVector.push_back(PDTVertex);*/
      m_LongTimePointList.push_front(
          TTimePoint(0.0f, D3DXVECTOR3(m_fx + matPoint._41, m_fy + matPoint._42,
                                       m_fz + matPoint._43)));
    }
  }

  // if (!BuildVertex())
  //	return;
}

bool CWeaponTrace::BuildVertex() {
  const int max_size = 300;
  // calculate speed
  float h[max_size];
  float stk[max_size];
  int sp = 0;
  D3DXVECTOR3 r[max_size];

  if (m_LongTimePointList.size() <= 1)
    return false;

  // Tracef("## %f %f %f\n", m_LongTimePointList[0].second.x,
  // m_LongTimePointList[0].second.y, m_LongTimePointList[0].second.z);

  /*m_LongTimePointList.clear();
  m_LongTimePointList.push_back(TTimePoint(0.00,D3DXVECTOR3(0,0,0)));
  m_LongTimePointList.push_back(TTimePoint(0.01,D3DXVECTOR3(0,1,0)));
  m_LongTimePointList.push_back(TTimePoint(0.04,D3DXVECTOR3(0,1,0)));
  m_LongTimePointList.push_back(TTimePoint(0.05,D3DXVECTOR3(0,0,0)));
  m_ShortTimePointList = m_LongTimePointList;

*/
  std::vector<TPDTVertex> m_ShortVertexVector, m_LongVertexVector;

  float length = std::min(m_fLifeTime, m_LongTimePointList.back().first);

  int n = m_LongTimePointList.size() - 1;
  assert(n < max_size - 1);

  // cubic spline

  for (int loop = 0; loop <= 1; ++loop) {
    TTimePointList &Input = (loop) ? m_LongTimePointList : m_ShortTimePointList;
    std::vector<TPDTVertex> &Output =
        (loop) ? m_LongVertexVector : m_ShortVertexVector;
    TTimePointList::iterator it;
    int i;

    for (i = 0; i < n; ++i) {
      h[i] = Input[i + 1].first - Input[i].first;
      r[i] = (Input[i + 1].second - Input[i].second) * (3 / h[i]);
    }
    r[n] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    for (i = n; i > 0; i--) {
      r[i] += r[i - 1];
    }

    float rate = 0.5f;
    r[0] *= 0.5f;
    stk[sp++] = rate;
    for (i = 1; i < n; i++) {
      r[i] -= r[i - 1];
      rate = 1 / (4 - rate);
      r[i] *= rate;
      stk[sp++] = rate;
    }
    r[n] -= r[n - 1];
    rate = 1 / (2 - rate);
    r[n] *= rate;

    for (i = n - 1; i >= 0; i--) {
      r[i] -= stk[--sp] * r[i + 1];
    }

    int base = 0;
    D3DXVECTOR3 a, b, c, d;
    D3DXVECTOR3 v3Tmp = Input[base + 1].second - Input[base].second;
    float timebase = 0, timenext = h[base], dt = m_fSamplingTime;
    a = Input[base].second;
    b = r[base];
    c = (3 * v3Tmp - r[base + 1] * h[base] - (2 * h[base]) * r[base]) *
        (1 / (h[base] * h[base]));
    d = (-2 * v3Tmp + (r[base + 1] + r[base]) * h[base]) *
        (1 / (h[base] * h[base] * h[base]));

    for (float t = 0; t <= length; t += dt) {
      while (t > timenext) {
        timebase = timenext;
        base++;
        if (base >= n)
          break;
        D3DXVECTOR3 v3Tmp = Input[base + 1].second - Input[base].second;
        a = Input[base].second;
        b = r[base];
        c = (3 * v3Tmp - r[base + 1] * h[base] - (2 * h[base]) * r[base]) *
            (1 / (h[base] * h[base]));
        d = (-2 * v3Tmp + (r[base + 1] + r[base]) * h[base]) *
            (1 / (h[base] * h[base] * h[base]));

        timenext += h[base];
        if (loop) {
          // Tracef("%f:%f %f
          // %f\n",Input[base].first,Input[base].second.x,Input[base].second.y,Input[base].second.z);
        }
      }
      if (base > n)
        break;
      float cc = t - timebase;

      TPDTVertex v;
      // v.diffuse = D3DXCOLOR(0.3f,0.8f,1.0f,
      // (loop)?max(1.0f-(t/m_fLifeTime),0.0f)/2:0.0f );
      float ttt =
          std::min(std::max((t + Input[0].first) / m_fLifeTime, 0.0f), 1.0f);
      v.diffuse = D3DXCOLOR(
          0.3f, 0.8f, 1.0f,
          (loop) ? std::min(std::max((1.0f - ttt) * (1.0f - ttt) / 2.5f - 0.1f,
                                     0.0f),
                            1.0f)
                 : 0.0f);
      // v.diffuse = D3DXCOLOR(0.0f,0.0f,0.0f,
      // (loop)?min(max((1.0f-ttt)*(1.0f-ttt)-0.1f,0.0f),1.0f):0.0f ); v.diffuse
      // =	0xffffffff;
      v.position = a + cc * (b + cc * (c + cc * d)); // next position
      v.texCoord.x = t / m_fLifeTime;
      v.texCoord.y = loop ? 0 : 1;
      Output.push_back(v);
      if (loop) {
        //	Tracef("%f %f %f\n", timebase,t,timenext);
        // Tracef("a:%f %f %f\nb:%f %f %f \nc:%f %f %f \nd:%f %f %f,
        // \n",,a.x,a.y,a.z,b.x,b.y,b.z,c.x,c.y,c.z,d.x,d.y,d.z);

        // Tracef("%f %f %f\n",v.position.x,v.position.y,v.position.z);
        /*D3DXMATRIX * pBoneMat;
        m_pInstance->GetBoneMatrix(m_dwModelInstanceIndex, 55, &pBoneMat);
        D3DXVECTOR3
        vbone(m_fx+pBoneMat->_41,m_fy+pBoneMat->_42,m_fz+pBoneMat->_43); float
        len = D3DXVec3Length(&(v.position-vbone));*/
      }
    }
  }

  // build vertex

  m_PDTVertexVector.clear();

  /*
  TTimePointList::iterator lit1,lit2, sit1,sit2;

  lit2 = lit1 = m_LongTimePointList.begin();
  ++lit2;

  sit2 = sit1 = m_ShortTimePointList.begin();
  ++sit2;
  */
  std::vector<TPDTVertex>::iterator lit, sit;
  for (lit = m_LongVertexVector.begin(), sit = m_ShortVertexVector.begin();
       lit != m_LongVertexVector.end(); ++lit, ++sit) {
    m_PDTVertexVector.push_back(*lit);
    m_PDTVertexVector.push_back(*sit);
    /*float len = D3DXVec3Length(&(lit->position - sit->position));
    if (len>160)
            Tracef("dist:%f\n",len);*/
  }

  return true;
}

void CWeaponTrace::Render() {
  // if (!m_isPlaying)
  //	return;
  // if (m_CurvingTraceVector.size() < 4)
  //	return;

  if (!BuildVertex())
    return;

  if (m_PDTVertexVector.size() < 4)
    return;

  LPDIRECT3DTEXTURE9 lpTexture = NULL;

  // Have to optimize
  D3DXMATRIX matWorld;
  D3DXMatrixIdentity(&matWorld);

  STATEMANAGER.SaveTransform(D3DTS_WORLD, &matWorld);
  STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
  STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

  STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000011);
  STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

  STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, TRUE);
  STATEMANAGER.SaveRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
  STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);

  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,
                                    (m_bUseTexture) ? D3DTOP_SELECTARG2
                                                    : D3DTOP_SELECTARG1);
  // STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
  // STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
  // STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
  STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,
                                    (m_bUseTexture) ? D3DTOP_SELECTARG2
                                                    : D3DTOP_SELECTARG1);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
  STATEMANAGER.SetTexture(0, lpTexture);
  STATEMANAGER.SetTexture(1, NULL);
  STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLESTRIP,
                               int(m_PDTVertexVector.size() - 2),
                               &m_PDTVertexVector[0], sizeof(TPDTVertex));

  STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

  STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ZFUNC);
  STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);

  STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
  STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

  STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
  STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
  STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);

  STATEMANAGER.RestoreTransform(D3DTS_WORLD);
  STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CWeaponTrace::UseAlpha() { m_bUseTexture = false; }

void CWeaponTrace::UseTexture() { m_bUseTexture = true; }

void CWeaponTrace::SetTexture(const char *c_szFileName) {
  CGraphicImage *pImage =
      (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
          "lot_ade10-2.tga");
  m_ImageInstance.SetImagePointer(pImage);

  // CGraphicTexture * pTexture = m_ImageInstance.GetTexturePointer();
  // m_lpTexture = pTexture->GetD3DTexture();
}

bool CWeaponTrace::SetWeaponInstance(CGraphicThingInstance *pInstance,
                                     DWORD dwModelIndex,
                                     const char *c_szBoneName) {
  pInstance->Update();
  pInstance->DeformNoSkin();

  D3DXVECTOR3 v3Min;
  D3DXVECTOR3 v3Max;
  if (!pInstance->GetBoundBox(dwModelIndex, &v3Min, &v3Max))
    return false;

  m_iBoneIndex = 0;
  m_dwModelInstanceIndex = dwModelIndex;

  m_pInstance = pInstance;
  D3DXMATRIX *pmat;
  pInstance->GetBoneMatrix(dwModelIndex, 0, &pmat);
  D3DXVECTOR3 v3Bone(pmat->_41, pmat->_42, pmat->_43);

  const auto vv1 = (v3Bone - v3Min);
  const auto vv2 = (v3Bone - v3Max);
  m_fLength = sqrtf(fMAX(D3DXVec3LengthSq(&vv1), D3DXVec3LengthSq(&vv2)));

  return true;
}

void CWeaponTrace::SetPosition(float fx, float fy, float fz) {
  m_fx = fx;
  m_fy = fy;
  m_fz = fz;
}

void CWeaponTrace::SetRotation(float fRotation) { m_fRotation = fRotation; }

void CWeaponTrace::SetLifeTime(float fLifeTime) { m_fLifeTime = fLifeTime; }

void CWeaponTrace::SetSamplingTime(float fSamplingTime) {
  m_fSamplingTime = fSamplingTime;
}

void CWeaponTrace::TurnOn() { m_isPlaying = TRUE; }
void CWeaponTrace::TurnOff() {
  m_isPlaying = FALSE;
  // Clear();
}

void CWeaponTrace::Clear() {
  // m_PDTVertexVector.clear();
  // m_CurvingTraceVector.clear();

  m_ShortTimePointList.clear();
  m_LongTimePointList.clear();
  Initialize();
}

void CWeaponTrace::Initialize() {
  m_pInstance = NULL;
  m_dwModelInstanceIndex = 0;

  m_fx = 0.0f;
  m_fy = 0.0f;
  m_fz = 0.0f;
  m_fRotation = 0.0f;

  m_fLifeTime = 0.18f;
  // m_fLifeTime = 3.0f;
  m_fSamplingTime = 0.003f;
  // m_fLifeTime = 3.0f;
  // m_fSamplingTime = 0.003f;

  m_isPlaying = FALSE;

  m_bUseTexture = false;

  m_iBoneIndex = 0;

  m_fLastUpdate = CTimer::Instance().GetCurrentSecond();
  ///////////////////////////////////////////////////////////////////////

  // const int c_iSplineCount = 8;
  // m_SplineValueVector.clear();
  // m_SplineValueVector.resize(c_iSplineCount);

  // for (int i = 0; i < c_iSplineCount; ++i)
  //{
  //	float fValue = float(i) / float(c_iSplineCount);
  //	m_SplineValueVector[i].fValue1 = fValue;
  //	m_SplineValueVector[i].fValue2 = fValue * fValue;
  //	m_SplineValueVector[i].fValue3 = fValue * fValue * fValue;
  // }
}

CWeaponTrace::CWeaponTrace() { Initialize(); }
CWeaponTrace::~CWeaponTrace() {}
