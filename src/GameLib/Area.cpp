#include "StdAfx.h"

#include "EterLib/ResourceManager.h"
#include "EterLib/StateManager.h"
#include "EffectLib/EffectManager.h"
#include "SpeedTreeLib/SpeedTreeForestDirectX.h"
#include "EterBase/Timer.h"

#include "Area.h"
#include "PropertyManager.h"
#include "Property.h"

CDynamicPool<CArea::TObjectInstance>	CArea::ms_ObjectInstancePool;
CDynamicPool<CAttributeInstance>		CArea::ms_AttributeInstancePool;
CDynamicPool<CArea::TAmbienceInstance>	CArea::ms_AmbienceInstancePool;
CDynamicPool<CDungeonBlock>				CArea::ms_DungeonBlockInstancePool;
CDynamicPool<CArea>						CArea::ms_kPool;

void CArea::TObjectData::InitializeRotation()
{
	m_fYaw=m_fPitch=m_fRoll=0.0f;
}

CArea* CArea::New()
{
	return ms_kPool.Alloc();
}

void CArea::Delete(CArea* pkArea)
{
	pkArea->Clear();
	ms_kPool.Free(pkArea);
}

void CArea::DestroySystem()
{
	ms_kPool.Destroy();

	ms_ObjectInstancePool.Destroy();
	ms_AttributeInstancePool.Destroy();
	ms_AmbienceInstancePool.Destroy();
	ms_DungeonBlockInstancePool.Destroy();
}

void CArea::__UpdateAniThingList()
{
	{
		CGraphicThingInstance* pkThingInst;

		TThingInstanceVector::iterator i=m_ThingCloneInstaceVector.begin();
		while (i!=m_ThingCloneInstaceVector.end())
		{
			pkThingInst=*i++;
			if (pkThingInst->isShow())
			{
				pkThingInst->UpdateLODLevel();
			}
		}
	}

	{
		CGraphicThingInstance* pkThingInst;

		TThingInstanceVector::iterator i=m_AniThingCloneInstanceVector.begin();
		while (i!=m_AniThingCloneInstanceVector.end())
		{
			pkThingInst=*i++;
			pkThingInst->Update();
		}
	}	
}

void CArea::__UpdateEffectList()
{
	//if (!CEffectManager::InstancePtr())
	//	return;

	CEffectManager& rkEftMgr=CEffectManager::Instance();

	// Effect
	TEffectInstanceIterator i;
	for (i = m_EffectInstanceMap.begin(); i != m_EffectInstanceMap.end();)
	{
		CEffectInstance * pEffectInstance = i->second;

		pEffectInstance->Update();

		if (!pEffectInstance->isAlive())
		{
			i = m_EffectInstanceMap.erase(i);
			rkEftMgr.DestroyUnsafeEffectInstance(pEffectInstance);
		}
		else
			++i;
	}
}

void CArea::Update()
{
	__UpdateAniThingList();		
}

void CArea::UpdateAroundAmbience(float fX, float fY, float fZ)
{
	// Ambience
	TAmbienceInstanceVector::iterator i;
	for (i = m_AmbienceCloneInstanceVector.begin(); i != m_AmbienceCloneInstanceVector.end(); ++i)
	{
		TAmbienceInstance * pInstance = *i;
		pInstance->__Update(fX, fY, fZ);
	}
}

struct CArea_LessEffectInstancePtrRenderOrder
{
	bool operator() (CEffectInstance* pkLeft, CEffectInstance* pkRight)
	{
		return pkLeft->LessRenderOrder(pkRight);		
	}
};

struct CArea_FEffectInstanceRender
{
	inline void operator () (CEffectInstance * pkEftInst)
	{
		pkEftInst->Render();
	}
};

void CArea::RenderEffect()
{
	__UpdateEffectList();

	// Effect
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

	bool m_isDisableSortRendering=false;

	if (m_isDisableSortRendering)
	{	
		TEffectInstanceIterator i;
		for (i = m_EffectInstanceMap.begin(); i != m_EffectInstanceMap.end();)
		{
			CEffectInstance * pEffectInstance = i->second;
			pEffectInstance->Render();
			++i;
		}
	}
	else
	{
		static std::vector<CEffectInstance*> s_kVct_pkEftInstSort;
		s_kVct_pkEftInstSort.clear();

		TEffectInstanceMap& rkMap_pkEftInstSrc=m_EffectInstanceMap;
		TEffectInstanceMap::iterator i;
		for (i=rkMap_pkEftInstSrc.begin(); i!=rkMap_pkEftInstSrc.end(); ++i)
			s_kVct_pkEftInstSort.push_back(i->second);

		std::sort(s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(), CArea_LessEffectInstancePtrRenderOrder());
		std::for_each(s_kVct_pkEftInstSort.begin(), s_kVct_pkEftInstSort.end(), CArea_FEffectInstanceRender());
		
	}
}

DWORD CArea::DEBUG_GetRenderedCRCNum() 
{ return m_kRenderedThingInstanceCRCWithNumberVector.size(); }

CArea::TCRCWithNumberVector & CArea::DEBUG_GetRenderedCRCWithNumVector() 
{ return m_kRenderedThingInstanceCRCWithNumberVector; }

DWORD CArea::DEBUG_GetRenderedGrapphicThingInstanceNum()
{ return m_kRenderedGrapphicThingInstanceVector.size(); }

void CArea::CollectRenderingObject(std::vector<CGraphicThingInstance*>& rkVct_pkOpaqueThingInst)
{
	TThingInstanceVector::iterator i;
	for (i=m_ThingCloneInstaceVector.begin(); i!=m_ThingCloneInstaceVector.end(); ++i)
	{
		CGraphicThingInstance* pkThingInst=*i;
		if (pkThingInst->isShow())
		{
			if (!pkThingInst->HaveBlendThing())
				rkVct_pkOpaqueThingInst.push_back(*i);	
		}
	}
}

