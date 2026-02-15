#include "StdAfx.h"
#include "EterLib/Camera.h"
#include "PRTerrainLib/StdAfx.h"

#include "MapOutdoor.h"
#include "TerrainPatch.h"
#include "AreaTerrain.h"
#include "TerrainQuadtree.h"
#include "ActorInstance.h"

// 2004.08.17.myevan.std::vector 를 사용할 경우 메모리 접근에 오래걸려 스택쪽으로 계산하도록 수정
class PCBlocker_CDynamicSphereInstanceVector
{
	public:
		typedef CDynamicSphereInstance* Iterator;

		enum
		{
			SIZE = 4,
		};

	public:
		PCBlocker_CDynamicSphereInstanceVector()
		{
		}
		~PCBlocker_CDynamicSphereInstanceVector()
		{
		}

		Iterator Begin()
		{
			return m_aDSI+0;
		}
		Iterator End()
		{
			return m_aDSI+4;
		}

	private:
		CDynamicSphereInstance m_aDSI[4];
};




bool CMapOutdoor::Update(float fX, float fY, float fZ)
{
	D3DXVECTOR3 v3Player(fX, fY, fZ);

	m_v3Player=v3Player;


	DWORD t1=ELTimer_GetMSec();

	int ix, iy;
	PR_FLOAT_TO_INT(fX, ix);
	if ( fY < 0 )
		fY = -fY;
	PR_FLOAT_TO_INT(fY, iy);
	
	short sCoordX = MINMAX(0, ix / CTerrainImpl::TERRAIN_XSIZE, m_sTerrainCountX - 1);
	short sCoordY = MINMAX(0, iy / CTerrainImpl::TERRAIN_YSIZE, m_sTerrainCountY - 1);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=ELTimer_GetMSec();
#endif

	bool bNeedInit = (m_PrevCoordinate.m_sTerrainCoordX == -1 || m_PrevCoordinate.m_sTerrainCoordY == -1);

	if ( bNeedInit ||
		(m_CurCoordinate.m_sTerrainCoordX/LOAD_SIZE_WIDTH) != (sCoordX/LOAD_SIZE_WIDTH) || 
		(m_CurCoordinate.m_sTerrainCoordY/LOAD_SIZE_WIDTH) != (sCoordY/LOAD_SIZE_WIDTH) )
	{
		if (bNeedInit)
		{
			m_PrevCoordinate.m_sTerrainCoordX = sCoordX;
			m_PrevCoordinate.m_sTerrainCoordY = sCoordY;
		}
		else
		{
			m_PrevCoordinate.m_sTerrainCoordX = m_CurCoordinate.m_sTerrainCoordX;
			m_PrevCoordinate.m_sTerrainCoordY = m_CurCoordinate.m_sTerrainCoordY;
		}
		
		m_CurCoordinate.m_sTerrainCoordX = sCoordX;
		m_CurCoordinate.m_sTerrainCoordY = sCoordY;
		m_lCurCoordStartX = sCoordX * CTerrainImpl::TERRAIN_XSIZE;
		m_lCurCoordStartY = sCoordY * CTerrainImpl::TERRAIN_YSIZE;

		WORD wCellCoordX = (ix % CTerrainImpl::TERRAIN_XSIZE) / CTerrainImpl::CELLSCALE;
		WORD wCellCoordY = (iy % CTerrainImpl::TERRAIN_YSIZE) / CTerrainImpl::CELLSCALE;

		short sReferenceCoordMinX, sReferenceCoordMaxX, sReferenceCoordMinY, sReferenceCoordMaxY;
		sReferenceCoordMinX = std::max(m_CurCoordinate.m_sTerrainCoordX - LOAD_SIZE_WIDTH, 0);
		sReferenceCoordMaxX = std::min(m_CurCoordinate.m_sTerrainCoordX + LOAD_SIZE_WIDTH, m_sTerrainCountX - 1);
		sReferenceCoordMinY = std::max(m_CurCoordinate.m_sTerrainCoordY - LOAD_SIZE_WIDTH, 0);
		sReferenceCoordMaxY = std::min(m_CurCoordinate.m_sTerrainCoordY + LOAD_SIZE_WIDTH, m_sTerrainCountY - 1);
		
		for (WORD usY = sReferenceCoordMinY; usY <=sReferenceCoordMaxY; ++usY)
		{
			for (WORD usX = sReferenceCoordMinX; usX <= sReferenceCoordMaxX; ++usX)
			{
				LoadTerrain(usX, usY, wCellCoordX, wCellCoordY);
  				LoadArea(usX, usY, wCellCoordX, wCellCoordY);
			}
		}

		AssignTerrainPtr();
		m_lOldReadX = -1;

		Tracenf("Update::Load spent %d ms\n", ELTimer_GetMSec() - t1);
	}
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3=ELTimer_GetMSec();
#endif
	CSpeedTreeForestDirectX::Instance().UpdateSystem(CTimer::Instance().GetCurrentSecond());
#ifdef __PERFORMANCE_CHECKER__
	DWORD t4=ELTimer_GetMSec();
#endif
	__UpdateGarvage();
#ifdef __PERFORMANCE_CHECKER__
	DWORD t5=ELTimer_GetMSec();
#endif
	UpdateTerrain(fX, fY);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t6=ELTimer_GetMSec();
#endif
	__UpdateArea(v3Player);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t7=ELTimer_GetMSec();
#endif
	UpdateSky();
#ifdef __PERFORMANCE_CHECKER__	
	DWORD t8=ELTimer_GetMSec();
#endif
	__HeightCache_Update();

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp=fopen("perf_outdoor_update.txt", "w");

		if (t8-t1>5)
		{
			fprintf(fp, "OD.Total %d (Time %f)\n", t3-t1, ELTimer_GetMSec()/1000.0f);
			fprintf(fp, "OD.INIT %d\n", t2-t1);
			fprintf(fp, "OD.LOAD %d\n", t3-t2);
			fprintf(fp, "OD.TREE %d\n", t4-t3);
			fprintf(fp, "OD.GVG %d\n", t5-t4);
			fprintf(fp, "OD.TRN %d\n", t6-t5);
			fprintf(fp, "OD.AREA %d\n", t7-t6);
			fprintf(fp, "OD.SKY %d\n", t8-t7);
			fflush(fp);
		}
	}
