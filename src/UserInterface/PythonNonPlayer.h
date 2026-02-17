#pragma once

/*
 *	NPC 데이터 프로토 타잎을 관리 한다.
 */
class CPythonNonPlayer : public CSingleton<CPythonNonPlayer> {
public:
  enum EClickEvent {
    ON_CLICK_EVENT_NONE = 0,
    ON_CLICK_EVENT_BATTLE = 1,
    ON_CLICK_EVENT_SHOP = 2,
    ON_CLICK_EVENT_TALK = 3,
    ON_CLICK_EVENT_VEHICLE = 4,

    ON_CLICK_EVENT_MAX_NUM,
  };

  enum EMobEnchants {
    MOB_ENCHANT_CURSE,
    MOB_ENCHANT_SLOW,
    MOB_ENCHANT_POISON,
    MOB_ENCHANT_STUN,
    MOB_ENCHANT_CRITICAL,
    MOB_ENCHANT_PENETRATE,
    MOB_ENCHANTS_MAX_NUM
  };
  enum EMobResists {
    MOB_RESIST_SWORD,
    MOB_RESIST_TWOHAND,
    MOB_RESIST_DAGGER,
    MOB_RESIST_BELL,
    MOB_RESIST_FAN,
    MOB_RESIST_BOW,
    MOB_RESIST_FIRE,
    MOB_RESIST_ELECT,
    MOB_RESIST_MAGIC,
    MOB_RESIST_WIND,
    MOB_RESIST_POISON,
    MOB_RESISTS_MAX_NUM
  };

#define MOB_ATTRIBUTE_MAX_NUM 12
#define MOB_SKILL_MAX_NUM 5

#pragma pack(push)
#pragma pack(1)
  typedef struct SMobSkillLevel {
    DWORD dwVnum;
    BYTE bLevel;
  } TMobSkillLevel;

  typedef struct SMobTable {
    DWORD dwVnum;
    char szName[CHARACTER_NAME_MAX_LEN + 1];
    char szLocaleName[CHARACTER_NAME_MAX_LEN + 1];

    BYTE bType;       // Monster, NPC
    BYTE bRank;       // PAWN, KNIGHT, KING
    BYTE bBattleType; // MELEE, etc..
    BYTE bLevel;      // Level
    BYTE bSize;

    DWORD dwGoldMin;
    DWORD dwGoldMax;
    DWORD dwExp;
    DWORD dwMaxHP;
    BYTE bRegenCycle;
    BYTE bRegenPercent;
    WORD wDef;

    DWORD dwAIFlag;
    DWORD dwRaceFlag;
    DWORD dwImmuneFlag;

    BYTE bStr, bDex, bCon, bInt;
    DWORD dwDamageRange[2];

    short sAttackSpeed;
    short sMovingSpeed;
    BYTE bAggresiveHPPct;
    WORD wAggressiveSight;
    WORD wAttackRange;

    char cEnchants[MOB_ENCHANTS_MAX_NUM];
    char cResists[MOB_RESISTS_MAX_NUM];

    DWORD dwResurrectionVnum;
    DWORD dwDropItemVnum;

    BYTE bMountCapacity;
    BYTE bOnClickType;

    BYTE bEmpire;
    char szFolder[64 + 1];
    float fDamMultiply;
    DWORD dwSummonVnum;
    DWORD dwDrainSP;
    DWORD dwMonsterColor;
    DWORD dwPolymorphItemVnum;

    TMobSkillLevel Skills[MOB_SKILL_MAX_NUM];

    BYTE bBerserkPoint;
    BYTE bStoneSkinPoint;
    BYTE bGodSpeedPoint;
    BYTE bDeathBlowPoint;
    BYTE bRevivePoint;
  } TMobTable;
#pragma pack(pop)

  typedef std::list<TMobTable *> TMobTableList;
  typedef std::map<DWORD, TMobTable *> TNonPlayerDataMap;

public:
  CPythonNonPlayer(void);
  virtual ~CPythonNonPlayer(void);

  void Clear();
  void Destroy();

  bool LoadNonPlayerData(const char *c_szFileName);

  const TMobTable *GetTable(DWORD dwVnum);
  bool GetName(DWORD dwVnum, const char **c_pszName);
  bool GetInstanceType(DWORD dwVnum, BYTE *pbType);
  BYTE GetEventType(DWORD dwVnum);
  BYTE GetEventTypeByVID(DWORD dwVID);
  DWORD GetMonsterColor(DWORD dwVnum);
  const char *GetMonsterName(DWORD dwVnum);

  // Function for outer
  void GetMatchableMobList(int iLevel, int iInterval,
                           TMobTableList *pMobTableList);

protected:
  TNonPlayerDataMap m_NonPlayerDataMap;
};