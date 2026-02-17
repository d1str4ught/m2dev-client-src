#pragma once
#include "GameLib/ItemData.h"
#include "EterBase/Poly/Poly.h"

class CInstanceBase;

class CPythonSkill : public CSingleton<CPythonSkill> {
public:
  enum {
    SKILL_TYPE_NONE,
    SKILL_TYPE_ACTIVE,
    SKILL_TYPE_SUPPORT,
    SKILL_TYPE_GUILD,
    SKILL_TYPE_HORSE,
    SKILL_TYPE_MAX_NUM,
  };

  enum ESkillTableTokenType {
    TABLE_TOKEN_TYPE_VNUM,
    TABLE_TOKEN_TYPE_NAME,
    TABLE_TOKEN_TYPE_TYPE,
    TABLE_TOKEN_TYPE_LEVEL_STEP,
    TABLE_TOKEN_TYPE_MAX_LEVEL,
    TABLE_TOKEN_TYPE_LEVEL_LIMIT,
    TABLE_TOKEN_TYPE_POINT_ON,
    TABLE_TOKEN_TYPE_POINT_POLY,
    TABLE_TOKEN_TYPE_SP_COST_POLY,
    TABLE_TOKEN_TYPE_DURATION_POLY,
    TABLE_TOKEN_TYPE_DURATION_SP_COST_POLY,
    TABLE_TOKEN_TYPE_COOLDOWN_POLY,
    TABLE_TOKEN_TYPE_MASTER_BONUS_POLY,
    TABLE_TOKEN_TYPE_ATTACK_GRADE_POLY,
    TABLE_TOKEN_TYPE_FLAG,
    TABLE_TOKEN_TYPE_AFFECT_FLAG,
    TABLE_TOKEN_TYPE_POINT_ON2,
    TABLE_TOKEN_TYPE_POINT_POLY2,
    TABLE_TOKEN_TYPE_DURATION_POLY2,
    TABLE_TOKEN_TYPE_AFFECT_FLAG2,
    TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_VNUM,
    TABLE_TOKEN_TYPE_PREREQUISITE_SKILL_LEVEL,
    TABLE_TOKEN_TYPE_SKILL_TYPE,
    TABLE_TOKEN_TYPE_MAX_HIT,
    TABLE_TOKEN_TYPE_SPLASH_AROUND_DAMAGE_ADJUST_POLY,
    TABLE_TOKEN_TYPE_TARGET_RANGE,
    TABLE_TOKEN_TYPE_SPLASH_RANGE,
    TABLE_TOKEN_TYPE_MAX_NUM,
  };

  enum ESkillDescTokenType {
    DESC_TOKEN_TYPE_VNUM,
    DESC_TOKEN_TYPE_JOB,
    DESC_TOKEN_TYPE_NAME1,
    DESC_TOKEN_TYPE_NAME2,
    DESC_TOKEN_TYPE_NAME3,
    DESC_TOKEN_TYPE_DESCRIPTION,
    DESC_TOKEN_TYPE_CONDITION1,
    DESC_TOKEN_TYPE_CONDITION2,
    DESC_TOKEN_TYPE_CONDITION3,
    DESC_TOKEN_TYPE_CONDITION4,
    DESC_TOKEN_TYPE_ATTRIBUTE,
    DESC_TOKEN_TYPE_WEAPON,
    DESC_TOKEN_TYPE_ICON_NAME,
    DESC_TOKEN_TYPE_MOTION_INDEX,
    DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM,
    DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA,
    DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA,
    DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_1,
    DESC_TOKEN_TYPE_AFFECT_MIN_1,
    DESC_TOKEN_TYPE_AFFECT_MAX_1,
    DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_2,
    DESC_TOKEN_TYPE_AFFECT_MIN_2,
    DESC_TOKEN_TYPE_AFFECT_MAX_2,
    DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_3,
    DESC_TOKEN_TYPE_AFFECT_MIN_3,
    DESC_TOKEN_TYPE_AFFECT_MAX_3,
    DESC_TOKEN_TYPE_LEVEL_LIMIT,
    DESC_TOKEN_TYPE_MAX_LEVEL,
    DESC_TOKEN_TYPE_MAX_NUM,

    CONDITION_COLUMN_COUNT = 3,
    AFFECT_COLUMN_COUNT = 3,
    AFFECT_STEP_COUNT = 3,
  };

