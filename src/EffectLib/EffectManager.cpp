#include "StdAfx.h"
#include "../eterBase/Random.h"
#include "../eterlib/StateManager.h"
#include "EffectManager.h"

extern std::unordered_map<LPDIRECT3DTEXTURE9, std::unordered_map<uint32_t, std::vector<TPDTVertex>>> g_particleVertexBatch;

void CEffectManager::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	
	sprintf(szInfo, "Effect: Inst - ED %d, EI %d Pool - PSI %d, MI %d, LI %d, PI %d, EI %d, ED %d, PSD %d, EM %d, LD %d", 		
		m_kEftDataMap.size(),
		m_kEftInstMap.size(),		
		CParticleSystemInstance::ms_kPool.GetCapacity(),
		CEffectMeshInstance::ms_kPool.GetCapacity(),
		CLightInstance::ms_kPool.GetCapacity(),		
		CParticleInstance::ms_kPool.GetCapacity(),
		//CRayParticleInstance::ms_kPool.GetCapacity(),
		CEffectInstance::ms_kPool.GetCapacity(),
		CEffectData::ms_kPool.GetCapacity(),
		CParticleSystemData::ms_kPool.GetCapacity(),
		CEffectMeshScript::ms_kPool.GetCapacity(),
		CLightData::ms_kPool.GetCapacity()
	);
	pstInfo->append(szInfo);
}

void CEffectManager::UpdateSound()
{
	for (TEffectInstanceMap::iterator itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end(); ++itor)
	{
		CEffectInstance * pEffectInstance = itor->second;

		pEffectInstance->UpdateSound();
	}
}

bool CEffectManager::IsAliveEffect(DWORD dwInstanceIndex)
{
	TEffectInstanceMap::iterator f = m_kEftInstMap.find(dwInstanceIndex);
	if (m_kEftInstMap.end()==f)
		return false;

	return f->second->isAlive() ? true : false;
}

void CEffectManager::Update()
{

	// 2004. 3. 1. myevan. 이펙트 모니터링 하는 코드
	/*
	if (GetAsyncKeyState(VK_F9))
	{
		Tracenf("CEffectManager::m_EffectInstancePool %d", m_EffectInstancePool.GetCapacity());
		Tracenf("CEffectManager::m_EffectDataPool %d", m_EffectDataPool.GetCapacity());
		Tracenf("CEffectInstance::ms_LightInstancePool %d", CEffectInstance::ms_LightInstancePool.GetCapacity());
		Tracenf("CEffectInstance::ms_MeshInstancePool %d", CEffectInstance::ms_MeshInstancePool.GetCapacity());
		Tracenf("CEffectInstance::ms_ParticleSystemInstancePool %d", CEffectInstance::ms_ParticleSystemInstancePool.GetCapacity());
		Tracenf("CParticleInstance::ms_ParticleInstancePool %d", CParticleInstance::ms_kPool.GetCapacity());
		Tracenf("CRayParticleInstance::ms_RayParticleInstancePool %d", CRayParticleInstance::ms_kPool.GetCapacity());		
		Tracen("---------------------------------------------");
	}
	*/

	for (TEffectInstanceMap::iterator itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end();)
	{
		CEffectInstance * pEffectInstance = itor->second;

		pEffectInstance->Update(/*fElapsedTime*/);

		if (!pEffectInstance->isAlive())
		{
			itor = m_kEftInstMap.erase(itor);
			
			CEffectInstance::Delete(pEffectInstance);			
		}
		else
		{
			++itor;
		}
	}
}

void CEffectManager::Render()
{
	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

	g_particleVertexBatch.clear();

	__RenderParticles();
	__RenderMeshes();
}

BOOL CEffectManager::RegisterEffect(const char * c_szFileName,bool isExistDelete,bool isNeedCache)
{
	std::string strFileName;
	StringPath(c_szFileName, strFileName);
	DWORD dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length());

	TEffectDataMap::iterator itor = m_kEftDataMap.find(dwCRC);
	if (m_kEftDataMap.end() != itor)
	{
		if (isExistDelete)
		{
			CEffectData* pkEftData=itor->second;
			CEffectData::Delete(pkEftData);			
			m_kEftDataMap.erase(itor);
		}
		else
		{
			//TraceError("CEffectManager::RegisterEffect - m_kEftDataMap.find [%s] Already Exist", c_szFileName);
			return TRUE;
		}
	}

	CEffectData * pkEftData = CEffectData::New();

	if (!pkEftData->LoadScript(c_szFileName))
	{
		TraceError("CEffectManager::RegisterEffect - LoadScript(%s) Error", c_szFileName);
		CEffectData::Delete(pkEftData);
		return FALSE;
	}

	m_kEftDataMap.insert(TEffectDataMap::value_type(dwCRC, pkEftData));

	if (isNeedCache)
	{
		if (m_kEftCacheMap.find(dwCRC)==m_kEftCacheMap.end())
		{
			CEffectInstance* pkNewEftInst=CEffectInstance::New();
			pkNewEftInst->SetEffectDataPointer(pkEftData);
			m_kEftCacheMap.insert(TEffectInstanceMap::value_type(dwCRC, pkNewEftInst));
		}
	}

	return TRUE;
}
// CEffectData 를 포인터형으로 리턴하게 하고..
// CEffectData에서 CRC를 얻을수 있게 한다
BOOL CEffectManager::RegisterEffect2(const char * c_szFileName, DWORD* pdwRetCRC, bool isNeedCache)
{	
	std::string strFileName;
	StringPath(c_szFileName, strFileName);
	DWORD dwCRC = GetCaseCRC32(strFileName.c_str(), strFileName.length());
	*pdwRetCRC=dwCRC;

	return RegisterEffect(c_szFileName,false,isNeedCache);
}

