#include "StdAfx.h"
#include "Eterbase/Debug.h"
#include "Eterlib/Camera.h"
#include "EterBase/Timer.h"
#include "ThingInstance.h"
#include "Thing.h"
#include "ModelInstance.h"

CDynamicPool<CGraphicThingInstance>		CGraphicThingInstance::ms_kPool;

CGraphicThing* CGraphicThingInstance::GetBaseThingPtr()
{
	if (m_modelThingSetVector.empty())
		return NULL;

	TModelThingSet& rkModelThingSet=m_modelThingSetVector[0];
	if (rkModelThingSet.m_pLODThingRefVector.empty())
		return NULL;

	CGraphicThing::TRef* proThing=rkModelThingSet.m_pLODThingRefVector[0];
	if (!proThing)
		return NULL;

	CGraphicThing::TRef roThing=*proThing;
	return roThing.GetPointer();
}

bool CGraphicThingInstance::LessRenderOrder(CGraphicThingInstance* pkThingInst)
{
	return (GetBaseThingPtr()<pkThingInst->GetBaseThingPtr());
}

void CGraphicThingInstance::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);
}

void CGraphicThingInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CGraphicThingInstance* CGraphicThingInstance::New()
{
	return ms_kPool.Alloc();
}

void CGraphicThingInstance::Delete(CGraphicThingInstance* pkThingInst)
{
	pkThingInst->Clear();
	ms_kPool.Free(pkThingInst);
}

void CGraphicThingInstance::SetMotionAtEnd()
{
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), std::mem_fn(&CGrannyLODController::SetMotionAtEnd));	
}

bool CGraphicThingInstance::Picking(const D3DXVECTOR3 & v, const D3DXVECTOR3 & dir, float & out_x, float & out_y)
{
	if (!m_pHeightAttributeInstance)
		return false;
	return m_pHeightAttributeInstance->Picking(v,dir,out_x,out_y);
}


void CGraphicThingInstance::OnUpdateCollisionData(const CStaticCollisionDataVector * pscdVector)
{
	assert(pscdVector);
	CStaticCollisionDataVector::const_iterator it;
	for(it = pscdVector->begin();it!=pscdVector->end();++it)
	{
		AddCollision(&(*it),&GetTransform());
	}
}

void CGraphicThingInstance::OnUpdateHeighInstance(CAttributeInstance * pAttributeInstance)
{
	assert(pAttributeInstance);
	SetHeightInstance(pAttributeInstance);
}

bool CGraphicThingInstance::OnGetObjectHeight(float fX, float fY, float * pfHeight)
{
	if (m_pHeightAttributeInstance && m_pHeightAttributeInstance->GetHeight(fX, fY, pfHeight))
		return true;
	return false;
}

void CGraphicThingInstance::BuildBoundingSphere()
{
	D3DXVECTOR3 v3Min, v3Max;
	GetBoundBox(0, &v3Min, &v3Max);
	m_v3Center = (v3Min + v3Max) * 0.5f;
	D3DXVECTOR3 vDelta = (v3Max - v3Min);

	m_fRadius = D3DXVec3Length(&vDelta) * 0.5f + 50.0f; // extra length for attached objects
}

bool CGraphicThingInstance::GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius)
{
	if (m_fRadius <= 0)
	{
		BuildBoundingSphere();

		fRadius = m_fRadius;
		v3Center = m_v3Center;
	}
	else
	{
		fRadius = m_fRadius;
		v3Center = m_v3Center;
	}

	D3DXVec3TransformCoord(&v3Center, &v3Center, &GetTransform());
	return true;
}

void CGraphicThingInstance::BuildBoundingAABB()
{
	D3DXVECTOR3 v3Min, v3Max;
	GetBoundBox(0, &v3Min, &v3Max);
	m_v3Center = (v3Min + v3Max) * 0.5f;
	m_v3Min = v3Min;
	m_v3Max = v3Max;
}

