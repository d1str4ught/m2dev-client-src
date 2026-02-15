#include "StdAfx.h"
#include "EterLib/StateManager.h"
#include "EterLib/Camera.h"
#include "PRTerrainLib/StdAfx.h"
#include "EffectLib/EffectManager.h"

#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "AreaTerrain.h"

//#define USE_LEVEL
// unsigned int uiNumSplat;

struct FGetObjectHeight
{
	bool m_bHeightFound;
	float m_fReturnHeight;
	float m_fRequestX, m_fRequestY;
	FGetObjectHeight(float fRequestX, float fRequestY)
	{
		m_fRequestX=fRequestX;
		m_fRequestY=fRequestY;
		m_bHeightFound=false;
		m_fReturnHeight=0.0f;
	}
	void operator () (CGraphicObjectInstance * pObject)
	{
		if (pObject->GetObjectHeight(m_fRequestX, m_fRequestY, &m_fReturnHeight))
		{
#ifdef SPHERELIB_STRICT
			printf("FIND %f\n", m_fReturnHeight);
#endif
			m_bHeightFound = true;
		}
	}
};

struct FGetPickingPoint
{	
	D3DXVECTOR3 m_v3Start;
	D3DXVECTOR3 m_v3Dir;
	D3DXVECTOR3 m_v3PickingPoint;
	bool		m_bPicked; 

	FGetPickingPoint(D3DXVECTOR3 & v3Start, D3DXVECTOR3 & v3Dir) : m_v3Start(v3Start), m_v3Dir(v3Dir), m_bPicked(false) {}
	void operator() (CGraphicObjectInstance * pInstance)
	{
		if( pInstance && pInstance->GetType() == CGraphicThingInstance::ID )
		{			
			CGraphicThingInstance * pThing = (CGraphicThingInstance *)pInstance;
			if (!pThing->IsObjectHeight())
				return;

			float fX, fY, fZ;
			if (pThing->Picking(m_v3Start, m_v3Dir, fX, fY))
			{
				if (pThing->GetObjectHeight(fX, -fY, &fZ))
				{
					m_v3PickingPoint.x = fX;
					m_v3PickingPoint.y = fY;
					m_v3PickingPoint.z = fZ;
					m_bPicked = true;
				}				
			}			
		}
	}
};

CMapOutdoor::CMapOutdoor()
{
	CGraphicImage * pAttrImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer("d:/ymir work/special/white.dds");
	CGraphicImage * pBuildTransparentImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer("d:/ymir Work/special/PCBlockerAlpha.dds");
	m_attrImageInstance.SetImagePointer(pAttrImage);
	m_BuildingTransparentImageInstance.SetImagePointer(pBuildTransparentImage);

	Initialize();

	__SoftwareTransformPatch_Initialize();
	__SoftwareTransformPatch_Create();
}

CMapOutdoor::~CMapOutdoor()
{
	__SoftwareTransformPatch_Destroy();

	// 2004.10.14.myevan.TEMP_CAreaLoaderThread
	//ms_AreaLoaderThread.Shutdown();
	Destroy();
}

bool CMapOutdoor::Initialize()
{
	BYTE i;
	for (i = 0; i < AROUND_AREA_NUM; ++i)
	{
 		m_pArea[i] = NULL;
		m_pTerrain[i] = NULL;
	}

	m_pTerrainPatchProxyList = NULL;

	m_lViewRadius	= 0L;
	m_fHeightScale	= 0.0f;
		
	m_sTerrainCountX = m_sTerrainCountY = 0;

	m_CurCoordinate.m_sTerrainCoordX = -1;
	m_CurCoordinate.m_sTerrainCoordY = -1;
	m_PrevCoordinate.m_sTerrainCoordX = -1;
	m_PrevCoordinate.m_sTerrainCoordY = -1;

	m_EntryPointMap.clear();

	m_lCenterX = m_lCenterY = 0;
	m_lOldReadX = m_lOldReadY = -1;

	memset(m_pwaIndices, 0, sizeof(m_pwaIndices));
	for (i = 0; i < TERRAINPATCH_LODMAX; ++i)
		m_IndexBuffer[i].Destroy();

	m_bSettingTerrainVisible = false;
	m_bDrawWireFrame	= false;
	m_bDrawShadow		= false;
	m_bDrawChrShadow	= false;

	m_iSplatLimit = 50000;

	m_wPatchCount = 0;

	m_pRootNode = NULL;
	
	//////////////////////////////////////////////////////////////////////////
	// Character Shadow
	m_lpCharacterShadowMapTexture = NULL;
	m_lpCharacterShadowMapRenderTargetSurface = NULL;
	m_lpCharacterShadowMapDepthSurface = NULL;

	m_lpBackupRenderTargetSurface = NULL;
	m_lpBackupDepthSurface = NULL;
	// Character Shadow
	//////////////////////////////////////////////////////////////////////////

	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;

	//////////////////////////////////////////////////////////////////////////
	m_fOpaqueWaterDepth = 400.0f;

	//////////////////////////////////////////////////////////////////////////
	m_TerrainVector.clear();
	m_TerrainDeleteVector.clear();
	m_TerrainLoadRequestVector.clear();
	m_TerrainLoadWaitVector.clear();

	m_AreaVector.clear();
	m_AreaDeleteVector.clear();
	m_AreaLoadRequestVector.clear();
	m_AreaLoadWaitVector.clear();
	//////////////////////////////////////////////////////////////////////////	
	
	m_PatchVector.clear();
	
	// 2004.10.14.myevan.TEMP_CAreaLoaderThread
	//m_bBGLoadingEnable = false;
	m_eTerrainRenderSort = DISTANCE_SORT;

	D3DXMatrixIdentity(&m_matWorldForCommonUse);
	
	InitializeVisibleParts();

	m_dwBaseX = 0;
	m_dwBaseY = 0;

	m_settings_envDataName = "";
	m_bShowEntirePatchTextureCount = false;
	m_bTransparentTree = true;
	
	CMapBase::Clear();

	__XMasTree_Initialize();
	SpecialEffect_Destroy();

	m_bEnableTerrainOnlyForHeight = FALSE;
	m_bEnablePortal = FALSE;

	m_wShadowMapSize = 512;
	return true;
}


bool CMapOutdoor::Destroy()
{
	m_bEnableTerrainOnlyForHeight = FALSE;
	m_bEnablePortal = FALSE;

	XMasTree_Destroy();

	DestroyTerrain();
 	DestroyArea();
	DestroyTerrainPatchProxyList();

	FreeQuadTree();
	ReleaseCharacterShadowTexture();

	CTerrain::DestroySystem();
	CArea::DestroySystem();

	RemoveAllMonsterAreaInfo();

	m_rkList_kGuildArea.clear();
	m_kPool_kMonsterAreaInfo.Destroy();
	CSpeedTreeForestDirectX::Instance().Clear();

	return true;
}

