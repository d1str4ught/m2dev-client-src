///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest Class

///////////////////////////////////////////////////////////////////////
//	Include Files
#include "StdAfx.h"

#include <vector>
#include "EterBase/Filename.h"
#include "PackLib/PackManager.h"

#include "SpeedTreeForest.h"
#include "SpeedTreeConfig.h"
#include <cfloat>

using namespace std;

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest constructor

CSpeedTreeForest::CSpeedTreeForest()
    : m_fWindStrength(0.0f), m_afFog{0.0f, 0.0f, 0.0f, 0.0f},
      m_fAccumTime(0.0f), m_afLighting{0.0f, 0.0f, 0.0f, 1.0f, // direction
                                       0.2f, 0.2f, 0.2f, 1.0f, // ambient
                                       0.8f, 0.8f, 0.8f, 1.0f} // diffuse
{
  CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);

  m_afForestExtents[0] = m_afForestExtents[1] = m_afForestExtents[2] = FLT_MAX;
  m_afForestExtents[3] = m_afForestExtents[4] = m_afForestExtents[5] = -FLT_MAX;
}

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest destructor

CSpeedTreeForest::~CSpeedTreeForest() { Clear(); }

void CSpeedTreeForest::Clear() {
  for (auto tr : m_pMainTreeMap) {
    SpeedTreeWrapperPtr pMainTree = tr.second;

    UINT uiCount;
    auto ppInstances = pMainTree->GetInstances(uiCount);

    for (auto it : ppInstances)
      it->Clear();

    pMainTree->Clear();
  }

  if (m_pMainTreeMap.begin() != m_pMainTreeMap.end()) {
    m_pMainTreeMap.clear();
  }
}

CSpeedTreeForest::SpeedTreeWrapperPtr
CSpeedTreeForest::GetMainTree(DWORD dwCRC) {
  TTreeMap::iterator itor = m_pMainTreeMap.find(dwCRC);

  if (itor == m_pMainTreeMap.end())
    return NULL;

  return itor->second;
}

BOOL CSpeedTreeForest::GetMainTree(DWORD dwCRC, SpeedTreeWrapperPtr &ppMainTree,
                                   const char *c_pszFileName) {
  TTreeMap::iterator itor = m_pMainTreeMap.find(dwCRC);

  SpeedTreeWrapperPtr pTree;

  if (itor != m_pMainTreeMap.end())
    pTree = itor->second;
  else {
    TPackFile file;

    if (!CPackManager::Instance().GetFile(c_pszFileName, file))
      return FALSE;

    pTree = std::make_shared<CSpeedTreeWrapper>();

    if (!pTree->LoadTree(c_pszFileName, (const BYTE *)file.data(),
                         file.size())) {
      pTree.reset();
      return FALSE;
    }

    m_pMainTreeMap.insert(
        std::map<DWORD, SpeedTreeWrapperPtr>::value_type(dwCRC, pTree));
  }

  ppMainTree = pTree;
  return TRUE;
}

CSpeedTreeForest::SpeedTreeWrapperPtr
CSpeedTreeForest::CreateInstance(float x, float y, float z, DWORD dwTreeCRC,
                                 const char *c_szTreeName) {
  SpeedTreeWrapperPtr pMainTree;
  if (!GetMainTree(dwTreeCRC, pMainTree, c_szTreeName)) {
    return NULL;
  }

  SpeedTreeWrapperPtr pTreeInst = pMainTree->MakeInstance();
  pTreeInst->SetPosition(x, y, z);
  pTreeInst->RegisterBoundingSphere();
  return pTreeInst;
}

void CSpeedTreeForest::DeleteInstance(SpeedTreeWrapperPtr pInstance) {
  if (!pInstance)
    return;

  SpeedTreeWrapperPtr pParentTree = pInstance->InstanceOf();

  if (!pParentTree)
    return;

  pParentTree->DeleteInstance(pInstance);
}

