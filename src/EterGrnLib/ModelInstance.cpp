#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"
#include "EterLib/ResourceManager.h"

CGrannyModel *CGrannyModelInstance::GetModel() { return m_pModel; }

void CGrannyModelInstance::SetMaterialImagePointer(const char *c_szImageName,
                                                   CGraphicImage *pImage) {
  m_kMtrlPal.SetMaterialImagePointer(c_szImageName, pImage);
}

void CGrannyModelInstance::SetMaterialData(
    const char *c_szImageName, const SMaterialData &c_rkMaterialData) {
  m_kMtrlPal.SetMaterialData(c_szImageName, c_rkMaterialData);

  material_data_ = c_rkMaterialData;
}

void CGrannyModelInstance::SetSpecularInfo(const char *c_szMtrlName,
                                           BOOL bEnable, float fPower) {
  m_kMtrlPal.SetSpecularInfo(c_szMtrlName, bEnable, fPower);
}

void CGrannyModelInstance::SetLocalTime(float fLocalTime) {
  m_fLocalTime = fLocalTime;
}

int CGrannyModelInstance::ResetLocalTime() {
  m_fLocalTime = 0.0f;
  return 0;
}

float CGrannyModelInstance::GetLocalTime() { return m_fLocalTime; }

void CGrannyModelInstance::SetParentModelInstance(
    const CGrannyModelInstance *c_pParentModelInstance,
    const char *c_szBoneName) {
  int iBoneIndex;
  if (!c_pParentModelInstance->GetBoneIndexByName(c_szBoneName, &iBoneIndex))
    return;

  SetParentModelInstance(c_pParentModelInstance, iBoneIndex);
}

void CGrannyModelInstance::SetParentModelInstance(
    const CGrannyModelInstance *c_pParentModelInstance, int iBone) {
  mc_pParentInstance = c_pParentModelInstance;
  m_iParentBoneIndex = iBone;
}

bool CGrannyModelInstance::IsEmpty() {
  if (m_pModel) {
    // NO_MESH_BUG_FIX
    if (!m_meshMatrices)
      return true;
    // END_OF_NO_MESH_BUG_FIX
    return false;
  }

  return true;
}

bool CGrannyModelInstance::CreateDeviceObjects() {
  __CreateDynamicVertexBuffer();

  return true;
}

void CGrannyModelInstance::DestroyDeviceObjects() {
  __DestroyDynamicVertexBuffer();
}

void CGrannyModelInstance::__Initialize() {
  if (m_pModel) {
    m_pModel->Release();
  }

  m_pModel = NULL;
  mc_pParentInstance = NULL;
  m_iParentBoneIndex = 0;

  m_pgrnModelInstance = NULL;

  // WORK
  m_pgrnWorldPoseReal = NULL;
  // END_OF_WORK

  // TEST
  // m_pgrnWorldPose = NULL;
  m_ppkSkeletonInst = NULL;
  // END_OF_TEST

  m_meshMatrices = NULL;
  m_pgrnCtrl = NULL;
  m_pgrnAni = NULL;

  // MR-12: Fix specular isolation issue
  material_data_ = {};
  m_dwOldUpdateFrame = 0;
  // MR-12: -- END OF -- Fix specular isolation issue
}

CGrannyModelInstance::CGrannyModelInstance() {
  m_pModel = NULL;
  __Initialize();
}

CGrannyModelInstance::~CGrannyModelInstance() { Clear(); }

CDynamicPool<CGrannyModelInstance> CGrannyModelInstance::ms_kPool;

CGrannyModelInstance *CGrannyModelInstance::New() { return ms_kPool.Alloc(); }

void CGrannyModelInstance::Delete(CGrannyModelInstance *pkInst) {
  pkInst->Clear();
  ms_kPool.Free(pkInst);
}

void CGrannyModelInstance::DestroySystem() { ms_kPool.Destroy(); }