void CMapOutdoor::Clear()
{
	UnloadWaterTexture();
	Destroy();		// 해제
	Initialize();	// 초기화
}

bool CMapOutdoor::SetTerrainCount(short sTerrainCountX, short sTerrainCountY)
{
	if (0 == sTerrainCountX || MAX_MAPSIZE < sTerrainCountX)
		return false;
	
	if (0 == sTerrainCountY || MAX_MAPSIZE < sTerrainCountY)
		return false;

	m_sTerrainCountX = sTerrainCountX;
	m_sTerrainCountY = sTerrainCountY;
	return true;
}

void CMapOutdoor::OnBeginEnvironment()
{
	if (!mc_pEnvironmentData)
		return;

	CSpeedTreeForestDirectX& rkForest=CSpeedTreeForestDirectX::Instance();
	rkForest.SetFog(
		mc_pEnvironmentData->GetFogNearDistance(), 
		mc_pEnvironmentData->GetFogFarDistance()
	);

	const D3DLIGHT9& c_rkLight = mc_pEnvironmentData->DirLights[ENV_DIRLIGHT_CHARACTER];
	rkForest.SetLight(
		(const float *)&c_rkLight.Direction,
		(const float *)&c_rkLight.Ambient, 
		(const float *)&c_rkLight.Diffuse);
	
	rkForest.SetWindStrength(mc_pEnvironmentData->fWindStrength);
}

void CMapOutdoor::OnSetEnvironmentDataPtr()
{
	SetEnvironmentScreenFilter();
	SetEnvironmentSkyBox();
	SetEnvironmentLensFlare();
}

void CMapOutdoor::OnResetEnvironmentDataPtr()
{
	m_SkyBox.Unload();
	SetEnvironmentScreenFilter();
	SetEnvironmentSkyBox();
	SetEnvironmentLensFlare();
}

void CMapOutdoor::SetEnvironmentScreenFilter()
{
	if (!mc_pEnvironmentData)
		return;

	m_ScreenFilter.SetEnable(mc_pEnvironmentData->bFilteringEnable);
	m_ScreenFilter.SetBlendType(mc_pEnvironmentData->byFilteringAlphaSrc, mc_pEnvironmentData->byFilteringAlphaDest);
	m_ScreenFilter.SetColor(mc_pEnvironmentData->FilteringColor);
}

void CMapOutdoor::SetEnvironmentSkyBox()
{
	if (!mc_pEnvironmentData)
		return;

	m_SkyBox.SetSkyBoxScale(mc_pEnvironmentData->v3SkyBoxScale);
	m_SkyBox.SetGradientLevel(mc_pEnvironmentData->bySkyBoxGradientLevelUpper, mc_pEnvironmentData->bySkyBoxGradientLevelLower);
	m_SkyBox.SetRenderMode( (mc_pEnvironmentData->bSkyBoxTextureRenderMode == TRUE) ? CSkyObject::SKY_RENDER_MODE_TEXTURE : CSkyObject::SKY_RENDER_MODE_DIFFUSE);

	for( int i = 0; i < 6; ++i )
	{
		if (!mc_pEnvironmentData->strSkyBoxFaceFileName[i].empty())
			m_SkyBox.SetFaceTexture( mc_pEnvironmentData->strSkyBoxFaceFileName[i].c_str(), i );
	}

	if (!mc_pEnvironmentData->strCloudTextureFileName.empty())
		m_SkyBox.SetCloudTexture(mc_pEnvironmentData->strCloudTextureFileName.c_str());

	m_SkyBox.SetCloudScale(mc_pEnvironmentData->v2CloudScale);
	m_SkyBox.SetCloudHeight(mc_pEnvironmentData->fCloudHeight);
	m_SkyBox.SetCloudTextureScale(mc_pEnvironmentData->v2CloudTextureScale);
	m_SkyBox.SetCloudScrollSpeed(mc_pEnvironmentData->v2CloudSpeed);
	m_SkyBox.Refresh();

	// Temporary
	m_SkyBox.SetCloudColor(mc_pEnvironmentData->CloudGradientColor, mc_pEnvironmentData->CloudGradientColor, 1);

	if (!mc_pEnvironmentData->SkyBoxGradientColorVector.empty())
		m_SkyBox.SetSkyColor(mc_pEnvironmentData->SkyBoxGradientColorVector, mc_pEnvironmentData->SkyBoxGradientColorVector, 1);
	// Temporary

	m_SkyBox.StartTransition();
}

void CMapOutdoor::SetEnvironmentLensFlare()
{
	if (!mc_pEnvironmentData)
		return;

	m_LensFlare.CharacterizeFlare(mc_pEnvironmentData->bLensFlareEnable == 1 ? true : false,
								  mc_pEnvironmentData->bMainFlareEnable == 1 ? true : false,
								  mc_pEnvironmentData->fLensFlareMaxBrightness,
								  mc_pEnvironmentData->LensFlareBrightnessColor);

	m_LensFlare.Initialize("d:/ymir work/environment");

	if (!mc_pEnvironmentData->strMainFlareTextureFileName.empty())
		m_LensFlare.SetMainFlare(mc_pEnvironmentData->strMainFlareTextureFileName.c_str(),
								 mc_pEnvironmentData->fMainFlareSize);
}

void CMapOutdoor::SetWireframe(bool bWireFrame)
{
	m_bDrawWireFrame = bWireFrame;
}

bool CMapOutdoor::IsWireframe()
{
	return m_bDrawWireFrame;
}

//////////////////////////////////////////////////////////////////////////
// TerrainPatchList
//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::CreateTerrainPatchProxyList()
{
	m_wPatchCount = ((m_lViewRadius * 2) / TERRAIN_PATCHSIZE) + 2;
	
	m_pTerrainPatchProxyList = new CTerrainPatchProxy[m_wPatchCount * m_wPatchCount];
	
	m_iPatchTerrainVertexCount = (TERRAIN_PATCHSIZE+1)*(TERRAIN_PATCHSIZE+1);
	m_iPatchWaterVertexCount = TERRAIN_PATCHSIZE * TERRAIN_PATCHSIZE * 6;
	m_iPatchTerrainVertexSize = 24;
	m_iPatchWaterVertexSize = 16;

	SetIndexBuffer();
}

void CMapOutdoor::DestroyTerrainPatchProxyList()
{
	if (m_pTerrainPatchProxyList)
	{
		delete [] m_pTerrainPatchProxyList;
		m_pTerrainPatchProxyList = NULL;
	}

	for (int i = 0; i < TERRAINPATCH_LODMAX; ++i)
		m_IndexBuffer[i].Destroy();
}

//////////////////////////////////////////////////////////////////////////
// Area
//////////////////////////////////////////////////////////////////////////

void CMapOutdoor::EnablePortal(bool bFlag)
{
	m_bEnablePortal = bFlag;

	for (int i = 0; i < AROUND_AREA_NUM; ++i)
		if (m_pArea[i])
			m_pArea[i]->EnablePortal(bFlag);
}