void CArea::CollectBlendRenderingObject(std::vector<CGraphicThingInstance*>& rkVct_pkBlendThingInst)
{
	TThingInstanceVector::iterator i;
	for (i=m_ThingCloneInstaceVector.begin(); i!=m_ThingCloneInstaceVector.end(); ++i)
	{
		CGraphicThingInstance* pkThingInst=*i;
		if (pkThingInst->isShow())
		{
			if (pkThingInst->HaveBlendThing())
				rkVct_pkBlendThingInst.push_back(*i);	
		}
	}
}

void CArea::Render()
{		
	{
		CGraphicThingInstance* pkThingInst;

		TThingInstanceVector::iterator i=m_AniThingCloneInstanceVector.begin();
		while (i!=m_AniThingCloneInstanceVector.end())
		{
			pkThingInst=*i++;
			pkThingInst->Deform();
		}
	}

	CGraphicThingInstance* pkThingInst;

	TThingInstanceVector::iterator i=m_ThingCloneInstaceVector.begin();

	m_kRenderedThingInstanceCRCWithNumberVector.clear();
	m_kRenderedGrapphicThingInstanceVector.clear();
	TGraphicThingInstanceCRCMap::iterator aGraphicThingInstanceCRCMapIterator;

	while (i!=m_ThingCloneInstaceVector.end())
	{
		pkThingInst=*i++;
		if (pkThingInst->Render())
		{
			aGraphicThingInstanceCRCMapIterator = m_GraphicThingInstanceCRCMap.find(pkThingInst);
			DWORD dwCRC = (*aGraphicThingInstanceCRCMapIterator).second;

			m_kRenderedGrapphicThingInstanceVector.push_back(pkThingInst);

			TCRCWithNumberVector::iterator aCRCWithNumberVectorIterator = std::find_if(m_kRenderedThingInstanceCRCWithNumberVector.begin(), m_kRenderedThingInstanceCRCWithNumberVector.end(), FFindIfCRC(dwCRC));
			
			if ( m_kRenderedThingInstanceCRCWithNumberVector.end() == aCRCWithNumberVectorIterator)
			{
				TCRCWithNumber aCRCWithNumber;
				aCRCWithNumber.dwCRC = dwCRC;
				aCRCWithNumber.dwNumber = 1;
				m_kRenderedThingInstanceCRCWithNumberVector.push_back(aCRCWithNumber);
			}
			else
			{
				TCRCWithNumber & rCRCWithNumber = *aCRCWithNumberVectorIterator;
				rCRCWithNumber.dwNumber += 1;
			}
		}
	}
 	std::sort(m_kRenderedThingInstanceCRCWithNumberVector.begin(), m_kRenderedThingInstanceCRCWithNumberVector.end(), CRCNumComp());
}


void CArea::RenderCollision()
{
	DWORD i;

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff000000);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	for(i=0;i<GetObjectInstanceCount();i++)
	{
		const TObjectInstance * po;
		if (GetObjectInstancePointer(i,&po))
		{
			if (po->pTree && po->pTree->isShow())
			{
				DWORD j;
				for(j=0;j<po->pTree->GetCollisionInstanceCount();j++)
				{
					po->pTree->GetCollisionInstanceData(j)->Render();
				}
			}
			if (po->pThingInstance && po->pThingInstance->isShow())
			{
				DWORD j;
				for(j=0;j<po->pThingInstance->GetCollisionInstanceCount();j++)
				{
					po->pThingInstance->GetCollisionInstanceData(j)->Render();
				}
			}
			if (po->pDungeonBlock && po->pDungeonBlock->isShow())
			{
				DWORD j;
				for(j=0;j<po->pDungeonBlock->GetCollisionInstanceCount();j++)
				{
					po->pDungeonBlock->GetCollisionInstanceData(j)->Render();
				}
			}
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
}

void CArea::RenderAmbience()
{
	DWORD dwColorArg1, dwColorOp;
	STATEMANAGER.GetTextureStageState(0, D3DTSS_COLORARG1, &dwColorArg1);
	STATEMANAGER.GetTextureStageState(0, D3DTSS_COLOROP, &dwColorOp);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	TAmbienceInstanceVector::iterator itor = m_AmbienceCloneInstanceVector.begin();
	for (; itor != m_AmbienceCloneInstanceVector.end(); ++itor)
	{
		TAmbienceInstance * pInstance = *itor;
		pInstance->Render();
	}
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, dwColorArg1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, dwColorOp);
}

void CArea::RenderDungeon()
{
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	TDungeonBlockInstanceVector::iterator itor = m_DungeonBlockCloneInstanceVector.begin();
	for (; itor != m_DungeonBlockCloneInstanceVector.end(); ++itor)
	{
		(*itor)->Render();
	}

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);
}