#endif
	
	return true;
}

void CMapOutdoor::UpdateSky()
{
	m_SkyBox.Update();
}

struct FGetShadowReceiverFromCollisionData
{
	bool m_bCollide;
	std::vector<CGraphicObjectInstance *>* m_pkVct_pkShadowReceiver;
	CDynamicSphereInstance * m_pdsi;
	FGetShadowReceiverFromCollisionData(CDynamicSphereInstance * pdsi, std::vector<CGraphicObjectInstance *>* pkVct_pkShadowReceiver) : m_pdsi(pdsi), m_bCollide(false)
	{
		m_pkVct_pkShadowReceiver=pkVct_pkShadowReceiver;
		m_pkVct_pkShadowReceiver->clear();
	}
	void operator () (CGraphicObjectInstance * pInstance)
	{
		if (!pInstance)
			return;

		if (TREE_OBJECT == pInstance->GetType() || ACTOR_OBJECT == pInstance->GetType() || EFFECT_OBJECT == pInstance->GetType())
			return;
		if (pInstance->CollisionDynamicSphere(*m_pdsi))
		{
			m_pkVct_pkShadowReceiver->push_back(pInstance);
			m_bCollide = true;
		}
	}
};



struct FPCBlockerDistanceSort
{
	D3DXVECTOR3 m_v3Eye;
	FPCBlockerDistanceSort(D3DXVECTOR3 & v3Eye) : m_v3Eye(v3Eye) { }

	bool operator () (CGraphicObjectInstance * plhs, CGraphicObjectInstance * prhs) const
	{
		const auto vv = (plhs->GetPosition() - m_v3Eye);
		const auto vv2 = (prhs->GetPosition() - m_v3Eye);
		return D3DXVec3LengthSq(&vv) > D3DXVec3LengthSq(&vv2);
	}
};

void CMapOutdoor::UpdateAroundAmbience(float fX, float fY, float fZ)
{
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		CArea * pArea;
		if (GetAreaPointer(i, &pArea))
			pArea->UpdateAroundAmbience(fX, fY, fZ);
	}
}

void CMapOutdoor::__UpdateArea(D3DXVECTOR3& v3Player)
{
	__Game_UpdateArea(v3Player);
}

void CMapOutdoor::__Game_UpdateArea(D3DXVECTOR3& v3Player)
{
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=timeGetTime();
#endif
	m_PCBlockerVector.clear();	
	m_ShadowReceiverVector.clear();
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=timeGetTime();
#endif
	CCameraManager& rCmrMgr=CCameraManager::Instance();
	CCamera * pCamera = rCmrMgr.GetCurrentCamera();
	if (!pCamera)
		return;

	float fDistance = pCamera->GetDistance();	

	D3DXVECTOR3 v3View= pCamera->GetView();		
	D3DXVECTOR3 v3Target = pCamera->GetTarget();
	D3DXVECTOR3 v3Eye= pCamera->GetEye();

	D3DXVECTOR3 v3Light = D3DXVECTOR3(1.732f, 1.0f, -3.464f); // 빛의 방향
	v3Light *= 50.0f / D3DXVec3Length(&v3Light);

	/*
	if (v3Target!=v3Player)
	{
		printf("%.2f %.2f %.2f -> target(%.2f %.2f %.2f) player(%.2f %.2f %.2f)\n",
		v3Eye.x, v3Eye.y, v3Eye.z,
		v3Target.x, v3Target.y, v3Target.z,
		v3Player.x, v3Player.y, v3Player.z
	);
	}
	*/
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3=timeGetTime();
#endif
	__CollectShadowReceiver(v3Player, v3Light);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t4=timeGetTime();
#endif
	__CollectCollisionPCBlocker(v3Eye, v3Player, fDistance);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t5=timeGetTime();
#endif
	__CollectCollisionShadowReceiver(v3Player, v3Light);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t6=timeGetTime();
#endif
	__UpdateAroundAreaList();

#ifdef __PERFORMANCE_CHECKER__
	DWORD t7=timeGetTime();
	{
		static FILE* fp=fopen("perf_area_update.txt", "w");

		if (t7-t1>5)
		{
			fprintf(fp, "UA.Total %d (Time %f)\n", t3-t1, ELTimer_GetMSec()/1000.0f);
			fprintf(fp, "UA.Clear %d\n", t2-t1);
			fprintf(fp, "UA.Vector %d\n", t3-t2);
			fprintf(fp, "UA.Shadow %d\n", t4-t3);
			fprintf(fp, "UA.Blocker %d\n", t5-t4);
			fprintf(fp, "UA.ColliShadow %d\n", t6-t5);
			fprintf(fp, "UA.Area %d\n", t7-t6);			
			fflush(fp);
		}
	}
#endif
}