void CMapOutdoor::DestroyArea()
{
	m_AreaVector.clear();
	m_AreaDeleteVector.clear();

	CArea::ms_kPool.FreeAll();
	
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
		m_pArea[i] = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Terrain
//////////////////////////////////////////////////////////////////////////

void CMapOutdoor::DestroyTerrain()
{
	m_TerrainVector.clear();
	m_TerrainDeleteVector.clear();

	CTerrain::ms_kPool.FreeAll();
	for (int i=0; i < AROUND_AREA_NUM; ++i)
		m_pTerrain[i] = NULL;
}

//////////////////////////////////////////////////////////////////////////
// New
//////////////////////////////////////////////////////////////////////////

bool CMapOutdoor::GetTerrainNum(float fx, float fy, BYTE * pbyTerrainNum)
{
	if (fy < 0)
		fy = -fy;

	int ix, iy;
	
	PR_FLOAT_TO_INT(fx, ix);
	PR_FLOAT_TO_INT(fy, iy);

	WORD wTerrainNumX = ix / (CTerrainImpl::TERRAIN_XSIZE);
	WORD wTerrainNumY = iy / (CTerrainImpl::TERRAIN_YSIZE);

	return GetTerrainNumFromCoord(wTerrainNumX, wTerrainNumY, pbyTerrainNum);
}

bool CMapOutdoor::GetPickingPoint(D3DXVECTOR3 * v3IntersectPt)
{
	return GetPickingPointWithRay(ms_Ray, v3IntersectPt);
}

bool CMapOutdoor::__PickTerrainHeight(float& fPos, const D3DXVECTOR3& v3Start, const D3DXVECTOR3& v3End, float fStep, float fRayRange, float fLimitRange, D3DXVECTOR3* pv3Pick)
{
	CTerrain * pTerrain;

	D3DXVECTOR3 v3CurPos;

	float fRayRangeInv=1.0f/fRayRange;
	while (fPos < fRayRange && fPos<fLimitRange)
	{
		D3DXVec3Lerp(&v3CurPos, &v3Start, &v3End, fPos*fRayRangeInv);
		BYTE byTerrainNum;
		float fMultiplier = 1.0f;
		if (GetTerrainNum(v3CurPos.x, v3CurPos.y, &byTerrainNum))
		{
			if (GetTerrainPointer(byTerrainNum, &pTerrain))
			{
				int ix, iy;
				PR_FLOAT_TO_INT(v3CurPos.x, ix);
				PR_FLOAT_TO_INT(fabs(v3CurPos.y), iy);
				float fMapHeight = pTerrain->GetHeight(ix, iy);
				if ( fMapHeight >= v3CurPos.z)
				{
					*pv3Pick = v3CurPos;
					return true;
				}
				else
				{
					fMultiplier = fMAX(1.0f, 0.01f * ( v3CurPos.z - fMapHeight ) );
				}
			}
		}
		fPos += fStep * fMultiplier;
	}

	return false;
}
bool CMapOutdoor::GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	bool bObjectPick = false;
	bool bTerrainPick = false;
	D3DXVECTOR3 v3ObjectPick, v3TerrainPick;

	D3DXVECTOR3 v3Start, v3End, v3Dir, v3CurPos;
 	float fRayRange;
	rRay.GetStartPoint(&v3Start);
	rRay.GetDirection(&v3Dir, &fRayRange);
	rRay.GetEndPoint(&v3End);
	
	Vector3d v3dStart, v3dEnd;
	v3dStart.Set(v3Start.x, v3Start.y, v3Start.z);
	v3dEnd.Set(v3End.x - v3Start.x, v3End.y - v3Start.y, v3End.z - v3Start.z);
	
	if (!m_bEnableTerrainOnlyForHeight)
	{
		//DWORD baseTime = timeGetTime();
		CCullingManager & rkCullingMgr = CCullingManager::Instance();
		FGetPickingPoint kGetPickingPoint(v3Start, v3Dir);	
		rkCullingMgr.ForInRange2d(v3dStart, &kGetPickingPoint);

		if (kGetPickingPoint.m_bPicked)
		{
			bObjectPick = true;
			v3ObjectPick = kGetPickingPoint.m_v3PickingPoint;
		}		
	}	
	
	float fPos = 0.0f;
	//float fStep = 1.0f;
	//float fRayRangeInv=1.0f/fRayRange;

	bTerrainPick=true;
	if (!__PickTerrainHeight(fPos, v3Start, v3End, 5.0f, fRayRange, 5000.0f, &v3TerrainPick))
		if (!__PickTerrainHeight(fPos, v3Start, v3End, 10.0f, fRayRange, 10000.0f, &v3TerrainPick))
			if (!__PickTerrainHeight(fPos, v3Start, v3End, 100.0f, fRayRange, 100000.0f, &v3TerrainPick))
					bTerrainPick=false;
	
	
	if (bObjectPick && bTerrainPick)
	{
		const auto vv = (v3TerrainPick - v3Start);
		const auto vv2 = (v3ObjectPick - v3Start);
		if ( D3DXVec3Length( &vv2) >= D3DXVec3Length( & vv) )
			*v3IntersectPt = v3TerrainPick;
		else
			*v3IntersectPt = v3ObjectPick;
		return true;
	}
	else if (bObjectPick)
	{
		*v3IntersectPt = v3ObjectPick;
		return true;
	}
	else if (bTerrainPick)
	{
		*v3IntersectPt = v3TerrainPick;
		return true;
	}
	
	return false;
}

bool CMapOutdoor::GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	bool bTerrainPick = false;
	D3DXVECTOR3 v3TerrainPick;

	D3DXVECTOR3 v3Start, v3End, v3Dir, v3CurPos;
 	float fRayRange;
	rRay.GetStartPoint(&v3Start);
	rRay.GetDirection(&v3Dir, &fRayRange);
	rRay.GetEndPoint(&v3End);
	
	Vector3d v3dStart, v3dEnd;
	v3dStart.Set(v3Start.x, v3Start.y, v3Start.z);
	v3dEnd.Set(v3End.x - v3Start.x, v3End.y - v3Start.y, v3End.z - v3Start.z);
	

	
	float fPos = 0.0f;	
	bTerrainPick=true;
	if (!__PickTerrainHeight(fPos, v3Start, v3End, 5.0f, fRayRange, 5000.0f, &v3TerrainPick))
		if (!__PickTerrainHeight(fPos, v3Start, v3End, 10.0f, fRayRange, 10000.0f, &v3TerrainPick))
			if (!__PickTerrainHeight(fPos, v3Start, v3End, 100.0f, fRayRange, 100000.0f, &v3TerrainPick))
					bTerrainPick=false;
	
	if (bTerrainPick)
	{
		*v3IntersectPt = v3TerrainPick;
		return true;
	}
	
	return false;
}

