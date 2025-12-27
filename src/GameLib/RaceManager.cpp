#include "StdAfx.h"
#include "RaceManager.h"
#include "RaceMotionData.h"
#include "PackLib/PackManager.h"

bool CRaceManager::s_bPreloaded = false;

bool __IsGuildRace(unsigned race)
{
	if (race >= 14000 && race < 15000)
		return true;

	if (20043 == race)
		return true;

	return false;
}

bool __IsNPCRace(unsigned race)
{
	if (race > 9000)
		return true;

	return false;
}

void __GetRaceResourcePathes(unsigned race, std::vector <std::string>& vec_stPathes)
{
	if (__IsGuildRace(race))
	{
		vec_stPathes.push_back ("d:/ymir work/guild/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/monster2/");
	}
	else if (__IsNPCRace(race))
	{
		if (race >= 30000)
		{
			vec_stPathes.push_back ("d:/ymir work/npc2/");
			vec_stPathes.push_back ("d:/ymir work/npc/");
			vec_stPathes.push_back ("d:/ymir work/monster/");
			vec_stPathes.push_back ("d:/ymir work/monster2/");
			vec_stPathes.push_back ("d:/ymir work/guild/");
		}
		else
		{
			vec_stPathes.push_back ("d:/ymir work/npc/");
			vec_stPathes.push_back ("d:/ymir work/npc2/");
			vec_stPathes.push_back ("d:/ymir work/monster/");
			vec_stPathes.push_back ("d:/ymir work/monster2/");
			vec_stPathes.push_back ("d:/ymir work/guild/");
		}
	}
	// 만우절 이벤트용 예외 몬스터
	else if (8507 == race || 8510 == race)
	{
		vec_stPathes.push_back ("d:/ymir work/monster2/");
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/guild/");
	}
	else if (race > 8000)
	{
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/monster2/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/guild/");
	}
	else if (race > 2000)
	{
		vec_stPathes.push_back ("d:/ymir work/monster2/");
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/guild/");
	}
	else if (race>=1400 && race<=1700)
	{
		vec_stPathes.push_back ("d:/ymir work/monster2/");
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/guild/");
	}
	else
	{
		vec_stPathes.push_back ("d:/ymir work/monster/");
		vec_stPathes.push_back ("d:/ymir work/monster2/");
		vec_stPathes.push_back ("d:/ymir work/npc/");
		vec_stPathes.push_back ("d:/ymir work/npc2/");
		vec_stPathes.push_back ("d:/ymir work/guild/");
	}
	return;
}

CRaceData* CRaceManager::__LoadRaceData(DWORD dwRaceIndex)
{
	std::map<DWORD, std::string>::iterator fRaceName=m_kMap_dwRaceKey_stRaceName.find(dwRaceIndex);
	if (m_kMap_dwRaceKey_stRaceName.end()==fRaceName)
		return NULL;

	const std::string& c_rstRaceName=fRaceName->second;

	if (c_rstRaceName.empty())
		return NULL;
	
	// LOAD_LOCAL_RESOURCE
	if (c_rstRaceName[0] == '#')
	{	
		const char* pathName = c_rstRaceName.c_str() + 1;
		char shapeFileName[256];
		char motionListFileName[256];
		_snprintf(shapeFileName, sizeof(shapeFileName), "%sshape.msm", pathName);
		_snprintf(motionListFileName, sizeof(motionListFileName), "%smotlist.txt", pathName);
				
		CRaceData * pRaceData = CRaceData::New();
		pRaceData->SetRace(dwRaceIndex);
		if (!pRaceData->LoadRaceData(shapeFileName))
		{
			TraceError("CRaceManager::RegisterRacePath(race=%u).LoadRaceData(%s)", dwRaceIndex, shapeFileName);
			CRaceData::Delete(pRaceData);
			return NULL;
		}

		__LoadRaceMotionList(*pRaceData, pathName, motionListFileName);		

		return pRaceData;
	}
	// END_OF_LOAD_LOCAL_RESOURCE
	std::vector <std::string> vec_stFullPathName;
	__GetRaceResourcePathes(dwRaceIndex, vec_stFullPathName);

	CRaceData * pRaceData = CRaceData::New();
	pRaceData->SetRace(dwRaceIndex);
	
	for (int i = 0; i < vec_stFullPathName.size(); i++)
	{
		std::string stFullPathName = vec_stFullPathName[i];
		{
			std::map<std::string, std::string>::iterator fRaceSrcName=m_kMap_stRaceName_stSrcName.find(c_rstRaceName);
			if (m_kMap_stRaceName_stSrcName.end()==fRaceSrcName)
				stFullPathName+=c_rstRaceName;
			else
				stFullPathName+=fRaceSrcName->second;
		}

		stFullPathName+="/";

		std::string stMSMFileName=stFullPathName;
		stMSMFileName+=c_rstRaceName;
		stMSMFileName+=".msm";

		if (!pRaceData->LoadRaceData(stMSMFileName.c_str()))
		{
			if (i != vec_stFullPathName.size() - 1)
			{
				TraceError("CRaceManager::RegisterRacePath : RACE[%u] LOAD MSMFILE[%s] ERROR. Will Find Another Path.", dwRaceIndex, stMSMFileName.c_str());
				continue;
			}
			
			TraceError("CRaceManager::RegisterRacePath : RACE[%u] LOAD MSMFILE[%s] ERROR", dwRaceIndex, stMSMFileName.c_str());
			CRaceData::Delete(pRaceData);
			return NULL;
		}

		std::string stMotionListFileName=stFullPathName;
		stMotionListFileName+=pRaceData->GetMotionListFileName();

		__LoadRaceMotionList(*pRaceData, stFullPathName.c_str(), stMotionListFileName.c_str());		

		return pRaceData;
	}
	TraceError("CRaceManager::RegisterRacePath : RACE[%u] HAVE NO PATH ERROR", dwRaceIndex);
	CRaceData::Delete(pRaceData);
	return NULL;
}