bool CGraphicThingInstance::GetBoundingAABB(D3DXVECTOR3 & v3Min, D3DXVECTOR3 & v3Max)
{
	BuildBoundingAABB();
	
	v3Min = m_v3Min;
	v3Max = m_v3Max;

	D3DXVec3TransformCoord(&m_v3Center, &m_v3Center, &GetTransform());
	return true;
}

void CGraphicThingInstance::CalculateBBox()
{
	GetBoundBox(&m_v3BBoxMin, &m_v3BBoxMax);
	
	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX & c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}	
}

bool CGraphicThingInstance::CreateDeviceObjects()
{
	CGrannyLODController::FCreateDeviceObjects createDeviceObjects;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), createDeviceObjects);
	return true;
}

void CGraphicThingInstance::DestroyDeviceObjects()
{
	CGrannyLODController::FDestroyDeviceObjects destroyDeviceObjects;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), destroyDeviceObjects);
}

bool CGraphicThingInstance::CheckModelInstanceIndex(int iModelInstance)
{
	if (iModelInstance < 0)
		return false;

	int max = m_LODControllerVector.size();

	if (iModelInstance >= max)
		return false;

	return true;
}

bool CGraphicThingInstance::CheckModelThingIndex(int iModelThing)
{
	if (iModelThing < 0)
		return false;

	int max = m_modelThingSetVector.size();

	if (iModelThing >= max)
		return false;

	return true;
}

bool CGraphicThingInstance::CheckMotionThingIndex(DWORD dwMotionKey)
{
	std::map<DWORD, CGraphicThing::TRef *>::iterator itor = m_roMotionThingMap.find(dwMotionKey);

	if (m_roMotionThingMap.end() == itor)
		return false;

	return true;
}

bool CGraphicThingInstance::GetMotionThingPointer(DWORD dwKey, CGraphicThing ** ppMotion)
{
	if (!CheckMotionThingIndex(dwKey))
		return false;

	*ppMotion = m_roMotionThingMap[dwKey]->GetPointer();
	return true;
}

bool CGraphicThingInstance::IsMotionThing()
{
	return !m_roMotionThingMap.empty();
}

void CGraphicThingInstance::ReserveModelInstance(int iCount)
{
	stl_wipe(m_LODControllerVector);

	for (int i = 0; i < iCount; ++i)
	{
		CGrannyLODController * pInstance = new CGrannyLODController;
		m_LODControllerVector.push_back(pInstance);
	}
}

void CGraphicThingInstance::ReserveModelThing(int iCount)
{
	m_modelThingSetVector.resize(iCount);
}

bool CGraphicThingInstance::FindBoneIndex(int iModelInstance, const char* c_szBoneName, int* iRetBone)
{
	assert(CheckModelInstanceIndex(iModelInstance));

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[iModelInstance]->GetModelInstance();

	if (!pModelInstance)
		return false;

	return pModelInstance->GetBoneIndexByName(c_szBoneName, iRetBone);
}

void CGraphicThingInstance::AttachModelInstance(int iDstModelInstance, const char * c_szBoneName, int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)", iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)", iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}

	CGrannyLODController * pSrcLODController = m_LODControllerVector[iSrcModelInstance];
	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance];
	pDstLODController->AttachModelInstance(pSrcLODController, c_szBoneName);
}

void CGraphicThingInstance::AttachModelInstance(int iDstModelInstance, const char * c_szBoneName, CGraphicThingInstance & rSrcInstance, int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)", iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}
	if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, c_szBoneName=%s, iSrcModelInstance=%d)", iDstModelInstance, c_szBoneName, iSrcModelInstance);
		return;
	}

	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance];
	CGrannyLODController * pSrcLODController = rSrcInstance.m_LODControllerVector[iSrcModelInstance];
	pDstLODController->AttachModelInstance(pSrcLODController, c_szBoneName);
}