void CMapOutdoor::__UpdateAroundAreaList()
{
#ifdef __PERFORMANCE_CHECKER__
	DWORD ft1=timeGetTime();
#endif
	DWORD at[AROUND_AREA_NUM];
	for (int i = 0; i < AROUND_AREA_NUM; ++i)
	{
		DWORD t1=timeGetTime();
		CArea * pArea;
		if (GetAreaPointer(i, &pArea))
			pArea->Update();
		DWORD t2=timeGetTime();

		at[i]=t2-t1;
	}	
#ifdef __PERFORMANCE_CHECKER__
	DWORD ft2=timeGetTime();
	if (ft2-ft1>5)
	{
		for (int i=0; i<AROUND_AREA_NUM; ++i)
			Tracef("Area %d %d\n", i, at[i]);
	}
#endif
}


struct FGetShadowReceiverFromHeightData
{
	enum
	{
		COLLECT_MAX = 100,
	};
	
	DWORD m_dwCollectOverCount;
	DWORD m_dwCollectCount;
	DWORD m_dwCheckCount;
	bool m_bReceiverFound;
	float m_fFromX, m_fFromY, m_fToX, m_fToY;
	float m_fReturnHeight;

	CGraphicObjectInstance* m_apkShadowReceiver[COLLECT_MAX];
	
	FGetShadowReceiverFromHeightData(float fFromX, float fFromY, float fToX, float fToY) :
	m_fFromX(fFromX), m_fFromY(fFromY), m_fToX(fToX), m_fToY(fToY), m_bReceiverFound(false)
	{
		m_dwCheckCount=0;
		m_dwCollectOverCount=0;
		m_dwCollectCount=0;		
	}

	CGraphicObjectInstance* GetCollectItem(UINT uIndex)
	{
		if (uIndex>=m_dwCollectCount)
			return NULL;

		return m_apkShadowReceiver[uIndex];
	}

	UINT GetCollectCount()
	{
		return m_dwCollectCount;
	}

	void operator () (CGraphicObjectInstance * pInstance)
	{
		m_dwCheckCount++;

		if (!pInstance)
			return;

		if (m_fFromY < 0)
			m_fFromY = -m_fFromY;
		if (m_fToY < 0)
			m_fToY = -m_fToY;
		if (pInstance->GetObjectHeight(m_fFromX, m_fFromY, &m_fReturnHeight) ||
			pInstance->GetObjectHeight(m_fToX, m_fToY, &m_fReturnHeight))
		{
			if (m_dwCollectCount<COLLECT_MAX)
				m_apkShadowReceiver[m_dwCollectCount++]=pInstance;
			else
				m_dwCollectOverCount++;

			m_bReceiverFound = true;
		}
	}
};


void CMapOutdoor::__CollectShadowReceiver(D3DXVECTOR3& v3Target, D3DXVECTOR3& v3Light)
{
	CDynamicSphereInstance s;
	s.v3LastPosition = v3Target + v3Light;
	s.v3Position = s.v3LastPosition + v3Light;
	s.fRadius = 50.0f;

	Vector3d aVector3d;
	aVector3d.Set(v3Target.x, v3Target.y, v3Target.z);

	CCullingManager & rkCullingMgr = CCullingManager::Instance();

#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=ELTimer_GetMSec();
#endif

	FGetShadowReceiverFromHeightData kGetShadowReceiverFromHeightData(v3Target.x, v3Target.y, s.v3Position.x, s.v3Position.y);
	rkCullingMgr.ForInRange(aVector3d, 10.0f, &kGetShadowReceiverFromHeightData);

#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=ELTimer_GetMSec();
#endif

	if (kGetShadowReceiverFromHeightData.m_bReceiverFound)
	{
		for (UINT i=0; i<kGetShadowReceiverFromHeightData.GetCollectCount(); ++i)
		{
			CGraphicObjectInstance * pObjInstEach = kGetShadowReceiverFromHeightData.GetCollectItem(i);
			if (!__IsInShadowReceiverList(pObjInstEach))
				m_ShadowReceiverVector.push_back(pObjInstEach);	
		}
	}

#ifdef __PERFORMANCE_CHECKER__
	static FILE* fp=fopen("perf_shadow_collect.txt", "w");
	DWORD t3=ELTimer_GetMSec();

	if (t3-t1>5)
	{
		fprintf(fp, "SC.Total %d (Time %f)\n", t3-t1, ELTimer_GetMSec()/1000.0f);
		fprintf(fp, "SC.Find %d\n", t2-t1);
		fprintf(fp, "SC.Push %d\n", t3-t2);
		fprintf(fp, "SC.Count (Collect %d, Over %d, Check %d)\n", 
			kGetShadowReceiverFromHeightData.m_dwCollectCount, 
			kGetShadowReceiverFromHeightData.m_dwCollectOverCount,
			kGetShadowReceiverFromHeightData.m_dwCheckCount);
		fflush(fp);
	}
#endif
}


struct PCBlocker_SInstanceList
{
	typedef CGraphicObjectInstance* Item;
	typedef Item* Iterator;

	enum
	{
		CAPACITY = 512,
	};

	DWORD m_dwInstCount;
	DWORD m_dwBlockerCount;
	DWORD m_dwBlockerOverCount;

	Item m_apkPCBlocker[CAPACITY];
	
	PCBlocker_CDynamicSphereInstanceVector* m_pkDSIVector;
	
	CCamera * m_pCamera;
	D3DXVECTOR2 m_v2View;
	D3DXVECTOR2 m_v2Target;
	
