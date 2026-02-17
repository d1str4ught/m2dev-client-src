#include "StdAfx.h"
#include "Mesh.h"
#include "Model.h"
#include "Material.h"
#include "Deform.h"

granny_data_type_definition GrannyPNT3322VertexType[5] = {
    {GrannyReal32Member, GrannyVertexPositionName, 0, 3},
    {GrannyReal32Member, GrannyVertexNormalName, 0, 3},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "0", 0, 2},
    {GrannyReal32Member, GrannyVertexTextureCoordinatesName "1", 0, 2},
    {GrannyEndMember}};

void CGrannyMesh::LoadIndices(void *dstBaseIndices) {
  const granny_mesh *pgrnMesh = GetGrannyMeshPointer();

  TIndex *dstIndices = ((TIndex *)dstBaseIndices) + m_idxBasePos;
  GrannyCopyMeshIndices(pgrnMesh, sizeof(TIndex), dstIndices);
}

void CGrannyMesh::LoadPNTVertices(void *dstBaseVertices) {
  const granny_mesh *pgrnMesh = GetGrannyMeshPointer();

  if (!GrannyMeshIsRigid(pgrnMesh))
    return;

  TPNTVertex *dstVertices = ((TPNTVertex *)dstBaseVertices) + m_vtxBasePos;
  GrannyCopyMeshVertices(pgrnMesh, m_pgrnMeshType, dstVertices);
}

void CGrannyMesh::NEW_LoadVertices(void *dstBaseVertices) {
  const granny_mesh *pgrnMesh = GetGrannyMeshPointer();

  if (!GrannyMeshIsRigid(pgrnMesh))
    return;

  TPNTVertex *dstVertices = ((TPNTVertex *)dstBaseVertices) + m_vtxBasePos;
  GrannyCopyMeshVertices(pgrnMesh, m_pgrnMeshType, dstVertices);
}

void CGrannyMesh::DeformPNTVertices(
    void *dstBaseVertices, D3DXMATRIX *boneMatrices,
    granny_mesh_binding *pgrnMeshBinding) const {
  assert(dstBaseVertices != NULL);
  assert(boneMatrices != NULL);
  assert(m_pgrnMeshDeformer != NULL);

  const granny_mesh *pgrnMesh = GetGrannyMeshPointer();

  TPNTVertex *srcVertices = (TPNTVertex *)GrannyGetMeshVertices(pgrnMesh);
  TPNTVertex *dstVertices = ((TPNTVertex *)dstBaseVertices) + m_vtxBasePos;

  int vtxCount = GrannyGetMeshVertexCount(pgrnMesh);

  // WORK
  granny_int32x *boneIndices =
      (granny_int32x *)GrannyGetMeshBindingToBoneIndices(pgrnMeshBinding);
  // END_OF_WORK

  extern bool CPU_HAS_SSE2;
  if (CPU_HAS_SSE2) {
    DeformPWNT3432toGrannyPNGBT33332(
        vtxCount, srcVertices, dstVertices, boneIndices,
        (granny_matrix_4x4 const *)boneMatrices, sizeof(granny_pwnt3432_vertex),
        sizeof(granny_pnt332_vertex));
  } else {
    GrannyDeformVertices(m_pgrnMeshDeformer, boneIndices, (float *)boneMatrices,
                         vtxCount, srcVertices, dstVertices);
  }
}

bool CGrannyMesh::CanDeformPNTVertices() const { return m_canDeformPNTVertex; }

const granny_mesh *CGrannyMesh::GetGrannyMeshPointer() const {
  return m_pgrnMesh;
}

const CGrannyMesh::TTriGroupNode *
CGrannyMesh::GetTriGroupNodeList(CGrannyMaterial::EType eMtrlType) const {
  return m_triGroupNodeLists[eMtrlType];
}

int CGrannyMesh::GetVertexCount() const {
  assert(m_pgrnMesh != NULL);
  return GrannyGetMeshVertexCount(m_pgrnMesh);
}

int CGrannyMesh::GetVertexBasePosition() const { return m_vtxBasePos; }

int CGrannyMesh::GetIndexBasePosition() const { return m_idxBasePos; }

// WORK
int *CGrannyMesh::GetDefaultBoneIndices() const {
  return (int *)GrannyGetMeshBindingToBoneIndices(m_pgrnMeshBindingTemp);
}
// END_OF_WORK

bool CGrannyMesh::IsEmpty() const {
  if (m_pgrnMesh)
    return false;

  return true;
}