void CArea::Refresh()
{
	m_TreeCloneInstaceVector.clear();
	m_ThingCloneInstaceVector.clear();
	m_DungeonBlockCloneInstanceVector.clear();
	m_AniThingCloneInstanceVector.clear();
	m_ShadowThingCloneInstaceVector.clear();
	m_AmbienceCloneInstanceVector.clear();

	TObjectInstanceVector::iterator it;

	for(it = m_ObjectInstanceVector.begin();it!=m_ObjectInstanceVector.end();++it)
	{
		TObjectInstance * pObjectInstance = *it;

		if (prt::PROPERTY_TYPE_TREE == pObjectInstance->dwType)
		{
			if (pObjectInstance->pTree)
			{
				m_TreeCloneInstaceVector.push_back(pObjectInstance->pTree);

				const float * pfPosition;
				pfPosition = pObjectInstance->pTree->GetPosition();
				pObjectInstance->pTree->UpdateBoundingSphere();
				pObjectInstance->pTree->UpdateCollisionData();
			}
		}
		else if (prt::PROPERTY_TYPE_BUILDING == pObjectInstance->dwType)
		{
			pObjectInstance->pThingInstance->Update();
			pObjectInstance->pThingInstance->Transform();
			pObjectInstance->pThingInstance->Show();
			pObjectInstance->pThingInstance->DeformAll();
			m_ThingCloneInstaceVector.push_back(pObjectInstance->pThingInstance);

			pObjectInstance->pThingInstance->BuildBoundingSphere();
			pObjectInstance->pThingInstance->UpdateBoundingSphere();

			if (pObjectInstance->pThingInstance->IsMotionThing())
			{
				m_AniThingCloneInstanceVector.push_back(pObjectInstance->pThingInstance);
				pObjectInstance->pThingInstance->SetMotion(0);
			}

			if (pObjectInstance->isShadowFlag)
			{
				m_ShadowThingCloneInstaceVector.push_back(pObjectInstance->pThingInstance);
			}

			if (pObjectInstance->pAttributeInstance)
			{
				pObjectInstance->pThingInstance->UpdateCollisionData(&pObjectInstance->pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
  				pObjectInstance->pAttributeInstance->RefreshObject(pObjectInstance->pThingInstance->GetTransform());
				pObjectInstance->pThingInstance->UpdateHeightInstance(pObjectInstance->pAttributeInstance);
			}
		}
		else if (prt::PROPERTY_TYPE_EFFECT == pObjectInstance->dwType)
		{
		}
		else if (prt::PROPERTY_TYPE_AMBIENCE == pObjectInstance->dwType)
		{
			m_AmbienceCloneInstanceVector.push_back(pObjectInstance->pAmbienceInstance);
		}
		else if (prt::PROPERTY_TYPE_DUNGEON_BLOCK == pObjectInstance->dwType)
		{
			pObjectInstance->pDungeonBlock->Update();
			pObjectInstance->pDungeonBlock->Deform();
			pObjectInstance->pDungeonBlock->UpdateBoundingSphere();
			m_DungeonBlockCloneInstanceVector.push_back(pObjectInstance->pDungeonBlock);

			if (pObjectInstance->pAttributeInstance)
			{
				pObjectInstance->pDungeonBlock->UpdateCollisionData(&pObjectInstance->pAttributeInstance->GetObjectPointer()->GetCollisionDataVector());
  				pObjectInstance->pAttributeInstance->RefreshObject(pObjectInstance->pDungeonBlock->GetTransform());
				pObjectInstance->pDungeonBlock->UpdateHeightInstance(pObjectInstance->pAttributeInstance);
			}
		}
	}
}

void CArea::__Load_BuildObjectInstances()
{
	m_ObjectInstanceVector.clear();
	m_ObjectInstanceVector.resize(GetObjectDataCount());

	m_GraphicThingInstanceCRCMap.clear();

 	std::sort(m_ObjectDataVector.begin(), m_ObjectDataVector.end(), ObjectDataComp());

	DWORD i=0;
	TObjectInstanceVector::iterator it;
	for (it = m_ObjectInstanceVector.begin(); it!=m_ObjectInstanceVector.end();++it,++i)
	{
		*it = ms_ObjectInstancePool.Alloc();
		(*it)->Clear();

		const TObjectData * c_pObjectData;

		if (!GetObjectDataPointer(i, &c_pObjectData))
			continue;

		__SetObjectInstance(*it, c_pObjectData);

		// 최적화용
		if ((*it)->dwType == prt::PROPERTY_TYPE_BUILDING)
			m_GraphicThingInstanceCRCMap.insert(TGraphicThingInstanceCRCMap::value_type( (*it)->pThingInstance, c_pObjectData->dwCRC ) );
	}

	//////////
	Refresh();
	//////////
}

void CArea::__SetObjectInstance(TObjectInstance * pObjectInstance, const TObjectData * c_pData)
{
	CProperty * pProperty;
	if (!CPropertyManager::Instance().Get(c_pData->dwCRC, &pProperty))
		return;

	const char * c_szPropertyType;

	if (!pProperty->GetString("PropertyType", &c_szPropertyType))
		return;

	switch (prt::GetPropertyType(c_szPropertyType))
	{
		case prt::PROPERTY_TYPE_TREE:
			__SetObjectInstance_SetTree(pObjectInstance, c_pData, pProperty);
			break;

		case prt::PROPERTY_TYPE_BUILDING:
			__SetObjectInstance_SetBuilding(pObjectInstance, c_pData, pProperty);
			break;

		case prt::PROPERTY_TYPE_EFFECT:
			__SetObjectInstance_SetEffect(pObjectInstance, c_pData, pProperty);
			break;

		case prt::PROPERTY_TYPE_AMBIENCE:
			__SetObjectInstance_SetAmbience(pObjectInstance, c_pData, pProperty);
			break;

		case prt::PROPERTY_TYPE_DUNGEON_BLOCK:
			__SetObjectInstance_SetDungeonBlock(pObjectInstance, c_pData, pProperty);
			break;
	}
}

void CArea::__SetObjectInstance_SetEffect(TObjectInstance * pObjectInstance, const TObjectData * c_pData, CProperty * pProperty)
{
	prt::TPropertyEffect Data;
	if (!prt::PropertyEffectStringToData(pProperty, &Data))
		return;

	pObjectInstance->dwType = prt::PROPERTY_TYPE_EFFECT;
	pObjectInstance->dwEffectID = GetCaseCRC32(Data.strFileName.c_str(),Data.strFileName.size());
	CEffectManager & rem = CEffectManager::Instance();
	CEffectData * pData;
	if (!rem.GetEffectData(pObjectInstance->dwEffectID,&pData))
	{
		if (!rem.RegisterEffect(Data.strFileName.c_str()))
		{
			pObjectInstance->dwEffectID = 0xffffffff;
			TraceError("CArea::SetEffect effect register error %s\n",Data.strFileName.c_str());
			return;
		}
	}

	CEffectInstance * pEffectInstance;
	rem.CreateUnsafeEffectInstance(pObjectInstance->dwEffectID, &pEffectInstance);

	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat,
		D3DXToRadian(c_pData->m_fYaw),
		D3DXToRadian(c_pData->m_fPitch),
		D3DXToRadian(c_pData->m_fRoll)
	);

	mat._41 = c_pData->Position.x;
	mat._42 = c_pData->Position.y;
	mat._43 = c_pData->Position.z + c_pData->m_fHeightBias;

	pEffectInstance->SetGlobalMatrix(mat);

	pObjectInstance->dwEffectInstanceIndex = m_EffectInstanceMap.size();
	m_EffectInstanceMap.insert(TEffectInstanceMap::value_type(pObjectInstance->dwEffectInstanceIndex, pEffectInstance));
}