void CGraphicThingInstance::DetachModelInstance(int iDstModelInstance, CGraphicThingInstance & rSrcInstance, int iSrcModelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, iSrcModelInstance=%d)", iDstModelInstance, iSrcModelInstance);
		return;
	}
	if (!rSrcInstance.CheckModelInstanceIndex(iSrcModelInstance))
	{
		TraceError("CGraphicThingInstance::AttachModelInstance(iDstModelInstance=%d, iSrcModelInstance=%d)", iDstModelInstance, iSrcModelInstance);
		return;
	}

	CGrannyLODController * pDstLODController = m_LODControllerVector[iDstModelInstance];
	CGrannyLODController * pSrcLODController = rSrcInstance.m_LODControllerVector[iSrcModelInstance];
	pDstLODController->DetachModelInstance(pSrcLODController);
}

bool CGraphicThingInstance::GetBonePosition(int iModelIndex, int iBoneIndex, float * pfx, float * pfy, float * pfz)
{
	assert(CheckModelInstanceIndex(iModelIndex));

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[iModelIndex]->GetModelInstance();

	if (!pModelInstance)
		return false;

	const float * pfMatrix = pModelInstance->GetBoneMatrixPointer(iBoneIndex);

	*pfx = pfMatrix[12];
	*pfy = pfMatrix[13];
	*pfz = pfMatrix[14];
	return true;
}
//iSkelInstance 가 있으면 기본 본에 Link(본이 붙는것)시키고,
//없으면 기본 본에 attach(좌표만 가져다 쓰는것) 됩니다.
bool CGraphicThingInstance::SetModelInstance(int iDstModelInstance, int iSrcModelThing, int iSrcModel,int iSkelInstance)
{
	if (!CheckModelInstanceIndex(iDstModelInstance))
	{
		TraceError("CGraphicThingInstance::SetModelInstance(iDstModelInstance=%d, pModelThing=%d, iSrcModel=%d)\n", iDstModelInstance, iSrcModelThing, iSrcModel);
		return false;
	}
	if (!CheckModelThingIndex(iSrcModelThing))
	{
		TraceError("CGraphicThingInstance::SetModelInstance(iDstModelInstance=%d, pModelThing=%d, iSrcModel=%d)\n", iDstModelInstance, iSrcModelThing, iSrcModel);
		return false;
	}

	CGrannyLODController * pController = m_LODControllerVector[iDstModelInstance];
	if (!pController)
		return false;

	// HAIR_LINK
	CGrannyLODController * pSkelController = NULL;
	if (iSkelInstance != DONTUSEVALUE)
	{
		if (!CheckModelInstanceIndex(iSkelInstance))
		{
			TraceError("CGraphicThingInstance::SetModelInstanceByOtherSkeletonInstance(iSkelInstance=%d, iDstModelInstance=%d, pModelThing=%d, iSrcModel=%d)\n", iSkelInstance, iDstModelInstance, iSrcModelThing, iSrcModel);
			return false;
		}
		pSkelController = m_LODControllerVector[iSkelInstance];
		if (!pSkelController)
			return false;
	}
	// END_OF_HAIR_LINK
	
	TModelThingSet & rModelThingSet = m_modelThingSetVector[iSrcModelThing];

	pController->Clear();


	for (DWORD i = 0; i < rModelThingSet.m_pLODThingRefVector.size(); ++i)
	{
		if (rModelThingSet.m_pLODThingRefVector[i]->IsNull())
			return false;

		pController->AddModel(rModelThingSet.m_pLODThingRefVector[i]->GetPointer(), iSrcModel, pSkelController);
	}
	return true;
}

