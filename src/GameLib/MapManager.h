#pragma once

class CMapBase;
#include "MapOutdoor.h"
#include "PropertyManager.h"

// VICTIM_COLLISION_TEST
#include "PhysicsObject.h"
// VICTIM_COLLISION_TEST_END

// Map Manager
class CMapManager : public CScreen, public IPhysicsWorld
{
	public:
		CMapManager();
		virtual ~CMapManager();

		bool IsMapOutdoor();
		CMapOutdoor& GetMapOutdoorRef();

		bool	IsSoftwareTilingEnable();
		void	ReserveSoftwareTilingEnable(bool isEnable);

		//////////////////////////////////////////////////////////////////////////
		// Contructor / Destructor
		//////////////////////////////////////////////////////////////////////////
		void					Initialize();
		void					Destroy();

		void					Create();		// AllocMap 호출 해서 m_pMap 을 만듬

		virtual void			Clear();
		virtual CMapBase *		AllocMap();

		//////////////////////////////////////////////////////////////////////////
		// Map 관리 함수
		//////////////////////////////////////////////////////////////////////////
		bool					IsMapReady();

		virtual bool			LoadMap(const std::string & c_rstrMapName, float x, float y, float z);
		bool					UnloadMap(const std::string c_strMapName);

		bool					UpdateMap(float fx, float fy, float fz);
		void					UpdateAroundAmbience(float fx, float fy, float fz);
		float					GetHeight(float fx, float fy);
		float					GetTerrainHeight(float fx, float fy);
		bool					GetWaterHeight(int iX, int iY, long * plWaterHeight);
		
		bool					GetNormal(int ix, int iy, D3DXVECTOR3 * pv3Normal);

		//////////////////////////////////////////////////////////////////////////
		// Environment
		///
		// NOTE : 다음 Environment로 서서히 블렌딩 시킨다
		//        아직 세부 구현은 되어있지 않음. 이 함수들은 Protected로 넣고,
		//        MapManager에 TimeControl 부분을 구현하도록 한다. - [levites]
		void					SetEnvironmentDataPtr(const TEnvironmentData * c_pEnvironmentData);
		void					ResetEnvironmentDataPtr(const TEnvironmentData * c_pEnvironmentData);
		void					SetEnvironmentData(int nEnvDataIndex);

		void					BeginEnvironment();
		void					EndEnvironment();

		void					BlendEnvironmentData(const TEnvironmentData * c_pEnvironmentData, int iTransitionTime);

		void					GetCurrentEnvironmentData(const TEnvironmentData ** c_ppEnvironmentData);
		bool					RegisterEnvironmentData(DWORD dwIndex, const char * c_szFileName);
		bool					GetEnvironmentData(DWORD dwIndex, const TEnvironmentData ** c_ppEnvironmentData);

		// Portal
		void					RefreshPortal();
		void					ClearPortal();
		void					AddShowingPortalID(int iID);

		// External interface
		void					LoadProperty();

		DWORD					GetShadowMapColor(float fx, float fy);

		// VICITM_COLLISION_TEST
		virtual bool isPhysicalCollision(const D3DXVECTOR3 & c_rvCheckPosition);
		// VICITM_COLLISION_TEST_END
		
		bool					isAttrOn(float fX, float fY, BYTE byAttr);
		bool					GetAttr(float fX, float fY, BYTE * pbyAttr);
		bool					isAttrOn(int iX, int iY, BYTE byAttr);
		bool					GetAttr(int iX, int iY, BYTE * pbyAttr);

		std::vector<int> &		GetRenderedSplatNum(int * piPatch, int * piSplat, float * pfSplatRatio);
		CArea::TCRCWithNumberVector & GetRenderedGraphicThingInstanceNum(DWORD * pdwGraphicThingInstanceNum, DWORD * pdwCRCNum);

	protected:
		TEnvironmentData *		AllocEnvironmentData();
		void					DeleteEnvironmentData(TEnvironmentData * pEnvironmentData);
		BOOL					LoadEnvironmentData(const char * c_szFileName, TEnvironmentData * pEnvironmentData);

	protected:
		CPropertyManager			m_PropertyManager;

		//////////////////////////////////////////////////////////////////////////
		// Environment
		//////////////////////////////////////////////////////////////////////////
		TEnvironmentDataMap			m_EnvironmentDataMap;
		const TEnvironmentData *	mc_pcurEnvironmentData;

		//////////////////////////////////////////////////////////////////////////
		// Map
		//////////////////////////////////////////////////////////////////////////
		CMapOutdoor *				m_pkMap;
		CSpeedTreeForestDirectX	m_Forest;

	public:
		// 2004.10.14.myevan.TEMP_CAreaLoaderThread
		//bool	BGLoadingEnable();
		//void	BGLoadingEnable(bool bBGLoadingEnable);
		void	SetTerrainRenderSort(CMapOutdoor::ETerrainRenderSort eTerrainRenderSort);
		CMapOutdoor::ETerrainRenderSort	GetTerrainRenderSort();
		
		void	GetBaseXY(DWORD * pdwBaseX, DWORD * pdwBaseY);
		
	public:
		void	SetTransparentTree(bool bTransparenTree);

	public:
		typedef struct 
		{
			std::string	m_strName;
			DWORD		m_dwBaseX;
			DWORD		m_dwBaseY;
			DWORD		m_dwSizeX;
			DWORD		m_dwSizeY;
			DWORD		m_dwEndX;
			DWORD		m_dwEndY;
		} TMapInfo;
		typedef std::vector<TMapInfo>		TMapInfoVector;
		typedef TMapInfoVector::iterator	TMapInfoVectorIterator;

	protected:
		TMapInfoVector			m_kVct_kMapInfo;

		bool m_isSoftwareTilingEnableReserved;

	protected:
		void	__LoadMapInfoVector();

	protected:
		struct FFindMapName
		{
			std::string strNametoFind;
			FFindMapName(const std::string & c_rMapName)
			{
				strNametoFind = c_rMapName;
				stl_lowers(strNametoFind);
			}
			bool operator() (TMapInfo & rMapInfo)
			{
				if (rMapInfo.m_strName == strNametoFind)
					return true;
				return false;
			}
		};
	public:
		void SetAtlasInfoFileName(const char* filename)
		{
			m_stAtlasInfoFileName = filename;
		}
	private:
		std::string m_stAtlasInfoFileName;
};