	PCBlocker_SInstanceList(PCBlocker_CDynamicSphereInstanceVector* pkDSIVector)
	{		
		m_pCamera = CCameraManager::Instance().GetCurrentCamera();
		if (!m_pCamera)
			return;

		D3DXVECTOR3 m_v3View = m_pCamera->GetView();
		D3DXVECTOR3 m_v3Target = m_pCamera->GetTarget();

		m_v2View.x = m_v3View.x;
		m_v2View.y = m_v3View.y;

		m_v2Target.x = m_v3Target.x;
		m_v2Target.y = m_v3Target.y;

		m_pkDSIVector=pkDSIVector;
		m_dwBlockerCount=0;
		m_dwBlockerOverCount=0;
		m_dwInstCount=0;
	}
	~PCBlocker_SInstanceList()
	{
#ifdef _DEBUG
		__DEBUG_ShowInstanceMaxCount();
#endif
	}
	void __DEBUG_ShowInstanceMaxCount()
	{
		static DWORD s_dwInstMaxCount=0;
		if (s_dwInstMaxCount<m_dwInstCount)
		{
			s_dwInstMaxCount=m_dwInstCount;
			//Tracenf("PCBlocker MaxInstanceCount %d", m_dwInstCount);
		}
	}
	
	Iterator Begin()
	{
		return m_apkPCBlocker;
	}
	Iterator End()
	{
		return m_apkPCBlocker+m_dwBlockerCount;
	}

	DWORD Size()
	{
		return m_dwBlockerCount;
	}

	bool IsEmpty()
	{
		if (m_dwBlockerCount>0)
			return false;

		return true;
	}

	void __AppendPCBlocker(CGraphicObjectInstance * pInstance)
	{
		if (m_dwBlockerCount<CAPACITY)
			m_apkPCBlocker[m_dwBlockerCount++]=pInstance;
		else
			m_dwBlockerOverCount++;
	}

	void __AppendObject(CGraphicObjectInstance * pInstance)
	{
		D3DXVECTOR3 v3Center;
		float fRadius;
		pInstance->GetBoundingSphere(v3Center, fRadius);

		D3DXVECTOR2 v2TargetToCenter;
		v2TargetToCenter.x = v3Center.x - m_v2Target.x;
		v2TargetToCenter.y = v3Center.y - m_v2Target.y;
		if (D3DXVec2Dot(&m_v2View, &v2TargetToCenter) <= 0)
		{
			__AppendPCBlocker(pInstance);
			return;
		}
	}

	void operator () (CGraphicObjectInstance * pInstance)
	{
		if (!m_pCamera)
			return;

		if (!pInstance)
			return;

		++m_dwInstCount;

		PCBlocker_CDynamicSphereInstanceVector::Iterator i;

		for (i=m_pkDSIVector->Begin(); i!=m_pkDSIVector->End(); ++i)
		{
			CDynamicSphereInstance& rkDSI = *i;
			if (pInstance->CollisionDynamicSphere(rkDSI) )
			{
				if (TREE_OBJECT == pInstance->GetType())
				{
					__AppendPCBlocker(pInstance);
					return;
				}
				else if (THING_OBJECT == pInstance->GetType())
				{
					__AppendObject(pInstance);
				}
				else if (ACTOR_OBJECT == pInstance->GetType())
				{
					if (((CActorInstance *)pInstance)->IsBuilding())
					{
						__AppendObject(pInstance);
					}
				}
			}
		}
	}
};


void CMapOutdoor::__CollectCollisionPCBlocker(D3DXVECTOR3& v3Eye, D3DXVECTOR3& v3Target, float fDistance)
{
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=timeGetTime();
#endif

	Vector3d v3dRayStart;
	v3dRayStart.Set(v3Eye.x, v3Eye.y, v3Eye.z);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=timeGetTime();
#endif

	PCBlocker_CDynamicSphereInstanceVector aDynamicSphereInstanceVector;
	{
		CDynamicSphereInstance* pkDSI=aDynamicSphereInstanceVector.Begin();
		pkDSI->fRadius = fDistance * 0.5f;
		pkDSI->v3LastPosition = v3Eye;
		pkDSI->v3Position = v3Eye + 0.5f * (v3Target - v3Eye);
		++pkDSI;

		pkDSI->fRadius = fDistance * 0.5f;
		pkDSI->v3LastPosition = v3Eye + 0.5f * (v3Target - v3Eye);
		pkDSI->v3Position = v3Target;
		++pkDSI;

		pkDSI->fRadius = fDistance * 0.5f;
		pkDSI->v3LastPosition = v3Target;
		pkDSI->v3Position = v3Eye + 0.5f * (v3Target - v3Eye);
		++pkDSI;

		pkDSI->fRadius = fDistance * 0.5f;
		pkDSI->v3LastPosition = v3Eye + 0.5f * (v3Target - v3Eye);
		pkDSI->v3Position = v3Eye;
		++pkDSI;
	}
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3=timeGetTime();	
#endif
	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	PCBlocker_SInstanceList kPCBlockerList(&aDynamicSphereInstanceVector);
	RangeTester<PCBlocker_SInstanceList> kPCBlockerRangeTester(&kPCBlockerList);
	rkCullingMgr.RangeTest(v3dRayStart, fDistance, &kPCBlockerRangeTester);
#ifdef __PERFORMANCE_CHECKER__
 	DWORD t4=timeGetTime();
#endif

	if (!kPCBlockerList.IsEmpty())
	{
		PCBlocker_SInstanceList::Iterator i;

		for (i=kPCBlockerList.Begin(); i!=kPCBlockerList.End(); ++i)
		{
			CGraphicObjectInstance * pObjInstEach = *i;

			if (!pObjInstEach)
				continue;

			if (TREE_OBJECT == pObjInstEach->GetType() && !m_bTransparentTree)
				continue;

			if (!__IsInShadowReceiverList(pObjInstEach))
				if (!__IsInPCBlockerList(pObjInstEach))
					m_PCBlockerVector.push_back(pObjInstEach);
		}
	}
#ifdef __PERFORMANCE_CHECKER__
	DWORD t5=timeGetTime();
#endif
	std::sort(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), FPCBlockerDistanceSort(v3Eye));