void CGraphicThingInstance::SetMaterialImagePointer(UINT ePart, const char* c_szImageName, CGraphicImage* pImage)
{
	if (ePart>=m_LODControllerVector.size())
	{
		TraceError("CGraphicThingInstance::SetMaterialImagePointer(ePart(%d)<uPartCount(%d), c_szImageName=%s, pImage=%s) - ePart OUT OF RANGE",
			ePart, m_LODControllerVector.size(), c_szImageName, pImage->GetFileName());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetMaterialImagePointer(ePart(%d), c_szImageName=%s, pImage=%s) - ePart Data is NULL",
			ePart, m_LODControllerVector.size(), c_szImageName, pImage->GetFileName());

		return;
	}

	m_LODControllerVector[ePart]->SetMaterialImagePointer(c_szImageName, pImage);	
}

void CGraphicThingInstance::SetMaterialData(UINT ePart, const char* c_szImageName, SMaterialData kMaterialData)
{
	if (ePart>=m_LODControllerVector.size())
	{
		TraceError("CGraphicThingInstance::SetMaterialData(ePart(%d)<uPartCount(%d)) - ePart OUT OF RANGE",
			ePart, m_LODControllerVector.size());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetMaterialData(ePart(%d)) - ePart Data is NULL",
			ePart, m_LODControllerVector.size());

		return;
	}

	m_LODControllerVector[ePart]->SetMaterialData(c_szImageName, kMaterialData);	
}

void CGraphicThingInstance::SetSpecularInfo(UINT ePart, const char* c_szMtrlName, BOOL bEnable, float fPower)
{
	if (ePart>=m_LODControllerVector.size())
	{
		TraceError("CGraphicThingInstance::SetSpecularInfo(ePart(%d)<uPartCount(%d)) - ePart OUT OF RANGE",
			ePart, m_LODControllerVector.size());

		return;
	}

	if (!m_LODControllerVector[ePart])
	{
		TraceError("CGraphicThingInstance::SetSpecularInfo(ePart(%d)) - ePart Data is NULL",
			ePart, m_LODControllerVector.size());

		return;
	}

	m_LODControllerVector[ePart]->SetSpecularInfo(c_szMtrlName, bEnable, fPower);	
}

bool CGraphicThingInstance::SetMotion(DWORD dwMotionKey, float blendTime, int loopCount, float speedRatio)
{
	if (!CheckMotionThingIndex(dwMotionKey))
		return false;

	std::map<DWORD, CGraphicThing::TRef *>::iterator itor = m_roMotionThingMap.find(dwMotionKey);
	CGraphicThing::TRef * proMotionThing = itor->second;
	CGraphicThing * pMotionThing = proMotionThing->GetPointer();

	if (!pMotionThing)
		return false;

	if (!pMotionThing->CheckMotionIndex(0))
		return false;

	CGrannyLODController::FSetMotionPointer SetMotionPointer;
	SetMotionPointer.m_pMotion = pMotionThing->GetMotionPointer(0);
	SetMotionPointer.m_blendTime = blendTime;
	SetMotionPointer.m_loopCount = loopCount;
	SetMotionPointer.m_speedRatio = speedRatio;

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), SetMotionPointer);
	return true;
}

bool CGraphicThingInstance::ChangeMotion(DWORD dwMotionKey, int loopCount, float speedRatio)
{
	if (!CheckMotionThingIndex(dwMotionKey))
		return false;

	std::map<DWORD, CGraphicThing::TRef *>::iterator itor = m_roMotionThingMap.find(dwMotionKey);
	CGraphicThing::TRef * proMotionThing = itor->second;
	CGraphicThing * pMotionThing = proMotionThing->GetPointer();

	if (!pMotionThing)
		return false;

	if (!pMotionThing->CheckMotionIndex(0))
		return false;

	CGrannyLODController::FChangeMotionPointer ChangeMotionPointer;
	ChangeMotionPointer.m_pMotion = pMotionThing->GetMotionPointer(0);
	ChangeMotionPointer.m_loopCount = loopCount;
	ChangeMotionPointer.m_speedRatio = speedRatio;

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), ChangeMotionPointer);
	return true;
}

