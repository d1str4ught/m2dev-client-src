#include "Stdafx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::MakeBoundBox(TBoundBox* pBoundBox, 
										 const float* mat, 
										 const float* OBBMin, 
										 const float* OBBMax, 
										 D3DXVECTOR3* vtMin, 
										 D3DXVECTOR3* vtMax)
{
	pBoundBox->sx = OBBMin[0] * mat[0] + OBBMin[1] * mat[4] + OBBMin[2] * mat[8] + mat[12];
	pBoundBox->sy = OBBMin[0] * mat[1] + OBBMin[1] * mat[5] + OBBMin[2] * mat[9] + mat[13];
	pBoundBox->sz = OBBMin[0] * mat[2] + OBBMin[1] * mat[6] + OBBMin[2] * mat[10] + mat[14];

	pBoundBox->ex = OBBMax[0] * mat[0] + OBBMax[1] * mat[4] + OBBMax[2] * mat[8] + mat[12];
	pBoundBox->ey = OBBMax[0] * mat[1] + OBBMax[1] * mat[5] + OBBMax[2] * mat[9] + mat[13];
	pBoundBox->ez = OBBMax[0] * mat[2] + OBBMax[1] * mat[6] + OBBMax[2] * mat[10] + mat[14];

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

bool CGrannyModelInstance::Intersect(const D3DXMATRIX * c_pMatrix,
									 float * /*pu*/, float * /*pv*/, float * pt)
{
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

	for (int m = 0; m < meshCount; ++m)
	{
		//const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(m);
		const granny_mesh * pgrnMesh = m_pModel->GetGrannyModelPointer()->MeshBindings[m].Mesh;

		for (int b = 0; b < pgrnMesh->BoneBindingCount; ++b)
		{
			const granny_bone_binding& rgrnBoneBinding = pgrnMesh->BoneBindings[b];

			TBoundBox * pBoundBox = s_boundBoxPool.alloc();

			// WORK
			float * Transform = GrannyGetWorldPose4x4(__GetWorldPosePtr(), __GetMeshBoneIndices(m)[b]);
			// END_OF_WORK

			MakeBoundBox(pBoundBox,
						 Transform,
						 rgrnBoneBinding.OBBMin,
						 rgrnBoneBinding.OBBMax,
						 &vtMin,
						 &vtMax);

			pBoundBox->meshIndex = m;
			pBoundBox->boneIndex = b;
		}
	}

	if (!IntersectCube(c_pMatrix,
					   vtMin.x, vtMin.y, vtMin.z,
					   vtMax.x, vtMax.y, vtMax.z,
					   ms_vtPickRayOrig, ms_vtPickRayDir,
					   &u, &v, &t))
	{
		return ret;
	}

	return true;

/*
	TBoundBox* boundBoxs = s_boundBoxPool.base();
	for (int i = 0; i < s_boundBoxPool.size(); ++i)
	{
		TBoundBox& rcurBoundBox=boundBoxs[i];

		if (!IntersectBoundBox(c_pMatrix, rcurBoundBox, &u, &v, &t)) 
			continue;

		granny_matrix_4x4* pgrnMatCompositeBuffer = GrannyGetWorldPoseComposite4x4Array(m_pgrnWorldPose);
		const CGrannyMesh* c_pMesh = m_pModel->GetMeshPointer(rcurBoundBox.meshIndex);
		if (!c_pMesh)
			continue;
		const granny_mesh* c_pgrnMesh = c_pMesh->GetGrannyMeshPointer();

		if (!GrannyMeshIsRigid(c_pgrnMesh))
		{
			//continue;
			ret = true;
		}
		else
		{
			D3DXMATRIX matMesh;
			int* toBoneIndices = c_pMesh->GetBoneIndices();
			D3DXMatrixMultiply(&matMesh, (D3DXMATRIX*) pgrnMatCompositeBuffer[toBoneIndices[0]], c_pMatrix);

			granny_tri_material_group* pgrnTriGroups = GrannyGetMeshTriangleGroups(c_pgrnMesh);
			int mtrlCount = c_pMesh->GetGrannyMeshPointer()->MaterialBindingCount;
			int vtxCount = GrannyGetMeshVertexCount(c_pgrnMesh);
			int groupCount = GrannyGetMeshTriangleGroupCount(c_pgrnMesh);

			TIndex*	modelIndices;
			TPNTVertex*	modelVertices;

			if (m_pModel->LockVertices((void**)&modelIndices, (void**)&modelVertices))
			{
				TIndex* meshIndices = modelIndices + c_pMesh->GetIndexBasePosition();
				TPNTVertex* meshVertices = modelVertices + c_pMesh->GetVertexBasePosition();

				for (int i = 0; i < groupCount; ++i)
				{
					granny_tri_material_group& rgrnTriGroup = pgrnTriGroups[i];

					if (rgrnTriGroup.MaterialIndex < 0 || rgrnTriGroup.MaterialIndex >= mtrlCount)
						continue;

					if (IntersectMesh(&matMesh,
									  meshVertices,
									  sizeof(TPNTVertex),
									  vtxCount,
									  meshIndices,
									  GrannyGetMeshIndexCount(c_pgrnMesh),
									  ms_vtPickRayOrig,
									  ms_vtPickRayDir,
									  &u, &v, &t))
					{
						ret = true;
						break;
					}
				}
				
				m_pModel->UnlockVertices();
			}
		}

		if (ret)
		{
			*pu = u;
			*pv = v;
			*pt = -t;

			if (c_szModelName)
			{
				if (!strncmp(c_pgrnMesh->Name, c_szModelName, strlen(c_szModelName)))
					return ret;

				ret = false;
				continue;
			}

			return ret;
		}
	}

	return (ret);
*/
}

#include "EterBase/Timer.h"

void CGrannyModelInstance::GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax)
{
	if (!m_pgrnModelInstance)
		return;

	TBoundBox BoundBox;

	vtMin->x = vtMin->y = vtMin->z = +100000.0f;
	vtMax->x = vtMax->y = vtMax->z = -100000.0f;

	int meshCount = m_pModel->GetMeshCount();
	for (int m = 0; m < meshCount; ++m)
	{
		//const CGrannyMesh* pMesh = m_pModel->GetMeshPointer(m);
		const granny_mesh* pgrnMesh = m_pModel->GetGrannyModelPointer()->MeshBindings[m].Mesh;

		// WORK
		int* boneIndices = __GetMeshBoneIndices(m);
		// END_OF_WORK
		for (int b = 0; b < pgrnMesh->BoneBindingCount; ++b)
		{
			const granny_bone_binding& rgrnBoneBinding = pgrnMesh->BoneBindings[b];

			MakeBoundBox(&BoundBox,
						 GrannyGetWorldPose4x4(__GetWorldPosePtr(), boneIndices[b]),
						 rgrnBoneBinding.OBBMin, rgrnBoneBinding.OBBMax, vtMin, vtMax);
		}
	}
}