#ifdef __PERFORMANCE_CHECKER__
	DWORD t6=timeGetTime();

	static FILE* fp=fopen("perf_pbc_collect.txt", "w");

	if (t3-t1>5)
	{
		fprintf(fp, "PBC.Total %d (Time %f)\n", t3-t1, ELTimer_GetMSec()/1000.0f);
		fprintf(fp, "PBC.INIT %d\n", t2-t1);
		fprintf(fp, "PBC.SET %d\n", t3-t2);
		fprintf(fp, "PBC.CALC %d\n", t4-t2);
		fprintf(fp, "PBC.PUSH %d\n", t5-t2);
		fprintf(fp, "PBC.SORT %d (%d)\n", t6-t2, m_PCBlockerVector.size());
		fprintf(fp, "PBC.Count (Collect %d, Over %d, Check %d)\n", 
			kPCBlockerList.m_dwBlockerCount,
			kPCBlockerList.m_dwBlockerOverCount,
			kPCBlockerList.m_dwInstCount);
		fflush(fp);
	}
#endif
}

void CMapOutdoor::__CollectCollisionShadowReceiver(D3DXVECTOR3& v3Target, D3DXVECTOR3& v3Light)
{
	CDynamicSphereInstance s;
	s.fRadius = 50.0f;
	s.v3LastPosition = v3Target + v3Light;
	s.v3Position = s.v3LastPosition + v3Light;

	Vector3d aVector3d;
	aVector3d.Set(v3Target.x, v3Target.y, v3Target.z);

	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	std::vector<CGraphicObjectInstance *> kVct_pkShadowReceiver;
	FGetShadowReceiverFromCollisionData kGetShadowReceiverFromCollisionData(&s, &kVct_pkShadowReceiver);
	rkCullingMgr.ForInRange(aVector3d, 100.0f, &kGetShadowReceiverFromCollisionData);
	if (!kGetShadowReceiverFromCollisionData.m_bCollide)
		return;
		 
	std::vector<CGraphicObjectInstance * >::iterator i;		
	for ( i = kVct_pkShadowReceiver.begin(); i != kVct_pkShadowReceiver.end(); ++i)
	{
		CGraphicObjectInstance * pObjInstEach = *i;
		if (!__IsInPCBlockerList(pObjInstEach))
			if (!__IsInShadowReceiverList(pObjInstEach))				
				m_ShadowReceiverVector.push_back(pObjInstEach);			
	}	
}

bool CMapOutdoor::__IsInShadowReceiverList(CGraphicObjectInstance* pkObjInstTest)
{
	if (m_ShadowReceiverVector.end() == std::find(m_ShadowReceiverVector.begin(), m_ShadowReceiverVector.end(), pkObjInstTest))
		return false;

	return true;
}

bool CMapOutdoor::__IsInPCBlockerList(CGraphicObjectInstance* pkObjInstTest)
{
	if (m_PCBlockerVector.end() == std::find(m_PCBlockerVector.begin(), m_PCBlockerVector.end(), pkObjInstTest))
		return false;

	return true;
}

// Updates the position of the terrain
void CMapOutdoor::UpdateTerrain(float fX, float fY)
{
	if (fY < 0)
		fY = -fY;

	int sx, sy;
	PR_FLOAT_TO_INT(fX, sx);
	PR_FLOAT_TO_INT(fY, sy);

	long lDivider = (CTerrainImpl::CELLSCALE * TERRAIN_PATCHSIZE);

	m_lCenterX = (sx - m_lCurCoordStartX) / lDivider;
	m_lCenterY = (sy - m_lCurCoordStartY) / lDivider;
	
	if ((m_lCenterX != m_lOldReadX) || (m_lCenterY != m_lOldReadY))
	{
		long lRealCenterX = m_lCenterX * TERRAIN_PATCHSIZE;
		long lRealCenterY = m_lCenterY * TERRAIN_PATCHSIZE;
		m_lOldReadX = m_lCenterX; 
		m_lOldReadY = m_lCenterY;
		
		ConvertTerrainToTnL(lRealCenterX, lRealCenterY);
		UpdateAreaList(lRealCenterX, lRealCenterY);
		//Tracef("사용하는 Area, Terrain : (%d, %d), 지울 Area, Terrain : (%d, %d)\n",
		//	m_AreaVector.size(), m_TerrainVector.size(), m_AreaDeleteVector.size(), m_TerrainDeleteVector.size());		
	}
}

