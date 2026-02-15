#include "StdAfx.h"
#include "MapOutdoor.h"
#include "AreaTerrain.h"
#include "AreaLoaderThread.h"
#include "EterLib/ResourceManager.h"
#include "PackLib/PackManager.h"

//CAreaLoaderThread CMapOutdoor::ms_AreaLoaderThread;

bool CMapOutdoor::Load(float x, float y, float z)
{
	Destroy();

	{
		static std::string s_strOldPathName="";

		// 2004.08.09.myevan.Pack파일을 찾을때.. 폴더명만으로는 그냥 리턴되는 부분이 있다
		std::string c_rstrNewPathName=GetName()+"\\cache";
		
		s_strOldPathName=c_rstrNewPathName;
	}

	std::string strFileName = GetMapDataDirectory() + "\\Setting.txt";

	if (!LoadSetting(strFileName.c_str()))
		TraceError("CMapOutdoor::Load : LoadSetting(%s) Failed", strFileName.c_str());

	CreateTerrainPatchProxyList();
	BuildQuadTree();
	LoadWaterTexture();
	CreateCharacterShadowTexture();

	m_lOldReadX = -1;

	// TODO: SetRenderingDevice에서 Environment로 부터 라이트 속성을 넘겨줘야 스태틱 라이트가 제대로 작동한다.
	CSpeedTreeForestDirectX::Instance().SetRenderingDevice(ms_lpd3dDevice);

	Update(x, y, z);

	__HeightCache_Init();

	// LOCAL_ENVIRONMENT_DATA
	std::string local_envDataName = GetMapDataDirectory() + "\\" + m_settings_envDataName;
	if (CPackManager::instance().IsExist(local_envDataName.c_str()))
	{
		m_envDataName = local_envDataName;
	}
	else
	{
		const std::string& c_rstrEnvironmentRoot = "d:/ymir work/environment/";
		const std::string& c_rstrMapName = GetName();
		m_envDataName = c_rstrEnvironmentRoot + m_settings_envDataName;

		if (0 == m_envDataName.compare(c_rstrEnvironmentRoot))
		{
			const std::string& strAppendName = c_rstrMapName.substr(c_rstrMapName.size() - 2, 2);
			m_envDataName = c_rstrEnvironmentRoot + strAppendName + ".msenv";
		}
	}
	// LOCAL_ENVIRONMENT_DATA_END
	return true;
}

std::string& CMapOutdoor::GetEnvironmentDataName()
{
	return m_envDataName;
}


bool CMapOutdoor::isTerrainLoaded(WORD wX, WORD wY)
{
	for (DWORD i = 0; i < m_TerrainVector.size(); ++i)
	{
		CTerrain * pTerrain = m_TerrainVector[i];
		WORD usCoordX, usCoordY;
		pTerrain->GetCoordinate(&usCoordX, &usCoordY);
		
		if (usCoordX == wX && usCoordY == wY)
			return true;
	}
	return false;
}

bool CMapOutdoor::isAreaLoaded(WORD wX, WORD wY)
{
	for (DWORD i = 0; i < m_AreaVector.size(); ++i)
	{
		CArea * pArea = m_AreaVector[i];
		WORD usCoordX, usCoordY;
		pArea->GetCoordinate(&usCoordX, &usCoordY);
		
		if (usCoordX == wX && usCoordY == wY)
			return true;
	}
	return false;
}