void CMapOutdoor::GetHeightMap(const BYTE & c_rucTerrainNum, WORD ** pwHeightMap)
{
	if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
	{
		*pwHeightMap = NULL;
		return;
	}
	
	*pwHeightMap = m_pTerrain[c_rucTerrainNum]->GetHeightMap();
}

void CMapOutdoor::GetNormalMap(const BYTE & c_rucTerrainNum, char ** pucNormalMap)
{
	if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
	{
		*pucNormalMap = NULL;
		return;
	}
	
	*pucNormalMap = m_pTerrain[c_rucTerrainNum]->GetNormalMap();
}

void CMapOutdoor::GetWaterMap(const BYTE & c_rucTerrainNum, BYTE ** pucWaterMap)
{
	if (c_rucTerrainNum < 0 || c_rucTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[c_rucTerrainNum])
	{
		*pucWaterMap = NULL;
		return;
	}

	*pucWaterMap = m_pTerrain[c_rucTerrainNum]->GetWaterMap();
}

void CMapOutdoor::GetWaterHeight(BYTE byTerrainNum, BYTE byWaterNum, long * plWaterHeight)
{
	if (byTerrainNum < 0 || byTerrainNum > AROUND_AREA_NUM - 1 || !m_pTerrain[byTerrainNum])
	{
		*plWaterHeight = -1;
		return;
	}

	m_pTerrain[byTerrainNum]->GetWaterHeight(byWaterNum, plWaterHeight);
}

bool CMapOutdoor::GetWaterHeight(int iX, int iY, long * plWaterHeight)
{
	if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE || iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
		return false;

	WORD wTerrainCoordX, wTerrainCoordY;
	wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
	wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;

	BYTE byTerrainNum;
	if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
		return false;
	CTerrain * pTerrain;
	if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		return false;

	WORD wLocalX, wLocalY;
	wLocalX = (iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::WATERMAP_XSIZE);
	wLocalY = (iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::WATERMAP_YSIZE);
	
	return pTerrain->GetWaterHeight(wLocalX, wLocalY, plWaterHeight);
}

//////////////////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////////////////

bool CMapOutdoor::GetTerrainNumFromCoord(WORD wCoordX, WORD wCoordY, BYTE * pbyTerrainNum)
{
	*pbyTerrainNum = (wCoordY - m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH) * 3 + 
		(wCoordX - m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH);
	
	if (*pbyTerrainNum < 0 || *pbyTerrainNum > AROUND_AREA_NUM)
		return false;
	return true;
}

void CMapOutdoor::BuildViewFrustum(D3DXMATRIX & mat)
{
	//m_plane[0] = D3DXPLANE(mat._14 + mat._13, mat._24 + mat._23, mat._34 + mat._33, mat._44 + mat._43);
	m_plane[0] = D3DXPLANE(          mat._13,           mat._23,           mat._33,           mat._43);		// Near
	m_plane[1] = D3DXPLANE(mat._14 - mat._13, mat._24 - mat._23, mat._34 - mat._33, mat._44 - mat._43);		// Far
	m_plane[2] = D3DXPLANE(mat._14 + mat._11, mat._24 + mat._21, mat._34 + mat._31, mat._44 + mat._41);		// Left
	m_plane[3] = D3DXPLANE(mat._14 - mat._11, mat._24 - mat._21, mat._34 - mat._31, mat._44 - mat._41);		// Right
	m_plane[4] = D3DXPLANE(mat._14 + mat._12, mat._24 + mat._22, mat._34 + mat._32, mat._44 + mat._42);		// Bottom
	m_plane[5] = D3DXPLANE(mat._14 - mat._12, mat._24 - mat._22, mat._34 - mat._32, mat._44 - mat._42);		// Top

	for (int i = 0; i < 6; ++i)
		D3DXPlaneNormalize(&m_plane[i],&m_plane[i]);
}

bool MAPOUTDOOR_GET_HEIGHT_USE2D = true;
bool MAPOUTDOOR_GET_HEIGHT_TRACE = false;

void CMapOutdoor::__HeightCache_Update()
{
	m_kHeightCache.m_isUpdated=true;
}

void CMapOutdoor::__HeightCache_Init()
{
	m_kHeightCache.m_isUpdated=false;

	for (UINT uIndex=0; uIndex!=SHeightCache::HASH_SIZE; ++uIndex)
		m_kHeightCache.m_akVct_kItem[uIndex].clear();
}

float CMapOutdoor::GetHeight(float fx, float fy)
{
	float fTerrainHeight = GetTerrainHeight(fx, fy);

	if (!m_bEnableTerrainOnlyForHeight)
	{
		CCullingManager & rkCullingMgr = CCullingManager::Instance();

		float CHECK_HEIGHT = 25000.0f;
		float fObjectHeight = -CHECK_HEIGHT;

		Vector3d aVector3d;
		aVector3d.Set(fx, -fy, fTerrainHeight);

		FGetObjectHeight kGetObjHeight(fx, fy);

		RangeTester<FGetObjectHeight> kRangeTester_kGetObjHeight(&kGetObjHeight);
		rkCullingMgr.PointTest2d(aVector3d, &kRangeTester_kGetObjHeight);

		if (kGetObjHeight.m_bHeightFound)
			fObjectHeight = kGetObjHeight.m_fReturnHeight;

		return fMAX(fObjectHeight, fTerrainHeight);
	}

	return fTerrainHeight;
}