void CGraphicThingInstance::SetEndStopMotion()
{
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), CGrannyLODController::FEndStopMotionPointer());
}

void CGraphicThingInstance::RegisterModelThing(int iModelThing, CGraphicThing * pModelThing)
{
	if (!CheckModelThingIndex(iModelThing))
	{
		TraceError("CGraphicThingInstance::RegisterModelThing(iModelThing=%d, pModelThing=%s)\n", iModelThing, pModelThing->GetFileName());
		return;
	}

	m_modelThingSetVector[iModelThing].Clear();

	if (pModelThing)
		RegisterLODThing(iModelThing, pModelThing);
}

void CGraphicThingInstance::RegisterLODThing(int iModelThing, CGraphicThing * pModelThing)
{
	assert(CheckModelThingIndex(iModelThing));
	CGraphicThing::TRef * pModelRef = new CGraphicThing::TRef;
	pModelRef->SetPointer(pModelThing);
	m_modelThingSetVector[iModelThing].m_pLODThingRefVector.push_back(pModelRef);
}

void CGraphicThingInstance::RegisterMotionThing(DWORD dwMotionKey, CGraphicThing* pMotionThing)
{
	CGraphicThing::TRef * pMotionRef = new CGraphicThing::TRef;
	pMotionRef->SetPointer(pMotionThing);
	m_roMotionThingMap.insert(std::make_pair(dwMotionKey, pMotionRef));
}

void CGraphicThingInstance::ResetLocalTime()
{
	//m_clockLast = GrannyGetSystemSeconds();
	m_fLastLocalTime = 0.0f;
	m_fLocalTime = 0.0f;

	CGrannyLODController::FResetLocalTime resetLocalTime;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), resetLocalTime);
}

/*
void CGraphicThingInstance::SetMotionSpeed(float fRate)
{
	m_fMotionTimeSpeed = fRate;
}
*/
void CGraphicThingInstance::InsertDelay(float fDelay)
{
	m_fDelay = fDelay;
}

float CGraphicThingInstance::GetLastLocalTime()
{
	return m_fLastLocalTime;
}

float CGraphicThingInstance::GetLocalTime()
{
	return m_fLocalTime;
}

float CGraphicThingInstance::GetSecondElapsed()
{
	return m_fSecondElapsed;
}

float CGraphicThingInstance::GetAverageSecondElapsed()
{
	return m_fAverageSecondElapsed;
}

bool CGraphicThingInstance::Intersect(float* pu, float* pv, float* pt)
{
	if (!CGraphicObjectInstance::isShow())
		return false;
	if (!m_bUpdated)
		return false;

	if (m_LODControllerVector.empty())
	{
		//TraceError("CGraphicThingInstance::Intersect - m_LODControllerVector is empty");
		return false;
	}
	
	return m_LODControllerVector[0]->Intersect(&GetTransform(), pu, pv, pt);
}

void CGraphicThingInstance::GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax)
{
	vtMin->x = vtMin->y = vtMin->z = 100000.0f;
	vtMax->x = vtMax->y = vtMax->z = -100000.0f;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), CGrannyLODController::FBoundBox(vtMin, vtMax));
}

BOOL CGraphicThingInstance::GetBoundBox(DWORD dwModelInstanceIndex, D3DXVECTOR3 * vtMin, D3DXVECTOR3 * vtMax)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	vtMin->x = vtMin->y = vtMin->z = 100000.0f;
	vtMax->x = vtMax->y = vtMax->z = -100000.0f;

	CGrannyLODController * pController = m_LODControllerVector[dwModelInstanceIndex];
	if (!pController->isModelInstance())
		return FALSE;

	CGrannyModelInstance * pModelInstance = pController->GetModelInstance();
	pModelInstance->GetBoundBox(vtMin, vtMax);
	return TRUE;
}

