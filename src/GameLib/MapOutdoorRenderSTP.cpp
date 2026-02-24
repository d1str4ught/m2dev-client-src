#include "StdAfx.h"
#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "TerrainQuadtree.h"

#include "EterLib/Camera.h"
#include "EterLib/StateManager.h"

struct SoftwareTransformPatch_SSplatVertex
{
	D3DXVECTOR4 kPosition;
	DWORD		dwDiffuse;
	DWORD		dwSpecular;
	D3DXVECTOR2 kTex1;
	D3DXVECTOR2 kTex2;
};



void CMapOutdoor::__RenderTerrain_RenderSoftwareTransformPatch()
{	
	SoftwareTransformPatch_SRenderState kTPRS;

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);

	__SoftwareTransformPatch_ApplyRenderState();

	__SoftwareTransformPatch_BuildPipeline(kTPRS);

	std::pair<float, long> fog_far(kTPRS.m_fFogFarDistance+800.0f, 0);
	std::pair<float, long> fog_near(kTPRS.m_fFogNearDistance-3200.0f, 0);

	std::vector<std::pair<float ,long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(),m_PatchVector.end(),fog_far);
	std::vector<std::pair<float ,long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(),m_PatchVector.end(),fog_near);

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	STATEMANAGER.SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX2);

	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();

	for( ; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}
		
		__SoftwareTransformPatch_RenderPatchSplat(kTPRS, it->second, wPrimitiveCount, ePrimitiveType, false);
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;
		
	}

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for(it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__SoftwareTransformPatch_RenderPatchSplat(kTPRS, it->second, wPrimitiveCount, ePrimitiveType, true);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

		}
	}

	
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);
	
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);	

	STATEMANAGER.SetFVF(D3DFVF_XYZRHW);

	if (IsFastTNL())
	{
		if (byCUrrentLODLevel != 2)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}

		if (m_iRenderedSplatNum < m_iSplatLimit)
		{
			for(it = far_it; it != m_PatchVector.end(); ++it)
			{
				__SoftwareTransformPatch_RenderPatchNone(kTPRS, it->second, wPrimitiveCount, ePrimitiveType);

				if (m_iRenderedSplatNum >= m_iSplatLimit)
					break;

			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Render State & TextureStageState
	__SoftwareTransformPatch_RestoreRenderState(dwFogEnable);
}

void CMapOutdoor::__SoftwareTransformPatch_RenderPatchSplat(SoftwareTransformPatch_SRenderState& rkTPRS, long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType, bool isFogEnable)
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::__SoftwareTransformPatch_RenderPatchSplat");

	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	
	if (!pTerrainPatchProxy->isUsed())
		return;

	bool isDynamicShadow = pTerrainPatchProxy->IsIn(rkTPRS.m_v3Player, 3000.0f);

	if (!m_bDrawChrShadow)
		isDynamicShadow = false;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();

	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();

	if (0xFF == ucTerrainNum)
		return;

	CTerrain * pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch & rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();
	
	SoftwareTransformPatch_STLVertex akTransVertex[CTerrainPatch::TERRAIN_VERTEX_COUNT];
	if (!__SoftwareTransformPatch_SetTransform(rkTPRS, akTransVertex, *pTerrainPatchProxy, wCoordX, wCoordY, isFogEnable, isDynamicShadow))
		return;
	
	if (!__SoftwareTransformPatch_SetSplatStream(akTransVertex))
		return;
	
	if (isFogEnable)
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_BLENDDIFFUSEALPHA);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	}

	int iPrevRenderedSplatNum=m_iRenderedSplatNum;
	
	bool isFirst=true;
	for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j)
	{
		TTerainSplat & rSplat = rTerrainSplatPatch.Splats[j];
		
		if (!rSplat.Active)
			continue;
		
		if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
			continue;
		
		const TTerrainTexture & rTexture = m_TextureSet.GetTexture(j);
		
		if (isFirst)
		{
			STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
			STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
			STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
			STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
			isFirst=false;
		}
		else
		{
			STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
			STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);			
		}
		
		std::vector<int>::iterator aIterator = std::find(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end(), (int)j);
		if (aIterator == m_RenderedTextureNumVector.end())
			m_RenderedTextureNumVector.push_back(j);
		++m_iRenderedSplatNum;
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;
	}	

	// 그림자
	if (m_bDrawShadow)
	{	
		__SoftwareTransformPatch_SetShadowStream(akTransVertex);
		__SoftwareTransformPatch_ApplyStaticShadowRenderState();

		if (isDynamicShadow)
			__SoftwareTransformPatch_ApplyDynamicShadowRenderState();
		else
			__SoftwareTransformPatch_ApplyFogShadowRenderState();

		if (isFogEnable)
		{
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, 0xFFFFFFFF);
			STATEMANAGER.SetTexture(0, pTerrain->GetShadowTexture());
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
			STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, rkTPRS.m_dwFogColor);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
		}
		else
		{
			STATEMANAGER.SetTexture(0, pTerrain->GetShadowTexture());
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
		}

		if (isDynamicShadow)
			__SoftwareTransformPatch_RestoreDynamicShadowRenderState();
		else
			__SoftwareTransformPatch_RestoreFogShadowRenderState();
		
		ms_faceCount += wPrimitiveCount;
  		++m_iRenderedSplatNum;

		

		__SoftwareTransformPatch_RestoreStaticShadowRenderState();
	}

	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum=m_iRenderedSplatNum-iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum+=iCurRenderedSplatNum*iCurRenderedSplatNum;
}