// 현재 좌표를 기반으로 주위(ex. 3x3)에 있는 Terrain과 Area포인터를 
// m_pTerrain과 m_pArea에 연결한다.
void CMapOutdoor::AssignTerrainPtr()
{
	// 월드에디터에서 화면을 죽죽죽 넘길 때 터레인을 저장해야 하기
	// 때문에 이 virtual method를 호출 한다. 이 메소드는 CMapOutDoor에서는 아무 행동도
	// 하지 않는다.
	OnPreAssignTerrainPtr();

	short sReferenceCoordMinX, sReferenceCoordMaxX, sReferenceCoordMinY, sReferenceCoordMaxY;
	sReferenceCoordMinX = std::max(m_CurCoordinate.m_sTerrainCoordX - LOAD_SIZE_WIDTH, 0);
	sReferenceCoordMaxX = std::min(m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH, m_sTerrainCountX - 1);
	sReferenceCoordMinY = std::max(m_CurCoordinate.m_sTerrainCoordY - LOAD_SIZE_WIDTH, 0);
	sReferenceCoordMaxY = std::min(m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH, m_sTerrainCountY - 1);

	DWORD i;
	for (i = 0; i < AROUND_AREA_NUM; ++i)
	{
		m_pArea[i] = NULL;
		m_pTerrain[i] = NULL;
	}

	for (i = 0; i < m_TerrainVector.size(); ++i)
	{
		CTerrain * pTerrain = m_TerrainVector[i];
		WORD usCoordX, usCoordY;
		pTerrain->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX >= sReferenceCoordMinX &&
			usCoordX <= sReferenceCoordMaxX &&
			usCoordY >= sReferenceCoordMinY &&
			usCoordY <= sReferenceCoordMaxY)
		{
			m_pTerrain[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
				(usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH) ] = pTerrain;
		}
	}

	for (i = 0; i < m_AreaVector.size(); ++i)
	{
		CArea * pArea = m_AreaVector[i];
		WORD usCoordX, usCoordY;
		pArea->GetCoordinate(&usCoordX, &usCoordY);

		if (usCoordX >= sReferenceCoordMinX &&
			usCoordX <= sReferenceCoordMaxX &&
			usCoordY >= sReferenceCoordMinY &&
			usCoordY <= sReferenceCoordMaxY)
		{
			m_pArea[(usCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 +
				(usCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH) ] = pArea;
		}
	}
}

bool CMapOutdoor::LoadArea(WORD wAreaCoordX, WORD wAreaCoordY, WORD wCellCoordX, WORD wCellCoordY)
{
	if (isAreaLoaded(wAreaCoordX, wAreaCoordY))
		return true;
#ifdef _DEBUG
	DWORD dwStartTime = ELTimer_GetMSec();
#endif
	unsigned long ulID = (unsigned long) (wAreaCoordX) * 1000L + (unsigned long) (wAreaCoordY);
	char szAreaPathName[64+1];
	_snprintf(szAreaPathName, sizeof(szAreaPathName), "%s\\%06u\\", GetMapDataDirectory().c_str(), ulID);

	CArea * pArea = CArea::New();
	pArea->SetMapOutDoor(this);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea1 %d\n", ELTimer_GetMSec() - dwStartTime);
	dwStartTime = ELTimer_GetMSec();
#endif

	pArea->SetCoordinate(wAreaCoordX, wAreaCoordY);
	if ( !pArea->Load(szAreaPathName) )
		TraceError(" CMapOutdoor::LoadArea(%d, %d) LoadShadowMap ERROR", wAreaCoordX, wAreaCoordY);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea2 %d\n", ELTimer_GetMSec() - dwStartTime);
	dwStartTime = ELTimer_GetMSec();
#endif

	m_AreaVector.push_back(pArea);

	pArea->EnablePortal(m_bEnablePortal);
#ifdef _DEBUG
	Tracef("CMapOutdoor::LoadArea3 %d\n", ELTimer_GetMSec() - dwStartTime);
#endif

	return true;
}