BOOL CGraphicThingInstance::GetBoneMatrix(DWORD dwModelInstanceIndex, DWORD dwBoneIndex, D3DXMATRIX ** ppMatrix)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[dwModelInstanceIndex]->GetModelInstance();
	if (!pModelInstance)
		return FALSE;

	*ppMatrix = (D3DXMATRIX *)pModelInstance->GetBoneMatrixPointer(dwBoneIndex);
	if (!*ppMatrix)
		return FALSE;

	return TRUE;
}

BOOL CGraphicThingInstance::GetCompositeBoneMatrix(DWORD dwModelInstanceIndex, DWORD dwBoneIndex, D3DXMATRIX ** ppMatrix)
{
	if (!CheckModelInstanceIndex(dwModelInstanceIndex))
		return FALSE;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[dwModelInstanceIndex]->GetModelInstance();
	if (!pModelInstance)
	{
		//TraceError("CGraphicThingInstance::GetCompositeBoneMatrix(dwModelInstanceIndex=%d, dwBoneIndex=%d, D3DXMATRIX ** ppMatrix)", dwModelInstanceIndex, dwBoneIndex);
		return FALSE;
	}
	
	*ppMatrix = (D3DXMATRIX *)pModelInstance->GetCompositeBoneMatrixPointer(dwBoneIndex);

	return TRUE;
}

void CGraphicThingInstance::UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed, int iModelInstanceIndex)
{
	//TraceError("%s",GetBaseThingPtr()->GetFileName());
	int nLODCount=m_LODControllerVector.size();
	if (iModelInstanceIndex>=nLODCount)
	{
		//TraceError("void CGraphicThingInstance::UpdateTransform(pMatrix, fSecondsElapsed=%f, iModelInstanceIndex=%d/nLODCount=%d)",
		//	fSecondsElapsed, iModelInstanceIndex, nLODCount);
		return;
	}

	CGrannyLODController* pkLODCtrl=m_LODControllerVector[iModelInstanceIndex];
	if (!pkLODCtrl)
	{
		//TraceError("void CGraphicThingInstance::UpdateTransform(pMatrix, fSecondsElapsed=%f, iModelInstanceIndex=%d/nLODCount=%d) - m_LODControllerVector[iModelInstanceIndex] == NULL",
		//	fSecondsElapsed, iModelInstanceIndex, nLODCount);
		return;
	}

	CGrannyModelInstance * pModelInstance = pkLODCtrl->GetModelInstance();
	if (!pModelInstance)
	{
	/*	TraceError("void CGraphicThingInstance::UpdateTransform(pMatrix, fSecondsElapsed=%f, iModelInstanceIndex=%d/nLODCount=%d) - pkLODCtrl->GetModelInstance() == NULL",
			fSecondsElapsed, iModelInstanceIndex, nLODCount);*/
		return;
	}

	pModelInstance->UpdateTransform(pMatrix, fSecondsElapsed);
}

void CGraphicThingInstance::DeformAll()
{
	m_bUpdated = true;

	CGrannyLODController::FDeformAll deform;
	deform.mc_pWorldMatrix = &m_worldMatrix;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), deform);
}

void CGraphicThingInstance::DeformNoSkin()
{
	m_bUpdated = true;

	std::vector<CGrannyLODController*>::iterator i;
	for (i=m_LODControllerVector.begin(); i!=m_LODControllerVector.end(); ++i)
	{
		CGrannyLODController* pkLOD=*i;
		if (pkLOD->isModelInstance())
			pkLOD->DeformNoSkin(&m_worldMatrix);
	}
}

void CGraphicThingInstance::OnDeform()
{
	m_bUpdated = true;

	CGrannyLODController::FDeform deform;
	deform.mc_pWorldMatrix = &m_worldMatrix;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), deform);
}