void CMapOutdoor::__SoftwareTransformPatch_RenderPatchNone(SoftwareTransformPatch_SRenderState& rkTPRS, long patchnum,	WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::__SoftwareTransformPatch_RenderPatchNone");

	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];
	
	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain * pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	SoftwareTransformPatch_SSourceVertex* akSrcVertex=pTerrainPatchProxy->SoftwareTransformPatch_GetTerrainVertexDataPtr();
	if (!akSrcVertex)
		return;
	
	float fScreenHalfWidth=rkTPRS.m_fScreenHalfWidth;
	float fScreenHalfHeight=rkTPRS.m_fScreenHalfHeight;

	D3DXMATRIX m4Frustum=rkTPRS.m_m4Frustum;

	SoftwareTransformPatch_STVertex akTransVertex[CTerrainPatch::TERRAIN_VERTEX_COUNT];

	D3DXVECTOR4* akPosition=(D3DXVECTOR4*)akTransVertex;
	D3DXVECTOR4* pkPosition;
	for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
	{		
		pkPosition=akPosition+uIndex;
		D3DXVec3Transform(pkPosition, &akSrcVertex[uIndex].kPosition, &m4Frustum);
		if (pkPosition->w < 0.0001f)
			pkPosition->w = 0.0001f;

		pkPosition->w=1.0f/pkPosition->w;
		pkPosition->z*=pkPosition->w;
		pkPosition->y=(pkPosition->y*pkPosition->w-1.0f)*fScreenHalfHeight;	
		pkPosition->x=(pkPosition->x*pkPosition->w+1.0f)*fScreenHalfWidth;
	}
	

	IDirect3DVertexBuffer9* pkVB=m_kSTPD.m_pkVBNone[m_kSTPD.m_dwNonePos++];
	m_kSTPD.m_dwNonePos%=SoftwareTransformPatch_SData::NONE_VB_NUM;
	if (!pkVB)
		return;

	DWORD dwVBSize=sizeof(SoftwareTransformPatch_STVertex)*CTerrainPatch::TERRAIN_VERTEX_COUNT;
	SoftwareTransformPatch_STVertex* akDstVertex;
	if (FAILED(
		pkVB->Lock(0, dwVBSize, (void**)&akDstVertex, D3DLOCK_DISCARD)
	)) return;

	memcpy(akDstVertex, akTransVertex, dwVBSize);

	pkVB->Unlock();

	STATEMANAGER.SetStreamSource(0, pkVB, sizeof(SoftwareTransformPatch_STVertex));
	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
	ms_faceCount += wPrimitiveCount;
}

void CMapOutdoor::__SoftwareTransformPatch_ApplyStaticShadowRenderState()
{
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

}