void CMapOutdoor::FPushTerrainToDeleteVector::operator () (CTerrain * pTerrain)
{
	TTerrainPtrVectorIterator aIterator = std::find(m_ReturnTerrainVector.begin(), m_ReturnTerrainVector.end(), pTerrain);
	if (aIterator != m_ReturnTerrainVector.end())
		return;

	WORD wReferenceCoordX = m_CurCoordinate.m_sTerrainCoordX;
	WORD wReferenceCoordY = m_CurCoordinate.m_sTerrainCoordY;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	
	switch(m_eLRDeleteDir)
	{
	case DELETE_LEFT:
		if (wCoordX < wReferenceCoordX - LOAD_SIZE_WIDTH)
			m_ReturnTerrainVector.push_back(pTerrain);
		break;
	case DELETE_RIGHT:
		if (wCoordX > wReferenceCoordX + LOAD_SIZE_WIDTH)
			m_ReturnTerrainVector.push_back(pTerrain);
		break;
	}

	aIterator = std::find(m_ReturnTerrainVector.begin(), m_ReturnTerrainVector.end(), pTerrain);
	if (aIterator != m_ReturnTerrainVector.end())
		return;

	switch(m_eTBDeleteDir)
	{
	case DELETE_TOP:
		if (wCoordY < wReferenceCoordY - LOAD_SIZE_WIDTH)
			m_ReturnTerrainVector.push_back(pTerrain);
		break;
	case DELETE_BOTTOM:
		if (wCoordY > wReferenceCoordY + LOAD_SIZE_WIDTH)
			m_ReturnTerrainVector.push_back(pTerrain);
		break;
	}
}

void CMapOutdoor::FPushAreaToDeleteVector::operator () (CArea * pArea)
{
	TAreaPtrVectorIterator aIterator = std::find(m_ReturnAreaVector.begin(), m_ReturnAreaVector.end(), pArea);
	if (aIterator != m_ReturnAreaVector.end())
		return;

	WORD wReferenceCoordX = m_CurCoordinate.m_sTerrainCoordX;
	WORD wReferenceCoordY = m_CurCoordinate.m_sTerrainCoordY;
	
	WORD wCoordX, wCoordY;
	pArea->GetCoordinate(&wCoordX, &wCoordY);
	
	switch(m_eLRDeleteDir)
	{
	case DELETE_LEFT:
		if (wCoordX < wReferenceCoordX - LOAD_SIZE_WIDTH)
			m_ReturnAreaVector.push_back(pArea);
		break;
	case DELETE_RIGHT:
		if (wCoordX > wReferenceCoordX + LOAD_SIZE_WIDTH)
			m_ReturnAreaVector.push_back(pArea);
		break;
	}
	
	aIterator = std::find(m_ReturnAreaVector.begin(), m_ReturnAreaVector.end(), pArea);
	if (aIterator != m_ReturnAreaVector.end())
		return;

	switch(m_eTBDeleteDir)
	{
	case DELETE_TOP:
		if (wCoordY < wReferenceCoordY - LOAD_SIZE_WIDTH)
			m_ReturnAreaVector.push_back(pArea);
		break;
	case DELETE_BOTTOM:
		if (wCoordY > wReferenceCoordY + LOAD_SIZE_WIDTH)
			m_ReturnAreaVector.push_back(pArea);
		break;
	}
}

void CMapOutdoor::__ClearGarvage()
{
	std::for_each(m_TerrainDeleteVector.begin(), m_TerrainDeleteVector.end(), CTerrain::Delete);
	m_TerrainDeleteVector.clear();

	std::for_each(m_AreaDeleteVector.begin(), m_AreaDeleteVector.end(), CArea::Delete);	
	m_AreaDeleteVector.clear();
}

void CMapOutdoor::__UpdateGarvage()
{
	// Early exit if no garbage to collect - saves CPU cycles
	if (m_TerrainDeleteVector.empty() && m_AreaDeleteVector.empty())
		return;

	const DWORD dwTerrainEraseInterval = 1000 * 60;
	static DWORD dwEraseTime = ELTimer_GetMSec();
	
	if (!m_TerrainDeleteVector.empty())
	{
		if (ELTimer_GetMSec() - dwEraseTime <= dwTerrainEraseInterval)
			return;
		TTerrainPtrVectorIterator aTerrainPtrDeleteItertor = m_TerrainDeleteVector.begin();
		CTerrain * pTerrain = *aTerrainPtrDeleteItertor;
		CTerrain::Delete(pTerrain);
 		
		aTerrainPtrDeleteItertor = m_TerrainDeleteVector.erase(aTerrainPtrDeleteItertor);
		dwEraseTime = ELTimer_GetMSec();
		Trace("Delete Terrain \n");
		return;
	}

	if (!m_AreaDeleteVector.empty())
	{
		if (ELTimer_GetMSec() - dwEraseTime <= dwTerrainEraseInterval)
			return;
		TAreaPtrVectorIterator aAreaPtrDeleteItertor = m_AreaDeleteVector.begin();

		CArea * pArea = *aAreaPtrDeleteItertor;
		CArea::Delete(pArea);
		
		aAreaPtrDeleteItertor = m_AreaDeleteVector.erase(aAreaPtrDeleteItertor);
		dwEraseTime = ELTimer_GetMSec();
		Trace("Delete Area \n");
		return;
	}
}

