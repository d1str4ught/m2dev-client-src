#include "Stdafx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::MakeBoundBox(TBoundBox *pBoundBox, const float *mat,
                                        const float *OBBMin,
                                        const float *OBBMax, D3DXVECTOR3 *vtMin,
                                        D3DXVECTOR3 *vtMax) {
  pBoundBox->sx =
      OBBMin[0] * mat[0] + OBBMin[1] * mat[4] + OBBMin[2] * mat[8] + mat[12];
  pBoundBox->sy =
      OBBMin[0] * mat[1] + OBBMin[1] * mat[5] + OBBMin[2] * mat[9] + mat[13];
  pBoundBox->sz =
      OBBMin[0] * mat[2] + OBBMin[1] * mat[6] + OBBMin[2] * mat[10] + mat[14];

  pBoundBox->ex =
      OBBMax[0] * mat[0] + OBBMax[1] * mat[4] + OBBMax[2] * mat[8] + mat[12];
  pBoundBox->ey =
      OBBMax[0] * mat[1] + OBBMax[1] * mat[5] + OBBMax[2] * mat[9] + mat[13];
  pBoundBox->ez =
      OBBMax[0] * mat[2] + OBBMax[1] * mat[6] + OBBMax[2] * mat[10] + mat[14];

  vtMin->x = std::min(vtMin->x, pBoundBox->sx);
  vtMin->x = std::min(vtMin->x, pBoundBox->ex);
  vtMin->y = std::min(vtMin->y, pBoundBox->sy);
  vtMin->y = std::min(vtMin->y, pBoundBox->ey);
  vtMin->z = std::min(vtMin->z, pBoundBox->sz);
  vtMin->z = std::min(vtMin->z, pBoundBox->ez);

  vtMax->x = std::max(vtMax->x, pBoundBox->sx);
  vtMax->x = std::max(vtMax->x, pBoundBox->ex);
  vtMax->y = std::max(vtMax->y, pBoundBox->sy);
  vtMax->y = std::max(vtMax->y, pBoundBox->ey);
  vtMax->z = std::max(vtMax->z, pBoundBox->sz);
  vtMax->z = std::max(vtMax->z, pBoundBox->ez);
}

bool CGrannyModelInstance::Intersect(const D3DXMATRIX *c_pMatrix,
                                     float * /*pu*/, float * /*pv*/,
                                     float *pt) {
  if (!m_pgrnModelInstance)
    return false;

  float u, v, t;
  bool ret = false;
  *pt = 100000000.0f;

  float max = 10000000.0f;
  D3DXVECTOR3 vtMin, vtMax;
  vtMin.x = vtMin.y = vtMin.z = max;
  vtMax.x = vtMax.y = vtMax.z = -max;

  static stl_stack_pool<TBoundBox> s_boundBoxPool(1024);
  s_boundBoxPool.clear();

  int meshCount = m_pModel->GetMeshCount();

  for (int m = 0; m < meshCount; ++m) {
    // const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(m);
    const granny_mesh *pgrnMesh =
        m_pModel->GetGrannyModelPointer()->MeshBindings[m].Mesh;

    for (int b = 0; b < pgrnMesh->BoneBindingCount; ++b) {
      const granny_bone_binding &rgrnBoneBinding = pgrnMesh->BoneBindings[b];

      TBoundBox *pBoundBox = s_boundBoxPool.alloc();

      // WORK
      float *Transform = GrannyGetWorldPose4x4(__GetWorldPosePtr(),
                                               __GetMeshBoneIndices(m)[b]);
      // END_OF_WORK

      MakeBoundBox(pBoundBox, Transform, rgrnBoneBinding.OBBMin,
                   rgrnBoneBinding.OBBMax, &vtMin, &vtMax);

      pBoundBox->meshIndex = m;
      pBoundBox->boneIndex = b;
    }
  }

  if (!IntersectCube(c_pMatrix, vtMin.x, vtMin.y, vtMin.z, vtMax.x, vtMax.y,
                     vtMax.z, ms_vtPickRayOrig, ms_vtPickRayDir, &u, &v, &t)) {
    return ret;
  }

  return true;
}

#include "EterBase/Timer.h"

void CGrannyModelInstance::GetBoundBox(D3DXVECTOR3 *vtMin, D3DXVECTOR3 *vtMax) {
  if (!m_pgrnModelInstance)
    return;

  TBoundBox BoundBox;

  vtMin->x = vtMin->y = vtMin->z = +100000.0f;
  vtMax->x = vtMax->y = vtMax->z = -100000.0f;

  int meshCount = m_pModel->GetMeshCount();
  for (int m = 0; m < meshCount; ++m) {
    // const CGrannyMesh* pMesh = m_pModel->GetMeshPointer(m);
    const granny_mesh *pgrnMesh =
        m_pModel->GetGrannyModelPointer()->MeshBindings[m].Mesh;

    // WORK
    int *boneIndices = __GetMeshBoneIndices(m);
    // END_OF_WORK
    for (int b = 0; b < pgrnMesh->BoneBindingCount; ++b) {
      const granny_bone_binding &rgrnBoneBinding = pgrnMesh->BoneBindings[b];

      MakeBoundBox(
          &BoundBox, GrannyGetWorldPose4x4(__GetWorldPosePtr(), boneIndices[b]),
          rgrnBoneBinding.OBBMin, rgrnBoneBinding.OBBMax, vtMin, vtMax);
    }
  }
}

bool CGrannyModelInstance::GetMeshMatrixPointer(
    int iMesh, const D3DXMATRIX **c_ppMatrix) const {
  if (!m_pgrnModelInstance)
    return false;

  int meshCount = m_pModel->GetMeshCount();

  if (meshCount <= 0)
    return false;

  // WORK
  // const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(iMesh);
  *c_ppMatrix = (D3DXMATRIX *)GrannyGetWorldPose4x4(
      __GetWorldPosePtr(), __GetMeshBoneIndices(iMesh)[0]);
  // END_OF_WORK

  return true;
}