void CMapOutdoor::__SoftwareTransformPatch_ApplyDynamicShadowRenderState()
{
 	STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

void CMapOutdoor::__SoftwareTransformPatch_ApplyFogShadowRenderState()
{
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}
void CMapOutdoor::__SoftwareTransformPatch_RestoreStaticShadowRenderState()
{
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}



void CMapOutdoor::__SoftwareTransformPatch_RestoreDynamicShadowRenderState()
{
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}



void CMapOutdoor::__SoftwareTransformPatch_RestoreFogShadowRenderState()
{
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	
	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
}

void CMapOutdoor::__SoftwareTransformPatch_ApplyRenderState()
{
    DWORD dwFogColor = 0xffffffff;
    if (mc_pEnvironmentData)
        dwFogColor = mc_pEnvironmentData->FogColor;

    STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
    STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    // Use SetSamplerState for addressing in D3D9Ex (D3DTSS_ADDRESSx is deprecated)
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    STATEMANAGER.SetBestFiltering(0);

    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

    // Use SetSamplerState for addressing in D3D9Ex (D3DTSS_ADDRESSx is deprecated)
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    STATEMANAGER.SetBestFiltering(1);

    CSpeedTreeWrapper::ms_bSelfShadowOn = true;

    m_iRenderedSplatNumSqSum = 0;
    m_iRenderedPatchNum      = 0;
    m_iRenderedSplatNum      = 0;
    m_RenderedTextureNumVector.clear();
    m_matWorldForCommonUse._41 = 0.0f;
    m_matWorldForCommonUse._42 = 0.0f;
    STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);
}

void CMapOutdoor::__SoftwareTransformPatch_RestoreRenderState(DWORD dwFogEnable)
{
    STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

    std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());

    STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
    STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

}

void CMapOutdoor::__SoftwareTransformPatch_BuildPipeline(SoftwareTransformPatch_SRenderState& rkTPRS)
{
	memset(&rkTPRS, 0, sizeof(rkTPRS));

	if (mc_pEnvironmentData)
	{
		rkTPRS.m_dwFogColor = mc_pEnvironmentData->FogColor;
		rkTPRS.m_fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
		rkTPRS.m_fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
	}
	else
	{
		rkTPRS.m_dwFogColor = 0xffffffff;
		rkTPRS.m_fFogNearDistance = 5000.0f;
		rkTPRS.m_fFogFarDistance = 10000.0f;
	}

	UINT uScreenWidth;
	UINT uScreenHeight;
	CScreen::GetBackBufferSize(&uScreenWidth, &uScreenHeight);

	rkTPRS.m_fScreenHalfWidth = +float(uScreenWidth) / 2.0f;
	rkTPRS.m_fScreenHalfHeight = -float(uScreenHeight) / 2.0f;

	STATEMANAGER.GetLight(0, &rkTPRS.m_kLight);
	STATEMANAGER.GetMaterial(&rkTPRS.m_kMtrl);

	D3DXMATRIX m4View;STATEMANAGER.GetTransform(D3DTS_VIEW, &m4View);
	D3DXMATRIX m4Proj;STATEMANAGER.GetTransform(D3DTS_PROJECTION, &m4Proj);
	
	D3DXMatrixMultiply(&rkTPRS.m_m4Frustum, &m4View, &m4Proj);

	rkTPRS.m_v3Player.x = +m_v3Player.x;
	rkTPRS.m_v3Player.y = -m_v3Player.y;
	rkTPRS.m_v3Player.z = +m_v3Player.z;

	rkTPRS.m_m4Proj = m4Proj;
	rkTPRS.m_m4DynamicShadow = m_matLightView * m_matDynamicShadowScale;

	rkTPRS.m_fFogNearTransZ = rkTPRS.m_fFogNearDistance;
	rkTPRS.m_fFogFarTransZ  = rkTPRS.m_fFogFarDistance;
	rkTPRS.m_fFogLenInv     = 1.0f / (rkTPRS.m_fFogFarDistance - rkTPRS.m_fFogNearDistance);


}
		
