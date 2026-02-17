#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::Clear() {
  m_kMtrlPal.Clear();

  DestroyDeviceObjects();
  // WORK
  __DestroyMeshBindingVector();
  // END_OF_WORK
  __DestroyMeshMatrices();
  __DestroyModelInstance();
  __DestroyWorldPose();

  __Initialize();
}

void CGrannyModelInstance::SetMainModelPointer(
    CGrannyModel *pModel,
    CGraphicVertexBuffer *pkSharedDeformableVertexBuffer) {
  SetLinkedModelPointer(pModel, pkSharedDeformableVertexBuffer, NULL);
}

void CGrannyModelInstance::SetLinkedModelPointer(
    CGrannyModel *pkModel, CGraphicVertexBuffer *pkSharedDeformableVertexBuffer,
    CGrannyModelInstance **ppkSkeletonInst) {
  Clear();

  if (m_pModel)
    m_pModel->Release();

  m_pModel = pkModel;

  m_pModel->AddReference();

  if (pkSharedDeformableVertexBuffer)
    __SetSharedDeformableVertexBuffer(pkSharedDeformableVertexBuffer);
  else
    __CreateDynamicVertexBuffer();

  __CreateModelInstance();

  // WORK
  if (ppkSkeletonInst && *ppkSkeletonInst) {
    m_ppkSkeletonInst = ppkSkeletonInst;
    __CreateWorldPose(*ppkSkeletonInst);
    __CreateMeshBindingVector(*ppkSkeletonInst);
  } else {
    __CreateWorldPose(NULL);
    __CreateMeshBindingVector(NULL);
  }
  // END_OF_WORK

  __CreateMeshMatrices();

  ResetLocalTime();

  m_kMtrlPal.Copy(pkModel->GetMaterialPalette());
}

// WORK
granny_world_pose *CGrannyModelInstance::__GetWorldPosePtr() const {
  if (m_pgrnWorldPoseReal)
    return m_pgrnWorldPoseReal;

  if (m_ppkSkeletonInst && *m_ppkSkeletonInst)
    return (*m_ppkSkeletonInst)->m_pgrnWorldPoseReal;

  assert(m_ppkSkeletonInst != NULL && "__GetWorldPosePtr - NO HAVE SKELETON");
  return NULL;
}

int *CGrannyModelInstance::__GetMeshBoneIndices(
    unsigned int iMeshBinding) const {
  assert(iMeshBinding < m_vct_pgrnMeshBinding.size());
  return (int *)GrannyGetMeshBindingToBoneIndices(
      m_vct_pgrnMeshBinding[iMeshBinding]);
}

bool CGrannyModelInstance::__CreateMeshBindingVector(
    CGrannyModelInstance *pkDstModelInst) {
  assert(m_vct_pgrnMeshBinding.empty());

  if (!m_pModel)
    return false;

  granny_model *pgrnModel = m_pModel->GetGrannyModelPointer();
  if (!pgrnModel)
    return false;

  granny_skeleton *pgrnDstSkeleton = pgrnModel->Skeleton;
  if (pkDstModelInst && pkDstModelInst->m_pModel &&
      pkDstModelInst->m_pModel->GetGrannyModelPointer())
    pgrnDstSkeleton =
        pkDstModelInst->m_pModel->GetGrannyModelPointer()->Skeleton;

  m_vct_pgrnMeshBinding.reserve(pgrnModel->MeshBindingCount);

  granny_int32 iMeshBinding;
  for (iMeshBinding = 0; iMeshBinding != pgrnModel->MeshBindingCount;
       ++iMeshBinding)
    m_vct_pgrnMeshBinding.push_back(
        GrannyNewMeshBinding(pgrnModel->MeshBindings[iMeshBinding].Mesh,
                             pgrnModel->Skeleton, pgrnDstSkeleton));

  return true;
}

void CGrannyModelInstance::__DestroyMeshBindingVector() {
  std::for_each(m_vct_pgrnMeshBinding.begin(), m_vct_pgrnMeshBinding.end(),
                GrannyFreeMeshBinding);
  m_vct_pgrnMeshBinding.clear();
}

// END_OF_WORK

void CGrannyModelInstance::__CreateWorldPose(
    CGrannyModelInstance *pkSkeletonInst) {
  assert(m_pgrnModelInstance != NULL);
  assert(m_pgrnWorldPoseReal == NULL);

  // WORK
  if (pkSkeletonInst)
    return;
  // END_OF_WORK

  granny_skeleton *pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);

  // WORK
  m_pgrnWorldPoseReal = GrannyNewWorldPose(pgrnSkeleton->BoneCount);
  // END_OF_WORK
}

void CGrannyModelInstance::__DestroyWorldPose() {
  if (!m_pgrnWorldPoseReal)
    return;

  GrannyFreeWorldPose(m_pgrnWorldPoseReal);
  m_pgrnWorldPoseReal = NULL;
}

void CGrannyModelInstance::__CreateModelInstance() {
  assert(m_pModel != NULL);
  assert(m_pgrnModelInstance == NULL);

  const granny_model *pgrnModel = m_pModel->GetGrannyModelPointer();
  m_pgrnModelInstance = GrannyInstantiateModel(pgrnModel);
}