float CMapOutdoor::GetCacheHeight(float fx, float fy)
{
	unsigned int nx=int(fx);
	unsigned int ny=int(fy);

	DWORD dwKey=0;

#ifdef __HEIGHT_CACHE_TRACE__
	static DWORD s_dwTotalCount=0;
	static DWORD s_dwHitCount=0;
	static DWORD s_dwErrorCount=0;

	s_dwTotalCount++;
#endif

	std::vector<SHeightCache::SItem>* pkVct_kItem=NULL;
	if (m_kHeightCache.m_isUpdated && nx<16*30000 && ny<16*30000)
	{
		nx>>=4;
		ny>>=4;
		//short aPos[2]={nx, ny};

		dwKey=(ny<<16)|nx;//CalcCRC16Words(2, aPos);
		pkVct_kItem=&m_kHeightCache.m_akVct_kItem[dwKey%SHeightCache::HASH_SIZE];
		std::vector<SHeightCache::SItem>::iterator i;
		for (i=pkVct_kItem->begin(); i!=pkVct_kItem->end(); ++i)
		{
			SHeightCache::SItem& rkItem=*i;
			if (rkItem.m_dwKey==dwKey)
			{
#ifdef __HEIGHT_CACHE_TRACE__
				s_dwHitCount++;
				
				if (s_dwTotalCount>1000)
				{
					DWORD dwHitRate=s_dwHitCount*1000/s_dwTotalCount;
					static DWORD s_dwMaxHitRate=0;
					if (s_dwMaxHitRate<dwHitRate)
					{
						s_dwMaxHitRate=dwHitRate;
						printf("HitRate %f\n", s_dwMaxHitRate*0.1f);
					}


				}			
#endif
				return rkItem.m_fHeight;
			}
		}		
	}
	else
	{
#ifdef __HEIGHT_CACHE_TRACE__
		s_dwErrorCount++;
		//printf("NoCache (%f, %f)\n", fx/100.0f, fy/100.0f);
#endif
	}
#ifdef __HEIGHT_CACHE_TRACE__	
	if (s_dwTotalCount>=1000000)
	{
		printf("HitRate %f\n", s_dwHitCount*1000/s_dwTotalCount*0.1f);
		printf("ErrRate %f\n", s_dwErrorCount*1000/s_dwTotalCount*0.1f);
		s_dwHitCount=0;
		s_dwTotalCount=0;
		s_dwErrorCount=0;
	}
#endif
	
	float fTerrainHeight = GetTerrainHeight(fx, fy);
#ifdef SPHERELIB_STRICT
	if (MAPOUTDOOR_GET_HEIGHT_TRACE)
		printf("Terrain %f\n", fTerrainHeight);
#endif
	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	float CHECK_HEIGHT = 25000.0f;
	float fObjectHeight = -CHECK_HEIGHT;

	if (MAPOUTDOOR_GET_HEIGHT_USE2D)
	{
		Vector3d aVector3d;
		aVector3d.Set(fx, -fy, fTerrainHeight);
		
		FGetObjectHeight kGetObjHeight(fx, fy);
		
		RangeTester<FGetObjectHeight> kRangeTester_kGetObjHeight(&kGetObjHeight);
		rkCullingMgr.PointTest2d(aVector3d, &kRangeTester_kGetObjHeight);

		if (kGetObjHeight.m_bHeightFound)
			fObjectHeight = kGetObjHeight.m_fReturnHeight;
	}
	else
	{
		Vector3d aVector3d;
		aVector3d.Set(fx, -fy, fTerrainHeight);

		Vector3d toTop;
		toTop.Set(0,0,CHECK_HEIGHT);

		FGetObjectHeight kGetObjHeight(fx, fy);
		rkCullingMgr.ForInRay(aVector3d, toTop, &kGetObjHeight);

		if (kGetObjHeight.m_bHeightFound)
			fObjectHeight = kGetObjHeight.m_fReturnHeight;
	}

	float fHeight=fMAX(fObjectHeight, fTerrainHeight);

	if (pkVct_kItem)
	{
		if (pkVct_kItem->size()>=200)
		{
#ifdef __HEIGHT_CACHE_TRACE__
			printf("ClearCacheHeight[%d]\n", dwKey%SHeightCache::HASH_SIZE);
#endif
			pkVct_kItem->clear();
		}
		
		SHeightCache::SItem kItem;
		kItem.m_dwKey=dwKey;
		kItem.m_fHeight=fHeight;
		pkVct_kItem->push_back(kItem);
	}
	
	return fHeight;
}