bool CRaceManager::__LoadRaceMotionList(CRaceData& rkRaceData, const char* pathName, const char* motionListFileName)
{
	static std::map<std::string, DWORD> s_kMap_stType_dwIndex;
	static bool s_isInit=false;

	if (!s_isInit)
	{
		s_isInit=true;

		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPAWN", CRaceMotionData::NAME_SPAWN));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WAIT", CRaceMotionData::NAME_WAIT));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WAIT1", CRaceMotionData::NAME_WAIT));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WAIT2", CRaceMotionData::NAME_WAIT));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WALK", CRaceMotionData::NAME_WALK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WALK1", CRaceMotionData::NAME_WALK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("WALK2", CRaceMotionData::NAME_WALK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("RUN", CRaceMotionData::NAME_RUN));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("RUN1", CRaceMotionData::NAME_RUN));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("RUN2", CRaceMotionData::NAME_RUN));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("STOP", CRaceMotionData::NAME_STOP));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("DEAD", CRaceMotionData::NAME_DEAD));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("COMBO_ATTACK", CRaceMotionData::NAME_COMBO_ATTACK_1));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("COMBO_ATTACK1", CRaceMotionData::NAME_COMBO_ATTACK_2));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("COMBO_ATTACK2", CRaceMotionData::NAME_COMBO_ATTACK_3));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("NORMAL_ATTACK", CRaceMotionData::NAME_NORMAL_ATTACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("NORMAL_ATTACK1", CRaceMotionData::NAME_NORMAL_ATTACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("NORMAL_ATTACK2", CRaceMotionData::NAME_NORMAL_ATTACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DAMAGE", CRaceMotionData::NAME_DAMAGE));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DAMAGE1", CRaceMotionData::NAME_DAMAGE));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DAMAGE2", CRaceMotionData::NAME_DAMAGE));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DAMAGE3", CRaceMotionData::NAME_DAMAGE));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DEAD", CRaceMotionData::NAME_DEAD));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DEAD1", CRaceMotionData::NAME_DEAD));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_DEAD2", CRaceMotionData::NAME_DEAD));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_KNOCKDOWN", CRaceMotionData::NAME_DAMAGE_FLYING));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_KNOCKDOWN1", CRaceMotionData::NAME_DAMAGE_FLYING));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_STANDUP", CRaceMotionData::NAME_STAND_UP));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("FRONT_STANDUP1", CRaceMotionData::NAME_STAND_UP));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_DAMAGE", CRaceMotionData::NAME_DAMAGE_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_DAMAGE1", CRaceMotionData::NAME_DAMAGE_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_DEAD", CRaceMotionData::NAME_DEAD_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_DEAD1", CRaceMotionData::NAME_DEAD_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_DEAD2", CRaceMotionData::NAME_DEAD_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_KNOCKDOWN", CRaceMotionData::NAME_DAMAGE_FLYING_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_KNOCKDOWN1", CRaceMotionData::NAME_DAMAGE_FLYING_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_STANDUP", CRaceMotionData::NAME_STAND_UP_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("BACK_STANDUP1", CRaceMotionData::NAME_STAND_UP_BACK));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL", CRaceMotionData::NAME_SPECIAL_1));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL1", CRaceMotionData::NAME_SPECIAL_2));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL2", CRaceMotionData::NAME_SPECIAL_3));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL3", CRaceMotionData::NAME_SPECIAL_4));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL4", CRaceMotionData::NAME_SPECIAL_5));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SPECIAL5", CRaceMotionData::NAME_SPECIAL_6));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SKILL1", CRaceMotionData::NAME_SKILL+121));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SKILL2", CRaceMotionData::NAME_SKILL+122));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SKILL3", CRaceMotionData::NAME_SKILL+123));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SKILL4", CRaceMotionData::NAME_SKILL+124));
		s_kMap_stType_dwIndex.insert(std::map<std::string, DWORD>::value_type("SKILL5", CRaceMotionData::NAME_SKILL+125));
	}
	
	TPackFile kMappedFile;
	if (!CPackManager::Instance().GetFile(motionListFileName, kMappedFile))
		return false;
	

	CMemoryTextFileLoader kTextFileLoader;
	kTextFileLoader.Bind(kMappedFile.size(), kMappedFile.data());

	rkRaceData.RegisterMotionMode(CRaceMotionData::MODE_GENERAL);

	char szMode[256];
	char szType[256];
	char szFile[256];
	int nPercent = 0;

	bool isSpawn=false;

	static std::string stSpawnMotionFileName;
	static std::string stMotionFileName;

	stSpawnMotionFileName = "";
	stMotionFileName = "";

	UINT uLineCount=kTextFileLoader.GetLineCount();
	for (UINT uLineIndex=0; uLineIndex<uLineCount; ++uLineIndex)
	{
		DWORD motionType = CRaceMotionData::NAME_NONE;

		const std::string& c_rstLine=kTextFileLoader.GetLineString(uLineIndex);
		sscanf(c_rstLine.c_str(), "%s %s %s %d", szMode, szType, szFile, &nPercent);

		std::map<std::string, DWORD>::iterator fTypeIndex=s_kMap_stType_dwIndex.find(szType);

		if (s_kMap_stType_dwIndex.end() == fTypeIndex)
		{
			// 모션 목록에 WAIT, WAIT4, WAIT20  이런 식으로 등록되어 있을 때,
			// WAIT4, WAIT20을 WAIT로 인식할 수 있도록 처리
			const size_t c_cutLengthLimit = 2;
			bool bFound = false;

			if (c_cutLengthLimit < strlen(szType) + 1)
			{
				for (size_t i = 1; i <= c_cutLengthLimit; ++i)
				{
					std::string typeName = std::string(szType).substr(0, strlen(szType) - i);
					fTypeIndex = s_kMap_stType_dwIndex.find(typeName);

					if (s_kMap_stType_dwIndex.end() != fTypeIndex)
					{
						bFound = true;
						break;
					}
				}
			}

			if (false == bFound)
				continue;
		}

		motionType = fTypeIndex->second;

		stMotionFileName = pathName;
		stMotionFileName += szFile; 

		rkRaceData.RegisterMotionData(CRaceMotionData::MODE_GENERAL, motionType, stMotionFileName.c_str(), nPercent);

		switch (motionType)
		{
			case CRaceMotionData::NAME_SPAWN:
				isSpawn=true;
				break;
			case CRaceMotionData::NAME_DAMAGE:
				stSpawnMotionFileName=stMotionFileName;
				break;
		}
	}

	if (!isSpawn && stSpawnMotionFileName!="")
	{
		rkRaceData.RegisterMotionData(CRaceMotionData::MODE_GENERAL, CRaceMotionData::NAME_SPAWN, stSpawnMotionFileName.c_str(), nPercent);
	}

	rkRaceData.RegisterNormalAttack(CRaceMotionData::MODE_GENERAL, CRaceMotionData::NAME_NORMAL_ATTACK);

	return true;
}