void CGraphicThingInstance::__SetLocalTime(float fLocalTime)
{
	m_fLastLocalTime = m_fLocalTime;
	m_fLocalTime = fLocalTime;

	CGrannyLODController::FSetLocalTime SetLocalTime;
	SetLocalTime.fLocalTime = fLocalTime;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), SetLocalTime);
}

void CGraphicThingInstance::UpdateLODLevel()
{
	CCamera * pcurCamera = CCameraManager::Instance().GetCurrentCamera();
	if (!pcurCamera)
	{
		TraceError("CGraphicThingInstance::UpdateLODLevel - GetCurrentCamera() == NULL");
		return;
	}

	const D3DXVECTOR3 & c_rv3TargetPosition = pcurCamera->GetTarget();
	const D3DXVECTOR3 & c_rv3CameraPosition = pcurCamera->GetEye();
	const D3DXVECTOR3 & c_v3Position = GetPosition();

	// NOTE : 중심으로부터의 거리 계산에 z값 차이는 사용하지 않는다. - [levites]
	CGrannyLODController::FUpdateLODLevel update;
	update.fDistanceFromCenter = sqrtf((c_rv3TargetPosition.x - c_v3Position.x) * (c_rv3TargetPosition.x - c_v3Position.x) +
									   (c_rv3TargetPosition.y - c_v3Position.y) * (c_rv3TargetPosition.y - c_v3Position.y));
	update.fDistanceFromCamera = sqrtf((c_rv3CameraPosition.x - c_v3Position.x) * (c_rv3CameraPosition.x - c_v3Position.x) +
									   (c_rv3CameraPosition.y - c_v3Position.y) * (c_rv3CameraPosition.y - c_v3Position.y) +
									   (c_rv3CameraPosition.z - c_v3Position.z) * (c_rv3CameraPosition.z - c_v3Position.z));

	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), update);
}

void CGraphicThingInstance::UpdateTime()
{
	//granny_system_clock clockNow = GrannyGetSystemSeconds();
	//m_fSecondElapsed = GrannyGetSecondsElapsed(&m_clockLast, &clockNow) * m_fMotionTimeSpeed;
	
	//DWORD t1=ELTimer_GetMSec();

	m_fSecondElapsed=CTimer::Instance().GetElapsedSecond();

	if (m_fDelay > m_fSecondElapsed)
	{
		m_fDelay -= m_fSecondElapsed;
		m_fSecondElapsed = 0.0f;
	}
	else
	{
		m_fSecondElapsed -= m_fDelay;
		m_fDelay = 0.0f;
	}

	m_fLastLocalTime = m_fLocalTime;
	m_fLocalTime += m_fSecondElapsed;
	m_fAverageSecondElapsed = m_fAverageSecondElapsed + (m_fSecondElapsed - m_fAverageSecondElapsed) / 4.0f;
	//m_clockLast = clockNow;

	CGrannyLODController::FUpdateTime update;
	update.fElapsedTime = m_fSecondElapsed;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), update);
}

void CGraphicThingInstance::OnUpdate()
{
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=timeGetTime();
#endif
	UpdateLODLevel();
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=timeGetTime();
#endif
	UpdateTime();
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3=timeGetTime();
#endif

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp=fopen("perf_thing_onupdate.txt", "w");

		if (t3-t1>3)
		{
			fprintf(fp, "GTU.Total %d (Time %f)\n", 
				t3-t1, ELTimer_GetMSec()/1000.0f);
			fprintf(fp, "GTU.CAL %d\n", t2-t1);
			fprintf(fp, "GTU.UP %d\n", t3-t2);
			fprintf(fp, "-------------------------------- \n");
			fflush(fp);
		}
		fflush(fp);
	}
#endif
}

void CGraphicThingInstance::OnRender()
{
	RenderWithOneTexture();
}

void CGraphicThingInstance::OnBlendRender()
{
	BlendRenderWithOneTexture();
}

void CGraphicThingInstance::RenderWithOneTexture()
{
	//assert(m_bUpdated);
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithOneTexture render;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), render);
}