bool CGrannyModelInstance::GetMeshMatrixPointer(int iMesh, const D3DXMATRIX ** c_ppMatrix) const
{
	if (!m_pgrnModelInstance)
		return false;

	int meshCount = m_pModel->GetMeshCount();

	if (meshCount <= 0)
		return false;

	// WORK
	//const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(iMesh);
	*c_ppMatrix = (D3DXMATRIX *)GrannyGetWorldPose4x4(__GetWorldPosePtr(), __GetMeshBoneIndices(iMesh)[0]);
	// END_OF_WORK

	return true;
}

/*
void CGraphicThingInstance::DrawBoundBox()
{
	if (!mc_pMeshVector)
		return;

	if (!m_pgrnWorldPose)
		return;

	D3DXVECTOR3 vtMin;
	D3DXVECTOR3 vtMax;

	SetDiffuseColor(0.0f, 1.0f, 0.0f);
	// 캐릭터 꽉차는 바운딩 박스
	//GetBoundBox(&vtMin, &vtMax);
	//DrawLineCube(vtMin.x, vtMin.y, vtMin.z, vtMax.x, vtMax.y, vtMax.z);
	//const CThing::TMeshVector& rmeshVector=mc_pModel->meshVector;    
	ms_lpd3dMatStack->LoadMatrix(&m_Matrix);

    for (size_t m=0; m<mc_pMeshVector->size(); ++m)
    {
        const CThing::TMesh& rmesh=mc_pMeshVector->at(m);

		for (int b=0; b<rmesh.pgrnMesh->BoneBindingCount; ++b)
		{
			granny_bone_binding& rgrnBoneBinding=rmesh.pgrnMesh->BoneBindings[b];

			int* toBoneIndices=GrannyGetMeshBindingToBoneIndices(rmesh.pgrnMeshBinding);

			D3DXMATRIX* pmat=(D3DXMATRIX*)GrannyGetWorldPose4x4(m_pgrnWorldPose, toBoneIndices[b]);

			D3DXVec3TransformCoord(&vtMin, &D3DXVECTOR3(rgrnBoneBinding.OBBMin), pmat);
			D3DXVec3TransformCoord(&vtMax, &D3DXVECTOR3(rgrnBoneBinding.OBBMax), pmat);

			DrawLineCube(vtMin.x, vtMin.y, vtMin.z, vtMax.x, vtMax.y, vtMax.z);
		}		
	}
}
*/