void CMapOutdoor::UpdateAreaList(long lCenterX, long lCenterY)
{
	if (m_TerrainVector.size() <= AROUND_AREA_NUM && m_AreaVector.size() <= AROUND_AREA_NUM)
		return;

	__ClearGarvage();
	
	FPushToDeleteVector::EDeleteDir eDeleteLRDir, eDeleteTBDir;

	if (lCenterX > CTerrainImpl::XSIZE / 2)
		eDeleteLRDir = FPushToDeleteVector::DELETE_LEFT;
	else
		eDeleteLRDir = FPushToDeleteVector::DELETE_RIGHT;
	if (lCenterY > CTerrainImpl::YSIZE / 2)
		eDeleteTBDir = FPushToDeleteVector::DELETE_TOP;
	else
		eDeleteTBDir = FPushToDeleteVector::DELETE_BOTTOM;

	FPushTerrainToDeleteVector rPushTerrainToDeleteVector = std::for_each(m_TerrainVector.begin(), m_TerrainVector.end(),
		FPushTerrainToDeleteVector(eDeleteLRDir, eDeleteTBDir, m_CurCoordinate));
	FPushAreaToDeleteVector rPushAreaToDeleteVector = std::for_each(m_AreaVector.begin(), m_AreaVector.end(),
		FPushAreaToDeleteVector(eDeleteLRDir, eDeleteTBDir, m_CurCoordinate));

	if (!rPushTerrainToDeleteVector.m_ReturnTerrainVector.empty())
	{
		m_TerrainDeleteVector.resize(rPushTerrainToDeleteVector.m_ReturnTerrainVector.size());
		std::copy(rPushTerrainToDeleteVector.m_ReturnTerrainVector.begin(), rPushTerrainToDeleteVector.m_ReturnTerrainVector.end(), m_TerrainDeleteVector.begin());

		for (DWORD dwIndex = 0; dwIndex < rPushTerrainToDeleteVector.m_ReturnTerrainVector.size(); ++dwIndex)
		{
			bool isDel=false;
			TTerrainPtrVectorIterator aTerrainPtrItertor = m_TerrainVector.begin();
			while(aTerrainPtrItertor != m_TerrainVector.end())
			{
				CTerrain * pTerrain = *aTerrainPtrItertor;
				if (pTerrain == rPushTerrainToDeleteVector.m_ReturnTerrainVector[dwIndex])
				{
					aTerrainPtrItertor = m_TerrainVector.erase(aTerrainPtrItertor);
					isDel=true;
				}
				else
					++aTerrainPtrItertor;
			}
		}
	}
	if (!rPushAreaToDeleteVector.m_ReturnAreaVector.empty())
	{
		m_AreaDeleteVector.resize(rPushAreaToDeleteVector.m_ReturnAreaVector.size());
		std::copy(rPushAreaToDeleteVector.m_ReturnAreaVector.begin(), rPushAreaToDeleteVector.m_ReturnAreaVector.end(), m_AreaDeleteVector.begin());
		
		for (DWORD dwIndex = 0; dwIndex < rPushAreaToDeleteVector.m_ReturnAreaVector.size(); ++dwIndex)
		{
			TAreaPtrVectorIterator aAreaPtrItertor = m_AreaVector.begin();
			while(aAreaPtrItertor != m_AreaVector.end())
			{
				CArea * pArea = *aAreaPtrItertor;
				if (pArea == rPushAreaToDeleteVector.m_ReturnAreaVector[dwIndex])
					aAreaPtrItertor = m_AreaVector.erase(aAreaPtrItertor);
				else
					++aAreaPtrItertor;
			}
		}
	}
}

void CMapOutdoor::ConvertTerrainToTnL(long lx, long ly)
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::ConvertTerrainToTnL");
	
	for (long i = 0; i < m_wPatchCount * m_wPatchCount; i++)
		m_pTerrainPatchProxyList[i].SetUsed(false);

	lx -= m_lViewRadius;          /* Move to the top left corner of the */
	ly -= m_lViewRadius;          /* input rectangle */

	long diameter = m_lViewRadius * 2;
	
	long x0 = lx / TERRAIN_PATCHSIZE;
	long y0 = ly / TERRAIN_PATCHSIZE;
	long x1 = ( lx + diameter - 1 ) / TERRAIN_PATCHSIZE;
	long y1 = ( ly + diameter - 1 ) / TERRAIN_PATCHSIZE;

	long xw = x1 - x0 + 1;     /* Figure out how many patches are needed */
	long yw = y1 - y0 + 1;
	
	long ex = lx + diameter;
	long ey = ly + diameter;
	
	y0 = ly;
	for (long yp = 0; yp < yw; yp++)
    {
		x0 = lx;
		y1 = (y0 / TERRAIN_PATCHSIZE + 1) * TERRAIN_PATCHSIZE;
		if (y1 > ey)
			y1 = ey;
		for (long xp = 0; xp < xw; xp++)
		{
			x1 = (x0 / TERRAIN_PATCHSIZE + 1) * TERRAIN_PATCHSIZE;
			if (x1 > ex)
				x1 = ex;
 			AssignPatch(yp * m_wPatchCount + xp, x0, y0, x1, y1);
			x0 = x1;
		}
		y0 = y1;
    }
	UpdateQuadTreeHeights(m_pRootNode);
}