void CArea::__SetObjectInstance_SetTree(TObjectInstance * pObjectInstance, const TObjectData * c_pData, CProperty * pProperty)
{
	const char * c_szTreeName;
	if (!pProperty->GetString("TreeFile", &c_szTreeName))
		return;

	pObjectInstance->SetTree(		
		c_pData->Position.x,
		c_pData->Position.y,
		c_pData->Position.z + c_pData->m_fHeightBias,
		c_pData->dwCRC,
		c_szTreeName
	);
}

void CArea::TObjectInstance::SetTree(float x, float y, float z, DWORD dwTreeCRC, const char* c_szTreeName)
{
	CSpeedTreeForestDirectX& rkForest=CSpeedTreeForestDirectX::Instance();
	pTree=rkForest.CreateInstance(x, y, z, dwTreeCRC, c_szTreeName);
	dwType = prt::PROPERTY_TYPE_TREE;
}

void CArea::__SetObjectInstance_SetBuilding(TObjectInstance * pObjectInstance, const TObjectData * c_pData, CProperty * pProperty)
{
	prt::TPropertyBuilding Data;
	if (!prt::PropertyBuildingStringToData(pProperty, &Data))
		return;

	CResourceManager& rkResMgr=CResourceManager::Instance();

	CGraphicThing * pThing = (CGraphicThing *)rkResMgr.GetResourcePointer(Data.strFileName.c_str());
	pThing->AddReference();

	if (pThing->IsEmpty())
	{
#ifdef _DEBUG
		TraceError("CArea::SetBuilding: There is no data: %s", Data.strFileName.c_str());
#endif
		return;
	}

	int iModelCount = pThing->GetModelCount();
	int iMotionCount = pThing->GetMotionCount();

	pObjectInstance->dwType = prt::PROPERTY_TYPE_BUILDING;
	pObjectInstance->pThingInstance = CGraphicThingInstance::New();
	pObjectInstance->pThingInstance->Initialize();
	pObjectInstance->pThingInstance->ReserveModelThing(iModelCount);
	pObjectInstance->pThingInstance->ReserveModelInstance(iModelCount);
	pObjectInstance->pThingInstance->RegisterModelThing(0, pThing);
	for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
		if (0 != c_pData->abyPortalID[j])
			pObjectInstance->pThingInstance->SetPortal(j, c_pData->abyPortalID[j]);

	{
		std::string stSrcModelFileName=Data.strFileName;
		std::string stLODModelFileName;

		char szLODModelFileNameEnd[256];
		for (UINT uLODIndex=1; uLODIndex<=3; ++uLODIndex)
		{
			sprintf(szLODModelFileNameEnd, "_lod_%.2d.gr2", uLODIndex);
			stLODModelFileName = CFileNameHelper::NoExtension(stSrcModelFileName) + szLODModelFileNameEnd;
			if (!rkResMgr.IsFileExist(stLODModelFileName.c_str()))
				break;

			CGraphicThing* pLODModelThing = (CGraphicThing *)rkResMgr.GetResourcePointer(stLODModelFileName.c_str());
			if (!pLODModelThing)
				break;

			pObjectInstance->pThingInstance->RegisterLODThing(0, pLODModelThing);
		}
	}

	for (int i = 0; i < iModelCount; ++i)
		pObjectInstance->pThingInstance->SetModelInstance(i, 0, i);

	if (iMotionCount)
	{
		pObjectInstance->pThingInstance->RegisterMotionThing(0, pThing);
	}

	pObjectInstance->pThingInstance->SetPosition(c_pData->Position.x, c_pData->Position.y, c_pData->Position.z + c_pData->m_fHeightBias);
	pObjectInstance->pThingInstance->SetRotation(
		c_pData->m_fYaw,
		c_pData->m_fPitch,
		c_pData->m_fRoll
	);
	pObjectInstance->isShadowFlag = Data.isShadowFlag;
	pObjectInstance->pThingInstance->RegisterBoundingSphere();
	__LoadAttribute(pObjectInstance, Data.strAttributeDataFileName.c_str());
	pThing->Release();
}

void CArea::__SetObjectInstance_SetAmbience(TObjectInstance * pObjectInstance, const TObjectData * c_pData, CProperty * pProperty)
{
	pObjectInstance->pAmbienceInstance = ms_AmbienceInstancePool.Alloc();
 	if (!prt::PropertyAmbienceStringToData(pProperty, &pObjectInstance->pAmbienceInstance->AmbienceData))
		return;

	pObjectInstance->dwType = prt::PROPERTY_TYPE_AMBIENCE;

	TAmbienceInstance * pAmbienceInstance = pObjectInstance->pAmbienceInstance;
	pAmbienceInstance->fx = c_pData->Position.x;
	pAmbienceInstance->fy = c_pData->Position.y;
	pAmbienceInstance->fz = c_pData->Position.z + c_pData->m_fHeightBias;
	pAmbienceInstance->dwRange = c_pData->dwRange;
	pAmbienceInstance->fMaxVolumeAreaPercentage = c_pData->fMaxVolumeAreaPercentage;

	if (0 == pAmbienceInstance->AmbienceData.strPlayType.compare("ONCE"))
	{
		pAmbienceInstance->Update = &TAmbienceInstance::UpdateOnceSound;
	}
	else if (0 == pAmbienceInstance->AmbienceData.strPlayType.compare("STEP"))
	{
		pAmbienceInstance->Update = &TAmbienceInstance::UpdateStepSound;
	}
	else if (0 == pAmbienceInstance->AmbienceData.strPlayType.compare("LOOP"))
	{
		pAmbienceInstance->Update = &TAmbienceInstance::UpdateLoopSound;
	}
}