void CRaceManager::RegisterRaceSrcName(const char * c_szName, const char * c_szSrcName)
{
	m_kMap_stRaceName_stSrcName.insert(std::map<std::string, std::string>::value_type(c_szName, c_szSrcName));
}

void CRaceManager::RegisterRaceName(DWORD dwRaceIndex, const char * c_szName)
{	
	m_kMap_dwRaceKey_stRaceName.insert(std::map<DWORD, std::string>::value_type(dwRaceIndex, c_szName));
}

void CRaceManager::CreateRace(DWORD dwRaceIndex)
{
	if (m_RaceDataMap.end() != m_RaceDataMap.find(dwRaceIndex))
	{
		TraceError("RaceManager::CreateRace : Race %u already created", dwRaceIndex);
		return;
	}

	CRaceData * pRaceData = CRaceData::New();
	pRaceData->SetRace(dwRaceIndex);
	m_RaceDataMap.insert(TRaceDataMap::value_type(dwRaceIndex, pRaceData));

	Tracenf("CRaceManager::CreateRace(dwRaceIndex=%d)", dwRaceIndex);
}

void CRaceManager::SelectRace(DWORD dwRaceIndex)
{
	TRaceDataIterator itor = m_RaceDataMap.find(dwRaceIndex);
	if (m_RaceDataMap.end() == itor)
	{
		assert(!"Failed to select race data!");
		return;
	}

	m_pSelectedRaceData = itor->second;
}