void CMapOutdoor::AssignPatch(long lPatchNum, long x0, long y0, long x1, long y1)
{
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::AssignPatch");
	
	CTerrainPatchProxy * pTerrainPatchProxy = &m_pTerrainPatchProxyList[lPatchNum];
	
	if (y0 < 0 && y1 <= 0)
	{
		if (x0 < 0 && x1 <= 0)
		{
			pTerrainPatchProxy->SetTerrainNum(0);
			x0 += CTerrainImpl::XSIZE;
			x1 += CTerrainImpl::XSIZE;
		}
		else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
		{
			pTerrainPatchProxy->SetTerrainNum(2);
			x0 -= CTerrainImpl::XSIZE;
			x1 -= CTerrainImpl::XSIZE;
		}
		else
			pTerrainPatchProxy->SetTerrainNum(1);
		
		y0 += CTerrainImpl::YSIZE;
		y1 += CTerrainImpl::YSIZE;
	}
	else if (y0 >= CTerrainImpl::YSIZE && y1 > CTerrainImpl::YSIZE)
	{
		if (x0 < 0 && x1 <= 0)
		{
			pTerrainPatchProxy->SetTerrainNum(6);
			x0 += CTerrainImpl::XSIZE;
			x1 += CTerrainImpl::XSIZE;
		}
		else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
		{
			pTerrainPatchProxy->SetTerrainNum(8);
			x0 -= CTerrainImpl::XSIZE;
			x1 -= CTerrainImpl::XSIZE;
		}
		else
			pTerrainPatchProxy->SetTerrainNum(7);
		
		y0 -= CTerrainImpl::YSIZE;
		y1 -= CTerrainImpl::YSIZE;
	}
	else
	{
		if (x0 < 0 && x1 <= 0)
		{
			pTerrainPatchProxy->SetTerrainNum(3);
			x0 += CTerrainImpl::XSIZE;
			x1 += CTerrainImpl::XSIZE;
		}
		else if (x0 >= CTerrainImpl::XSIZE && x1 > CTerrainImpl::XSIZE)
		{
			pTerrainPatchProxy->SetTerrainNum(5);
			x0 -= CTerrainImpl::XSIZE;
			x1 -= CTerrainImpl::XSIZE;
		}
		else
			pTerrainPatchProxy->SetTerrainNum(4);
	}

	CTerrain * pTerrain;
	if (!GetTerrainPointer(pTerrainPatchProxy->GetTerrainNum(), &pTerrain))
		return;

	BYTE byPatchNumX, byPatchNumY;
	byPatchNumX = x0 / CTerrainImpl::PATCH_XSIZE;
	byPatchNumY = y0 / CTerrainImpl::PATCH_YSIZE;
	
	CTerrainPatch * pTerrainPatch = pTerrain->GetTerrainPatchPtr(byPatchNumX, byPatchNumY);
	if (!pTerrainPatch)
		return;

	pTerrainPatchProxy->SetPatchNum(byPatchNumY * CTerrainImpl::PATCH_XCOUNT + byPatchNumX);
	pTerrainPatchProxy->SetTerrainPatch(pTerrainPatch);
	pTerrainPatchProxy->SetUsed(true);
}

void CMapOutdoor::UpdateQuadTreeHeights(CTerrainQuadtreeNode *Node)
{
	// Inserted by levites
	assert(NULL!=m_pTerrainPatchProxyList && "CMapOutdoor::UpdateQuadTreeHeights");
	if (!m_pTerrainPatchProxyList)
		return;
	
	float minx, maxx, miny, maxy, minz, maxz;
	minx = maxx = miny = maxy = minz = maxz = 0;

	if (m_pTerrainPatchProxyList[Node->PatchNum].isUsed())
	{
		minx = m_pTerrainPatchProxyList[Node->PatchNum].GetMinX();
		maxx = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxX();
		miny = m_pTerrainPatchProxyList[Node->PatchNum].GetMinY();
		maxy = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxY();
		minz = m_pTerrainPatchProxyList[Node->PatchNum].GetMinZ();
		maxz = m_pTerrainPatchProxyList[Node->PatchNum].GetMaxZ();
	}
	
	for (long y = Node->y0; y <= Node->y1; y++)
	{
		for (long x = Node->x0; x <= Node->x1; x++)
		{
			long patch = y * m_wPatchCount + x;

			if (!m_pTerrainPatchProxyList[patch].isUsed())
				continue;
			
			if (m_pTerrainPatchProxyList[patch].GetMinX() < minx)
				minx = m_pTerrainPatchProxyList[patch].GetMinX();
			if (m_pTerrainPatchProxyList[patch].GetMaxX() > maxx)
				maxx = m_pTerrainPatchProxyList[patch].GetMaxX();
			
			if (m_pTerrainPatchProxyList[patch].GetMinY() < miny)
				miny = m_pTerrainPatchProxyList[patch].GetMinY();
			if (m_pTerrainPatchProxyList[patch].GetMaxY() > maxy)
				maxy = m_pTerrainPatchProxyList[patch].GetMaxY();
			
			if (m_pTerrainPatchProxyList[patch].GetMinZ() < minz)
				minz = m_pTerrainPatchProxyList[patch].GetMinZ();
			if (m_pTerrainPatchProxyList[patch].GetMaxZ() > maxz)
				maxz = m_pTerrainPatchProxyList[patch].GetMaxZ();
		}      
	}
	
	Node->center.x = (maxx + minx) * 0.5f;
	Node->center.y = (maxy + miny) * 0.5f;
	Node->center.z = (maxz + minz) * 0.5f;
	
	Node->radius = sqrtf((maxx-minx)*(maxx-minx) +
		(maxy-miny)*(maxy-miny) +
		(maxz-minz)*(maxz-minz)) / 2.0f;
	
	if (Node->NW_Node != NULL)
		UpdateQuadTreeHeights(Node->NW_Node);
	
	if (Node->NE_Node != NULL)
		UpdateQuadTreeHeights(Node->NE_Node);
	
	if (Node->SW_Node != NULL)
		UpdateQuadTreeHeights(Node->SW_Node);
	
	if (Node->SE_Node != NULL)
		UpdateQuadTreeHeights(Node->SE_Node);
}