void CGraphicThingInstance::BlendRenderWithOneTexture()
{
	//assert(m_bUpdated);
	if (!m_bUpdated)
		return;

	CGrannyLODController::FBlendRenderWithOneTexture blendRender;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), blendRender);
}

void CGraphicThingInstance::RenderWithTwoTexture()
{
	//assert(m_bUpdated);
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithTwoTexture render;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), render);
}

void CGraphicThingInstance::BlendRenderWithTwoTexture()
{
	//assert(m_bUpdated);
	if (!m_bUpdated)
		return;

	CGrannyLODController::FRenderWithTwoTexture blendRender;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), blendRender);
}

void CGraphicThingInstance::OnRenderToShadowMap()
{
	if (!m_bUpdated)
		return;
	
	CGrannyLODController::FRenderToShadowMap RenderToShadowMap;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), RenderToShadowMap);
}

void CGraphicThingInstance::OnRenderShadow()
{
	CGrannyLODController::FRenderShadow RenderShadow;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), RenderShadow);
}

void CGraphicThingInstance::OnRenderPCBlocker()
{
	CGrannyLODController::FRenderWithOneTexture RenderPCBlocker;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), RenderPCBlocker);
}

DWORD CGraphicThingInstance::GetLODControllerCount() const
{
	return m_LODControllerVector.size();
}

CGrannyLODController * CGraphicThingInstance::GetLODControllerPointer(DWORD dwModelIndex) const
{
	assert(dwModelIndex < m_LODControllerVector.size());
	return m_LODControllerVector[dwModelIndex];
}

CGrannyLODController * CGraphicThingInstance::GetLODControllerPointer(DWORD dwModelIndex)
{
	assert(dwModelIndex < m_LODControllerVector.size());
	return m_LODControllerVector[dwModelIndex];
}

BYTE CGraphicThingInstance::GetLODLevel(DWORD dwModelInstance)
{
	assert(dwModelInstance < m_LODControllerVector.size());
	return (m_LODControllerVector[dwModelInstance]->GetLODLevel());
}

float CGraphicThingInstance::GetHeight()
{
	if (m_LODControllerVector.empty())
		return 0.0f;

	CGrannyModelInstance * pModelInstance = m_LODControllerVector[0]->GetModelInstance();
	if (!pModelInstance)
		return 0.0f;

	D3DXVECTOR3 vtMin, vtMax;
	pModelInstance->GetBoundBox(&vtMin, &vtMax);

	return fabs(vtMin.z - vtMax.z);
}

void CGraphicThingInstance::ReloadTexture()
{
	CGrannyLODController::FReloadTexture ReloadTexture;
	std::for_each(m_LODControllerVector.begin(), m_LODControllerVector.end(), ReloadTexture);
}

bool CGraphicThingInstance::HaveBlendThing()
{
	for (int i = 0; i < m_LODControllerVector.size(); i++)
	{
		if (m_LODControllerVector[i]->HaveBlendThing())
			return true;
	}
	return false;
}


void CGraphicThingInstance::OnClear()
{
	stl_wipe(m_LODControllerVector);
	stl_wipe_second(m_roMotionThingMap);

	for (DWORD d = 0; d < m_modelThingSetVector.size(); ++d)
		m_modelThingSetVector[d].Clear();
}

void CGraphicThingInstance::OnInitialize()
{	
	m_bUpdated = false;
	m_fLastLocalTime = 0.0f;
	m_fLocalTime = 0.0f;
	m_fDelay = 0.0;
	m_fSecondElapsed = 0.0f;
	m_fAverageSecondElapsed = 0.03f;
	m_fRadius = -1.0f;
	m_v3Center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	ResetLocalTime();
}

CGraphicThingInstance::CGraphicThingInstance()
{
	Initialize();
}

CGraphicThingInstance::~CGraphicThingInstance()
{
}
