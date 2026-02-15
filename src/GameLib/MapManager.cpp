#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "PackLib/PackManager.h"

#include "MapManager.h"
#include "MapOutdoor.h"

#include "PropertyLoader.h"

//////////////////////////////////////////////////////////////////////////
// 기본 함수
//////////////////////////////////////////////////////////////////////////

bool CMapManager::IsMapOutdoor()
{
	if (m_pkMap)
		return true;

	return false;
}

CMapOutdoor& CMapManager::GetMapOutdoorRef()
{
	assert(NULL!=m_pkMap);
	return *m_pkMap;
}


CMapManager::CMapManager() : mc_pcurEnvironmentData(NULL)
{
	m_pkMap = NULL;
	m_isSoftwareTilingEnableReserved=false;

//	Initialize();
}

CMapManager::~CMapManager()
{
	Destroy();
}

bool CMapManager::IsSoftwareTilingEnable()
{
	return CTerrainPatch::SOFTWARE_TRANSFORM_PATCH_ENABLE;
}

void CMapManager::ReserveSoftwareTilingEnable(bool isEnable)
{
	m_isSoftwareTilingEnableReserved=isEnable;
}


void CMapManager::Initialize()
{
	mc_pcurEnvironmentData = NULL;
	__LoadMapInfoVector();
}

void CMapManager::Create()
{
	assert(NULL==m_pkMap && "CMapManager::Create");
	if (m_pkMap)
	{
		Clear();
		return;
	}

	CTerrainPatch::SOFTWARE_TRANSFORM_PATCH_ENABLE=m_isSoftwareTilingEnableReserved;

	m_pkMap = (CMapOutdoor*)AllocMap();

	assert(NULL!=m_pkMap && "CMapManager::Create MAP is NULL");
		
}

void CMapManager::Destroy()
{
	stl_wipe_second(m_EnvironmentDataMap);

	if (m_pkMap)
	{
		m_pkMap->Clear();
		delete m_pkMap;
		m_pkMap = NULL;
	}
}

void CMapManager::Clear()
{
	if (m_pkMap)
		m_pkMap->Clear();
}

CMapBase * CMapManager::AllocMap()
{
	return new CMapOutdoor;
}

//////////////////////////////////////////////////////////////////////////
// Map
//////////////////////////////////////////////////////////////////////////
void CMapManager::LoadProperty()
{
	CPropertyLoader PropertyLoader;
	PropertyLoader.SetPropertyManager(&m_PropertyManager);
	PropertyLoader.Create("*.*", "Property");
}

bool CMapManager::LoadMap(const std::string & c_rstrMapName, float x, float y, float z)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();

	rkMap.Leave();
	rkMap.SetName(c_rstrMapName);
	rkMap.LoadProperty();

	if ( CMapBase::MAPTYPE_INDOOR == rkMap.GetType())
	{
		TraceError("CMapManager::LoadMap() Indoor Map Load Failed");
		return false;
	}
	else if (CMapBase::MAPTYPE_OUTDOOR == rkMap.GetType())
	{
		if (!rkMap.Load(x, y, z))
		{
			TraceError("CMapManager::LoadMap() Outdoor Map Load Failed");
			return false;
		}

		RegisterEnvironmentData(0, rkMap.GetEnvironmentDataName().c_str());
		
		SetEnvironmentData(0);
	}
	else
	{
		TraceError("CMapManager::LoadMap() Invalid Map Type");
		return false;
	}

	rkMap.Enter();
	return true;
}

bool CMapManager::IsMapReady()
{
	if (!m_pkMap)
		return false;

	return m_pkMap->IsReady();
}

bool CMapManager::UnloadMap(const std::string c_strMapName)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	if (c_strMapName != rkMap.GetName() && "" != rkMap.GetName())
	{
		LogBoxf("%s: Unload Map Failed", c_strMapName.c_str());
		return false;
	}

	Clear();
	return true;
}

bool CMapManager::UpdateMap(float fx, float fy, float fz)
{
	if (!m_pkMap)
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.Update(fx, -fy, fz);
}

void CMapManager::UpdateAroundAmbience(float fx, float fy, float fz)
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.UpdateAroundAmbience(fx, -fy, fz);
}