int CEffectManager::CreateEffect(const char * c_szFileName, const D3DXVECTOR3 & c_rv3Position, const D3DXVECTOR3 & c_rv3Rotation)
{
	DWORD dwID = GetCaseCRC32(c_szFileName, strlen(c_szFileName));
	return CreateEffect(dwID, c_rv3Position, c_rv3Rotation);
}

int CEffectManager::CreateEffect(DWORD dwID, const D3DXVECTOR3 & c_rv3Position, const D3DXVECTOR3 & c_rv3Rotation)
{
	int iInstanceIndex = GetEmptyIndex();

	CreateEffectInstance(iInstanceIndex, dwID);
	SelectEffectInstance(iInstanceIndex);
	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat,D3DXToRadian(c_rv3Rotation.x),D3DXToRadian(c_rv3Rotation.y),D3DXToRadian(c_rv3Rotation.z));
	mat._41 = c_rv3Position.x;
	mat._42 = c_rv3Position.y;
	mat._43 = c_rv3Position.z;
	SetEffectInstanceGlobalMatrix(mat);

	return iInstanceIndex;
}

void CEffectManager::CreateEffectInstance(DWORD dwInstanceIndex, DWORD dwID)
{
	if (!dwID)
		return;

	CEffectData * pEffect;
	if (!GetEffectData(dwID, &pEffect))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwID); 
		return;
	}

	CEffectInstance * pEffectInstance = CEffectInstance::New();	
	pEffectInstance->SetEffectDataPointer(pEffect);

	m_kEftInstMap.insert(TEffectInstanceMap::value_type(dwInstanceIndex, pEffectInstance));
}

bool CEffectManager::DestroyEffectInstance(DWORD dwInstanceIndex)
{
	TEffectInstanceMap::iterator itor = m_kEftInstMap.find(dwInstanceIndex);

	if (itor == m_kEftInstMap.end())
		return false;

	CEffectInstance * pEffectInstance = itor->second;

	m_kEftInstMap.erase(itor);

	CEffectInstance::Delete(pEffectInstance);

	return true;
}

void CEffectManager::DeactiveEffectInstance(DWORD dwInstanceIndex)
{
	TEffectInstanceMap::iterator itor = m_kEftInstMap.find(dwInstanceIndex);

	if (itor == m_kEftInstMap.end())
		return;

	CEffectInstance * pEffectInstance = itor->second;
	pEffectInstance->SetDeactive();
}

void CEffectManager::CreateUnsafeEffectInstance(DWORD dwEffectDataID, CEffectInstance ** ppEffectInstance)
{
	CEffectData * pEffect;
	if (!GetEffectData(dwEffectDataID, &pEffect))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwEffectDataID); 
		return;
	}

	CEffectInstance* pkEftInstNew=CEffectInstance::New();
	pkEftInstNew->SetEffectDataPointer(pEffect);

	*ppEffectInstance = pkEftInstNew;	
}

bool CEffectManager::DestroyUnsafeEffectInstance(CEffectInstance * pEffectInstance)
{
	if (!pEffectInstance)
		return false;

	CEffectInstance::Delete(pEffectInstance);
	
	return true;
}

BOOL CEffectManager::SelectEffectInstance(DWORD dwInstanceIndex)
{
	TEffectInstanceMap::iterator itor = m_kEftInstMap.find(dwInstanceIndex);

	m_pSelectedEffectInstance = NULL;

	if (m_kEftInstMap.end() == itor)
		return FALSE;

	m_pSelectedEffectInstance = itor->second;

	return TRUE;
}

void CEffectManager::SetEffectTextures(DWORD dwID, std::vector<std::string> textures)
{
	CEffectData * pEffectData;
	if (!GetEffectData(dwID, &pEffectData))
	{
		Tracef("CEffectManager::CreateEffectInstance - NO DATA :%d\n", dwID); 
		return;
	}

	for(DWORD i = 0; i < textures.size(); i++)
	{
		CParticleSystemData * pParticle = pEffectData->GetParticlePointer(i);
		pParticle->ChangeTexture(textures.at(i).c_str());
	}
}