bool CMapOutdoor::LoadTerrain(WORD wTerrainCoordX, WORD wTerrainCoordY, WORD wCellCoordX, WORD wCellCoordY)
{
	if (isTerrainLoaded(wTerrainCoordX, wTerrainCoordY))
		return true;

	//////////////////////////////////////////////////////////////////////////
	DWORD dwStartTime = ELTimer_GetMSec();
	
	unsigned long ulID = (unsigned long) (wTerrainCoordX) * 1000L + (unsigned long) (wTerrainCoordY);
	char filename[256];
	sprintf(filename, "%s\\%06u\\AreaProperty.txt", GetMapDataDirectory().c_str(), ulID);
	
	CTokenVectorMap stTokenVectorMap;
	
	if (!LoadMultipleTextData(filename, stTokenVectorMap))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty Read Error\n");
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 1\n");
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("areaname"))
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 2\n");
		return false;
	}
	
	const std::string & c_rstrType = stTokenVectorMap["scripttype"][0];
	const std::string & c_rstrAreaName = stTokenVectorMap["areaname"][0];
	
	if (c_rstrType != "AreaProperty")
	{
		TraceError("CMapOutdoor::LoadTerrain AreaProperty FileFormat Error 3\n");
		return false;
	}

	CTerrain * pTerrain = CTerrain::New();
	
	pTerrain->Clear();
	pTerrain->SetMapOutDoor(this);
	
	pTerrain->SetCoordinate(wTerrainCoordX, wTerrainCoordY);
	
	pTerrain->CopySettingFromGlobalSetting();
	
	char szRawHeightFieldname[64+1];
	char szWaterMapName[64+1];
	char szAttrMapName[64+1];
	char szShadowTexName[64+1];
	char szShadowMapName[64+1];
	char szMiniMapTexName[64+1];
	char szSplatName[64+1];
	
	_snprintf(szRawHeightFieldname, sizeof(szRawHeightFieldname), "%s\\%06u\\height.raw", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szSplatName, sizeof(szSplatName), "%s\\%06u\\tile.raw", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szAttrMapName, sizeof(szAttrMapName), "%s\\%06u\\attr.atr", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szWaterMapName, sizeof(szWaterMapName), "%s\\%06u\\water.wtr", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szShadowTexName, sizeof(szShadowTexName), "%s\\%06u\\shadowmap.dds", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szShadowMapName, sizeof(szShadowMapName), "%s\\%06u\\shadowmap.raw", GetMapDataDirectory().c_str(), ulID);
	_snprintf(szMiniMapTexName, sizeof(szMiniMapTexName), "%s\\%06u\\minimap.dds", GetMapDataDirectory().c_str(), ulID);
	
	if(!pTerrain->LoadWaterMap(szWaterMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadWaterMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->LoadHeightMap(szRawHeightFieldname))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadHeightMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->LoadAttrMap(szAttrMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadAttrMap ERROR", wTerrainCoordX, wTerrainCoordY);

	if (!pTerrain->RAW_LoadTileMap(szSplatName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) RAW_LoadTileMap ERROR", wTerrainCoordX, wTerrainCoordY);

	pTerrain->LoadShadowTexture(szShadowTexName);

	if (!pTerrain->LoadShadowMap(szShadowMapName))
		TraceError(" CMapOutdoor::LoadTerrain(%d, %d) LoadShadowMap ERROR", wTerrainCoordX, wTerrainCoordY);

	pTerrain->LoadMiniMapTexture(szMiniMapTexName);
	pTerrain->SetName(c_rstrAreaName.c_str());
	pTerrain->CalculateTerrainPatch();
	
	pTerrain->SetReady();
	
	Tracef("CMapOutdoor::LoadTerrain %d\n", ELTimer_GetMSec() - dwStartTime);

	m_TerrainVector.push_back(pTerrain);

	return true;
}

bool CMapOutdoor::LoadSetting(const char * c_szFileName)
{
	NANOBEGIN
	CTokenVectorMap stTokenVectorMap;

	if (!LoadMultipleTextData(c_szFileName, stTokenVectorMap))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - LoadMultipleTextData", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("scripttype"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'scripttype' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("viewradius"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'viewradius' - FAILED", c_szFileName);
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("cellscale"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'cellscale' - FAILED", c_szFileName);
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("heightscale"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'heightscale' - FAILED", c_szFileName);
		return false;
	}
	
	if (stTokenVectorMap.end() == stTokenVectorMap.find("mapsize"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'mapsize' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() == stTokenVectorMap.find("textureset"))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - FIND 'textureset' - FAILED", c_szFileName);
		return false;
	}

	if (stTokenVectorMap.end() != stTokenVectorMap.find("terrainvisible"))
	{
		m_bSettingTerrainVisible = (bool) (atoi(stTokenVectorMap["terrainvisible"][0].c_str()) != 0);
	}
	else
	{
		m_bSettingTerrainVisible = true;
	}

	const std::string & c_rstrType = stTokenVectorMap["scripttype"][0];
	const std::string & c_rstrViewRadius = stTokenVectorMap["viewradius"][0];
	//const std::string & c_rstrCellScale = stTokenVectorMap["cellscale"][0];
	const std::string & c_rstrHeightScale = stTokenVectorMap["heightscale"][0];
	const std::string & c_rstrMapSizeX = stTokenVectorMap["mapsize"][0];
	const std::string & c_rstrMapSizeY = stTokenVectorMap["mapsize"][1];

	std::string strTextureSet;
	TTokenVector & rkVec_strToken = stTokenVectorMap["textureset"];
	if (!rkVec_strToken.empty())
	{
		strTextureSet = rkVec_strToken[0];
	}

	if (c_rstrType != "MapSetting")
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - Resourse Type ERROR", c_szFileName);
		return false;
	}

	m_lViewRadius = atol(c_rstrViewRadius.c_str());

	if (0L >= m_lViewRadius)
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - VIEWRADIUS IS NOT GREATER THAN 0", c_szFileName);
		return false;
	}

	m_fHeightScale = atof(c_rstrHeightScale.c_str());

	SetTerrainCount(atoi(c_rstrMapSizeX.c_str()), atoi(c_rstrMapSizeY.c_str()));

	m_fTerrainTexCoordBase = 1.0f / (float) (CTerrainImpl::PATCH_XSIZE * CTerrainImpl::CELLSCALE);

	if (stTokenVectorMap.end() != stTokenVectorMap.find("baseposition"))
	{
		const std::string & c_rstrMapBaseX = stTokenVectorMap["baseposition"][0];
		const std::string & c_rstrMapBaseY = stTokenVectorMap["baseposition"][1];
		SetBaseXY((DWORD)atol(c_rstrMapBaseX.c_str()), (DWORD)atol(c_rstrMapBaseY.c_str()));
	}
	
	std::string stTextureSetFileName = strTextureSet;

	// TextureSet 이 이미 붙어 있을 경우 안붙인다.
	if (0 != stTextureSetFileName.find_first_of("textureset", 0))
		stTextureSetFileName = "textureset\\"+strTextureSet;

 	if (!m_TextureSet.Load(stTextureSetFileName.c_str(), m_fTerrainTexCoordBase))
	{
		TraceError("MapOutdoor::LoadSetting(c_szFileName=%s) - LOAD TEXTURE SET(%s) ERROR", c_szFileName, stTextureSetFileName.c_str());
		return false;
	}

	CTerrain::SetTextureSet(&m_TextureSet);
	
	if (stTokenVectorMap.end() != stTokenVectorMap.find("environment"))
	{
		const CTokenVector & c_rEnvironmentVector = stTokenVectorMap["environment"];
		if (!c_rEnvironmentVector.empty())
			m_settings_envDataName = c_rEnvironmentVector[0];
		else
			TraceError("CMapOutdoor::LoadSetting(c_szFileName=%s) - Failed to load environment data\n", c_szFileName);
	}

	m_fWaterTexCoordBase = 1.0f / (float)(CTerrainImpl::CELLSCALE * 4);

	D3DXMatrixScaling(&m_matSplatAlpha, 
		+m_fTerrainTexCoordBase * 2.0f * (float)(CTerrainImpl::PATCH_XSIZE) / (float)(CTerrainImpl::SPLATALPHA_RAW_XSIZE-2), 
		-m_fTerrainTexCoordBase * 2.0f * (float)(CTerrainImpl::PATCH_YSIZE) / (float)(CTerrainImpl::SPLATALPHA_RAW_XSIZE-2), 
		0.0f);
	m_matSplatAlpha._41 = m_fTerrainTexCoordBase * 4.6f;
	m_matSplatAlpha._42 = m_fTerrainTexCoordBase * 4.6f;

	D3DXMatrixScaling(&m_matStaticShadow, 
		+m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_XSIZE / static_cast<float>(CTerrainImpl::XSIZE)), 
		-m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_YSIZE / static_cast<float>(CTerrainImpl::XSIZE)),
		0.0f);
	m_matStaticShadow._41 = 0.0f;
	m_matStaticShadow._42 = 0.0f;
	
	D3DXMatrixScaling(&m_matDynamicShadowScale, 1.0f / 2550.0f, -1.0f / 2550.0f, 1.0f);
	m_matDynamicShadowScale._41 = 0.5f;
	m_matDynamicShadowScale._42 = 0.5f;

	// Transform
	D3DXMatrixScaling(&m_matBuildingTransparent, 1.0f / ((float)ms_iWidth), -1.0f / ((float)ms_iHeight), 1.0f);
	m_matBuildingTransparent._41 = 0.5f;
	m_matBuildingTransparent._42 = 0.5f;
	NANOEND
	return true;
}