bool CMapOutdoor::__SoftwareTransformPatch_SetTransform(SoftwareTransformPatch_SRenderState& rkTPRS, SoftwareTransformPatch_STLVertex* akTransVertex, CTerrainPatchProxy& rkTerrainPatchProxy, UINT uTerrainX, UINT uTerrainY, bool isFogEnable, bool isDynamicShadow)
{
	SoftwareTransformPatch_SSourceVertex* akSrcVertex=rkTerrainPatchProxy.SoftwareTransformPatch_GetTerrainVertexDataPtr();
	if (!akSrcVertex)
		return false;
	
	rkTerrainPatchProxy.SoftwareTransformPatch_UpdateTerrainLighting(
		m_kSTPD.m_dwLightVersion, 
		rkTPRS.m_kLight, rkTPRS.m_kMtrl);
	
	D3DXVECTOR3* pkSrcPosition;

	float fTilePatternX=+1/640.0f;
	float fTilePatternY=-1/640.0f;

	float fTerrainBaseX=-(float) (uTerrainX * CTerrainImpl::TERRAIN_XSIZE)+m_fTerrainTexCoordBase * 12.30769f;
	float fTerrainBaseY=+(float) (uTerrainY * CTerrainImpl::TERRAIN_YSIZE)+m_fTerrainTexCoordBase * 12.30769f;	

	float fScreenHalfWidth=rkTPRS.m_fScreenHalfWidth;
	float fScreenHalfHeight=rkTPRS.m_fScreenHalfHeight;

	float fAlphaPatternX=m_matSplatAlpha._11;
	float fAlphaPatternY=m_matSplatAlpha._22;
	float fAlphaBiasX=m_matSplatAlpha._41;
	float fAlphaBiasY=m_matSplatAlpha._42;	
	float fShadowPatternX=+m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_XSIZE / static_cast<float>(CTerrainImpl::XSIZE));		
	float fShadowPatternY=-m_fTerrainTexCoordBase * ((float) CTerrainImpl::PATCH_YSIZE / static_cast<float>(CTerrainImpl::YSIZE));

	D3DXMATRIX m4Frustum=rkTPRS.m_m4Frustum;
	
	if (isFogEnable)
	{
		float fFogCur;
		float fFogFar=rkTPRS.m_fFogFarTransZ;
		float fFogLenInv=rkTPRS.m_fFogLenInv;

		float fLocalX;
		float fLocalY;

		SoftwareTransformPatch_STLVertex kWorkVertex; 
		for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
		{		
			pkSrcPosition=&akSrcVertex[uIndex].kPosition;
			D3DXVec3Transform(&kWorkVertex.kPosition, pkSrcPosition, &m4Frustum);
			fLocalX=pkSrcPosition->x+fTerrainBaseX;
			fLocalY=pkSrcPosition->y+fTerrainBaseY;	
			if (kWorkVertex.kPosition.w < 0.0001f)
				kWorkVertex.kPosition.w = 0.0001f;
			float fViewDepth = kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.w=1.0f/kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.x*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.y*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.z*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.x=(kWorkVertex.kPosition.x+1.0f)*fScreenHalfWidth;
			kWorkVertex.kPosition.y=(kWorkVertex.kPosition.y-1.0f)*fScreenHalfHeight;			
			kWorkVertex.dwDiffuse=akSrcVertex[uIndex].dwDiffuse;
			kWorkVertex.kTexTile.x=pkSrcPosition->x*fTilePatternX;
			kWorkVertex.kTexTile.y=pkSrcPosition->y*fTilePatternY;	
			kWorkVertex.kTexAlpha.x=fLocalX*fAlphaPatternX+fAlphaBiasX;
			kWorkVertex.kTexAlpha.y=fLocalY*fAlphaPatternY+fAlphaBiasY;
			kWorkVertex.kTexStaticShadow.x=fLocalX*fShadowPatternX;
			kWorkVertex.kTexStaticShadow.y=fLocalY*fShadowPatternY;
			kWorkVertex.kTexDynamicShadow.x=0.0f;
			kWorkVertex.kTexDynamicShadow.y=0.0f;

			fFogCur=(fFogFar-fViewDepth)*fFogLenInv;
			if (fFogCur<0.0f)
				kWorkVertex.dwFog=kWorkVertex.dwDiffuse=0x00000000|(kWorkVertex.dwDiffuse&0xffffff);
			else if (fFogCur>1.0f)
				kWorkVertex.dwFog=kWorkVertex.dwDiffuse=0xFF000000|(kWorkVertex.dwDiffuse&0xffffff);
			else
				kWorkVertex.dwFog=kWorkVertex.dwDiffuse=BYTE(255.0f*fFogCur)<<24|(kWorkVertex.dwDiffuse&0xffffff);

			*(akTransVertex+uIndex)=kWorkVertex;
		}
	}
	else
	{
		float fLocalX;
		float fLocalY;
		
		SoftwareTransformPatch_STLVertex kWorkVertex; 
		for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
		{		
			pkSrcPosition=&akSrcVertex[uIndex].kPosition;
			D3DXVec3Transform(&kWorkVertex.kPosition, pkSrcPosition, &m4Frustum);
			fLocalX=pkSrcPosition->x+fTerrainBaseX;
			fLocalY=pkSrcPosition->y+fTerrainBaseY;	
			if (kWorkVertex.kPosition.w < 0.0001f)
				kWorkVertex.kPosition.w = 0.0001f;
			kWorkVertex.kPosition.w=1.0f/kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.x*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.y*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.z*=kWorkVertex.kPosition.w;
			kWorkVertex.kPosition.x=(kWorkVertex.kPosition.x+1.0f)*fScreenHalfWidth;
			kWorkVertex.kPosition.y=(kWorkVertex.kPosition.y-1.0f)*fScreenHalfHeight;
			kWorkVertex.dwDiffuse=akSrcVertex[uIndex].dwDiffuse;
			kWorkVertex.dwFog=0xffffffff;
			kWorkVertex.kTexTile.x=pkSrcPosition->x*fTilePatternX;
			kWorkVertex.kTexTile.y=pkSrcPosition->y*fTilePatternY;	
			kWorkVertex.kTexAlpha.x=fLocalX*fAlphaPatternX+fAlphaBiasX;
			kWorkVertex.kTexAlpha.y=fLocalY*fAlphaPatternY+fAlphaBiasY;
			kWorkVertex.kTexStaticShadow.x=fLocalX*fShadowPatternX;
			kWorkVertex.kTexStaticShadow.y=fLocalY*fShadowPatternY;
			kWorkVertex.kTexDynamicShadow.x=0.0f;
			kWorkVertex.kTexDynamicShadow.y=0.0f;

			*(akTransVertex+uIndex)=kWorkVertex;
		}		
	}

	if (isDynamicShadow)
	{
		D3DXMATRIX m4DynamicShadow=rkTPRS.m_m4DynamicShadow;

		D3DXVECTOR3 v3Shadow;
		for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
		{
			D3DXVec3TransformCoord(&v3Shadow, &akSrcVertex[uIndex].kPosition, &m4DynamicShadow);
			akTransVertex[uIndex].kTexDynamicShadow.x=v3Shadow.x;
			akTransVertex[uIndex].kTexDynamicShadow.y=v3Shadow.y;
		}
	}

	return true;
}