void CArea::__SetObjectInstance_SetDungeonBlock(TObjectInstance * pObjectInstance, const TObjectData * c_pData, CProperty * pProperty)
{
	prt::TPropertyDungeonBlock Data;
	if (!prt::PropertyDungeonBlockStringToData(pProperty, &Data))
		return;

	pObjectInstance->dwType = prt::PROPERTY_TYPE_DUNGEON_BLOCK;
	pObjectInstance->pDungeonBlock = ms_DungeonBlockInstancePool.Alloc();
	pObjectInstance->pDungeonBlock->Load(Data.strFileName.c_str());
	pObjectInstance->pDungeonBlock->SetPosition(c_pData->Position.x, c_pData->Position.y, c_pData->Position.z + c_pData->m_fHeightBias);
	pObjectInstance->pDungeonBlock->SetRotation(
		c_pData->m_fYaw,
		c_pData->m_fPitch,
		c_pData->m_fRoll
	);
	pObjectInstance->pDungeonBlock->Update();
	pObjectInstance->pDungeonBlock->BuildBoundingSphere();
	pObjectInstance->pDungeonBlock->RegisterBoundingSphere();
	for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
		if (0 != c_pData->abyPortalID[j])
			pObjectInstance->pDungeonBlock->SetPortal(j, c_pData->abyPortalID[j]);
	__LoadAttribute(pObjectInstance, Data.strAttributeDataFileName.c_str());
}


void CArea::__LoadAttribute(TObjectInstance * pObjectInstance, const char * c_szAttributeFileName)
{
	// OBB를 사용한 충돌 정보 자동 생성.
	const bool bFileExist = CResourceManager::Instance().IsFileExist(c_szAttributeFileName);
	
	CAttributeData * pAttributeData = (CAttributeData *) CResourceManager::Instance().GetResourcePointer(c_szAttributeFileName);

	CAttributeInstance * pAttrInstance = ms_AttributeInstancePool.Alloc();
	pAttrInstance->Clear();
	pAttrInstance->SetObjectPointer(pAttributeData);

	if (false == bFileExist)
	{
		std::string attrFileName(c_szAttributeFileName);
		std::transform(attrFileName.begin(), attrFileName.end(), attrFileName.begin(), [](unsigned char c) { return std::tolower(c); });
		const bool bIsDungeonObject = (std::string::npos != attrFileName.find("/dungeon/")) || (std::string::npos != attrFileName.find("\\dungeon\\"));

		// NOTE: dungeon 오브젝트는 Dummy Collision을 자동으로 생성하지 않도록 함 (던전의 경우 더미 컬리전때문에 문제가 된 경우가 수차례 있었음. 이렇게 하기로 그래픽 팀과 협의 완료)
		if (pAttributeData->IsEmpty() && false == bIsDungeonObject)
		{
			if (NULL != pObjectInstance && NULL != pObjectInstance->pThingInstance)
			{
				CGraphicThingInstance* object = pObjectInstance->pThingInstance;

				D3DXVECTOR3 v3Min, v3Max;

				object->GetBoundingAABB(v3Min, v3Max);
				
				CStaticCollisionData collision;
				collision.dwType = COLLISION_TYPE_OBB;
				D3DXQuaternionRotationYawPitchRoll(&collision.quatRotation, object->GetYaw(), object->GetPitch(), object->GetRoll());
				strcpy(collision.szName, "DummyCollisionOBB");
				collision.v3Position = (v3Min + v3Max) * 0.5f;

				D3DXVECTOR3 vDelta = (v3Max - v3Min);
				collision.fDimensions[0] = vDelta.x * 0.5f;
				collision.fDimensions[1] = vDelta.y * 0.5f;
				collision.fDimensions[2] = vDelta.z * 0.5f;

				pAttributeData->AddCollisionData(collision);
			}
		}
	}

	if (!pAttributeData->IsEmpty())
	{
		pObjectInstance->pAttributeInstance = pAttrInstance;
	}
	else
	{
		pAttrInstance->Clear();
		ms_AttributeInstancePool.Free(pAttrInstance);
	}
}



bool CArea::Load(const char * c_szPathName)
{
	Clear();

	std::string strObjectDataFileName = c_szPathName + std::string("AreaData.txt");
	std::string strAmbienceDataFileName = c_szPathName + std::string("AreaAmbienceData.txt");

	__Load_LoadObject(strObjectDataFileName.c_str());
	__Load_LoadAmbience(strAmbienceDataFileName.c_str());
	__Load_BuildObjectInstances();
	
	return true;
}