bool CMapOutdoor::LoadMonsterAreaInfo()
{
	RemoveAllMonsterAreaInfo();

	char c_szFileName[256];
	sprintf(c_szFileName, "%s\\regen.txt", GetMapDataDirectory().c_str());
	
	TPackFile File;
	
	if (!CPackManager::Instance().GetFile(c_szFileName, File))
	{
		//TraceError(" CMapOutdoorAccessor::LoadMonsterAreaInfo Load File %s ERROR", c_szFileName);
		return false;
	}
	
	CMemoryTextFileLoader textFileLoader;
	CTokenVector stTokenVector;
	
	textFileLoader.Bind(File.size(), File.data());
	
	for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i)
	{
		if (!textFileLoader.SplitLine(i, &stTokenVector))
			continue;
		
		stl_lowers(stTokenVector[0]);
		
		// Start or End
		if (0 == stTokenVector[0].compare("m") || 0 == stTokenVector[0].compare("g"))
		{
			if (stTokenVector.size() < 11)
			{
				TraceError("CMapOutdoorAccessor::LoadMonsterAreaInfo Get MonsterInfo File Format ERROR! continue....");
				continue;
			}

			CMonsterAreaInfo::EMonsterAreaInfoType eMonsterAreaInfoType;
			if (0 == stTokenVector[0].compare("m"))
			{
				eMonsterAreaInfoType = CMonsterAreaInfo::MONSTERAREAINFOTYPE_MONSTER;
			}
			else if (0 == stTokenVector[0].compare("g"))
			{
				eMonsterAreaInfoType = CMonsterAreaInfo::MONSTERAREAINFOTYPE_GROUP;
			}
			else
			{
				TraceError("CMapOutdoorAccessor::LoadMonsterAreaInfo Get MonsterInfo Data ERROR! continue....");
				continue;
			}
			
			const std::string & c_rstrOriginX	= stTokenVector[1].c_str();
			const std::string & c_rstrOriginY	= stTokenVector[2].c_str();
			const std::string & c_rstrSizeX		= stTokenVector[3].c_str();
			const std::string & c_rstrSizeY		= stTokenVector[4].c_str();
			const std::string & c_rstrZ			= stTokenVector[5].c_str();
			const std::string & c_rstrDir		= stTokenVector[6].c_str();
			const std::string & c_rstrTime		= stTokenVector[7].c_str();
			const std::string & c_rstrPercent	= stTokenVector[8].c_str();
			const std::string & c_rstrCount		= stTokenVector[9].c_str();
			const std::string & c_rstrVID		= stTokenVector[10].c_str();
			
			long lOriginX, lOriginY, lSizeX, lSizeY, lZ, lTime, lPercent;
			CMonsterAreaInfo::EMonsterDir eMonsterDir;
			DWORD dwMonsterCount;
			DWORD dwMonsterVID;
			
			lOriginX		= atol(c_rstrOriginX.c_str());
			lOriginY		= atol(c_rstrOriginY.c_str());
			lSizeX			= atol(c_rstrSizeX.c_str());
			lSizeY			= atol(c_rstrSizeY.c_str());
			lZ				= atol(c_rstrZ.c_str());
			eMonsterDir		= (CMonsterAreaInfo::EMonsterDir) atoi(c_rstrDir.c_str());
			lTime			= atol(c_rstrTime.c_str());
			lPercent		= atol(c_rstrPercent.c_str());
			dwMonsterCount	= (DWORD) atoi(c_rstrCount.c_str());
			dwMonsterVID	= (DWORD) atoi(c_rstrVID.c_str());

//			lOriginX -= m_dwBaseX / 100;
//			lOriginY -= m_dwBaseY / 100;
			
			CMonsterAreaInfo * pMonsterAreaInfo = AddMonsterAreaInfo(lOriginX, lOriginY, lSizeX, lSizeY);
			pMonsterAreaInfo->SetMonsterAreaInfoType(eMonsterAreaInfoType);
			if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_MONSTER == eMonsterAreaInfoType)
				pMonsterAreaInfo->SetMonsterVID(dwMonsterVID);
			else if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_GROUP == eMonsterAreaInfoType)
				pMonsterAreaInfo->SetMonsterGroupID(dwMonsterVID);
			pMonsterAreaInfo->SetMonsterCount(dwMonsterCount);
			pMonsterAreaInfo->SetMonsterDirection(eMonsterDir);
		}
	}
	
	return true;
}