float CMapManager::GetHeight(float fx, float fy)
{
	if (!m_pkMap)
	{
		TraceError("CMapManager::GetHeight(%f, %f) - 맵이 생성되지 않은 상태에서 접근", fx, fy);
		return 0.0f;
	}
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetHeight(fx, fy);
}

float CMapManager::GetTerrainHeight(float fx, float fy)
{
	if (!m_pkMap)
	{
		TraceError("CMapManager::GetTerrainHeight(%f, %f) - 맵이 생성되지 않은 상태에서 접근", fx, fy);
		return 0.0f;
	}
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetTerrainHeight(fx, fy);
}

bool CMapManager::GetWaterHeight(int iX, int iY, long * plWaterHeight)
{
	if (!m_pkMap)
	{
		TraceError("CMapManager::GetTerrainHeight(%f, %f) - 맵이 생성되지 않은 상태에서 접근", iX, iY);
		return false;
	}

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetWaterHeight(iX, iY, plWaterHeight);
}

//////////////////////////////////////////////////////////////////////////
// Environment
//////////////////////////////////////////////////////////////////////////
void CMapManager::BeginEnvironment()
{
	if (!m_pkMap)
		return;

	if (!mc_pcurEnvironmentData)
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	// Light always on
 	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, TRUE);

	// Fog
 	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, mc_pcurEnvironmentData->bFogEnable);

	// Material
	STATEMANAGER.SetMaterial(&mc_pcurEnvironmentData->Material);

	// Directional Light
	if (mc_pcurEnvironmentData->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND])
	{
		ms_lpd3dDevice->LightEnable(0, TRUE);

		rkMap.ApplyLight((DWORD)mc_pcurEnvironmentData, mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND]);		
	}
	else
		ms_lpd3dDevice->LightEnable(0, FALSE);

	if (mc_pcurEnvironmentData->bFogEnable)
	{
		DWORD dwFogColor = mc_pcurEnvironmentData->FogColor;
		STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, dwFogColor);

		if (mc_pcurEnvironmentData->bDensityFog)
		{
			float fDensity = 0.00015f;
			STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP);					// pixel fog
			STATEMANAGER.SetRenderState(D3DRS_FOGDENSITY, *((DWORD *) &fDensity));			// vertex fog
		}
		else
		{
			CSpeedTreeForestDirectX& rkForest=CSpeedTreeForestDirectX::Instance();
			rkForest.SetFog(
				mc_pcurEnvironmentData->GetFogNearDistance(), 
				mc_pcurEnvironmentData->GetFogFarDistance()
			);

			float fFogNear=mc_pcurEnvironmentData->GetFogNearDistance();
			float fFogFar=mc_pcurEnvironmentData->GetFogFarDistance();
			STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);								// vertex fox
			STATEMANAGER.SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);											// vertex fox
			STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((DWORD *) &fFogNear));	// USED BY D3DFOG_LINEAR
			STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((DWORD *) &fFogFar));		// USED BY D3DFOG_LINEAR
		}
	}

	rkMap.OnBeginEnvironment();
}