bool CMapOutdoor::__SoftwareTransformPatch_SetSplatStream(SoftwareTransformPatch_STLVertex* akSrcVertex)
{
	IDirect3DVertexBuffer9* pkVB=m_kSTPD.m_pkVBSplat[m_kSTPD.m_dwSplatPos++];
	m_kSTPD.m_dwSplatPos%=SoftwareTransformPatch_SData::SPLAT_VB_NUM;
	if (!pkVB)
		return false;

	DWORD dwVBSize=sizeof(SoftwareTransformPatch_SSplatVertex)*CTerrainPatch::TERRAIN_VERTEX_COUNT;
	SoftwareTransformPatch_SSplatVertex* akDstVertex;
	if (FAILED(
		pkVB->Lock(0, dwVBSize, (void**)&akDstVertex, D3DLOCK_DISCARD)
	)) return false;

	SoftwareTransformPatch_STLVertex* pkSrcVertex2;
	SoftwareTransformPatch_SSplatVertex* pkDstVertex2;
	for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
	{
		pkSrcVertex2 = akSrcVertex + uIndex;
		pkDstVertex2 = akDstVertex + uIndex;
		pkDstVertex2->kPosition  = pkSrcVertex2->kPosition;
		pkDstVertex2->dwDiffuse  = pkSrcVertex2->dwDiffuse;
		pkDstVertex2->dwSpecular = pkSrcVertex2->dwFog;
		pkDstVertex2->kTex1      = pkSrcVertex2->kTexTile;
		pkDstVertex2->kTex2      = pkSrcVertex2->kTexAlpha;
	}

	pkVB->Unlock();

	STATEMANAGER.SetStreamSource(0, pkVB, sizeof(SoftwareTransformPatch_SSplatVertex));
	return true;
}