bool CArea::__Load_LoadObject(const char * c_szFileName)
{
	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
	{
		TraceError(" CArea::Load File Load %s ERROR", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("areadatafile"))
	{
		TraceError(" CArea::__LoadObject File Format %s ERROR 1", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("objectcount"))
	{
		TraceError(" CArea::__LoadObject File Format %s ERROR 2", c_szFileName);
		return false;
	}

	const std::string & c_rstrCount = stTokenVectorMap["objectcount"][0];

	DWORD dwCount = atoi(c_rstrCount.c_str());

	char szObjectName[32+1];

	for (DWORD i = 0; i < dwCount; ++i)
	{
		_snprintf(szObjectName, sizeof(szObjectName), "object%03d", i);

		if (stTokenVectorMap.end() == stTokenVectorMap.find(szObjectName))
			continue;

		const CTokenVector & rVector = stTokenVectorMap[szObjectName];

		const std::string & c_rstrxPosition = rVector[0].c_str();
		const std::string & c_rstryPosition = rVector[1].c_str();
		const std::string & c_rstrzPosition = rVector[2].c_str();
		const std::string & c_rstrCRC = rVector[3].c_str();

		TObjectData ObjectData;
		ZeroMemory(&ObjectData, sizeof(ObjectData));
		ObjectData.Position.x = atof(c_rstrxPosition.c_str());
		ObjectData.Position.y = atof(c_rstryPosition.c_str());
		ObjectData.Position.z = atof(c_rstrzPosition.c_str());
		ObjectData.dwCRC = atoi	(c_rstrCRC.c_str());

		// 20041217.myevan.로테이션 공식 변경
		ObjectData.InitializeRotation(); //ObjectData.m_fYaw = ObjectData.m_fPitch = ObjectData.m_fRoll = 0;
		if (rVector.size() > 4)
		{
			std::string::size_type s=rVector[4].find('#');
			if (s!=rVector[4].npos)
			{
				ObjectData.m_fYaw = atoi(rVector[4].substr(0,s-1).c_str());
				int p = s+1;
				s = rVector[4].find('#',p);
				ObjectData.m_fPitch = atoi(rVector[4].substr(p,s-1-p+1).c_str());
				ObjectData.m_fRoll = atoi(rVector[4].substr(s+1).c_str());
			}
			else
			{
				ObjectData.m_fYaw = 0.0f;
				ObjectData.m_fPitch = 0.0f;
				ObjectData.m_fRoll = atoi(rVector[4].c_str());
			}
		}
		
		ObjectData.m_fHeightBias = 0.0f;
		if (rVector.size() > 5)
		{
			ObjectData.m_fHeightBias = atof(rVector[5].c_str());
		}

		if (rVector.size() > 6)
		{
			for (int portalIdx = 0; portalIdx < std::min(rVector.size()-6, (size_t)PORTAL_ID_MAX_NUM); ++portalIdx)
			{
				ObjectData.abyPortalID[portalIdx] = atoi(rVector[6+portalIdx].c_str());
			}
		}

		// If data is not inside property, then delete it.
		CProperty * pProperty;
		if (!CPropertyManager::Instance().Get(ObjectData.dwCRC, &pProperty))
		{
			TraceError(" CArea::LoadObject Property(%u) Load ERROR", ObjectData.dwCRC);
			continue;
		}

		m_ObjectDataVector.push_back(ObjectData);
	}

	return true;
}

bool CArea::__Load_LoadAmbience(const char * c_szFileName)
{
	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
	{
		TraceError(" CArea::Load File Load %s ERROR", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("areaambiencedatafile"))
	{
		TraceError(" CArea::__LoadAmbience File Format %s ERROR 1", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("objectcount"))
	{
		TraceError(" CArea::__LoadAmbience File Format %s ERROR 2", c_szFileName);
		return false;
	}

	const std::string & c_rstrCount = stTokenVectorMap["objectcount"][0];

	DWORD dwCount = atoi(c_rstrCount.c_str());

	char szObjectName[32+1];

	for (DWORD i = 0; i < dwCount; ++i)
	{
		_snprintf(szObjectName, sizeof(szObjectName), "object%03d", i);

		if (stTokenVectorMap.end() == stTokenVectorMap.find(szObjectName))
			continue;

		const CTokenVector & rVector = stTokenVectorMap[szObjectName];

		const std::string & c_rstrxPosition = rVector[0].c_str();
		const std::string & c_rstryPosition = rVector[1].c_str();
		const std::string & c_rstrzPosition = rVector[2].c_str();
		const std::string & c_rstrCRC = rVector[3].c_str();
		const std::string & c_rstrRange = rVector[4].c_str();

		TObjectData ObjectData;
		ZeroMemory(&ObjectData, sizeof(ObjectData));
		ObjectData.Position.x = atof(c_rstrxPosition.c_str());
		ObjectData.Position.y = atof(c_rstryPosition.c_str());
		ObjectData.Position.z = atof(c_rstrzPosition.c_str());
		ObjectData.dwCRC = atoi	(c_rstrCRC.c_str());
		ObjectData.dwRange = atoi(c_rstrRange.c_str());

		// 20041217.myevan.로테이션 초기화
		ObjectData.InitializeRotation();
		ObjectData.m_fHeightBias = 0.0f;
		ObjectData.fMaxVolumeAreaPercentage = 0.0f;

		if (rVector.size() >= 6)
		{
			const std::string & c_rstrPercentage = rVector[5].c_str();
			ObjectData.fMaxVolumeAreaPercentage = atof(c_rstrPercentage.c_str());
		}

		// If data is not inside property, then delete it.
		CProperty * pProperty;
		if (!CPropertyManager::Instance().Get(ObjectData.dwCRC, &pProperty))
		{
			TraceError(" CArea::LoadAmbience Property(%d) Load ERROR", ObjectData.dwCRC);
			continue;
		}

		m_ObjectDataVector.push_back(ObjectData);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool CArea::CheckObjectIndex(DWORD dwIndex) const
{
	if (dwIndex >= m_ObjectDataVector.size())
		return false;

	return true;
}

DWORD CArea::GetObjectDataCount()
{
	return m_ObjectDataVector.size();
}

bool CArea::GetObjectDataPointer(DWORD dwIndex, const TObjectData ** ppObjectData) const
{
	if (!CheckObjectIndex(dwIndex))
	{
		assert(!"Setting Object Index is corrupted!");
		return false;
	}

	*ppObjectData = &m_ObjectDataVector[dwIndex];
	return true;
}

const DWORD CArea::GetObjectInstanceCount() const
{
	return m_ObjectInstanceVector.size();
}

const bool CArea::GetObjectInstancePointer(const DWORD & dwIndex, const TObjectInstance ** ppObjectInstance) const
{
	if (dwIndex >= m_ObjectInstanceVector.size())
		return false;

	*ppObjectInstance = m_ObjectInstanceVector[dwIndex];
	return true;
}

void CArea::EnablePortal(BOOL bFlag)
{
	if (m_bPortalEnable == bFlag)
		return;

	m_bPortalEnable = bFlag;
}

void CArea::ClearPortal()
{
	m_kSet_ShowingPortalID.clear();
}

void CArea::AddShowingPortalID(int iNum)
{
	m_kSet_ShowingPortalID.insert(iNum);
}

void CArea::RefreshPortal()
{
	std::set<TObjectInstance *> kSet_ShowingObjectInstance;
	kSet_ShowingObjectInstance.clear();
	for (DWORD i = 0; i < m_ObjectDataVector.size(); ++i)
	{
		TObjectData & rData = m_ObjectDataVector[i];
		TObjectInstance * pInstance = m_ObjectInstanceVector[i];

		for (int j = 0; j < PORTAL_ID_MAX_NUM; ++j)
		{
			BYTE byPortalID = rData.abyPortalID[j];
			if (0 == byPortalID)
				break;

			if (m_kSet_ShowingPortalID.end() == m_kSet_ShowingPortalID.find(byPortalID))
				continue;

			kSet_ShowingObjectInstance.insert(pInstance);
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////

	m_TreeCloneInstaceVector.clear();
	m_ThingCloneInstaceVector.clear();
	m_DungeonBlockCloneInstanceVector.clear();

	TObjectInstanceVector::iterator it;
	for(it = m_ObjectInstanceVector.begin(); it != m_ObjectInstanceVector.end(); ++it)
	{
		TObjectInstance * pObjectInstance = *it;

		if (m_bPortalEnable)
		{
			if (kSet_ShowingObjectInstance.end() == kSet_ShowingObjectInstance.find(pObjectInstance))
				continue;
		}

		if (prt::PROPERTY_TYPE_TREE == pObjectInstance->dwType)
		{
			assert(pObjectInstance->pTree);
			m_TreeCloneInstaceVector.push_back(pObjectInstance->pTree);
		}
		else if (prt::PROPERTY_TYPE_BUILDING == pObjectInstance->dwType)
		{
			assert(pObjectInstance->pThingInstance);
			m_ThingCloneInstaceVector.push_back(pObjectInstance->pThingInstance);
		}
		else if (prt::PROPERTY_TYPE_DUNGEON_BLOCK == pObjectInstance->dwType)
		{
			assert(pObjectInstance->pDungeonBlock);
			m_DungeonBlockCloneInstanceVector.push_back(pObjectInstance->pDungeonBlock);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void CArea::Clear()
{
	// Real Instances
	TObjectInstanceVector::iterator it;
	for (it = m_ObjectInstanceVector.begin();it!=m_ObjectInstanceVector.end();++it)
		__Clear_DestroyObjectInstance(*it);

	m_ObjectDataVector.clear();
	m_ObjectInstanceVector.clear();

	// Clones
	m_TreeCloneInstaceVector.clear();
	m_ThingCloneInstaceVector.clear();
	m_DungeonBlockCloneInstanceVector.clear();
	m_AniThingCloneInstanceVector.clear();
	m_ShadowThingCloneInstaceVector.clear();
	m_AmbienceCloneInstanceVector.clear();
	m_GraphicThingInstanceCRCMap.clear();
	m_kRenderedThingInstanceCRCWithNumberVector.clear();
	m_kRenderedGrapphicThingInstanceVector.clear();

	m_bPortalEnable = FALSE;
	ClearPortal();

	CEffectManager& rkEftMgr=CEffectManager::Instance();

	TEffectInstanceIterator i;
	for (i = m_EffectInstanceMap.begin(); i != m_EffectInstanceMap.end(); ++i)
	{
		CEffectInstance * pEffectInstance = i->second;
		rkEftMgr.DestroyUnsafeEffectInstance(pEffectInstance);		
	}
	m_EffectInstanceMap.clear();
}

void CArea::__Clear_DestroyObjectInstance(TObjectInstance * pObjectInstance)
{
	if (pObjectInstance->dwEffectInstanceIndex!=0xffffffff)
	{
		TEffectInstanceIterator f= m_EffectInstanceMap.find(pObjectInstance->dwEffectInstanceIndex);
		if (m_EffectInstanceMap.end()!=f)
		{
			CEffectInstance * pEffectInstance = f->second;
			m_EffectInstanceMap.erase(f);

			if (CEffectManager::InstancePtr())
				CEffectManager::Instance().DestroyUnsafeEffectInstance(pEffectInstance);
		}
		pObjectInstance->dwEffectInstanceIndex = 0xffffffff;
	}

	if (pObjectInstance->pAttributeInstance)
	{
		pObjectInstance->pAttributeInstance->Clear();
		ms_AttributeInstancePool.Free(pObjectInstance->pAttributeInstance);
		pObjectInstance->pAttributeInstance = NULL;
	}

	if (pObjectInstance->pTree)
	{
		pObjectInstance->pTree->Clear();
		CSpeedTreeForestDirectX::Instance().DeleteInstance(pObjectInstance->pTree);
		pObjectInstance->pTree = NULL;
	}

	if (pObjectInstance->pThingInstance)
	{
		CGraphicThingInstance::Delete(pObjectInstance->pThingInstance);
		pObjectInstance->pThingInstance = NULL;
	}

	if (pObjectInstance->pAmbienceInstance)
	{
		ms_AmbienceInstancePool.Free(pObjectInstance->pAmbienceInstance);
		pObjectInstance->pAmbienceInstance = NULL;
	}

	if (pObjectInstance->pDungeonBlock)
	{
		ms_DungeonBlockInstancePool.Free(pObjectInstance->pDungeonBlock);
		pObjectInstance->pDungeonBlock = NULL;
	}

	pObjectInstance->Clear();
	
	ms_ObjectInstancePool.Free(pObjectInstance);
}


//////////////////////////////////////////////////////////////////////////
// Coordination 관련
void CArea::GetCoordinate(unsigned short * usCoordX, unsigned short * usCoordY)
{
	*usCoordX = m_wX;
	*usCoordY = m_wY;
}

void CArea::SetCoordinate(const unsigned short & usCoordX, const unsigned short & usCoordY)
{
	m_wX = usCoordX;
	m_wY = usCoordY;
}

//////////////////////////////////////////////////////////////////////////

void CArea::SetMapOutDoor(CMapOutdoor * pOwnerOutdoorMap)
{
	m_pOwnerOutdoorMap=pOwnerOutdoorMap;
}

CArea::CArea()
{
	m_wX = m_wY = 0xFF;
}

CArea::~CArea()
{
	Clear();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CArea::TAmbienceInstance::__Update(float fxCenter, float fyCenter, float fzCenter)
{
	if (0 == dwRange)
		return;

	(this->*Update)(fxCenter, fyCenter, fzCenter);
}

void CArea::TAmbienceInstance::UpdateOnceSound(float fxCenter, float fyCenter, float fzCenter)
{
	float fDistance = sqrtf((fx - fxCenter)*(fx - fxCenter) + (fy - fyCenter)*(fy - fyCenter) + (fz - fzCenter)*(fz - fzCenter));
	if (uint32_t(fDistance) < dwRange)
	{
		if (!playSoundInstance)
		{
			if (AmbienceData.AmbienceSoundVector.empty())
				return;

			const char* c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
			playSoundInstance = SoundEngine::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName);
		}
	}
	else
	{
		playSoundInstance = nullptr;
	}
}

void CArea::TAmbienceInstance::UpdateStepSound(float fxCenter, float fyCenter, float fzCenter)
{
	float fDistance = sqrtf((fx - fxCenter) * (fx - fxCenter) + (fy - fyCenter) * (fy - fyCenter) + (fz - fzCenter) * (fz - fzCenter));
	if (DWORD(fDistance) < dwRange)
	{
		float fcurTime = CTimer::Instance().GetCurrentSecond();

		if (fcurTime > fNextPlayTime)
		{
			if (AmbienceData.AmbienceSoundVector.empty())
				return;

			const char* c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
			playSoundInstance = SoundEngine::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName);

			fNextPlayTime = CTimer::Instance().GetCurrentSecond();
			fNextPlayTime += AmbienceData.fPlayInterval + frandom(0.0f, AmbienceData.fPlayIntervalVariation);
		}
	}
	else
	{
		playSoundInstance = nullptr;
		fNextPlayTime = 0.0f;
	}
}

void CArea::TAmbienceInstance::UpdateLoopSound(float fxCenter, float fyCenter, float fzCenter)
{
	float fDistance = sqrtf((fx - fxCenter) * (fx - fxCenter) + (fy - fyCenter) * (fy - fyCenter) + (fz - fzCenter) * (fz - fzCenter));
	if (uint32_t(fDistance) < dwRange)
	{
		if (!playSoundInstance)
		{
			if (AmbienceData.AmbienceSoundVector.empty())
				return;

			const char* c_szFileName = AmbienceData.AmbienceSoundVector[0].c_str();
			playSoundInstance = SoundEngine::Instance().PlayAmbienceSound3D(fx, fy, fz, c_szFileName, 0);
		}

		if (playSoundInstance)
			playSoundInstance->SetVolume(__GetVolumeFromDistance(fDistance));
	}
	else
	{
		playSoundInstance = nullptr;
	}
}

float CArea::TAmbienceInstance::__GetVolumeFromDistance(float fDistance)
{
	float fMaxVolumeAreaRadius = float(dwRange) * fMaxVolumeAreaPercentage;
	if (fMaxVolumeAreaRadius <= 0.0f)
		return 1.0f;
	if (fDistance <= fMaxVolumeAreaRadius)
		return 1.0f;

	return 1.0f - ((fDistance - fMaxVolumeAreaRadius) / (dwRange - fMaxVolumeAreaRadius));
}

void CArea::TAmbienceInstance::Render()
{
	float fBoxSize = 10.0f;
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xff00ff00);
	RenderCube(fx-fBoxSize, fy-fBoxSize, fz-fBoxSize, fx+fBoxSize, fy+fBoxSize, fz+fBoxSize);
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
	RenderSphere(NULL, fx, fy, fz, float(dwRange) * fMaxVolumeAreaPercentage, D3DFILL_POINT);
	RenderSphere(NULL, fx, fy, fz, float(dwRange), D3DFILL_POINT);
	RenderCircle2d(fx, fy, fz, float(dwRange) * fMaxVolumeAreaPercentage);
	RenderCircle2d(fx, fy, fz, float(dwRange));

	for (int i = 0; i < 4; ++i)
	{
		float fxAdd = cosf(float(i) * D3DX_PI/4.0f) * float(dwRange) / 2.0f;
		float fyAdd = sinf(float(i) * D3DX_PI/4.0f) * float(dwRange) / 2.0f;

		if (i%2)
		{
			fxAdd /= 2.0f;
			fyAdd /= 2.0f;
		}

		RenderLine2d(fx + fxAdd, fy + fyAdd, fx - fxAdd, fy - fyAdd, fz);
	}
}

bool CArea::SAmbienceInstance::Picking()
{
	return CGraphicCollisionObject::IntersectSphere(D3DXVECTOR3(fx, fy, fz), dwRange);
}

CArea::SAmbienceInstance::SAmbienceInstance()
{
	fx = 0.0f;
	fy = 0.0f;
	fz = 0.0f;
	dwRange = 0;
	playSoundInstance = nullptr;
	fNextPlayTime = 0.0f;
}