void CMapManager::EndEnvironment()
{
	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

void CMapManager::SetEnvironmentData(int nEnvDataIndex)
{
	const TEnvironmentData * c_pEnvironmenData;
	
	if (GetEnvironmentData(nEnvDataIndex, &c_pEnvironmenData))
		SetEnvironmentDataPtr(c_pEnvironmenData);
}

void CMapManager::SetEnvironmentDataPtr(const TEnvironmentData * c_pEnvironmentData)
{
	if (!m_pkMap)
		return;
	
	if (!c_pEnvironmentData)
	{
		assert(!"null environment data");
		TraceError("null environment data");
		return;
	}

	CMapOutdoor& rkMap=GetMapOutdoorRef();


	mc_pcurEnvironmentData = c_pEnvironmentData;

	
	rkMap.SetEnvironmentDataPtr(mc_pcurEnvironmentData);
}

void CMapManager::ResetEnvironmentDataPtr(const TEnvironmentData * c_pEnvironmentData)
{
	if (!m_pkMap)
		return;

	if (!c_pEnvironmentData)
	{
		assert(!"null environment data");
		TraceError("null environment data");
		return;
	}

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	mc_pcurEnvironmentData = c_pEnvironmentData;
	rkMap.ResetEnvironmentDataPtr(mc_pcurEnvironmentData);
}

void CMapManager::BlendEnvironmentData(const TEnvironmentData * c_pEnvironmentData, int iTransitionTime)
{
}

bool CMapManager::RegisterEnvironmentData(DWORD dwIndex, const char * c_szFileName)
{
	TEnvironmentData * pEnvironmentData = AllocEnvironmentData();

	if (!LoadEnvironmentData(c_szFileName, pEnvironmentData))
	{
		DeleteEnvironmentData(pEnvironmentData);
		return false;
	}

	TEnvironmentDataMap::iterator f=m_EnvironmentDataMap.find(dwIndex);
	if (m_EnvironmentDataMap.end()==f)
	{
		m_EnvironmentDataMap.insert(TEnvironmentDataMap::value_type(dwIndex, pEnvironmentData));
	}
	else
	{
		delete f->second;
		f->second=pEnvironmentData;
	}
	return true;
}

void CMapManager::GetCurrentEnvironmentData(const TEnvironmentData ** c_ppEnvironmentData)
{
	*c_ppEnvironmentData = mc_pcurEnvironmentData;
}

bool CMapManager::GetEnvironmentData(DWORD dwIndex, const TEnvironmentData ** c_ppEnvironmentData)
{
	TEnvironmentDataMap::iterator itor = m_EnvironmentDataMap.find(dwIndex);

	if (m_EnvironmentDataMap.end() == itor)
	{
		*c_ppEnvironmentData = NULL;
		return false;
	}

	*c_ppEnvironmentData = itor->second;
	return true;
}

void CMapManager::RefreshPortal()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rMap = GetMapOutdoorRef();
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (!rMap.GetAreaPointer(i, &pArea))
			continue;

		pArea->RefreshPortal();
	}
}

void CMapManager::ClearPortal()
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rMap = GetMapOutdoorRef();
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (!rMap.GetAreaPointer(i, &pArea))
			continue;

		pArea->ClearPortal();
	}
}

void CMapManager::AddShowingPortalID(int iID)
{
	if (!IsMapReady())
		return;

	CMapOutdoor & rMap = GetMapOutdoorRef();
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (!rMap.GetAreaPointer(i, &pArea))
			continue;

		pArea->AddShowingPortalID(iID);
	}
}

TEnvironmentData * CMapManager::AllocEnvironmentData()
{
	TEnvironmentData * pEnvironmentData = new TEnvironmentData;
	Environment_Init(*pEnvironmentData);
	return pEnvironmentData;
}

void CMapManager::DeleteEnvironmentData(TEnvironmentData * pEnvironmentData)
{
	delete pEnvironmentData;
	pEnvironmentData = NULL;
}

BOOL CMapManager::LoadEnvironmentData(const char * c_szFileName, TEnvironmentData * pEnvironmentData)
{
	if (!pEnvironmentData)
		return FALSE;

	return (BOOL)Environment_Load(*pEnvironmentData, c_szFileName);
}

DWORD CMapManager::GetShadowMapColor(float fx, float fy)
{
	if (!IsMapReady())
		return 0xFFFFFFFF;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetShadowMapColor(fx, fy);
}

std::vector<int> & CMapManager::GetRenderedSplatNum(int * piPatch, int * piSplat, float * pfSplatRatio)
{
	if (!m_pkMap)
	{
		static std::vector<int> s_emptyVector;
		*piPatch = 0;
		*piSplat = 0;
		return s_emptyVector;
	}
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetRenderedSplatNum(piPatch, piSplat, pfSplatRatio);
}

CArea::TCRCWithNumberVector & CMapManager::GetRenderedGraphicThingInstanceNum(DWORD * pdwGraphicThingInstanceNum, DWORD * pdwCRCNum)
{
	if (!m_pkMap)
	{
		static CArea::TCRCWithNumberVector s_emptyVector;
		*pdwGraphicThingInstanceNum = 0;
		*pdwCRCNum = 0;
		return s_emptyVector;
	}

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetRenderedGraphicThingInstanceNum(pdwGraphicThingInstanceNum, pdwCRCNum);
}