bool CMapOutdoor::__SoftwareTransformPatch_SetShadowStream(SoftwareTransformPatch_STLVertex* akSrcVertex)
{
	IDirect3DVertexBuffer9* pkVB=m_kSTPD.m_pkVBSplat[m_kSTPD.m_dwSplatPos++];
	m_kSTPD.m_dwSplatPos%=SoftwareTransformPatch_SData::SPLAT_VB_NUM;
	if (!pkVB)
		return false;
	
	DWORD dwVBSize=sizeof(SoftwareTransformPatch_SSplatVertex)*CTerrainPatch::TERRAIN_VERTEX_COUNT;
	SoftwareTransformPatch_SSplatVertex* akDstVertex;
	if (FAILED(
		pkVB->Lock(0, dwVBSize, (void**)&akDstVertex, D3DLOCK_DISCARD)
	)) return false;

	SoftwareTransformPatch_STLVertex* pkSrcVertex;
	SoftwareTransformPatch_SSplatVertex* pkDstVertex;
	for (UINT uIndex=0; uIndex!=CTerrainPatch::TERRAIN_VERTEX_COUNT; ++uIndex)
	{
		pkSrcVertex=akSrcVertex+uIndex;
		pkDstVertex=akDstVertex+uIndex;
		pkDstVertex->kPosition=pkSrcVertex->kPosition;
		pkDstVertex->dwDiffuse=pkSrcVertex->dwDiffuse;
		pkDstVertex->dwSpecular=pkSrcVertex->dwFog;
		pkDstVertex->kTex1=pkSrcVertex->kTexStaticShadow;
		pkDstVertex->kTex2=pkSrcVertex->kTexDynamicShadow;
	}	
	pkVB->Unlock();

    STATEMANAGER.SetStreamSource(0, pkVB, sizeof(SoftwareTransformPatch_SSplatVertex));
    return true;
}

void CMapOutdoor::__SoftwareTransformPatch_Initialize()
{
	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::SPLAT_VB_NUM; ++uIndex)
			m_kSTPD.m_pkVBSplat[uIndex]=NULL;	
		m_kSTPD.m_dwSplatPos=0;
	}

	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::NONE_VB_NUM; ++uIndex)
			m_kSTPD.m_pkVBNone[uIndex]=NULL;	
		m_kSTPD.m_dwNonePos=0;
	}
}


bool CMapOutdoor::__SoftwareTransformPatch_Create()
{
	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::SPLAT_VB_NUM; ++uIndex)
		{
			assert(NULL==m_kSTPD.m_pkVBSplat[uIndex]);
			if (FAILED(
				ms_lpd3dDevice->CreateVertexBuffer(
					sizeof(SoftwareTransformPatch_SSplatVertex)*CTerrainPatch::TERRAIN_VERTEX_COUNT,
					D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
					D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX2,
					D3DPOOL_DEFAULT,
					&m_kSTPD.m_pkVBSplat[uIndex],
					nullptr
				)
			)) return false;
		}
	}

	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::NONE_VB_NUM; ++uIndex)
		{
			assert(NULL==m_kSTPD.m_pkVBNone[uIndex]);
			if (FAILED(
				ms_lpd3dDevice->CreateVertexBuffer(
					sizeof(SoftwareTransformPatch_STVertex)*CTerrainPatch::TERRAIN_VERTEX_COUNT,
					D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY,
					D3DFVF_XYZRHW,
					D3DPOOL_DEFAULT,
					&m_kSTPD.m_pkVBNone[uIndex],
					nullptr
				)
			)) return false;
		}
	}
	return true;
}

void CMapOutdoor::__SoftwareTransformPatch_Destroy()
{
	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::SPLAT_VB_NUM; ++uIndex)
		{
			if (m_kSTPD.m_pkVBSplat[uIndex])
				m_kSTPD.m_pkVBSplat[uIndex]->Release();
		}
	}

	{
		for (UINT uIndex=0; uIndex!=SoftwareTransformPatch_SData::NONE_VB_NUM; ++uIndex)
		{
			if (m_kSTPD.m_pkVBNone[uIndex])
				m_kSTPD.m_pkVBNone[uIndex]->Release();
		}
	}
	__SoftwareTransformPatch_Initialize();
}