bool CMapOutdoor::GetNormal(int ix, int iy, D3DXVECTOR3 * pv3Normal)
{
	if (ix <= 0)
		ix = 0;
	else if (ix >= m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE)
		ix = m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;
	
	if (iy <= 0)
		iy = 0;
	else if (iy >= m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
		iy = m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;
	
	WORD usCoordX, usCoordY;
	
	usCoordX = (WORD) (ix / (CTerrainImpl::TERRAIN_XSIZE));
	usCoordY = (WORD) (iy / (CTerrainImpl::TERRAIN_YSIZE));
	
	if (usCoordX >= m_sTerrainCountX - 1)
		usCoordX = m_sTerrainCountX - 1;
	
	if (usCoordY >= m_sTerrainCountY - 1)
		usCoordY = m_sTerrainCountY - 1;
	
	BYTE byTerrainNum;
	if (!GetTerrainNumFromCoord(usCoordX, usCoordY, &byTerrainNum))
		return false;
	
	CTerrain * pTerrain;
	
	if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		return false;
	
	while (ix >= CTerrainImpl::TERRAIN_XSIZE)
		ix -= CTerrainImpl::TERRAIN_XSIZE;

	while (iy >= CTerrainImpl::TERRAIN_YSIZE)
		iy -= CTerrainImpl::TERRAIN_YSIZE;

	return pTerrain->GetNormal(ix, iy, pv3Normal);
}

float CMapOutdoor::GetTerrainHeight(float fx, float fy)
{
	if (fy < 0)
		fy = -fy;
	long lx, ly;
	PR_FLOAT_TO_INT(fx, lx);
	PR_FLOAT_TO_INT(fy, ly);

	WORD usCoordX, usCoordY;

	usCoordX = (WORD) (lx / CTerrainImpl::TERRAIN_XSIZE);
	usCoordY = (WORD) (ly / CTerrainImpl::TERRAIN_YSIZE);

	BYTE byTerrainNum;
	if (!GetTerrainNumFromCoord(usCoordX, usCoordY, &byTerrainNum))
		return 0.0f;

	CTerrain * pTerrain;
	
	if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		return 0.0f;

	return pTerrain->GetHeight(lx, ly);
}

//////////////////////////////////////////////////////////////////////////
// For Grass
float CMapOutdoor::GetHeight(float * pPos)
{
	pPos[2] = GetHeight(pPos[0], pPos[1]);
	return pPos[2];
}

bool CMapOutdoor::GetBrushColor(float fX, float fY, float* pLowColor, float* pHighColor)
{
	bool bSuccess = false;
	
//	float fU, fV;
//	
//	GetOneToOneMappingCoordinates(fX, fY, fU, fV);
//	
//	if (fU >= 0.0f && fU <= 1.0f && fV >= 0.0f && fV <= 1.0f)
//	{
//		int nImageCol = (m_cBrushMap.GetWidth() - 1) * fU;
//		int nImageRow = (m_cBrushMap.GetHeight() - 1) * fV;
//		
//		// low
//		BYTE* pPixel = m_cBrushMap.GetPixel(nImageCol, nImageRow);
//		pLowColor[0] = (pPixel[0] / 255.0f);
//		pLowColor[1] = (pPixel[1] / 255.0f);
//		pLowColor[2] = (pPixel[2] / 255.0f);
//		pLowColor[3] = (pPixel[3] / 255.0f);
//		
//		// high
//		pPixel = m_cBrushMap2.GetPixel(nImageCol, nImageRow);
//		pHighColor[0] = (pPixel[0] / 255.0f);
//		pHighColor[1] = (pPixel[1] / 255.0f);
//		pHighColor[2] = (pPixel[2] / 255.0f);
//		pHighColor[3] = (pPixel[3] / 255.0f);
//		
//		bSuccess = true;
//	}
	pLowColor[0] = (1.0f);
	pLowColor[1] = (1.0f);
	pLowColor[2] = (1.0f);
	pLowColor[3] = (1.0f);
	pHighColor[0] = (1.0f);
	pHighColor[1] = (1.0f);
	pHighColor[2] = (1.0f);
	pHighColor[3] = (1.0f);
	
	return bSuccess;
}

// End of for grass
//////////////////////////////////////////////////////////////////////////
BOOL CMapOutdoor::GetAreaPointer(const BYTE c_byAreaNum, CArea ** ppArea)
{
	if (c_byAreaNum >= AROUND_AREA_NUM)
	{
		*ppArea = NULL;
		return FALSE;
	}

	if (NULL == m_pArea[c_byAreaNum])
	{
		*ppArea = NULL;
		return FALSE;
	}

	*ppArea = m_pArea[c_byAreaNum];
	return TRUE;
}

BOOL CMapOutdoor::GetTerrainPointer(const BYTE c_byTerrainNum, CTerrain ** ppTerrain)
{
	if (c_byTerrainNum >= AROUND_AREA_NUM)
	{
		*ppTerrain = NULL;
		return FALSE;
	}

	if (NULL == m_pTerrain[c_byTerrainNum])
	{
		*ppTerrain = NULL;
		return FALSE;
	}

	*ppTerrain = m_pTerrain[c_byTerrainNum];
	return TRUE;
}

void CMapOutdoor::SetDrawShadow(bool bDrawShadow)
{
	m_bDrawShadow = bDrawShadow;
}

void CMapOutdoor::SetDrawCharacterShadow(bool bDrawChrShadow)
{
	m_bDrawChrShadow = bDrawChrShadow;
}

DWORD CMapOutdoor::GetShadowMapColor(float fx, float fy)
{
	if (fy < 0)
		fy = -fy;

	float fTerrainSize = (float) (CTerrainImpl::TERRAIN_XSIZE);
	float fXRef = fx - (float) (m_lCurCoordStartX);
	float fYRef = fy - (float) (m_lCurCoordStartY);

	CTerrain * pTerrain;

	if (fYRef < -fTerrainSize)
		return 0xFFFFFFFF;
	else if (fYRef >= -fTerrainSize && fYRef < 0.0f)
	{
		if (fXRef < -fTerrainSize)
			return 0xFFFFFFFF;
		else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
		{
			if (GetTerrainPointer(0, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef + fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= 0.0f && fXRef < fTerrainSize)
		{
			if (GetTerrainPointer(1, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef, fYRef + fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
		{
			if (GetTerrainPointer(2, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef + fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else
			return 0xFFFFFFFF;
	}
	else if (fYRef >= 0.0f && fYRef < fTerrainSize)
	{
		if (fXRef < -fTerrainSize)
			return 0xFFFFFFFF;
		else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
		{
			if (GetTerrainPointer(3, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= 0.0f && fXRef < fTerrainSize)
		{
			if (GetTerrainPointer(4, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef, fYRef);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
		{
			if (GetTerrainPointer(5, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef);
			else
				return 0xFFFFFFFF;
		}
		else
			return 0xFFFFFFFF;
	}
	else if (fYRef >= fTerrainSize && fYRef < 2.0f * fTerrainSize)
	{
		if (fXRef < -fTerrainSize)
			return 0xFFFFFFFF;
		else if (fXRef >= -fTerrainSize && fXRef < 0.0f)
		{
			if (GetTerrainPointer(6, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef + fTerrainSize, fYRef - fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= 0.0f && fXRef < fTerrainSize)
		{
			if (GetTerrainPointer(7, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef, fYRef - fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else if (fXRef >= fTerrainSize && fXRef < 2.0f * fTerrainSize)
		{
			if (GetTerrainPointer(8, &pTerrain))
				return pTerrain->GetShadowMapColor(fXRef - fTerrainSize, fYRef - fTerrainSize);
			else
				return 0xFFFFFFFF;
		}
		else
			return 0xFFFFFFFF;
	}
	else
		return 0xFFFFFFFF;

	return 0xFFFFFFFF;
}

bool CMapOutdoor::isAttrOn(float fX, float fY, BYTE byAttr)
{
	int iX, iY;
	PR_FLOAT_TO_INT(fX, iX);
	PR_FLOAT_TO_INT(fY, iY);

	return isAttrOn(iX, iY, byAttr);
}

bool CMapOutdoor::GetAttr(float fX, float fY, BYTE * pbyAttr)
{
	int iX, iY;
	PR_FLOAT_TO_INT(fX, iX);
	PR_FLOAT_TO_INT(fY, iY);

	return GetAttr(iX, iY, pbyAttr);
}

bool CMapOutdoor::isAttrOn(int iX, int iY, BYTE byAttr)
{
	if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE || iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
		return false;

	WORD wTerrainCoordX, wTerrainCoordY;
	wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
	wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;

	BYTE byTerrainNum;
	if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
		return false;
	CTerrain * pTerrain;
	if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		return false;

	WORD wLocalX, wLocalY;
	wLocalX = (iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::HALF_CELLSCALE);
	wLocalY = (iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::HALF_CELLSCALE);
	
	return pTerrain->isAttrOn(wLocalX, wLocalY, byAttr);
}

bool CMapOutdoor::GetAttr(int iX, int iY, BYTE * pbyAttr)
{
	if (iX < 0 || iY < 0 || iX > m_sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE || iY > m_sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE)
		return false;
	
	WORD wTerrainCoordX, wTerrainCoordY;
	wTerrainCoordX = iX / CTerrainImpl::TERRAIN_XSIZE;
	wTerrainCoordY = iY / CTerrainImpl::TERRAIN_YSIZE;
	
	BYTE byTerrainNum;
	if (!GetTerrainNumFromCoord(wTerrainCoordX, wTerrainCoordY, &byTerrainNum))
		return false;
	CTerrain * pTerrain;
	if (!GetTerrainPointer(byTerrainNum, &pTerrain))
		return false;

	WORD wLocalX, wLocalY;
	wLocalX = (WORD) (iX - wTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE) / (CTerrainImpl::HALF_CELLSCALE);
	wLocalY = (WORD) (iY - wTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE) / (CTerrainImpl::HALF_CELLSCALE);

	BYTE byAttr = pTerrain->GetAttr(wLocalX, wLocalY);

	*pbyAttr = byAttr;

	return true;
}

// MonsterAreaInfo
CMonsterAreaInfo * CMapOutdoor::AddMonsterAreaInfo(long lOriginX, long lOriginY, long lSizeX, long lSizeY)
{
	CMonsterAreaInfo * pMonsterAreaInfo = m_kPool_kMonsterAreaInfo.Alloc();
	pMonsterAreaInfo->Clear();
	pMonsterAreaInfo->SetOrigin(lOriginX, lOriginY);
	pMonsterAreaInfo->SetSize(lSizeX, lSizeY);
	m_MonsterAreaInfoPtrVector.push_back(pMonsterAreaInfo);

	return pMonsterAreaInfo;
}

void CMapOutdoor::RemoveAllMonsterAreaInfo()
{
	m_MonsterAreaInfoPtrVectorIterator = m_MonsterAreaInfoPtrVector.begin();
	while (m_MonsterAreaInfoPtrVectorIterator != m_MonsterAreaInfoPtrVector.end())
	{
		CMonsterAreaInfo * pMonsterAreaInfo = *m_MonsterAreaInfoPtrVectorIterator;
		pMonsterAreaInfo->Clear();
		++m_MonsterAreaInfoPtrVectorIterator;
	}
	m_kPool_kMonsterAreaInfo.FreeAll();
	m_MonsterAreaInfoPtrVector.clear();
}

bool CMapOutdoor::GetMonsterAreaInfoFromVectorIndex(DWORD dwMonsterAreaInfoVectorIndex, CMonsterAreaInfo ** ppMonsterAreaInfo)
{
	if (dwMonsterAreaInfoVectorIndex >= m_MonsterAreaInfoPtrVector.size())
		return false;
	
	*ppMonsterAreaInfo = m_MonsterAreaInfoPtrVector[dwMonsterAreaInfoVectorIndex];
	return true;
}

//////////////////////////////////////////////////////////////////////////
CMonsterAreaInfo * CMapOutdoor::AddNewMonsterAreaInfo(long lOriginX, long lOriginY, long lSizeX, long lSizeY,
										CMonsterAreaInfo::EMonsterAreaInfoType eMonsterAreaInfoType,
										DWORD dwVID, DWORD dwCount, CMonsterAreaInfo::EMonsterDir eMonsterDir)
{
	CMonsterAreaInfo * pMonsterAreaInfo = m_kPool_kMonsterAreaInfo.Alloc();
	pMonsterAreaInfo->Clear();
	pMonsterAreaInfo->SetOrigin(lOriginX, lOriginY);
	pMonsterAreaInfo->SetSize(lSizeX, lSizeY);

	pMonsterAreaInfo->SetMonsterAreaInfoType(eMonsterAreaInfoType);

	if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_MONSTER == eMonsterAreaInfoType)
		pMonsterAreaInfo->SetMonsterVID(dwVID);
	else if (CMonsterAreaInfo::MONSTERAREAINFOTYPE_GROUP == eMonsterAreaInfoType)
		pMonsterAreaInfo->SetMonsterGroupID(dwVID);
	pMonsterAreaInfo->SetMonsterCount(dwCount);
	pMonsterAreaInfo->SetMonsterDirection(eMonsterDir);
	m_MonsterAreaInfoPtrVector.push_back(pMonsterAreaInfo);

	return pMonsterAreaInfo;
}

//////////////////////////////////////////////////////////////////////////
void CMapOutdoor::GetBaseXY(DWORD * pdwBaseX, DWORD * pdwBaseY)
{
	*pdwBaseX = m_dwBaseX;
	*pdwBaseY = m_dwBaseY;
}

void CMapOutdoor::SetBaseXY(DWORD dwBaseX, DWORD dwBaseY)
{
	m_dwBaseX = dwBaseX;
	m_dwBaseY = dwBaseY;
}

void CMapOutdoor::SetEnvironmentDataName(const std::string& strEnvironmentDataName)
{
	m_envDataName = strEnvironmentDataName;
}

void CMapOutdoor::__XMasTree_Initialize()
{
	m_kXMas.m_pkTree=NULL;
	m_kXMas.m_iEffectID=-1;
}

void CMapOutdoor::XMasTree_Destroy()
{
	if (m_kXMas.m_pkTree)
	{
		CSpeedTreeForestDirectX& rkForest=CSpeedTreeForestDirectX::Instance();
		m_kXMas.m_pkTree->Clear();
		rkForest.DeleteInstance(m_kXMas.m_pkTree);
		m_kXMas.m_pkTree=NULL;
	}
	if (-1 != m_kXMas.m_iEffectID)
	{
		CEffectManager& rkEffMgr = CEffectManager::Instance();
		rkEffMgr.DestroyEffectInstance(m_kXMas.m_iEffectID);
		m_kXMas.m_iEffectID=-1;
	}
}

void CMapOutdoor::__XMasTree_Create(float x, float y, float z, const char* c_szTreeName, const char* c_szEffName)
{
	assert(NULL==m_kXMas.m_pkTree);
	assert(-1==m_kXMas.m_iEffectID);

	CSpeedTreeForestDirectX& rkForest=CSpeedTreeForestDirectX::Instance();
	DWORD dwCRC32 = GetCaseCRC32(c_szTreeName, strlen(c_szTreeName));
	m_kXMas.m_pkTree=rkForest.CreateInstance(x, y, z, dwCRC32, c_szTreeName);

	CEffectManager& rkEffMgr = CEffectManager::Instance();
	rkEffMgr.RegisterEffect(c_szEffName);
	m_kXMas.m_iEffectID = rkEffMgr.CreateEffect(c_szEffName,
												D3DXVECTOR3(x, y, z),
												D3DXVECTOR3(0.0f, 0.0f, 0.0f));
}

void CMapOutdoor::XMasTree_Set(float x, float y, float z, const char* c_szTreeName, const char* c_szEffName)
{
	XMasTree_Destroy();
	__XMasTree_Create(x, y, z, c_szTreeName, c_szEffName);
}

void CMapOutdoor::SpecialEffect_Create(DWORD dwID, float x, float y, float z, const char* c_szEffName)
{
	CEffectManager& rkEffMgr = CEffectManager::Instance();

	TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.find(dwID);
	if (m_kMap_dwID_iEffectID.end() != itor)
	{
		DWORD dwEffectID = itor->second;
		if (rkEffMgr.SelectEffectInstance(dwEffectID))
		{
			D3DXMATRIX mat;
			D3DXMatrixIdentity(&mat);
			mat._41 = x;
			mat._42 = y;
			mat._43 = z;
			rkEffMgr.SetEffectInstanceGlobalMatrix(mat);
			return;
		}
	}

	rkEffMgr.RegisterEffect(c_szEffName);
	DWORD dwEffectID = rkEffMgr.CreateEffect(c_szEffName,
											 D3DXVECTOR3(x, y, z),
											 D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	m_kMap_dwID_iEffectID.insert(std::make_pair(dwID, dwEffectID));
}

void CMapOutdoor::SpecialEffect_Delete(DWORD dwID)
{
	TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.find(dwID);

	if (m_kMap_dwID_iEffectID.end() == itor)
		return;

	CEffectManager& rkEffMgr = CEffectManager::Instance();
	int iEffectID = itor->second;
	rkEffMgr.DestroyEffectInstance(iEffectID);
}

void CMapOutdoor::SpecialEffect_Destroy()
{
	CEffectManager& rkEffMgr = CEffectManager::Instance();

	TSpecialEffectMap::iterator itor = m_kMap_dwID_iEffectID.begin();
	for (; itor != m_kMap_dwID_iEffectID.end(); ++itor)
	{
		int iEffectID = itor->second;
		rkEffMgr.DestroyEffectInstance(iEffectID);
	}
}

void CMapOutdoor::ClearGuildArea()
{
	m_rkList_kGuildArea.clear();
}

void CMapOutdoor::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
	RECT rect;
	rect.left = isx;
	rect.top = isy;
	rect.right = iex;
	rect.bottom = iey;
	m_rkList_kGuildArea.push_back(rect);
}

void CMapOutdoor::VisibleMarkedArea()
{
	std::map<int, BYTE*> kMap_pbyMarkBuf;
	std::set<int> kSet_iProcessedMapIndex;

	std::list<RECT>::iterator itorRect = m_rkList_kGuildArea.begin();
	for (; itorRect != m_rkList_kGuildArea.end(); ++itorRect)
	{
		const RECT & rkRect = *itorRect;

		int ix1Cell;
		int iy1Cell;
		BYTE byx1SubCell;
		BYTE byy1SubCell;
		WORD wx1TerrainNum;
		WORD wy1TerrainNum;

		int ix2Cell;
		int iy2Cell;
		BYTE byx2SubCell;
		BYTE byy2SubCell;
		WORD wx2TerrainNum;
		WORD wy2TerrainNum;

		ConvertToMapCoords(float(rkRect.left), float(rkRect.top), &ix1Cell, &iy1Cell, &byx1SubCell, &byy1SubCell, &wx1TerrainNum, &wy1TerrainNum);
		ConvertToMapCoords(float(rkRect.right), float(rkRect.bottom), &ix2Cell, &iy2Cell, &byx2SubCell, &byy2SubCell, &wx2TerrainNum, &wy2TerrainNum);

		ix1Cell = ix1Cell + wx1TerrainNum*CTerrain::ATTRMAP_XSIZE;
		iy1Cell = iy1Cell + wy1TerrainNum*CTerrain::ATTRMAP_YSIZE;
		ix2Cell = ix2Cell + wx2TerrainNum*CTerrain::ATTRMAP_XSIZE;
		iy2Cell = iy2Cell + wy2TerrainNum*CTerrain::ATTRMAP_YSIZE;

		for (int ixCell = ix1Cell; ixCell <= ix2Cell; ++ixCell)
		for (int iyCell = iy1Cell; iyCell <= iy2Cell; ++iyCell)
		{
			int ixLocalCell = ixCell % CTerrain::ATTRMAP_XSIZE;
			int iyLocalCell = iyCell % CTerrain::ATTRMAP_YSIZE;
			int ixTerrain = ixCell / CTerrain::ATTRMAP_XSIZE;
			int iyTerrain = iyCell / CTerrain::ATTRMAP_YSIZE;
			int iTerrainNum = ixTerrain+iyTerrain*100;

			BYTE byTerrainNum;
			if (!GetTerrainNumFromCoord(ixTerrain, iyTerrain, &byTerrainNum))
				continue;
			CTerrain * pTerrain;
			if (!GetTerrainPointer(byTerrainNum, &pTerrain))
				continue;

			if (kMap_pbyMarkBuf.end() == kMap_pbyMarkBuf.find(iTerrainNum))
			{
				BYTE * pbyBuf = new BYTE [CTerrain::ATTRMAP_XSIZE*CTerrain::ATTRMAP_YSIZE];
				ZeroMemory(pbyBuf, CTerrain::ATTRMAP_XSIZE*CTerrain::ATTRMAP_YSIZE);
				kMap_pbyMarkBuf[iTerrainNum] = pbyBuf;
			}

			BYTE * pbyBuf = kMap_pbyMarkBuf[iTerrainNum];
			pbyBuf[ixLocalCell+iyLocalCell*CTerrain::ATTRMAP_XSIZE] = 0xff;
		}
	}

	std::map<int, BYTE*>::iterator itorTerrain = kMap_pbyMarkBuf.begin();
	for (; itorTerrain != kMap_pbyMarkBuf.end(); ++itorTerrain)
	{
		int iTerrainNum = itorTerrain->first;
		int ixTerrain = iTerrainNum%100;
		int iyTerrain = iTerrainNum/100;
		BYTE * pbyBuf = itorTerrain->second;

		BYTE byTerrainNum;
		if (!GetTerrainNumFromCoord(ixTerrain, iyTerrain, &byTerrainNum))
			continue;
		CTerrain * pTerrain;
		if (!GetTerrainPointer(byTerrainNum, &pTerrain))
			continue;

		pTerrain->AllocateMarkedSplats(pbyBuf);
	}

	stl_wipe_second(kMap_pbyMarkBuf);
}

void CMapOutdoor::DisableMarkedArea()
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		if (!m_pTerrain[i])
			continue;

		m_pTerrain[i]->DeallocateMarkedSplats();
	}
}

void CMapOutdoor::ConvertToMapCoords(float fx, float fy, int *iCellX, int *iCellY, BYTE * pucSubCellX, BYTE * pucSubCellY, WORD * pwTerrainNumX, WORD * pwTerrainNumY)
{
	if ( fy < 0 )
		fy = -fy;
	
	int ix, iy;
	PR_FLOAT_TO_INT(fx, ix);
	PR_FLOAT_TO_INT(fy, iy);
	
	*pwTerrainNumX = ix / (CTerrainImpl::TERRAIN_XSIZE);
	*pwTerrainNumY = iy / (CTerrainImpl::TERRAIN_YSIZE);
	
	float maxx = (float) CTerrainImpl::TERRAIN_XSIZE;
	float maxy = (float) CTerrainImpl::TERRAIN_YSIZE;
	
	while (fx < 0)
		fx += maxx;
	
	while (fy < 0)
		fy += maxy;
	
	while (fx >= maxx)
		fx -= maxx;
	
	while (fy >= maxy)
		fy -= maxy;
	
	float fooscale = 1.0f / (float)(CTerrainImpl::HALF_CELLSCALE);
	
	float fCellX, fCellY;
	
	fCellX = fx * fooscale;
	fCellY = fy * fooscale;
	
	PR_FLOAT_TO_INT(fCellX, *iCellX);
	PR_FLOAT_TO_INT(fCellY, *iCellY);
	
	float fRatioooscale = ((float)CTerrainImpl::HEIGHT_TILE_XRATIO) * fooscale;
	
	float fSubcellX, fSubcellY;
	fSubcellX = fx * fRatioooscale;
	fSubcellY = fy * fRatioooscale;
	
	PR_FLOAT_TO_INT(fSubcellX, *pucSubCellX);
	PR_FLOAT_TO_INT(fSubcellY, *pucSubCellY);
	*pucSubCellX = (*pucSubCellX) % CTerrainImpl::HEIGHT_TILE_XRATIO;
	*pucSubCellY = (*pucSubCellY) % CTerrainImpl::HEIGHT_TILE_YRATIO;
}