bool CMapManager::GetNormal(int ix, int iy, D3DXVECTOR3 * pv3Normal)
{
	if (!IsMapReady())
		return false;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetNormal(ix, iy, pv3Normal);
}

bool CMapManager::isPhysicalCollision(const D3DXVECTOR3 & c_rvCheckPosition)
{
	if (!IsMapReady())
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.isAttrOn(c_rvCheckPosition.x, -c_rvCheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK);
}

bool CMapManager::isAttrOn(float fX, float fY, BYTE byAttr)
{
	if (!IsMapReady())
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.isAttrOn(fX, fY, byAttr);
}

bool CMapManager::GetAttr(float fX, float fY, BYTE * pbyAttr)
{
	if (!IsMapReady())
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetAttr(fX, fY, pbyAttr);
}

bool CMapManager::isAttrOn(int iX, int iY, BYTE byAttr)
{
	if (!IsMapReady())
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.isAttrOn(iX, iY, byAttr);
}

bool CMapManager::GetAttr(int iX, int iY, BYTE * pbyAttr)
{
	if (!IsMapReady())
		return false;
	
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetAttr(iX, iY, pbyAttr);
}

// 2004.10.14.myevan.TEMP_CAreaLoaderThread
/*
bool CMapManager::BGLoadingEnable()
{
	if (!IsMapReady())
		return false;
	return ((CMapOutdoor*)m_pMap)->BGLoadingEnable();
}

void CMapManager::BGLoadingEnable(bool bBGLoadingEnable)
{
	if (!IsMapReady())
		return;
	((CMapOutdoor*)m_pMap)->BGLoadingEnable(bBGLoadingEnable);
}
*/

void CMapManager::SetTerrainRenderSort(CMapOutdoor::ETerrainRenderSort eTerrainRenderSort)
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.SetTerrainRenderSort(eTerrainRenderSort);
}

void CMapManager::SetTransparentTree(bool bTransparenTree)
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.SetTransparentTree(bTransparenTree);
}

CMapOutdoor::ETerrainRenderSort CMapManager::GetTerrainRenderSort()
{
	if (!IsMapReady())
		return CMapOutdoor::DISTANCE_SORT;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetTerrainRenderSort();
}

void CMapManager::GetBaseXY(DWORD * pdwBaseX, DWORD * pdwBaseY)
{
	if (!IsMapReady())
	{
		*pdwBaseX = 0;
		*pdwBaseY = 0;
	}

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.GetBaseXY(pdwBaseX, pdwBaseY);
}

void CMapManager::__LoadMapInfoVector()
{
	TPackFile kFile;
	if (!CPackManager::Instance().GetFile(m_stAtlasInfoFileName, kFile))
		if (!CPackManager::Instance().GetFile("AtlasInfo.txt", kFile))
			return;

	CMemoryTextFileLoader textFileLoader;
	textFileLoader.Bind(kFile.size(), kFile.data());

	char szMapName[256];
	int x, y;
	int width, height;
	for (UINT uLineIndex=0; uLineIndex<textFileLoader.GetLineCount(); ++uLineIndex)
	{
		const std::string& c_rstLine=textFileLoader.GetLineString(uLineIndex);
		sscanf(c_rstLine.c_str(), "%s %d %d %d %d", 
			szMapName, 
			&x, &y, &width, &height);

		if ('\0'==szMapName[0])
			continue;

		TMapInfo kMapInfo;
		kMapInfo.m_strName = szMapName;
		kMapInfo.m_dwBaseX = x;
		kMapInfo.m_dwBaseY = y;

		kMapInfo.m_dwSizeX = width;
		kMapInfo.m_dwSizeY = height;

		kMapInfo.m_dwEndX = kMapInfo.m_dwBaseX + kMapInfo.m_dwSizeX * CTerrainImpl::TERRAIN_XSIZE;
		kMapInfo.m_dwEndY = kMapInfo.m_dwBaseY + kMapInfo.m_dwSizeY * CTerrainImpl::TERRAIN_YSIZE;

		m_kVct_kMapInfo.push_back(kMapInfo);
	}

	return;
}