  enum {
    SKILL_ATTRIBUTE_NEED_TARGET = (1 << 0),
    SKILL_ATTRIBUTE_TOGGLE = (1 << 1),
    SKILL_ATTRIBUTE_WEAPON_LIMITATION = (1 << 2),
    SKILL_ATTRIBUTE_MELEE_ATTACK = (1 << 3),
    SKILL_ATTRIBUTE_USE_HP = (1 << 4),
    SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION = (1 << 5),
    SKILL_ATTRIBUTE_STANDING_SKILL = (1 << 6),
    SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE = (1 << 7),
    SKILL_ATTRIBUTE_CAN_USE_FOR_ME = (1 << 8),
    SKILL_ATTRIBUTE_NEED_CORPSE = (1 << 9),
    SKILL_ATTRIBUTE_FAN_RANGE = (1 << 10),
    SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH = (1 << 11),
    SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE = (1 << 12),
    SKILL_ATTRIBUTE_NEED_POISON_BOTTLE = (1 << 13),
    SKILL_ATTRIBUTE_ATTACK_SKILL = (1 << 14),
    SKILL_ATTRIBUTE_TIME_INCREASE_SKILL = (1 << 15), // 증지술 전용 attribute
    SKILL_ATTRIBUTE_CHARGE_ATTACK = (1 << 16),
    SKILL_ATTRIBUTE_PASSIVE = (1 << 17),
    SKILL_ATTRIBUTE_CANNOT_LEVEL_UP = (1 << 18),
    SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR = (1 << 19),
    SKILL_ATTRIBUTE_MOVING_SKILL = (1 << 20),
    SKILL_ATTRIBUTE_HORSE_SKILL = (1 << 21),
    SKILL_ATTRIBUTE_CIRCLE_RANGE = (1 << 22),
    SKILL_ATTRIBUTE_SEARCH_TARGET = (1 << 23),
  };

  enum {
    SKILL_NEED_WEAPON_SWORD = (1 << CItemData::WEAPON_SWORD),
    SKILL_NEED_WEAPON_DAGGER = (1 << CItemData::WEAPON_DAGGER),
    SKILL_NEED_WEAPON_BOW = (1 << CItemData::WEAPON_BOW),
    SKILL_NEED_WEAPON_TWO_HANDED = (1 << CItemData::WEAPON_TWO_HANDED),
    SKILL_NEED_WEAPON_DOUBLE_SWORD = (1 << CItemData::WEAPON_DAGGER),
    SKILL_NEED_WEAPON_BELL = (1 << CItemData::WEAPON_BELL),
    SKILL_NEED_WEAPON_FAN = (1 << CItemData::WEAPON_FAN),
    SKILL_NEED_WEAPON_ARROW = (1 << CItemData::WEAPON_ARROW),
    SKILL_NEED_WEAPON_EMPTY_HAND = (1 << CItemData::WEAPON_NONE),
  };

  enum {
    VALUE_TYPE_FREE,
    VALUE_TYPE_MIN,
    VALUE_TYPE_MAX,
  };

  enum {
    SKILL_GRADE_COUNT = 3,
    SKILL_EFFECT_COUNT = 4, // 스킬은 초,중,고급 이외에 마스터일때 이펙트 추가.
    SKILL_GRADE_STEP_COUNT = 20,
    SKILL_GRADEGAP = 25,
  };