void CSpeedTreeForest::UpdateSystem(float fCurrentTime) {
  // 업데이트 할 때 한번
  static float fLastTime = fCurrentTime;
  float fElapsedTime = fCurrentTime - fLastTime;
  CSpeedTreeRT::SetTime(fElapsedTime);

  m_fAccumTime += fElapsedTime;
  SetupWindMatrices(m_fAccumTime);
}

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest::AdjustExtents
void CSpeedTreeForest::AdjustExtents(float x, float y, float z) {
  // min
  m_afForestExtents[0] = __min(m_afForestExtents[0], x);
  m_afForestExtents[1] = __min(m_afForestExtents[1], y);
  m_afForestExtents[2] = __min(m_afForestExtents[2], z);

  // max
  m_afForestExtents[3] = __max(m_afForestExtents[3], x);
  m_afForestExtents[4] = __max(m_afForestExtents[4], y);
  m_afForestExtents[5] = __max(m_afForestExtents[5], z);
}

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest::SetWindStrength
void CSpeedTreeForest::SetWindStrength(float fStrength) {
  if (m_fWindStrength == fStrength)
    return;

  m_fWindStrength = fStrength;

  TTreeMap::iterator itor = m_pMainTreeMap.begin();
  UINT uiCount;

  while (itor != m_pMainTreeMap.end()) {
    auto pMainTree = (itor++)->second;
    auto ppInstances = pMainTree->GetInstances(uiCount);

    for (auto it : ppInstances)
      it->GetSpeedTree()->SetWindStrength(m_fWindStrength);
  }
}

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest::SetupWindMatrices
void CSpeedTreeForest::SetupWindMatrices(float fTimeInSecs) {
  // matrix computational data
  static float afMatrixTimes[c_nNumWindMatrices] = {0.0f};
  static float afFrequencies[c_nNumWindMatrices][2] = {
      {0.15f, 0.17f}, {0.25f, 0.15f}, {0.19f, 0.05f}, {0.15f, 0.22f}};

  // compute time since last call
  static float fTimeOfLastCall = 0.0f;
  float fTimeSinceLastCall = fTimeInSecs - fTimeOfLastCall;
  fTimeOfLastCall = fTimeInSecs;

  // wind strength
  static float fOldStrength = m_fWindStrength;

  // increment matrix times
  for (int i = 0; i < c_nNumWindMatrices; ++i)
    afMatrixTimes[i] += fTimeSinceLastCall;

  // compute maximum branch throw
  float fBaseAngle = m_fWindStrength * 35.0f;

  // build rotation matrices
  for (int j = 0; j < c_nNumWindMatrices; ++j) {
    // adjust time to prevent "jumping"
    if (m_fWindStrength != 0.0f)
      afMatrixTimes[j] = (afMatrixTimes[j] * fOldStrength) / m_fWindStrength;

    // compute percentages for each axis
    float fBaseFreq = m_fWindStrength * 20.0f;
    float fXPercent =
        sinf(fBaseFreq * afFrequencies[j % c_nNumWindMatrices][0] *
             afMatrixTimes[j]);
    float fYPercent =
        cosf(fBaseFreq * afFrequencies[j % c_nNumWindMatrices][1] *
             afMatrixTimes[j]);

    // build compound rotation matrix (rotate on 'x' then on 'y')
    const float c_fDeg2Rad = 57.2957795f;
    float fSinX = sinf(fBaseAngle * fXPercent / c_fDeg2Rad);
    float fSinY = sinf(fBaseAngle * fYPercent / c_fDeg2Rad);
    float fCosX = cosf(fBaseAngle * fXPercent / c_fDeg2Rad);
    float fCosY = cosf(fBaseAngle * fYPercent / c_fDeg2Rad);

    float afMatrix[16] = {0.0f};
    afMatrix[0] = fCosY;
    afMatrix[2] = -fSinY;
    afMatrix[4] = fSinX * fSinY;
    afMatrix[5] = fCosX;
    afMatrix[6] = fSinX * fCosY;
    afMatrix[8] = fSinY * fCosX;
    afMatrix[9] = -fSinX;
    afMatrix[10] = fCosX * fCosY;
    afMatrix[15] = 1.0f;

#ifdef WRAPPER_USE_CPU_WIND
    CSpeedTreeRT::SetWindMatrix(j, afMatrix);
#endif

#ifdef WRAPPER_USE_GPU_WIND
    // graphics API specific
    UploadWindMatrix(c_nVertexShader_WindMatrices + j * 4, afMatrix);
#endif
  }

  // track wind strength
  fOldStrength = m_fWindStrength;
}

///////////////////////////////////////////////////////////////////////
//	CSpeedTreeForest::SetLodLimits
/*
void CSpeedTreeForest::SetLodLimits(void)
{
        // find tallest tree
        float fTallest = -1.0f;

        TTreeMap::iterator itor = m_pMainTreeMap.begin();
        UINT uiCount;

        while (itor != m_pMainTreeMap.end())
        {
                CSpeedTreeWrapper * pMainTree = (itor++)->second;
                CSpeedTreeWrapper ** ppInstances =
pMainTree->GetInstances(uiCount);

                float fHeight;
                fHeight = pMainTree->GetBoundingBox()[5] -
pMainTree->GetBoundingBox()[0]; fTallest = __max(fHeight, fTallest);

                for (UINT i = 0; i < uiCount; ++i)
                {
                        fHeight = ppInstances[i]->GetBoundingBox()[5] -
ppInstances[i]->GetBoundingBox()[0]; fTallest = __max(fHeight, fTallest);
                }
        }

        itor = m_pMainTreeMap.begin();

        while (itor != m_pMainTreeMap.end())
        {
                CSpeedTreeWrapper * pMainTree = (itor++)->second;
                CSpeedTreeWrapper ** ppInstances =
pMainTree->GetInstances(uiCount);

                pMainTree->GetSpeedTree()->SetLodLimits(fTallest *
c_fNearLodFactor, fTallest * c_fFarLodFactor);

                for (UINT i = 0; i < uiCount; ++i)
                        ppInstances[i]->GetSpeedTree()->SetLodLimits(fTallest *
c_fNearLodFactor, fTallest * c_fFarLodFactor);
        }
}
*/
void CSpeedTreeForest::SetLight(const float *afDirection,
                                const float *afAmbient,
                                const float *afDiffuse) {
  m_afLighting[0] = afDirection[0];
  m_afLighting[1] = afDirection[1];
  m_afLighting[2] = afDirection[2];
  m_afLighting[3] = 1.0f;

  m_afLighting[4] = afAmbient[0];
  m_afLighting[5] = afAmbient[1];
  m_afLighting[6] = afAmbient[2];
  m_afLighting[7] = afAmbient[3];

  m_afLighting[8] = afDiffuse[0];
  m_afLighting[9] = afDiffuse[1];
  m_afLighting[10] = afDiffuse[2];
  m_afLighting[11] = afDiffuse[3];
}

void CSpeedTreeForest::SetFog(float fFogNear, float fFogFar) {
  const float c_fFogLinearScale = (1.0f / (fFogFar - fFogNear));

  m_afFog[0] = fFogNear;
  m_afFog[1] = fFogFar;
  m_afFog[2] = c_fFogLinearScale;
  m_afFog[3] = 0.0f;
}