bool CGrannyMesh::CreateFromGrannyMeshPointer(
    granny_skeleton *pgrnSkeleton, granny_mesh *pgrnMesh, int vtxBasePos,
    int idxBasePos, CGrannyMaterialPalette &rkMtrlPal) {
  assert(IsEmpty());

  m_pgrnMesh = pgrnMesh;
  m_vtxBasePos = vtxBasePos;
  m_idxBasePos = idxBasePos;

  if (m_pgrnMesh->BoneBindingCount < 0)
    return true;

  // WORK
  m_pgrnMeshBindingTemp =
      GrannyNewMeshBinding(m_pgrnMesh, pgrnSkeleton, pgrnSkeleton);
  // END_OF_WORK

  if (!GrannyMeshIsRigid(m_pgrnMesh)) {
    m_canDeformPNTVertex = true;

    granny_data_type_definition *pgrnInputType =
        GrannyGetMeshVertexType(m_pgrnMesh);
    granny_data_type_definition *pgrnOutputType = m_pgrnMeshType;

    m_pgrnMeshDeformer = GrannyNewMeshDeformer(pgrnInputType, pgrnOutputType,
                                               GrannyDeformPositionNormal,
                                               GrannyAllowUncopiedTail);
    assert(m_pgrnMeshDeformer != NULL && "Cannot create mesh deformer");
  }

  // Two Side Mesh
  if (!strncmp(m_pgrnMesh->Name, "2x", 2))
    m_isTwoSide = true;

  if (!LoadMaterials(rkMtrlPal))
    return false;

  if (!LoadTriGroupNodeList(rkMtrlPal))
    return false;

  return true;
}

bool CGrannyMesh::LoadTriGroupNodeList(CGrannyMaterialPalette &rkMtrlPal) {
  assert(m_pgrnMesh != NULL);
  assert(m_triGroupNodes == NULL);

  int mtrlCount = m_pgrnMesh->MaterialBindingCount;
  if (mtrlCount <= 0) // 천의 동굴 2층 크래쉬 발생
    return true;

  int GroupNodeCount = GrannyGetMeshTriangleGroupCount(m_pgrnMesh);
  if (GroupNodeCount <= 0)
    return true;

  m_triGroupNodes = new TTriGroupNode[GroupNodeCount];

  const granny_tri_material_group *c_pgrnTriGroups =
      GrannyGetMeshTriangleGroups(m_pgrnMesh);

  for (int g = 0; g < GroupNodeCount; ++g) {
    const granny_tri_material_group &c_rgrnTriGroup = c_pgrnTriGroups[g];
    TTriGroupNode *pTriGroupNode = m_triGroupNodes + g;

    pTriGroupNode->idxPos = m_idxBasePos + c_rgrnTriGroup.TriFirst * 3;
    pTriGroupNode->triCount = c_rgrnTriGroup.TriCount;

    int iMtrl = c_rgrnTriGroup.MaterialIndex;
    if (iMtrl < 0 || iMtrl >= mtrlCount) {
      pTriGroupNode->mtrlIndex = 0; // m_mtrlIndexVector[iMtrl];
    } else {
      pTriGroupNode->mtrlIndex = m_mtrlIndexVector[iMtrl];
    }

    const CGrannyMaterial &rkMtrl =
        rkMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
    pTriGroupNode->pNextTriGroupNode = m_triGroupNodeLists[rkMtrl.GetType()];
    m_triGroupNodeLists[rkMtrl.GetType()] = pTriGroupNode;
  }

  return true;
}

void CGrannyMesh::RebuildTriGroupNodeList() {
  assert(!"CGrannyMesh::RebuildTriGroupNodeList() - should not be called");
}

bool CGrannyMesh::LoadMaterials(CGrannyMaterialPalette &rkMtrlPal) {
  assert(m_pgrnMesh != NULL);

  if (m_pgrnMesh->MaterialBindingCount <= 0)
    return true;

  int mtrlCount = m_pgrnMesh->MaterialBindingCount;
  bool bHaveBlendThing = false;

  for (int m = 0; m < mtrlCount; ++m) {
    granny_material *pgrnMaterial = m_pgrnMesh->MaterialBindings[m].Material;
    DWORD mtrlIndex = rkMtrlPal.RegisterMaterial(pgrnMaterial);
    m_mtrlIndexVector.push_back(mtrlIndex);
    bHaveBlendThing |= rkMtrlPal.GetMaterialRef(mtrlIndex).GetType() ==
                       CGrannyMaterial::TYPE_BLEND_PNT;
  }
  m_bHaveBlendThing = bHaveBlendThing;

  return true;
}

bool CGrannyMesh::IsTwoSide() const { return m_isTwoSide; }

void CGrannyMesh::SetPNT2Mesh() { m_pgrnMeshType = GrannyPNT3322VertexType; }

void CGrannyMesh::Destroy() {
  if (m_triGroupNodes)
    delete[] m_triGroupNodes;

  m_mtrlIndexVector.clear();

  // WORK
  if (m_pgrnMeshBindingTemp)
    GrannyFreeMeshBinding(m_pgrnMeshBindingTemp);
  // END_OF_WORK

  if (m_pgrnMeshDeformer)
    GrannyFreeMeshDeformer(m_pgrnMeshDeformer);

  Initialize();
}

void CGrannyMesh::Initialize() {
  for (int r = 0; r < CGrannyMaterial::TYPE_MAX_NUM; ++r)
    m_triGroupNodeLists[r] = NULL;

  m_pgrnMeshType = GrannyPNT332VertexType;
  m_pgrnMesh = NULL;
  // WORK
  m_pgrnMeshBindingTemp = NULL;
  // END_OF_WORK
  m_pgrnMeshDeformer = NULL;

  m_triGroupNodes = NULL;

  m_vtxBasePos = 0;
  m_idxBasePos = 0;

  m_canDeformPNTVertex = false;
  m_isTwoSide = false;
  m_bHaveBlendThing = false;
}

CGrannyMesh::CGrannyMesh() { Initialize(); }

CGrannyMesh::~CGrannyMesh() { Destroy(); }