  typedef struct SAffectData {
    std::string strAffectDescription;
    std::string strAffectMinFormula;
    std::string strAffectMaxFormula;
  } TAffectData;
  typedef struct SAffectDataNew {
    std::string strPointType;
    std::string strPointPoly;
  } TAffectDataNew;
  typedef struct SRequireStatData {
    BYTE byPoint;
    BYTE byLevel;
  } TRequireStatData;
  typedef struct SGradeData {
    std::string strName;
    CGraphicImage *pImage;
    WORD wMotionIndex;
  } TGradeData;
  typedef struct SSkillData {
    static DWORD MELEE_SKILL_TARGET_RANGE;
    // Functions
    SSkillData();
    DWORD GetTargetRange() const;
    BOOL CanChangeDirection();
    BOOL IsFanRange();
    BOOL IsCircleRange();
    BOOL IsAutoSearchTarget();
    BOOL IsNeedTarget();
    BOOL IsNeedCorpse();
    BOOL IsToggleSkill();
    BOOL IsUseHPSkill();
    BOOL IsStandingSkill();
    BOOL CanUseWeaponType(DWORD dwWeaponType);
    BOOL IsOnlyForAlliance();
    BOOL CanUseForMe();
    BOOL CanUseIfNotEnough();
    BOOL IsNeedEmptyBottle();
    BOOL IsNeedPoisonBottle();
    BOOL IsNeedBow();
    BOOL IsAttackSkill();
    BOOL IsHorseSkill();
    BOOL IsMovingSkill();
    BOOL IsTimeIncreaseSkill();
    BOOL IsMeleeSkill();
    BOOL IsChargeSkill();
    BOOL IsOnlyForGuildWar();

    bool GetState(const char *c_szStateName, int *piState,
                  int iMinMaxType = VALUE_TYPE_FREE);
    float ProcessFormula(CPoly *pPoly, float fSkillLevel = 0.0f,
                         int iMinMaxType = VALUE_TYPE_FREE);
    const char *GetAffectDescription(DWORD dwIndex, float fSkillLevel);
    DWORD GetSkillCoolTime(float fSkillPoint);
    int GetNeedSP(float fSkillPoint);
    DWORD GetContinuationSP(float fSkillPoint);
    DWORD GetMotionLoopCount(float fSkillPoint);
    DWORD GetTargetCount(float fSkillPoint);
    DWORD GetDuration(float fSkillPoint);
    DWORD GetSkillMotionIndex(int iGrade = -1);
    BYTE GetMaxLevel();
    BYTE GetLevelUpPoint();
    bool IsCanUseSkill();
    BOOL IsNoMotion();

    const std::string GetName() const;
    BYTE GetType() const;

    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////

    // Variable
    BYTE byType;
    DWORD dwSkillIndex;
    BYTE byMaxLevel;
    BYTE byLevelUpPoint;
    BYTE byLevelLimit;
    BOOL bNoMotion;

    std::string strName;
    std::string strIconFileName;
    std::string strDescription;
    std::string strMotionScriptFileName;

    std::vector<std::string> ConditionDataVector;
    std::vector<TAffectData> AffectDataVector;
    std::vector<TAffectDataNew> AffectDataNewVector;
    std::vector<TRequireStatData> RequireStatDataVector;
    std::string strCoolTimeFormula;
    std::string strTargetCountFormula;
    std::string strMotionLoopCountFormula;
    std::string strNeedSPFormula;
    std::string strContinuationSPFormula;
    std::string strDuration;

    DWORD dwSkillAttribute;
    DWORD dwNeedWeapon;
    DWORD dwTargetRange;
    WORD wMotionIndex;      // 없어질 변수
    WORD wMotionIndexForMe; // 없어질 변수

    BOOL isRequirement;
    std::string strRequireSkillName;
    BYTE byRequireSkillLevel;

    TGradeData GradeData[SKILL_EFFECT_COUNT];

    CGraphicImage *pImage;

    /////

    static std::map<std::string, DWORD> ms_StatusNameMap;
    static std::map<std::string, DWORD> ms_NewMinStatusNameMap;
    static std::map<std::string, DWORD> ms_NewMaxStatusNameMap;
    static DWORD ms_dwTimeIncreaseSkillNumber;
  } TSkillData;

  typedef std::map<DWORD, TSkillData> TSkillDataMap;
  typedef std::map<DWORD, DWORD> TSkillMotionIndexMap;
  typedef std::map<std::string, std::string> TPathNameMap;

public:
  CPythonSkill();
  virtual ~CPythonSkill();

  void Destroy();
  bool RegisterSkill(DWORD dwSkillIndex, const char *c_szFileName);
  bool RegisterSkillTable(const char *c_szFileName);
  bool RegisterSkillDesc(const char *c_szFileName);
  BOOL GetSkillData(DWORD dwSkillIndex, TSkillData **ppSkillData);
  bool GetSkillDataByName(const char *c_szName, TSkillData **ppSkillData);

  void SetPathName(const char *c_szFileName);
  const char *GetPathName();

  void TEST();

protected:
  void __RegisterGradeIconImage(TSkillData &rData, const char *c_szHeader,
                                const char *c_szImageName);
  void __RegisterNormalIconImage(TSkillData &rData, const char *c_szHeader,
                                 const char *c_szImageName);

protected:
  TSkillDataMap m_SkillDataMap;

  std::string m_strPathName;

  std::map<std::string, DWORD> m_SkillTypeIndexMap;
  std::map<std::string, DWORD> m_SkillAttributeIndexMap;
  std::map<std::string, DWORD> m_SkillNeedWeaponIndexMap;
  std::map<std::string, DWORD> m_SkillWeaponTypeIndexMap;
  TPathNameMap m_PathNameMap;
};