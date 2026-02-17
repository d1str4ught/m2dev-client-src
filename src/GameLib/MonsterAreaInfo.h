// MonsterAreaInfo.h: interface for the CMonsterAreaInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(                                                                  \
    AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_)
#define AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMonsterAreaInfo {
public:
  enum EMonsterAreaInfoType {
    MONSTERAREAINFOTYPE_INVALID,
    MONSTERAREAINFOTYPE_MONSTER,
    MONSTERAREAINFOTYPE_GROUP,
  };
  enum EMonsterDir {
    DIR_RANDOM = 0,
    DIR_NORTH,
    DIR_NORTHEAST,
    DIR_EAST,
    DIR_SOUTHEAST,
    DIR_SOUTH,
    DIR_SOUTHWEST,
    DIR_WEST,
    DIR_NORTHWEST
  };

  CMonsterAreaInfo();
  virtual ~CMonsterAreaInfo();

  void SetID(DWORD dwID) { m_dwID = dwID; }
  DWORD GetID() { return m_dwID; }

  void Clear();

  //
  void SetOrigin(long lOriginX, long lOriginY);
  void GetOrigin(long *plOriginX, long *plOriginY);

  void SetSize(long lSizeX, long lSizeY);
  void GetSize(long *plSizeX, long *plSizeY);

  long GetLeft() { return m_lLeft; }
  long GetTop() { return m_lTop; }
  long GetRight() { return m_lRight; }
  long GetBottom() { return m_lBottom; }

public:
  void SetMonsterAreaInfoType(EMonsterAreaInfoType eMonsterAreaInfoType) {
    m_eMonsterAreaInfoType = eMonsterAreaInfoType;
  }
  EMonsterAreaInfoType GetMonsterAreaInfoType() {
    return m_eMonsterAreaInfoType;
  }

public:
  void SetMonsterGroupID(DWORD dwGroupID) { m_dwGroupID = dwGroupID; }
  void SetMonsterGroupName(std::string strGroupName) {
    m_strGroupName = strGroupName;
  }
  void SetMonsterGroupLeaderName(std::string strGroupLeaderName) {
    m_strLeaderName = strGroupLeaderName;
  }
  void SetMonsterGroupFollowerCount(DWORD dwFollowerCount) {
    m_dwFollowerCount = dwFollowerCount;
  }

  DWORD GetMonsterGroupID() { return m_dwGroupID; }
  std::string GetMonsterGroupName() { return m_strGroupName; }
  std::string GetMonsterGroupLeaderName() { return m_strLeaderName; }
  DWORD GetMonsterGroupFollowerCount() { return m_dwFollowerCount; }

public:
  void SetMonsterName(std::string strMonsterName) {
    m_strMonsterName = strMonsterName;
  }
  void SetMonsterVID(DWORD dwVID) { m_dwVID = dwVID; }

  std::string GetMonsterName() { return m_strMonsterName; }
  DWORD GetMonsterVID() { return m_dwVID; }

public:
  void SetMonsterCount(DWORD dwCount);
  void SetMonsterDirection(EMonsterDir eMonsterDir);
  void RemoveAllMonsters();

  DWORD GetMonsterCount() { return m_dwMonsterCount; }
  EMonsterDir GetMonsterDir() { return m_eMonsterDir; }
  D3DXVECTOR2 GetMonsterDirVector() { return m_v2Monsterdirection; }
  D3DXVECTOR2 GetTempMonsterPos(DWORD dwIndex);

protected:
  void SetLRTB();

protected:
  EMonsterAreaInfoType m_eMonsterAreaInfoType;

  // Group Type 정보
  DWORD m_dwGroupID;
  std::string m_strGroupName;
  std::string m_strLeaderName;
  DWORD m_dwFollowerCount;

  // Monster Type 정보
  DWORD m_dwVID;
  std::string m_strMonsterName;

  // 공통 정보
  DWORD m_dwMonsterCount;
  EMonsterDir m_eMonsterDir;
  D3DXVECTOR2 m_v2Monsterdirection;

  DWORD m_dwID;

  long m_lOriginX;
  long m_lOriginY;
  long m_lSizeX;
  long m_lSizeY;

  long m_lLeft;
  long m_lRight;
  long m_lTop;
  long m_lBottom;

  std::vector<D3DXVECTOR2> m_TempMonsterPosVector;
};

typedef std::vector<CMonsterAreaInfo *> TMonsterAreaInfoPtrVector;
typedef TMonsterAreaInfoPtrVector::iterator TMonsterAreaInfoPtrVectorIterator;

#endif // !defined(AFX_MONSTERAREAINFO_H__B177E257_2EB2_4CE5_89D3_D06B5618BE1B__INCLUDED_)