void CEffectManager::SetEffectInstancePosition(const D3DXVECTOR3 & c_rv3Position)
{
	if (!m_pSelectedEffectInstance)
	{
//		assert(!"Instance to use is not yet set!");
		return;
	}

	m_pSelectedEffectInstance->SetPosition(c_rv3Position);
}

void CEffectManager::SetEffectInstanceRotation(const D3DXVECTOR3 & c_rv3Rotation)
{
	if (!m_pSelectedEffectInstance)
	{
//		assert(!"Instance to use is not yet set!");
		return;
	}

	m_pSelectedEffectInstance->SetRotation(c_rv3Rotation.x,c_rv3Rotation.y,c_rv3Rotation.z);
}

void CEffectManager::SetEffectInstanceGlobalMatrix(const D3DXMATRIX & c_rmatGlobal)
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->SetGlobalMatrix(c_rmatGlobal);
}

void CEffectManager::ShowEffect()
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->Show();
}

void CEffectManager::HideEffect()
{
	if (!m_pSelectedEffectInstance)
		return;

	m_pSelectedEffectInstance->Hide();
}

bool CEffectManager::GetEffectData(DWORD dwID, CEffectData ** ppEffect)
{
	TEffectDataMap::iterator itor = m_kEftDataMap.find(dwID);

	if (itor == m_kEftDataMap.end())
		return false;

	*ppEffect = itor->second;

	return true;
}

bool CEffectManager::GetEffectData(DWORD dwID, const CEffectData ** c_ppEffect)
{
	TEffectDataMap::iterator itor = m_kEftDataMap.find(dwID);

	if (itor == m_kEftDataMap.end())
		return false;

	*c_ppEffect = itor->second;

	return true;
}

DWORD CEffectManager::GetRandomEffect()
{
	int iIndex = random() % m_kEftDataMap.size();

	TEffectDataMap::iterator itor = m_kEftDataMap.begin();
	for (int i = 0; i < iIndex; ++i, ++itor);

	return itor->first;
}

int CEffectManager::GetEmptyIndex()
{
	static int iMaxIndex=1;

	if (iMaxIndex>2100000000)
		iMaxIndex = 1;

	int iNextIndex = iMaxIndex++;
	while(m_kEftInstMap.find(iNextIndex) != m_kEftInstMap.end())
		iNextIndex++;

	return iNextIndex;
}

void CEffectManager::DeleteAllInstances()
{
	__DestroyEffectInstanceMap();
}

void CEffectManager::__DestroyEffectInstanceMap()
{
	for (TEffectInstanceMap::iterator i = m_kEftInstMap.begin(); i != m_kEftInstMap.end(); ++i)
	{
		CEffectInstance * pkEftInst = i->second;	
		CEffectInstance::Delete(pkEftInst);			
	}

	m_kEftInstMap.clear();
}

void CEffectManager::__DestroyEffectCacheMap()
{
	for (TEffectInstanceMap::iterator i = m_kEftCacheMap.begin(); i != m_kEftCacheMap.end(); ++i)
	{
		CEffectInstance * pkEftInst = i->second;	
		CEffectInstance::Delete(pkEftInst);			
	}

	m_kEftCacheMap.clear();
}

void CEffectManager::__DestroyEffectDataMap()
{
	for (TEffectDataMap::iterator i = m_kEftDataMap.begin(); i != m_kEftDataMap.end(); ++i)
	{
		CEffectData * pData = i->second;
		CEffectData::Delete(pData);				
	}

	m_kEftDataMap.clear();
}

void CEffectManager::__RenderParticles()
{
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end(); ++itor) {
		itor->second->BatchParticles();
	}

	for (auto& [pTexture, keyMap] : g_particleVertexBatch) {
		if (keyMap.empty())
			continue;

		STATEMANAGER.SetTexture(0, pTexture);
		for (auto& [opKey, vtxBatch] : keyMap) {
			if (vtxBatch.empty())
				continue;

			uint8_t* pKeyPart = (uint8_t*)&opKey;
			STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, pKeyPart[0]);
			STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, pKeyPart[1]);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, pKeyPart[2]);

			STATEMANAGER.DrawPrimitiveUP(
				D3DPT_TRIANGLELIST,
				vtxBatch.size() / 3,
				vtxBatch.data(),
				sizeof(TPDTVertex));
		}
	}

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);

	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
}

void CEffectManager::__RenderMeshes()
{
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);

	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_NONE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	for (auto itor = m_kEftInstMap.begin(); itor != m_kEftInstMap.end(); ++itor) {
		itor->second->RenderMeshes();
	}

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);

	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
}

void CEffectManager::Destroy()
{	
	__DestroyEffectInstanceMap();
	__DestroyEffectCacheMap();
	__DestroyEffectDataMap();
		
	__Initialize();
}

void CEffectManager::__Initialize()
{
	m_pSelectedEffectInstance = NULL;
	m_isDisableSortRendering = false;
}

CEffectManager::CEffectManager()
{
	__Initialize();
}

CEffectManager::~CEffectManager()
{
	Destroy();
}

// just for map effect