void CGrannyModelInstance::__DestroyModelInstance() {
  if (!m_pgrnModelInstance)
    return;

  GrannyFreeModelInstance(m_pgrnModelInstance);
  m_pgrnModelInstance = NULL;
}

void CGrannyModelInstance::__CreateMeshMatrices() {
  assert(m_pModel != NULL);

  if (m_pModel->GetMeshCount() <=
      0) // 메쉬가 없는 (카메라 같은) 모델도 간혹 있다..
    return;

  int meshCount = m_pModel->GetMeshCount();
  m_meshMatrices = new D3DXMATRIX[meshCount];
}

void CGrannyModelInstance::__DestroyMeshMatrices() {
  if (!m_meshMatrices)
    return;

  delete[] m_meshMatrices;
  m_meshMatrices = NULL;
}

DWORD CGrannyModelInstance::GetDeformableVertexCount() {
  if (!m_pModel)
    return 0;

  return m_pModel->GetDeformVertexCount();
}

DWORD CGrannyModelInstance::GetVertexCount() {
  if (!m_pModel)
    return 0;

  return m_pModel->GetVertexCount();
}

// WORK

void CGrannyModelInstance::__SetSharedDeformableVertexBuffer(
    CGraphicVertexBuffer *pkSharedDeformableVertexBuffer) {
  m_pkSharedDeformableVertexBuffer = pkSharedDeformableVertexBuffer;
}

bool CGrannyModelInstance::__IsDeformableVertexBuffer() {
  if (m_pkSharedDeformableVertexBuffer)
    return true;

  return m_kLocalDeformableVertexBuffer.IsEmpty();
}

IDirect3DVertexBuffer9 *
CGrannyModelInstance::__GetDeformableD3DVertexBufferPtr() {
  return __GetDeformableVertexBufferRef().GetD3DVertexBuffer();
}

CGraphicVertexBuffer &CGrannyModelInstance::__GetDeformableVertexBufferRef() {
  if (m_pkSharedDeformableVertexBuffer)
    return *m_pkSharedDeformableVertexBuffer;

  return m_kLocalDeformableVertexBuffer;
}

void CGrannyModelInstance::__CreateDynamicVertexBuffer() {
  assert(m_pModel != NULL);
  assert(m_kLocalDeformableVertexBuffer.IsEmpty());

  int vtxCount = m_pModel->GetDeformVertexCount();

  if (0 != vtxCount) {
    if (!m_kLocalDeformableVertexBuffer.Create(
            vtxCount, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1,
            D3DUSAGE_DYNAMIC, D3DPOOL_DEFAULT))
      return;
  }
}

void CGrannyModelInstance::__DestroyDynamicVertexBuffer() {
  m_kLocalDeformableVertexBuffer.Destroy();
  m_pkSharedDeformableVertexBuffer = NULL;
}

// END_OF_WORK

bool CGrannyModelInstance::GetBoneIndexByName(const char *c_szBoneName,
                                              int *pBoneIndex) const {
  assert(m_pgrnModelInstance != NULL);

  granny_skeleton *pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);

  if (!GrannyFindBoneByName(pgrnSkeleton, c_szBoneName, pBoneIndex))
    return false;

  return true;
}

const float *CGrannyModelInstance::GetBoneMatrixPointer(int iBone) const {
  const float *bones = GrannyGetWorldPose4x4(__GetWorldPosePtr(), iBone);
  if (!bones) {
    granny_model *pModel = m_pModel->GetGrannyModelPointer();
    // TraceError("GrannyModelInstance(%s).GetBoneMatrixPointer(boneIndex(%d)).NOT_FOUND_BONE",
    // pModel->Name, iBone);
    return NULL;
  }
  return bones;
}

const float *
CGrannyModelInstance::GetCompositeBoneMatrixPointer(int iBone) const {
  // NOTE : GrannyGetWorldPose4x4는 스케일 값등이 잘못나올 수 있음.. 그래니가
  // 속도를 위해
  //        GrannyGetWorldPose4x4에 모든 matrix 원소를 제 값으로 넣지 않음
  return GrannyGetWorldPoseComposite4x4(__GetWorldPosePtr(), iBone);
}

void CGrannyModelInstance::ReloadTexture() {
  assert("현재 사용하지 않음 - CGrannyModelInstance::ReloadTexture()");
  /*
          assert(m_pModel != NULL);
          const CGrannyMaterialPalette & c_rGrannyMaterialPalette =
     m_pModel->GetMaterialPalette(); DWORD dwMaterialCount =
     c_rGrannyMaterialPalette.GetMaterialCount(); for (DWORD dwMtrIndex = 0;
     dwMtrIndex < dwMaterialCount; ++dwMtrIndex)
          {
                  const CGrannyMaterial & c_rGrannyMaterial =
     c_rGrannyMaterialPalette.GetMaterialRef(dwMtrIndex); CGraphicImage *
     pImageStage0 = c_rGrannyMaterial.GetImagePointer(0); if (pImageStage0)
                          pImageStage0->Reload();
                  CGraphicImage * pImageStage1 =
     c_rGrannyMaterial.GetImagePointer(1); if (pImageStage1)
                          pImageStage1->Reload();
          }
  */
}