CRaceData * CRaceManager::GetSelectedRaceDataPointer()
{
	return m_pSelectedRaceData;
}

BOOL CRaceManager::GetRaceDataPointer(DWORD dwRaceIndex, CRaceData ** ppRaceData)
{
	TRaceDataIterator itor = m_RaceDataMap.find(dwRaceIndex);

	if (m_RaceDataMap.end() == itor)
	{
		CRaceData* pRaceData = __LoadRaceData(dwRaceIndex);

		if (pRaceData)
		{
			m_RaceDataMap.insert(TRaceDataMap::value_type(dwRaceIndex, pRaceData));
			*ppRaceData = pRaceData;
			return TRUE;
		}

		TraceError("CRaceManager::GetRaceDataPointer: cannot load data by dwRaceIndex %lu", dwRaceIndex);
		return FALSE;
	}

	*ppRaceData = itor->second;
	return TRUE;
}

void CRaceManager::SetPathName(const char * c_szPathName)
{
	m_strPathName = c_szPathName;
}

const char * CRaceManager::GetFullPathFileName(const char * c_szFileName)
{
	static std::string s_stFileName;
	
	if (c_szFileName[0] != '.')
	{
		s_stFileName = m_strPathName;
		s_stFileName += c_szFileName;
	}
	else
	{
		s_stFileName = c_szFileName;
	}

	return s_stFileName.c_str();
}


void CRaceManager::Create()
{
	CRaceMotionData::CreateSystem(2048);
	CRaceData::CreateSystem(256, 512);
}

void CRaceManager::__Initialize()
{
	m_pSelectedRaceData = NULL;
}

void CRaceManager::__DestroyRaceDataMap()
{
	TRaceDataMap::iterator i;
	for (i=m_RaceDataMap.begin(); i!=m_RaceDataMap.end(); ++i)
		CRaceData::Delete(i->second);

	m_RaceDataMap.clear();
}

void CRaceManager::Destroy()
{
	__DestroyRaceDataMap();

	__Initialize();
}

CRaceManager::CRaceManager()
{
	__Initialize();	
}

CRaceManager::~CRaceManager()
{
	Destroy();
}

void CRaceManager::PreloadPlayerRaceMotions()
{
	if (s_bPreloaded)
		return;

	// Preload all player races (0-7)
	for (DWORD dwRace = 0; dwRace <= 7; ++dwRace)
	{
		CRaceData* pRaceData = NULL;
		if (!Instance().GetRaceDataPointer(dwRace, &pRaceData))
			continue;

		CRaceData::TMotionModeDataIterator itor;

		if (pRaceData->CreateMotionModeIterator(itor))
		{
			do
			{
				CRaceData::TMotionModeData* pMotionModeData = itor->second;

				CRaceData::TMotionVectorMap::iterator itorMotion = pMotionModeData->MotionVectorMap.begin();
				for (; itorMotion != pMotionModeData->MotionVectorMap.end(); ++itorMotion)
				{
					const CRaceData::TMotionVector& c_rMotionVector = itorMotion->second;
					CRaceData::TMotionVector::const_iterator it;

					for (it = c_rMotionVector.begin(); it != c_rMotionVector.end(); ++it)
					{
						CGraphicThing* pMotion = it->pMotion;
						if (pMotion)
						{
							pMotion->AddReference();
						}
					}
				}
			}
			while (pRaceData->NextMotionModeIterator(itor));
		}
	}

	s_bPreloaded = true;
}
