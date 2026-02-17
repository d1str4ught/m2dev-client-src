#include "StdAfx.h"
#include "PythonSkill.h"

#include "EterBase/Poly/Poly.h"
#include "PackLib/PackManager.h"
#include "InstanceBase.h"
#include "PythonPlayer.h"

std::map<std::string, DWORD> CPythonSkill::SSkillData::ms_StatusNameMap;
std::map<std::string, DWORD> CPythonSkill::SSkillData::ms_NewMinStatusNameMap;
std::map<std::string, DWORD> CPythonSkill::SSkillData::ms_NewMaxStatusNameMap;
DWORD CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;

BOOL SKILL_EFFECT_UPGRADE_ENABLE = FALSE;

int SplitLine(const char *c_szText, CTokenVector *pstTokenVector,
              const char *c_szDelimeter) {
  pstTokenVector->reserve(10);
  pstTokenVector->clear();

  std::string stToken;
  std::string stLine = c_szText;

  DWORD basePos = 0;

  do {
    int beginPos = stLine.find_first_not_of(c_szDelimeter, basePos);

    if (beginPos < 0)
      return -1;

    int endPos;

    if (stLine[beginPos] == '"') {
      ++beginPos;
      endPos = stLine.find_first_of("\"", beginPos);

      if (endPos < 0)
        return -2;

      basePos = endPos + 1;
    } else {
      endPos = stLine.find_first_of(c_szDelimeter, beginPos);
      basePos = endPos;
    }

    pstTokenVector->push_back(stLine.substr(beginPos, endPos - beginPos));

    // 추가 코드. 맨뒤에 탭이 있는 경우를 체크한다. - [levites]
    if (int(stLine.find_first_not_of(c_szDelimeter, basePos)) < 0)
      break;
  } while (basePos < stLine.length());

  return 0;
}

// OVERWRITE_SKILLPROTO_POLY
void string_replace_word(const char *base, int base_len, const char *src,
                         int src_len, const char *dst, int dst_len,
                         std::string &result) {
  result.resize(0);
  if (base_len > 0 && dst_len > src_len)
    result.reserve(base_len + (dst_len - src_len) * (base_len / src_len));
  else
    result.reserve(base_len);

  int prev = 0;
  int cur = 0;
  while (cur < base_len) {
    if (cur + src_len <= base_len && memcmp(base + cur, src, src_len) == 0) {
      result.append(base + prev, cur - prev);
      result.append(dst, dst_len);
      cur += src_len;
      prev = cur;
    } else {
      ++cur;
    }
  }
  result.append(base + prev, cur - prev);
}
// END_OF_OVERWRITE_SKILLPROTO_POLY

bool CPythonSkill::RegisterSkillTable(const char *c_szFileName) {
  TPackFile kFile;
  if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
    return false;

  CMemoryTextFileLoader textFileLoader;
  textFileLoader.Bind(kFile.size(), kFile.data());

  // OVERWRITE_SKILLPROTO_POLY
  std::string src_poly_rand;
  std::string src_poly_atk;
  std::string src_poly_mwep;
  // END_OF_OVERWRITE_SKILLPROTO_POLY

  CTokenVector TokenVector;
  for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i) {
    if (!textFileLoader.SplitLineByTab(i, &TokenVector))
      continue;

    std::string strLine = textFileLoader.GetLineString(i);

    if (TABLE_TOKEN_TYPE_MAX_NUM != TokenVector.size()) {
      Tracef("CPythonSkill::RegisterSkillTable(%s) - Strange Token Count "
             "[Line:%d / TokenCount:%d]\n",
             c_szFileName, i, TokenVector.size());
      continue;
    }

    int iVnum = atoi(TokenVector[TABLE_TOKEN_TYPE_VNUM].c_str());

    TSkillDataMap::iterator itor = m_SkillDataMap.find(iVnum);
    if (m_SkillDataMap.end() == itor) {
      Tracef("CPythonSkill::RegisterSkillTable(%s) - NOT EXIST SkillDesc "
             "[Vnum:%d Line:%d]\n",
             c_szFileName, iVnum, i);
      continue;
    }

    TSkillData &rSkillData = itor->second;

    const std::string &c_strSPCostPoly =
        TokenVector[TABLE_TOKEN_TYPE_SP_COST_POLY];
    if (!c_strSPCostPoly.empty()) {
      rSkillData.strNeedSPFormula = c_strSPCostPoly;
    }

    const std::string &c_strCooldownPoly =
        TokenVector[TABLE_TOKEN_TYPE_COOLDOWN_POLY];
    if (!c_strCooldownPoly.empty()) {
      rSkillData.strCoolTimeFormula = c_strCooldownPoly;
    }

    const std::string &c_strDurationSPCostPoly =
        TokenVector[TABLE_TOKEN_TYPE_DURATION_SP_COST_POLY];
    if (!c_strDurationSPCostPoly.empty()) {
      rSkillData.strContinuationSPFormula = c_strDurationSPCostPoly;
    }

    const std::string &c_strTargetRange =
        TokenVector[TABLE_TOKEN_TYPE_TARGET_RANGE];
    if (!c_strTargetRange.empty()) {
      rSkillData.dwTargetRange = atoi(c_strTargetRange.c_str());
    }

    rSkillData.strDuration = TokenVector[TABLE_TOKEN_TYPE_DURATION_POLY];

    const DWORD LevelEmpty = 1;

    const std::string &c_strMaxLevel = TokenVector[TABLE_TOKEN_TYPE_MAX_LEVEL];
    if (!c_strMaxLevel.empty()) {
      int maxLevel = atoi(c_strMaxLevel.c_str());
      if (maxLevel > LevelEmpty)
        rSkillData.byMaxLevel = maxLevel;
    }

    const std::string &c_strLevelLimit =
        TokenVector[TABLE_TOKEN_TYPE_LEVEL_LIMIT];
    if (!c_strLevelLimit.empty()) {
      int levelLimit = atoi(c_strLevelLimit.c_str());
      if (rSkillData.byLevelLimit > LevelEmpty)
        rSkillData.byLevelLimit = levelLimit;
    }
    const std::string &c_strPointPoly =
        TokenVector[TABLE_TOKEN_TYPE_POINT_POLY];

    // OVERWRITE_SKILLPROTO_POLY
    bool USE_SKILL_PROTO = true;

    switch (iVnum) {
    case 34:

    // GUILD_SKILL_DISPLAY_BUG_FIX
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
      // END_OF_GUILD_SKILL_DISPLAY_BUG_FIX
      USE_SKILL_PROTO = false;
      break;
    }

    if (!rSkillData.AffectDataVector.empty() && USE_SKILL_PROTO) {
      TAffectData &affect = rSkillData.AffectDataVector[0];

      if (strstr(c_strPointPoly.c_str(), "atk") != NULL ||
          strstr(c_strPointPoly.c_str(), "mwep") != NULL ||
          strstr(c_strPointPoly.c_str(), "number") != NULL) {
        src_poly_rand = "";
        src_poly_atk = "";
        src_poly_mwep = "";

        // MIN
        string_replace_word(c_strPointPoly.c_str(), c_strPointPoly.length(),
                            "number", 6, "min", 3, src_poly_rand);
        string_replace_word(src_poly_rand.c_str(), src_poly_rand.length(),
                            "atk", 3, "minatk", 6, src_poly_atk);
        string_replace_word(src_poly_atk.c_str(), src_poly_atk.length(), "mwep",
                            4, "minmwep", 7, affect.strAffectMinFormula);
        // END_OF_MIN

        // MAX
        string_replace_word(c_strPointPoly.c_str(), c_strPointPoly.length(),
                            "number", 6, "max", 3, src_poly_rand);
        string_replace_word(src_poly_rand.c_str(), src_poly_rand.length(),
                            "atk", 3, "maxatk", 6, src_poly_atk);
        string_replace_word(src_poly_atk.c_str(), src_poly_atk.length(), "mwep",
                            4, "maxmwep", 7, affect.strAffectMaxFormula);
        // END_OF_MAX

        switch (iVnum) {
        case 1:
          affect.strAffectMinFormula += "* 3";
          affect.strAffectMaxFormula += "* 3";
          break;
        }
      } else {
        affect.strAffectMinFormula = c_strPointPoly;
        affect.strAffectMaxFormula = "";
      }
    }
    // END_OF_OVERWRITE_SKILLPROTO_POLY
  }

  return true;
}

void CPythonSkill::__RegisterGradeIconImage(TSkillData &rData,
                                            const char *c_szHeader,
                                            const char *c_szImageName) {
  for (int j = 0; j < SKILL_GRADE_COUNT; ++j) {
    TGradeData &rGradeData = rData.GradeData[j];

    char szCount[8 + 1];
    _snprintf(szCount, sizeof(szCount), "_%02d", j + 1);

    std::string strFileName = "";
    strFileName += c_szHeader;
    strFileName += c_szImageName;
    strFileName += szCount;
    strFileName += ".sub";
    rGradeData.pImage =
        (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
            strFileName.c_str());
  }
}

void CPythonSkill::__RegisterNormalIconImage(TSkillData &rData,
                                             const char *c_szHeader,
                                             const char *c_szImageName) {
  std::string strFileName = "";
  strFileName += c_szHeader;
  strFileName += c_szImageName;
  strFileName += ".sub";
  rData.pImage =
      (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
          strFileName.c_str());
  for (int j = 0; j < SKILL_GRADE_COUNT; ++j) {
    TGradeData &rGradeData = rData.GradeData[j];
    rGradeData.pImage = rData.pImage;
  }
}
extern const DWORD c_iSkillIndex_Riding;
bool CPythonSkill::RegisterSkillDesc(const char *c_szFileName) {
  TPackFile kFile;
  if (!CPackManager::Instance().GetFile(c_szFileName, kFile))
    return false;

  CMemoryTextFileLoader textFileLoader;
  textFileLoader.Bind(kFile.size(), kFile.data());

  CTokenVector TokenVector;
  for (DWORD i = 0; i < textFileLoader.GetLineCount(); ++i) {
    if (!textFileLoader.SplitLineByTab(i, &TokenVector))
      continue;

    if (DESC_TOKEN_TYPE_VNUM >= TokenVector.size()) {
      TraceError("SkillDesc.line(%d).NO_VNUM_ERROR\n", i + 1);
      continue;
    }

    DWORD iSkillIndex = atoi(TokenVector[DESC_TOKEN_TYPE_VNUM].c_str());
    if (iSkillIndex == 0) {
      TraceError("SkillDesc.line(%d).NO_INDEX_ERROR\n", i + 1);
      continue;
    }

    if (DESC_TOKEN_TYPE_JOB >= TokenVector.size()) {
      TraceError("SkillDesc.line(%d).NO_JOB_ERROR\n", i + 1);
      continue;
    }

    m_SkillDataMap.insert(std::make_pair(iSkillIndex, TSkillData()));
    TSkillData &rSkillData = m_SkillDataMap[iSkillIndex];

    // Vnum
    rSkillData.dwSkillIndex = iSkillIndex;

    // Type
    {
      std::string strJob = TokenVector[DESC_TOKEN_TYPE_JOB];
      stl_lowers(strJob);

      std::map<std::string, DWORD>::iterator it =
          m_SkillTypeIndexMap.find(strJob.c_str());
      if (m_SkillTypeIndexMap.end() == it) {
        TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d, "
                   "strType=%s).STRANGE_SKILL_TYPE",
                   iSkillIndex, strJob.c_str());
        continue;
      }

      rSkillData.byType = BYTE(it->second);
    }

    // Name
    {
      rSkillData.strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
      rSkillData.GradeData[0].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
      rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME2];
      rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME3];

      if (rSkillData.GradeData[1].strName.empty())
        rSkillData.GradeData[1].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
      if (rSkillData.GradeData[2].strName.empty())
        rSkillData.GradeData[2].strName = TokenVector[DESC_TOKEN_TYPE_NAME1];
    }

    // Description
    {
      rSkillData.strDescription = TokenVector[DESC_TOKEN_TYPE_DESCRIPTION];
    }

    // Condition
    {
      rSkillData.ConditionDataVector.clear();
      for (int j = 0; j < CONDITION_COLUMN_COUNT; ++j) {
        const std::string &c_rstrCondition =
            TokenVector[DESC_TOKEN_TYPE_CONDITION1 + j];
        if (!c_rstrCondition.empty())
          rSkillData.ConditionDataVector.push_back(c_rstrCondition);
      }
    }

    // Attribute
    {
      const std::string &c_rstrAttribute =
          TokenVector[DESC_TOKEN_TYPE_ATTRIBUTE];
      CTokenVector AttributeTokenVector;
      SplitLine(c_rstrAttribute.c_str(), &AttributeTokenVector, "|");

      for (CTokenVector::iterator it = AttributeTokenVector.begin();
           it != AttributeTokenVector.end(); ++it) {
        std::string &rstrToken = *it;
        std::map<std::string, DWORD>::iterator it2 =
            m_SkillAttributeIndexMap.find(rstrToken);
        if (m_SkillAttributeIndexMap.end() == it2) {
          TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d) - "
                     "Strange Skill Attribute(%s)",
                     iSkillIndex, rstrToken.c_str());
          continue;
        }
        rSkillData.dwSkillAttribute |= it2->second;
      }
    }

    // Weapon
    {
      const std::string &c_rstrWeapon = TokenVector[DESC_TOKEN_TYPE_WEAPON];
      CTokenVector WeaponTokenVector;
      SplitLine(c_rstrWeapon.c_str(), &WeaponTokenVector, "|");

      for (CTokenVector::iterator it = WeaponTokenVector.begin();
           it != WeaponTokenVector.end(); ++it) {
        std::string &rstrToken = *it;
        std::map<std::string, DWORD>::iterator it2 =
            m_SkillNeedWeaponIndexMap.find(rstrToken);
        if (m_SkillNeedWeaponIndexMap.end() == it2) {
          TraceError("CPythonSkill::RegisterSkillDesc(dwSkillIndex=%d) - "
                     "Strange Skill Need Weapon(%s)",
                     iSkillIndex, rstrToken.c_str());
          continue;
        }
        rSkillData.dwNeedWeapon |= it2->second;
      }
    }

    // Icon Name
    {
      const std::string &c_rstrJob = TokenVector[DESC_TOKEN_TYPE_JOB];
      const std::string &c_rstrIconName =
          TokenVector[DESC_TOKEN_TYPE_ICON_NAME];

      // NOTE : Support 스킬임에도 3단계를 지원해야 해서 예외 처리 - [levites]
      if (c_iSkillIndex_Riding == iSkillIndex) {
        char szIconFileNameHeader[64 + 1];
        _snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader),
                  "%sskill/common/support/", g_strImagePath.c_str());

        __RegisterGradeIconImage(rSkillData, szIconFileNameHeader,
                                 c_rstrIconName.c_str());
      } else if (m_PathNameMap.end() != m_PathNameMap.find(c_rstrJob)) {
        char szIconFileNameHeader[64 + 1];
        _snprintf(szIconFileNameHeader, sizeof(szIconFileNameHeader),
                  "%sskill/%s/", g_strImagePath.c_str(),
                  m_PathNameMap[c_rstrJob].c_str());

        switch (rSkillData.byType) {
        case SKILL_TYPE_ACTIVE: {
          __RegisterGradeIconImage(rSkillData, szIconFileNameHeader,
                                   c_rstrIconName.c_str());
          break;
        }
        case SKILL_TYPE_SUPPORT:
        case SKILL_TYPE_GUILD:
        case SKILL_TYPE_HORSE: {
          __RegisterNormalIconImage(rSkillData, szIconFileNameHeader,
                                    c_rstrIconName.c_str());
          break;
        }
        }
      }
    }

    // Motion Index
    if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX) {
      const std::string &c_rstrMotionIndex =
          TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX];
      if (c_rstrMotionIndex.empty()) {
        rSkillData.bNoMotion = TRUE;
        rSkillData.wMotionIndex = 0;
      } else {
        rSkillData.bNoMotion = FALSE;
        rSkillData.wMotionIndex = (WORD)atoi(c_rstrMotionIndex.c_str());
      }
    } else {
      rSkillData.wMotionIndex = 0;
    }

    if (TokenVector.size() > DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA)
      rSkillData.strTargetCountFormula =
          TokenVector[DESC_TOKEN_TYPE_TARGET_COUNT_FORMULA].c_str();
    if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA)
      rSkillData.strMotionLoopCountFormula =
          TokenVector[DESC_TOKEN_TYPE_MOTION_LOOP_COUNT_FORMULA].c_str();

    rSkillData.AffectDataNewVector.clear();
    rSkillData.AffectDataNewVector.reserve(3);

    // Affect
    for (int j = 0; j < AFFECT_COLUMN_COUNT; ++j) {
      int iDescriptionSlotIndex =
          DESC_TOKEN_TYPE_AFFECT_DESCRIPTION_1 + j * AFFECT_STEP_COUNT;
      int iMinSlotIndex = DESC_TOKEN_TYPE_AFFECT_MIN_1 + j * AFFECT_STEP_COUNT;
      int iMaxSlotIndex = DESC_TOKEN_TYPE_AFFECT_MAX_1 + j * AFFECT_STEP_COUNT;

      if (TokenVector.size() > iDescriptionSlotIndex)
        if (!TokenVector[iDescriptionSlotIndex].empty()) {
          rSkillData.AffectDataVector.push_back(TAffectData());

          TAffectData &rAffectData = *rSkillData.AffectDataVector.rbegin();
          rAffectData.strAffectDescription = "";
          rAffectData.strAffectMinFormula = "";
          rAffectData.strAffectMaxFormula = "";

          rAffectData.strAffectDescription = TokenVector[iDescriptionSlotIndex];
          if (TokenVector.size() > iMinSlotIndex)
            if (!TokenVector[iMinSlotIndex].empty())
              rAffectData.strAffectMinFormula = TokenVector[iMinSlotIndex];
          if (TokenVector.size() > iMaxSlotIndex)
            if (!TokenVector[iMaxSlotIndex].empty())
              rAffectData.strAffectMaxFormula = TokenVector[iMaxSlotIndex];
        }
    }

    if (TokenVector.size() > DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM) {
      int numGrade =
          atoi(TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str());
      if (SKILL_EFFECT_COUNT < numGrade) {
        TraceError("%s[%s] 가 등급 제한[%d]을 넘어갔습니다.",
                   rSkillData.strName.c_str(),
                   TokenVector[DESC_TOKEN_TYPE_MOTION_INDEX_GRADE_NUM].c_str(),
                   SKILL_EFFECT_COUNT);
        return false;
      }
      for (int iGrade = 0; iGrade < numGrade; iGrade++) {
        // if (iGrade == SKILL_GRADE_COUNT)
        //	rSkillData.GradeData[iGrade] = rSkillData.GradeData[iGrade-1];
        rSkillData.GradeData[iGrade].wMotionIndex =
            rSkillData.wMotionIndex + iGrade * SKILL_GRADEGAP;
      }
      for (int iSpace = numGrade; iSpace < SKILL_EFFECT_COUNT; iSpace++) {
        rSkillData.GradeData[iSpace].wMotionIndex = rSkillData.wMotionIndex;
      }
    }

    if (TokenVector.size() > DESC_TOKEN_TYPE_LEVEL_LIMIT) {
      const std::string &c_rstrLevelLimit =
          TokenVector[DESC_TOKEN_TYPE_LEVEL_LIMIT];
      if (c_rstrLevelLimit.empty())
        rSkillData.byLevelLimit = 0;
      else
        rSkillData.byLevelLimit = (WORD)atoi(c_rstrLevelLimit.c_str());
    }

    if (TokenVector.size() > DESC_TOKEN_TYPE_MAX_LEVEL) {
      const std::string &c_rstrMaxLevel =
          TokenVector[DESC_TOKEN_TYPE_MAX_LEVEL];
      const DWORD LevelLimitEmpty = 1;
      if (c_rstrMaxLevel.empty()) {
        if (rSkillData.byLevelLimit > LevelLimitEmpty)
          rSkillData.byMaxLevel = rSkillData.byLevelLimit;
        else
          rSkillData.byMaxLevel = 20;
      } else
        rSkillData.byMaxLevel = (BYTE)atoi(c_rstrMaxLevel.c_str());
    }
  }

  return true;
}

void CPythonSkill::Destroy() { m_SkillDataMap.clear(); }

bool CPythonSkill::RegisterSkill(DWORD dwSkillIndex, const char *c_szFileName) {
  CTextFileLoader TextFileLoader;
  if (!TextFileLoader.Load(c_szFileName)) {
    TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) "
               "- Failed to find file",
               dwSkillIndex, c_szFileName);
    return false;
  }

  TextFileLoader.SetTop();

  TSkillData SkillData;
  SkillData.dwSkillIndex = dwSkillIndex;

  std::string strTypeName;
  if (TextFileLoader.GetTokenString("type", &strTypeName)) {
    stl_lowers(strTypeName);
    std::map<std::string, DWORD>::iterator it =
        m_SkillTypeIndexMap.find(strTypeName.c_str());
    if (m_SkillTypeIndexMap.end() == it) {
      TraceError(
          "Strange Skill Type - CPythonSkill::RegisterSkill(dwSkillIndex=%d, "
          "c_szFileName=%s)",
          dwSkillIndex, c_szFileName);
      return false;
    }

    SkillData.byType = (BYTE)it->second;
  } else {
    TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) "
               "- Failed to find key [type]",
               dwSkillIndex, c_szFileName);
    return false;
  }

  CTokenVector *pAttributeTokenVector;
  if (TextFileLoader.GetTokenVector("attribute", &pAttributeTokenVector)) {
    for (CTokenVector::iterator it = pAttributeTokenVector->begin();
         it != pAttributeTokenVector->end(); ++it) {
      std::string &rstrToken = *it;
      std::map<std::string, DWORD>::iterator it2 =
          m_SkillAttributeIndexMap.find(rstrToken.c_str());
      if (m_SkillAttributeIndexMap.end() == it2) {
        TraceError(
            "Strange Skill Attribute - "
            "CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s)",
            dwSkillIndex, c_szFileName);
        continue;
      }
      SkillData.dwSkillAttribute |= it2->second;
    }
  }

  CTokenVector *pNeedWeaponVector;
  if (TextFileLoader.GetTokenVector("needweapon", &pNeedWeaponVector)) {
    for (CTokenVector::iterator it = pNeedWeaponVector->begin();
         it != pNeedWeaponVector->end(); ++it) {
      std::string &rstrToken = *it;
      std::map<std::string, DWORD>::iterator it2 =
          m_SkillNeedWeaponIndexMap.find(rstrToken.c_str());
      if (m_SkillNeedWeaponIndexMap.end() == it2) {
        TraceError(
            "Strange Skill Need Weapon - "
            "CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s)",
            dwSkillIndex, c_szFileName);
        continue;
      }
      SkillData.dwNeedWeapon |= it2->second;
    }
  }

  {
    char szName[256];
    strncpy_s(szName, "name", _TRUNCATE);
    if (!TextFileLoader.GetTokenString(szName, &SkillData.strName))
      if (!TextFileLoader.GetTokenString("name", &SkillData.strName)) {
        TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, "
                   "c_szFileName=%s) - Failed to find [%s]",
                   dwSkillIndex, c_szFileName, szName);
        return false;
      }
  }

  {
    char szName[256];
    strncpy_s(szName, "description", _TRUNCATE);
    if (!TextFileLoader.GetTokenString(szName, &SkillData.strDescription))
      TextFileLoader.GetTokenString("description", &SkillData.strDescription);
  }

  if (!TextFileLoader.GetTokenString("iconfilename",
                                     &SkillData.strIconFileName)) {
    TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, c_szFileName=%s) "
               "- Failed to find [iconfilename]",
               dwSkillIndex, c_szFileName);
    return false;
  }

  {
    CTokenVector *pConditionDataVector;

    char szConditionData[256];
    strncpy_s(szConditionData, "conditiondata", _TRUNCATE);

    bool isConditionData = true;
    if (!TextFileLoader.GetTokenVector(szConditionData, &pConditionDataVector))
      if (!TextFileLoader.GetTokenVector("conditiondata",
                                         &pConditionDataVector))
        isConditionData = false;

    if (isConditionData) {
      DWORD dwSize = pConditionDataVector->size();
      SkillData.ConditionDataVector.clear();
      SkillData.ConditionDataVector.resize(dwSize);
      for (DWORD i = 0; i < dwSize; ++i) {
        SkillData.ConditionDataVector[i] = pConditionDataVector->at(i);
      }
    }
  }

  {
    CTokenVector *pAffectDataVector;

    char szAffectData[256];
    strncpy_s(szAffectData, "affectdata", _TRUNCATE);

    bool isAffectData = true;
    if (!TextFileLoader.GetTokenVector(szAffectData, &pAffectDataVector))
      if (!TextFileLoader.GetTokenVector("affectdata", &pAffectDataVector))
        isAffectData = false;

    if (isAffectData) {
      DWORD dwSize = pAffectDataVector->size() / 3;
      SkillData.AffectDataVector.clear();
      SkillData.AffectDataVector.resize(dwSize);
      for (DWORD i = 0; i < dwSize; ++i) {
        SkillData.AffectDataVector[i].strAffectDescription =
            pAffectDataVector->at(i * 3 + 0);
        SkillData.AffectDataVector[i].strAffectMinFormula =
            pAffectDataVector->at(i * 3 + 1);
        SkillData.AffectDataVector[i].strAffectMaxFormula =
            pAffectDataVector->at(i * 3 + 2);
      }
    }
  }

  {
    CTokenVector *pGradeDataVector;

    char szGradeData[256];
    strncpy_s(szGradeData, "gradedata", _TRUNCATE);

    if (TextFileLoader.GetTokenVector(szGradeData, &pGradeDataVector)) {
      if (SKILL_GRADE_COUNT * 2 != pGradeDataVector->size())
        TraceError("CPythonSkill::RegisterSkill(dwSkillIndex=%d, "
                   "c_szFileName=%s) - Strange Grade Data Count",
                   dwSkillIndex, c_szFileName);

      for (DWORD i = 0; i < std::min((size_t)SKILL_GRADE_COUNT,
                                     pGradeDataVector->size() / 2);
           ++i) {
        SkillData.GradeData[i].strName = pGradeDataVector->at(i * 2 + 0);
        std::string strIconFileName =
            g_strImagePath + pGradeDataVector->at(i * 2 + 1);
        SkillData.GradeData[i].pImage =
            (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
                strIconFileName.c_str());
      }
    }
  }

  TextFileLoader.GetTokenString("cooltimeformula",
                                &SkillData.strCoolTimeFormula);
  TextFileLoader.GetTokenString("targetcountformula",
                                &SkillData.strTargetCountFormula);
  TextFileLoader.GetTokenString("motionloopcountformula",
                                &SkillData.strMotionLoopCountFormula);
  TextFileLoader.GetTokenString("needspformula", &SkillData.strNeedSPFormula);
  TextFileLoader.GetTokenString("continuationspformula",
                                &SkillData.strContinuationSPFormula);
  TextFileLoader.GetTokenDoubleWord("range", &SkillData.dwTargetRange);
  TextFileLoader.GetTokenByte("maxlevel", &SkillData.byMaxLevel);
  TextFileLoader.GetTokenByte("leveluppoint", &SkillData.byLevelUpPoint);

  WORD wMotionIndex;
  if (TextFileLoader.GetTokenWord("motionindex", &wMotionIndex)) {
    SkillData.wMotionIndex = wMotionIndex;
  } else {
    SkillData.wMotionIndex = 0;
  }

  WORD wMotionIndexForMe;
  if (TextFileLoader.GetTokenWord("motionindexforme", &wMotionIndexForMe)) {
    SkillData.wMotionIndexForMe = wMotionIndexForMe;
  } else {
    SkillData.wMotionIndexForMe = 0;
  }

  SkillData.strIconFileName = g_strImagePath + SkillData.strIconFileName;
  SkillData.pImage =
      (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(
          SkillData.strIconFileName.c_str());

  m_SkillDataMap.insert(TSkillDataMap::value_type(dwSkillIndex, SkillData));

  /////

  if (SkillData.IsTimeIncreaseSkill()) {
    CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber =
        SkillData.dwSkillIndex;
  }

  return true;
}

BOOL CPythonSkill::GetSkillData(DWORD dwSkillIndex, TSkillData **ppSkillData) {
  TSkillDataMap::iterator it = m_SkillDataMap.find(dwSkillIndex);

  if (m_SkillDataMap.end() == it)
    return FALSE;

  *ppSkillData = &(it->second);
  return TRUE;
}

bool CPythonSkill::GetSkillDataByName(const char *c_szName,
                                      TSkillData **ppSkillData) {
  TSkillDataMap::iterator itor = m_SkillDataMap.begin();
  for (; itor != m_SkillDataMap.end(); ++itor) {
    TSkillData *pData = &(itor->second);
    if (0 == pData->strName.compare(c_szName)) {
      *ppSkillData = &(itor->second);
      return true;
    }
  }
  return false;
}

void CPythonSkill::SetPathName(const char *c_szFileName) {
  m_strPathName = c_szFileName;
}

const char *CPythonSkill::GetPathName() { return m_strPathName.c_str(); }

void CPythonSkill::TEST() {
  BOOL isFirst;
  std::map<std::string, DWORD>::iterator itorSub;

  FILE *File = fopen("test.txt", "w");

  for (TSkillDataMap::iterator itor = m_SkillDataMap.begin();
       itor != m_SkillDataMap.end(); ++itor) {
    TSkillData &rSkillData = itor->second;

    std::string strLine = "";
    strLine += rSkillData.strName.c_str();
    strLine += "\t";
    // Name2
    strLine += "\t";
    // Name3
    strLine += "\t";
    strLine += rSkillData.strDescription;
    strLine += "\t";
    if (rSkillData.ConditionDataVector.size() > 0)
      strLine += rSkillData.ConditionDataVector[0];
    strLine += "\t";
    if (rSkillData.ConditionDataVector.size() > 1)
      strLine += rSkillData.ConditionDataVector[1];
    strLine += "\t";
    if (rSkillData.ConditionDataVector.size() > 2)
      strLine += rSkillData.ConditionDataVector[2];
    strLine += "\t";
    if (rSkillData.ConditionDataVector.size() > 3)
      strLine += rSkillData.ConditionDataVector[3];

    strLine += "\t";
    isFirst = TRUE;
    for (itorSub = m_SkillAttributeIndexMap.begin();
         itorSub != m_SkillAttributeIndexMap.end(); ++itorSub) {
      if (itorSub->second & rSkillData.dwSkillAttribute) {
        if (isFirst) {
          isFirst = FALSE;
        } else {
          strLine += "|";
        }
        strLine += itorSub->first.c_str();
      }
    }

    strLine += "\t";
    isFirst = TRUE;
    for (itorSub = m_SkillNeedWeaponIndexMap.begin();
         itorSub != m_SkillNeedWeaponIndexMap.end(); ++itorSub) {
      if (itorSub->second & rSkillData.dwNeedWeapon) {
        if (isFirst) {
          isFirst = FALSE;
        } else {
          strLine += "|";
        }
        strLine += itorSub->first.c_str();
      }
    }

    strLine += "\t";
    std::string strFileName = rSkillData.strIconFileName;
    int iPos =
        strFileName.find_last_of("/", rSkillData.strIconFileName.length());
    if (iPos > 0)
      strFileName =
          strFileName.substr(iPos + 1, strFileName.length() - iPos - 4 - 1);
    strLine += strFileName;

    strLine += "\t";
    char szMotionIndex[32 + 1];
    _snprintf(szMotionIndex, sizeof(szMotionIndex), "%d",
              rSkillData.wMotionIndex);
    strLine += szMotionIndex;

    strLine += "\t";
    if (rSkillData.wMotionIndexForMe > 1) {
      char szMotionIndexForMe[32 + 1];
      _snprintf(szMotionIndexForMe, sizeof(szMotionIndexForMe), "%d",
                rSkillData.wMotionIndexForMe);
      strLine += szMotionIndexForMe;
    }

    assert(rSkillData.AffectDataVector.size() <= 3);
    strLine += "\t";
    if (rSkillData.AffectDataVector.size() > 0) {
      strLine += rSkillData.AffectDataVector[0].strAffectDescription;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[0].strAffectMinFormula;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[0].strAffectMaxFormula;
    }
    strLine += "\t";
    if (rSkillData.AffectDataVector.size() > 1) {
      strLine += rSkillData.AffectDataVector[1].strAffectDescription;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[1].strAffectMinFormula;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[1].strAffectMaxFormula;
    }
    strLine += "\t";
    if (rSkillData.AffectDataVector.size() > 2) {
      strLine += rSkillData.AffectDataVector[2].strAffectDescription;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[2].strAffectMinFormula;
      strLine += "\t";
      strLine += rSkillData.AffectDataVector[2].strAffectMaxFormula;
    }

    fprintf(File, "%s\n", strLine.c_str());
  }

  fclose(File);
}

CPythonSkill::CPythonSkill() {
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("none", SKILL_TYPE_NONE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("active", SKILL_TYPE_ACTIVE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("support", SKILL_TYPE_SUPPORT));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("guild", SKILL_TYPE_GUILD));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("horse", SKILL_TYPE_HORSE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("warrior", SKILL_TYPE_ACTIVE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("assassin", SKILL_TYPE_ACTIVE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("sura", SKILL_TYPE_ACTIVE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("shaman", SKILL_TYPE_ACTIVE));
  m_SkillTypeIndexMap.insert(
      std::map<std::string, DWORD>::value_type("passive", SKILL_TYPE_ACTIVE));

  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "NEED_POISON_BOTTLE", SKILL_ATTRIBUTE_NEED_POISON_BOTTLE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "NEED_EMPTY_BOTTLE", SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CAN_USE_IF_NOT_ENOUGH", SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "FAN_RANGE", SKILL_ATTRIBUTE_FAN_RANGE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "NEED_CORPSE", SKILL_ATTRIBUTE_NEED_CORPSE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "NEED_TARGET", SKILL_ATTRIBUTE_NEED_TARGET));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "TOGGLE", SKILL_ATTRIBUTE_TOGGLE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "WEAPON_LIMITATION", SKILL_ATTRIBUTE_WEAPON_LIMITATION));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "MELEE_ATTACK", SKILL_ATTRIBUTE_MELEE_ATTACK));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CHARGE_ATTACK", SKILL_ATTRIBUTE_CHARGE_ATTACK));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "USE_HP", SKILL_ATTRIBUTE_USE_HP));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CAN_CHANGE_DIRECTION", SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "STANDING_SKILL", SKILL_ATTRIBUTE_STANDING_SKILL));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "ONLY_FOR_ALLIANCE", SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CAN_USE_FOR_ME", SKILL_ATTRIBUTE_CAN_USE_FOR_ME));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "ATTACK_SKILL", SKILL_ATTRIBUTE_ATTACK_SKILL));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "MOVING_SKILL", SKILL_ATTRIBUTE_MOVING_SKILL));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "HORSE_SKILL", SKILL_ATTRIBUTE_HORSE_SKILL));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "TIME_INCREASE_SKILL", SKILL_ATTRIBUTE_TIME_INCREASE_SKILL));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "PASSIVE", SKILL_ATTRIBUTE_PASSIVE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CANNOT_LEVEL_UP", SKILL_ATTRIBUTE_CANNOT_LEVEL_UP));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "ONLY_FOR_GUILD_WAR", SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "CIRCLE_RANGE", SKILL_ATTRIBUTE_CIRCLE_RANGE));
  m_SkillAttributeIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "SEARCH_TARGET", SKILL_ATTRIBUTE_SEARCH_TARGET));

  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "SWORD", SKILL_NEED_WEAPON_SWORD));
  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "DAGGER", SKILL_NEED_WEAPON_DAGGER));
  m_SkillNeedWeaponIndexMap.insert(
      std::map<std::string, DWORD>::value_type("BOW", SKILL_NEED_WEAPON_BOW));
  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "TWO_HANDED", SKILL_NEED_WEAPON_TWO_HANDED));
  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "DOUBLE_SWORD", SKILL_NEED_WEAPON_DOUBLE_SWORD));
  m_SkillNeedWeaponIndexMap.insert(
      std::map<std::string, DWORD>::value_type("BELL", SKILL_NEED_WEAPON_BELL));
  m_SkillNeedWeaponIndexMap.insert(
      std::map<std::string, DWORD>::value_type("FAN", SKILL_NEED_WEAPON_FAN));
  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "ARROW", SKILL_NEED_WEAPON_ARROW));
  m_SkillNeedWeaponIndexMap.insert(std::map<std::string, DWORD>::value_type(
      "EMPTY_HAND", SKILL_NEED_WEAPON_EMPTY_HAND));

  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("SWORD"), CItemData::WEAPON_SWORD));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("DAGGER"), CItemData::WEAPON_DAGGER));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("BOW"), CItemData::WEAPON_BOW));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("TWO_HANDED"), CItemData::WEAPON_TWO_HANDED));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("DOUBLE_SWORD"), CItemData::WEAPON_DAGGER));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("BELL"), CItemData::WEAPON_BELL));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("FAN"), CItemData::WEAPON_FAN));
  m_SkillWeaponTypeIndexMap.insert(
      std::make_pair(std::string("ARROW"), CItemData::WEAPON_ARROW));

  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("chain"), POINT_NONE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("HR"), POINT_HIT_RATE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("LV"), POINT_LEVEL));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("Level"), POINT_LEVEL));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MaxHP"), POINT_MAX_HP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MaxSP"), POINT_MAX_SP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MinMWEP"), POINT_MIN_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MaxMWEP"), POINT_MAX_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MinWEP"), POINT_MIN_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MaxWEP"), POINT_MAX_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MinATK"), POINT_MIN_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MaxATK"), POINT_MAX_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("ATKSPD"), POINT_ATT_SPEED));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("AttackPower"), POINT_MIN_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("AtkMin"), POINT_MIN_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("AtkMax"), POINT_MAX_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("DefencePower"), POINT_DEF_GRADE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("DEF"), POINT_DEF_GRADE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MWEP"), POINT_MAGIC_ATT_GRADE));

  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("MagicAttackPower"), POINT_MAGIC_ATT_GRADE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("INT"), POINT_IQ));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("STR"), POINT_ST));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("DEX"), POINT_DX));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("CON"), POINT_HT));

  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("minatk"), POINT_MIN_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxatk"), POINT_MAX_ATK));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("minmtk"), POINT_MIN_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxmtk"), POINT_MAX_WEP));

  // GUILD_SKILL_DISPLAY_BUG_FIX
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxhp"), POINT_MAX_HP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxsp"), POINT_MAX_SP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("odef"), POINT_DEF_GRADE));
  // END_OF_GUILD_SKILL_DISPLAY_BUG_FIX

  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("minwep"), POINT_MIN_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxwep"), POINT_MAX_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("minmwep"), POINT_MIN_MAGIC_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("maxmwep"), POINT_MAX_MAGIC_WEP));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("lv"), POINT_LEVEL));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("ar"), POINT_HIT_RATE));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("iq"), POINT_IQ));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("str"), POINT_ST));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("dex"), POINT_DX));
  SSkillData::ms_StatusNameMap.insert(
      std::make_pair(std::string("con"), POINT_HT));

  /////

  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("atk"), POINT_MIN_ATK));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("mtk"), POINT_MIN_WEP));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("wep"), POINT_MIN_WEP));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("lv"), POINT_LEVEL));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("ar"), POINT_HIT_RATE));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("iq"), POINT_IQ));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("str"), POINT_ST));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("dex"), POINT_DX));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("con"), POINT_HT));

  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("atk"), POINT_MAX_ATK));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("mtk"), POINT_MAX_WEP));
  SSkillData::ms_NewMinStatusNameMap.insert(
      std::make_pair(std::string("wep"), POINT_MAX_WEP));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("lv"), POINT_LEVEL));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("ar"), POINT_HIT_RATE));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("iq"), POINT_IQ));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("str"), POINT_ST));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("dex"), POINT_DX));
  SSkillData::ms_NewMaxStatusNameMap.insert(
      std::make_pair(std::string("con"), POINT_HT));

  m_PathNameMap.insert(
      std::make_pair(std::string("WARRIOR"), std::string("warrior")));
  m_PathNameMap.insert(
      std::make_pair(std::string("ASSASSIN"), std::string("assassin")));
  m_PathNameMap.insert(
      std::make_pair(std::string("SURA"), std::string("sura")));
  m_PathNameMap.insert(
      std::make_pair(std::string("SHAMAN"), std::string("shaman")));
  m_PathNameMap.insert(
      std::make_pair(std::string("PASSIVE"), std::string("passive")));
  m_PathNameMap.insert(
      std::make_pair(std::string("SUPPORT"), std::string("common/support")));
  m_PathNameMap.insert(
      std::make_pair(std::string("GUILD"), std::string("common/guild")));
  m_PathNameMap.insert(
      std::make_pair(std::string("HORSE"), std::string("common/horse")));
}
CPythonSkill::~CPythonSkill() {}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

DWORD CPythonSkill::SSkillData::MELEE_SKILL_TARGET_RANGE = 170;

const std::string CPythonSkill::SSkillData::GetName() const { return strName; }

BYTE CPythonSkill::SSkillData::GetType() const { return byType; }

BOOL CPythonSkill::SSkillData::IsMeleeSkill() {
  if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
    return TRUE;

  return FALSE;
}

BOOL CPythonSkill::SSkillData::IsChargeSkill() {
  if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
    return TRUE;

  return FALSE;
}

BOOL CPythonSkill::SSkillData::IsOnlyForGuildWar() {
  if (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_GUILD_WAR)
    return TRUE;

  return FALSE;
}

DWORD CPythonSkill::SSkillData::GetTargetRange() const {
  if (dwSkillAttribute & SKILL_ATTRIBUTE_MELEE_ATTACK)
    return MELEE_SKILL_TARGET_RANGE;

  if (dwSkillAttribute & SKILL_ATTRIBUTE_CHARGE_ATTACK)
    return MELEE_SKILL_TARGET_RANGE;

  return dwTargetRange;
}

BOOL CPythonSkill::SSkillData::CanChangeDirection() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_CHANGE_DIRECTION);
}

BOOL CPythonSkill::SSkillData::IsFanRange() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_FAN_RANGE);
}

BOOL CPythonSkill::SSkillData::IsCircleRange() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CIRCLE_RANGE);
}

BOOL CPythonSkill::SSkillData::IsAutoSearchTarget() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_SEARCH_TARGET);
}

BOOL CPythonSkill::SSkillData::IsNeedCorpse() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_CORPSE);
}

BOOL CPythonSkill::SSkillData::IsNeedTarget() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_TARGET);
}

BOOL CPythonSkill::SSkillData::IsToggleSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TOGGLE);
}

BOOL CPythonSkill::SSkillData::IsUseHPSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_USE_HP);
}

BOOL CPythonSkill::SSkillData::IsStandingSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_STANDING_SKILL);
}

BOOL CPythonSkill::SSkillData::CanUseWeaponType(DWORD dwWeaponType) {
  if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
    return TRUE;

  return 0 != (dwNeedWeapon & (1 << dwWeaponType));
}

BOOL CPythonSkill::SSkillData::IsOnlyForAlliance() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ONLY_FOR_ALLIANCE);
}

BOOL CPythonSkill::SSkillData::CanUseForMe() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_FOR_ME);
}

BOOL CPythonSkill::SSkillData::CanUseIfNotEnough() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_CAN_USE_IF_NOT_ENOUGH);
}

BOOL CPythonSkill::SSkillData::IsNeedEmptyBottle() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_EMPTY_BOTTLE);
}

BOOL CPythonSkill::SSkillData::IsNeedPoisonBottle() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_NEED_POISON_BOTTLE);
}

BOOL CPythonSkill::SSkillData::IsNeedBow() {
  if (!(dwSkillAttribute & SKILL_ATTRIBUTE_WEAPON_LIMITATION))
    return FALSE;

  return 0 != (dwNeedWeapon & SKILL_NEED_WEAPON_BOW);
}

BOOL CPythonSkill::SSkillData::IsHorseSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_HORSE_SKILL);
}

BOOL CPythonSkill::SSkillData::IsMovingSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_MOVING_SKILL);
}

BOOL CPythonSkill::SSkillData::IsAttackSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_ATTACK_SKILL);
}

BOOL CPythonSkill::SSkillData::IsTimeIncreaseSkill() {
  return 0 != (dwSkillAttribute & SKILL_ATTRIBUTE_TIME_INCREASE_SKILL);
}

bool CPythonSkill::SSkillData::GetState(const char *c_szStateName, int *piState,
                                        int iMinMaxType) {
  std::map<std::string, DWORD> *pStatusNameMap = NULL;

  switch (iMinMaxType) {
  case VALUE_TYPE_FREE:
    pStatusNameMap = &ms_StatusNameMap;
    break;
  case VALUE_TYPE_MIN:
    pStatusNameMap = &ms_NewMinStatusNameMap;
    break;
  case VALUE_TYPE_MAX:
    pStatusNameMap = &ms_NewMaxStatusNameMap;
    break;
  default:
    return false;
    break;
  }

  std::map<std::string, DWORD>::iterator it =
      pStatusNameMap->find(c_szStateName);

  if (it != pStatusNameMap->end()) {
    *piState = CPythonPlayer::Instance().GetStatus(it->second);
  } else if (0 == strncmp(c_szStateName, "JeungJi", 7)) {
    *piState = 0;

    // 증지술 임시 제외
    /*
                    if (0 != ms_dwTimeIncreaseSkillNumber)
                    {
                            DWORD dwSkillSlotIndex;
                            if
       (!CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(ms_dwTimeIncreaseSkillNumber,
       &dwSkillSlotIndex)) return false;

                            *piState =
       CPythonPlayer::Instance().GetSkillLevel(dwSkillSlotIndex);
                    }
    */
  }

  return true;
}

float CPythonSkill::SSkillData::ProcessFormula(CPoly *pPoly, float fSkillLevel,
                                               int iMinMaxType) {
  if (pPoly->Analyze()) {
    for (DWORD i = 0; i < pPoly->GetVarCount(); ++i) {
      const char *c_szVarName = pPoly->GetVarName(i);
      float fState;
      if (!strcmp("SkillPoint", c_szVarName) || !strcmp("k", c_szVarName)) {
        fState = fSkillLevel;
      } else {
        int iState;
        if (!GetState(c_szVarName, &iState, iMinMaxType))
          return 0.0f;

        fState = float(iState);

        if (!strcmp("ar", c_szVarName))
          fState /= 100.0f;
      }
      pPoly->SetVar(c_szVarName, fState);
    }
  } else {
    TraceError("skillGetAffect - Strange Formula [%s]", strName.c_str());
    return 0.0f;
  }

  return pPoly->Eval();
}

// Format specifiers supported in skill descriptions
static const char *FORMAT_SPECIFIERS[] = {
    "%.0f", // Integer (no decimals)
    "%.1f", // 1 decimal place
    "%.2f", // 2 decimal places
    "%d",   // Integer (alternative)
};
static const size_t FORMAT_SPECIFIER_COUNT =
    sizeof(FORMAT_SPECIFIERS) / sizeof(FORMAT_SPECIFIERS[0]);

// Find and replace the first occurrence of any format specifier with the given
// value Returns true if a replacement was made
static bool ReplaceNextFormatSpecifier(std::string &s, float value) {
  size_t bestPos = std::string::npos;
  size_t bestLen = 0;
  const char *bestSpec = nullptr;

  // Find the first (leftmost) format specifier in the string
  for (size_t i = 0; i < FORMAT_SPECIFIER_COUNT; ++i) {
    size_t pos = s.find(FORMAT_SPECIFIERS[i]);
    if (pos != std::string::npos &&
        (bestPos == std::string::npos || pos < bestPos)) {
      bestPos = pos;
      bestLen = strlen(FORMAT_SPECIFIERS[i]);
      bestSpec = FORMAT_SPECIFIERS[i];
    }
  }

  if (bestPos == std::string::npos)
    return false;

  // Format the value according to the specifier found
  char szValue[64];
  if (strcmp(bestSpec, "%.0f") == 0 || strcmp(bestSpec, "%d") == 0)
    _snprintf(szValue, sizeof(szValue), "%.0f", floorf(value));
  else if (strcmp(bestSpec, "%.1f") == 0)
    _snprintf(szValue, sizeof(szValue), "%.1f", value);
  else // %.2f or other
    _snprintf(szValue, sizeof(szValue), "%.2f", value);

  s.replace(bestPos, bestLen, szValue);
  return true;
}

// Replace all occurrences of "%%" with "%" (escaped percent sign)
static void UnescapePercent(std::string &s) {
  size_t pos = 0;
  while ((pos = s.find("%%", pos)) != std::string::npos) {
    s.replace(pos, 2, "%");
    ++pos; // Move past the replaced '%'
  }
}

const char *CPythonSkill::SSkillData::GetAffectDescription(DWORD dwIndex,
                                                           float fSkillLevel) {
  if (dwIndex >= AffectDataVector.size())
    return NULL;

  const std::string &desc = AffectDataVector[dwIndex].strAffectDescription;
  const std::string &minF = AffectDataVector[dwIndex].strAffectMinFormula;
  const std::string &maxF = AffectDataVector[dwIndex].strAffectMaxFormula;

  CPoly minPoly;
  CPoly maxPoly;
  minPoly.SetStr(minF.c_str());
  maxPoly.SetStr(maxF.c_str());

  float fMinValue = ProcessFormula(&minPoly, fSkillLevel);
  float fMaxValue = ProcessFormula(&maxPoly, fSkillLevel);

  // Take absolute values
  if (fMinValue < 0.0f)
    fMinValue = -fMinValue;
  if (fMaxValue < 0.0f)
    fMaxValue = -fMaxValue;

  static std::string out;
  out = desc;

  // Replace format specifiers in order of appearance
  // First specifier gets min value, second gets max value
  ReplaceNextFormatSpecifier(out, fMinValue);
  ReplaceNextFormatSpecifier(out, fMaxValue);

  // Convert escaped %% to single % (for display like "30%")
  UnescapePercent(out);

  return out.c_str();
}

DWORD CPythonSkill::SSkillData::GetSkillCoolTime(float fSkillPoint) {
  if (strCoolTimeFormula.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strCoolTimeFormula.c_str());

  return DWORD(ProcessFormula(&poly, fSkillPoint));
}

DWORD CPythonSkill::SSkillData::GetTargetCount(float fSkillPoint) {
  if (strTargetCountFormula.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strTargetCountFormula.c_str());

  return DWORD(ProcessFormula(&poly, fSkillPoint));
}

DWORD CPythonSkill::SSkillData::GetSkillMotionIndex(int iGrade) {
  if (-1 != iGrade && SKILL_EFFECT_UPGRADE_ENABLE) {
    assert(iGrade >= 0 && iGrade < SKILL_EFFECT_COUNT);
    /*
    if (SKILL_GRADE_COUNT == iGrade)
    {
            return GradeData[SKILL_GRADE_COUNT-1].wMotionIndex;
    }
    else */
    if (iGrade >= 0 && iGrade < SKILL_EFFECT_COUNT) {
      return GradeData[iGrade].wMotionIndex;
    }
  }

  return wMotionIndex;
}

BYTE CPythonSkill::SSkillData::GetMaxLevel() { return byMaxLevel; }

BYTE CPythonSkill::SSkillData::GetLevelUpPoint() { return byLevelUpPoint; }

BOOL CPythonSkill::SSkillData::IsNoMotion() { return bNoMotion; }

bool CPythonSkill::SSkillData::IsCanUseSkill() {
  if (0 != (dwSkillAttribute & SKILL_ATTRIBUTE_PASSIVE))
    return false;

  return true;
}

DWORD CPythonSkill::SSkillData::GetMotionLoopCount(float fSkillPoint) {
  if (strMotionLoopCountFormula.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strMotionLoopCountFormula.c_str());

  return DWORD(ProcessFormula(&poly, fSkillPoint));
}

int CPythonSkill::SSkillData::GetNeedSP(float fSkillPoint) {
  if (strNeedSPFormula.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strNeedSPFormula.c_str());

  return int(ProcessFormula(&poly, fSkillPoint));
}

DWORD CPythonSkill::SSkillData::GetContinuationSP(float fSkillPoint) {
  if (strContinuationSPFormula.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strContinuationSPFormula.c_str());

  return DWORD(ProcessFormula(&poly, fSkillPoint));
}

DWORD CPythonSkill::SSkillData::GetDuration(float fSkillPoint) {
  if (strDuration.empty())
    return 0;

  CPoly poly;
  poly.SetStr(strDuration.c_str());

  return DWORD(ProcessFormula(&poly, fSkillPoint));
}

CPythonSkill::SSkillData::SSkillData() {
  byType = SKILL_TYPE_ACTIVE;
  byMaxLevel = 20;
  byLevelUpPoint = 1;
  dwSkillAttribute = 0;
  dwNeedWeapon = 0;
  dwTargetRange = 0;
  strCoolTimeFormula = "";
  strMotionLoopCountFormula = "";
  strNeedSPFormula = "";
  strContinuationSPFormula = "";
  isRequirement = FALSE;
  strRequireSkillName = "";
  byRequireSkillLevel = 0;
  strDuration = "";
  byLevelLimit = 0;
  bNoMotion = FALSE;

  strName = "";
  pImage = NULL;

  for (int j = 0; j < SKILL_GRADE_COUNT; ++j) {
    TGradeData &rGradeData = GradeData[j];
    rGradeData.strName = "";
    rGradeData.pImage = NULL;
    rGradeData.wMotionIndex = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *skillSetPathName(PyObject *poSelf, PyObject *poArgs) {
  char *szPathName;
  if (!PyTuple_GetString(poArgs, 0, &szPathName))
    return Py_BadArgument();

  CPythonSkill::Instance().SetPathName(szPathName);
  return Py_BuildNone();
}

PyObject *skillRegisterSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 1, &szFileName))
    return Py_BadArgument();

  std::string strFullFileName;
  strFullFileName = CPythonSkill::Instance().GetPathName();
  strFullFileName += szFileName;

  if (!CPythonSkill::Instance().RegisterSkill(iSkillIndex,
                                              strFullFileName.c_str()))
    return Py_BuildException(
        "skill.RegisterSkill - Failed to find skill data file : %d, %s",
        iSkillIndex, strFullFileName.c_str());

  return Py_BuildNone();
}

PyObject *skillLoadSkillData(PyObject *poSelf, PyObject *poArgs) {

  return Py_BuildNone();
}

PyObject *skillRegisterSkillDesc(PyObject *poSelf, PyObject *poArgs) {
  char *szFileName;
  if (!PyTuple_GetString(poArgs, 0, &szFileName))
    return Py_BadArgument();

  if (!CPythonSkill::Instance().RegisterSkillDesc(szFileName))
    return Py_BuildException("Failed to load Skill Desc");

  return Py_BuildNone();
}

PyObject *skillClearSkillData(PyObject *poSelf, PyObject *poArgs) {
  CPythonSkill::SSkillData::ms_dwTimeIncreaseSkillNumber = 0;
  return Py_BuildNone();
}

PyObject *skillGetSkillName(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iSkillGrade = -1;
  PyTuple_GetInteger(poArgs, 1, &iSkillGrade);

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.GetSkillName - Failed to find skill by %d",
                             iSkillIndex);

  if (-1 != iSkillGrade)
    if (iSkillGrade >= 0 && iSkillGrade < CPythonSkill::SKILL_GRADE_COUNT) {
      return Py_BuildValue(
          "s", c_pSkillData->GradeData[iSkillGrade].strName.c_str());
    }

  return Py_BuildValue("s", c_pSkillData->strName.c_str());
}

PyObject *skillGetSkillDescription(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillDescription - Failed to find skill by %d", iSkillIndex);

  return Py_BuildValue("s", c_pSkillData->strDescription.c_str());
}

PyObject *skillGetSkillType(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.GetSkillType - Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->byType);
}

PyObject *skillGetSkillConditionDescriptionCount(PyObject *poSelf,
                                                 PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillConditionDescriptionCount - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->ConditionDataVector.size());
}

PyObject *skillGetSkillConditionDescription(PyObject *poSelf,
                                            PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iConditionIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iConditionIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillConditionDescription() - Failed to find skill by %d",
        iSkillIndex);

  if (iConditionIndex >= c_pSkillData->ConditionDataVector.size())
    return Py_BuildValue("None");

  return Py_BuildValue(
      "s", c_pSkillData->ConditionDataVector[iConditionIndex].c_str());
}

PyObject *skillGetSkillAffectDescriptionCount(PyObject *poSelf,
                                              PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillAffectDescriptionCount - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->AffectDataVector.size());
}

PyObject *skillGetSkillAffectDescription(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iAffectIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
    return Py_BadArgument();

  float fSkillPoint;
  if (!PyTuple_GetFloat(poArgs, 2, &fSkillPoint))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillAffectDescription - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue(
      "s", c_pSkillData->GetAffectDescription(iAffectIndex, fSkillPoint));
}

PyObject *skillGetSkillCoolTime(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;

  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  float fSkillPoint;

  if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;

  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillCoolTime - Failed to find skill by %d", iSkillIndex);

  // MR-3: Calculate casting speed bonus on skill cool time
  DWORD dwSkillCoolTime = c_pSkillData->GetSkillCoolTime(fSkillPoint);
  int iCastingSpeed = CPythonPlayer::Instance().GetStatus(POINT_CASTING_SPEED);

  int iSpd = 100 - iCastingSpeed;
  if (iSpd > 0)
    iSpd = 100 + iSpd;
  else if (iSpd < 0)
    iSpd = 10000 / (100 - iSpd);
  else
    iSpd = 100;

  dwSkillCoolTime = dwSkillCoolTime * iSpd / 100;

  return Py_BuildValue("i", dwSkillCoolTime);
  // MR-3: -- END OF -- Calculate casting speed bonus on skill cool time
}

PyObject *skillGetSkillNeedSP(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  float fSkillPoint;
  if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.GetSkillNeedSP Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->GetNeedSP(fSkillPoint));
}

PyObject *skillGetSkillContinuationSP(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  float fSkillPoint;
  if (!PyTuple_GetFloat(poArgs, 1, &fSkillPoint))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillContinuationSP - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->GetContinuationSP(fSkillPoint));
}

PyObject *skillGetSkillMaxLevel(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillMaxLevel - Failed to find skill by %d", iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->GetMaxLevel());
}

PyObject *skillGetSkillLevelUpPoint(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillLevelUpPoint - Failed to find skill by %d", iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->GetLevelUpPoint());
}

PyObject *skillGetSkillLevelLimit(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillLevelLimit - Failed to find skill by %d", iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->byLevelLimit);
}

PyObject *skillIsSkillRequirement(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.IsSkillRequirement - Failed to find skill by %d", iSkillIndex);

  if (c_pSkillData->isRequirement) {
    CPythonSkill::SSkillData *pRequireSkillData;
    if (!CPythonSkill::Instance().GetSkillDataByName(
            c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData)) {
      TraceError("skill.IsSkillRequirement - Failed to find skill : [%d/%s] "
                 "can't find [%s]\n",
                 c_pSkillData->dwSkillIndex, c_pSkillData->strName.c_str(),
                 c_pSkillData->strRequireSkillName.c_str());
      return Py_BuildValue("i", FALSE);
    }

    DWORD dwRequireSkillSlotIndex;
    if (!CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(
            pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex))
      return Py_BuildValue("i", FALSE);
  }

  return Py_BuildValue("i", c_pSkillData->isRequirement);
}

PyObject *skillGetSkillRequirementData(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillRequirementData - Failed to find skill by %d",
        iSkillIndex);

  CPythonSkill::SSkillData *pRequireSkillData;
  if (!CPythonSkill::Instance().GetSkillDataByName(
          c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData))
    return Py_BuildValue("si", 0, "None", 0);

  int ireqLevel =
      (int)ceil(float(c_pSkillData->byRequireSkillLevel) /
                float(std::max(1, (int)pRequireSkillData->byLevelUpPoint)));
  return Py_BuildValue("si", c_pSkillData->strRequireSkillName.c_str(),
                       ireqLevel);
}

PyObject *skillGetSkillRequireStatCount(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillRequireStatCount - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->RequireStatDataVector.size());
}

PyObject *skillGetSkillRequireStatData(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();
  int iStatIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iStatIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetSkillRequireStatData - Failed to find skill by %d",
        iSkillIndex);

  if (iStatIndex >= c_pSkillData->RequireStatDataVector.size())
    return Py_BuildValue("ii", 0, 0);

  const CPythonSkill::TRequireStatData &c_rRequireStatData =
      c_pSkillData->RequireStatDataVector[iStatIndex];

  return Py_BuildValue("ii", c_rRequireStatData.byPoint,
                       c_rRequireStatData.byLevel);
}

PyObject *skillCanLevelUpSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();
  int iSkillLevel;
  if (!PyTuple_GetInteger(poArgs, 1, &iSkillLevel))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.CanLevelUpSkill - Failed to find skill by %d", iSkillIndex);

  if (iSkillLevel >= c_pSkillData->GetMaxLevel())
    return Py_BuildValue("i", FALSE);

  if (c_pSkillData->isRequirement) {
    CPythonSkill::SSkillData *pRequireSkillData;
    if (CPythonSkill::Instance().GetSkillDataByName(
            c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData)) {
      DWORD dwRequireSkillSlotIndex;
      if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(
              pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex)) {
        int iSkillGrade =
            CPythonPlayer::Instance().GetSkillGrade(dwRequireSkillSlotIndex);
        int iSkillLevel =
            CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);
        if (iSkillGrade <= 0)
          if (iSkillLevel < c_pSkillData->byRequireSkillLevel)
            return Py_BuildValue("i", FALSE);
      }
    }
  }

  for (DWORD i = 0; i < c_pSkillData->RequireStatDataVector.size(); ++i) {
    const CPythonSkill::TRequireStatData &c_rRequireStatData =
        c_pSkillData->RequireStatDataVector[i];
    if (CPythonPlayer::Instance().GetStatus(c_rRequireStatData.byPoint) <
        c_rRequireStatData.byLevel)
      return Py_BuildValue("i", FALSE);
  }

  if (0 != (c_pSkillData->dwSkillAttribute &
            CPythonSkill::SKILL_ATTRIBUTE_CANNOT_LEVEL_UP))
    return Py_BuildValue("i", FALSE);

  return Py_BuildValue("i", TRUE);
}

PyObject *skillCheckRequirementSueccess(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.CheckRequirementSueccess - Failed to find skill by %d",
        iSkillIndex);

  if (c_pSkillData->isRequirement) {
    CPythonSkill::SSkillData *pRequireSkillData;
    if (CPythonSkill::Instance().GetSkillDataByName(
            c_pSkillData->strRequireSkillName.c_str(), &pRequireSkillData)) {
      DWORD dwRequireSkillSlotIndex;
      if (CPythonPlayer::Instance().FindSkillSlotIndexBySkillIndex(
              pRequireSkillData->dwSkillIndex, &dwRequireSkillSlotIndex)) {
        int iSkillGrade =
            CPythonPlayer::Instance().GetSkillGrade(dwRequireSkillSlotIndex);
        int iSkillLevel =
            CPythonPlayer::Instance().GetSkillLevel(dwRequireSkillSlotIndex);
        if (iSkillGrade <= 0)
          if (iSkillLevel < c_pSkillData->byRequireSkillLevel)
            return Py_BuildValue("i", FALSE);
      }
    }
  }

  return Py_BuildValue("i", TRUE);
}

PyObject *skillGetNeedCharacterLevel(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetNeedCharacterLevel - Failed to find skill by %d",
        iSkillIndex);

  std::vector<CPythonSkill::TRequireStatData>::iterator itor =
      c_pSkillData->RequireStatDataVector.begin();
  for (; itor != c_pSkillData->RequireStatDataVector.end(); ++itor) {
    const CPythonSkill::TRequireStatData &c_rRequireStatData = *itor;

    if (POINT_LEVEL == c_rRequireStatData.byPoint)
      return Py_BuildValue("i", c_rRequireStatData.byLevel);
  }

  return Py_BuildValue("i", 0);
}

PyObject *skillIsToggleSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.IsToggleSkill - Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->IsToggleSkill());
}

PyObject *skillIsUseHPSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.IsUseHPSkill - Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->IsUseHPSkill());
}

PyObject *skillIsStandingSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.IsStandingSkill - Failed to find skill by %d", iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->IsStandingSkill());
}

PyObject *skillCanUseSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.CanUseSkill - Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->IsCanUseSkill());
}

PyObject *skillIsLevelUpSkill(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.IsLevelUpSkill - Failed to find skill by %d", iSkillIndex);

  // 모두 레벨업 가능

  return Py_BuildValue("i", TRUE);
}

PyObject *skillGetIconName(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildValue("s", "");

  return Py_BuildValue("s", c_pSkillData->strIconFileName.c_str());
}

PyObject *skillGetIconImage(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildValue("K", 0); // 익셉션을 내는 대신 0을 리턴한다.

  return Py_BuildValue("K", c_pSkillData->pImage);
}

PyObject *skillGetIconInstance(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetIconInstance - Failed to find skill by %d", iSkillIndex);

  CGraphicImageInstance *pImageInstance = CGraphicImageInstance::New();
  pImageInstance->SetImagePointer(c_pSkillData->pImage);

  return Py_BuildValue("K", pImageInstance);
}

PyObject *skillGetIconImageNew(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iGradeIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildValue("i", 0); // 익셉션을 내는 대신 0을 리턴한다.

  if (iGradeIndex < 0)
    iGradeIndex = 0;

  if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
    iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

  return Py_BuildValue("K", c_pSkillData->GradeData[iGradeIndex].pImage);
}

PyObject *skillGetIconInstanceNew(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iGradeIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetIconInstanceNew - Failed to find skill by %d", iSkillIndex);

  // CGraphicImage * pImage = c_pSkillData->pImage;

  if (iGradeIndex < 0)
    iGradeIndex = 0;

  if (iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
    iGradeIndex = CPythonSkill::SKILL_GRADE_COUNT - 1;

  CGraphicImageInstance *pImageInstance = CGraphicImageInstance::New();
  pImageInstance->SetImagePointer(c_pSkillData->GradeData[iGradeIndex].pImage);

  return Py_BuildValue("K", pImageInstance);
}

PyObject *skillDeleteIconInstance(PyObject *poSelf, PyObject *poArgs) {
  CGraphicImageInstance *pImgInstance;
  if (!PyTuple_GetPointer(poArgs, 0, &pImgInstance))
    return Py_BadArgument();

  CGraphicImageInstance::Delete(pImgInstance);
  return Py_BuildNone();
}

PyObject *skillGetGradeData(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  int iGradeIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iGradeIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.GetGradeData - Failed to find skill by %d",
                             iSkillIndex);

  if (iGradeIndex < 0 || iGradeIndex >= CPythonSkill::SKILL_GRADE_COUNT)
    return Py_BuildException("Strange grade index [%d]", iSkillIndex,
                             iGradeIndex);

  return Py_BuildValue("i", c_pSkillData->GradeData[iGradeIndex]);
}

PyObject *skillGetNewAffectDataCount(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException(
        "skill.GetNewAffectDataCount - Failed to find skill by %d",
        iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->AffectDataNewVector.size());
}

PyObject *skillGetNewAffectData(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();
  int iAffectIndex;
  if (!PyTuple_GetInteger(poArgs, 1, &iAffectIndex))
    return Py_BadArgument();
  float fSkillLevel;
  if (!PyTuple_GetFloat(poArgs, 2, &fSkillLevel))
    return Py_BadArgument();

  CPythonSkill::SSkillData *pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &pSkillData))
    return Py_BuildException(
        "skill.GetNewAffectData - Failed to find skill by %d", iSkillIndex);

  if (iAffectIndex < 0 ||
      iAffectIndex >= pSkillData->AffectDataNewVector.size())
    return Py_BuildException(" skill.GetNewAffectData - Strange AffectIndex %d",
                             iAffectIndex);

  CPythonSkill::TAffectDataNew &rAffectData =
      pSkillData->AffectDataNewVector[iAffectIndex];

  CPoly minPoly;
  CPoly maxPoly;
  minPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MIN);
  maxPoly.SetRandom(CPoly::RANDOM_TYPE_FORCE_MAX);
  minPoly.SetStr(rAffectData.strPointPoly.c_str());
  maxPoly.SetStr(rAffectData.strPointPoly.c_str());
  float fMinValue = pSkillData->ProcessFormula(&minPoly, fSkillLevel,
                                               CPythonSkill::VALUE_TYPE_MIN);
  float fMaxValue = pSkillData->ProcessFormula(&maxPoly, fSkillLevel,
                                               CPythonSkill::VALUE_TYPE_MAX);

  return Py_BuildValue("sff", rAffectData.strPointType.c_str(), fMinValue,
                       fMaxValue);
}

PyObject *skillGetDuration(PyObject *poSelf, PyObject *poArgs) {
  int iSkillIndex;
  if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
    return Py_BadArgument();
  float fSkillLevel;
  if (!PyTuple_GetFloat(poArgs, 1, &fSkillLevel))
    return Py_BadArgument();

  CPythonSkill::SSkillData *c_pSkillData;
  if (!CPythonSkill::Instance().GetSkillData(iSkillIndex, &c_pSkillData))
    return Py_BuildException("skill.GetDuration - Failed to find skill by %d",
                             iSkillIndex);

  return Py_BuildValue("i", c_pSkillData->GetDuration(fSkillLevel));
}

PyObject *skillTEST(PyObject *poSelf, PyObject *poArgs) {
  CPythonSkill::Instance().TEST();
  return Py_BuildNone();
}

void initskill() {
  static PyMethodDef s_methods[] = {
      {"SetPathName", skillSetPathName, METH_VARARGS},
      {"RegisterSkill", skillRegisterSkill, METH_VARARGS},
      {"LoadSkillData", skillLoadSkillData, METH_VARARGS},
      {"ClearSkillData", skillClearSkillData, METH_VARARGS},

      /////

      {"GetSkillName", skillGetSkillName, METH_VARARGS},
      {"GetSkillDescription", skillGetSkillDescription, METH_VARARGS},
      {"GetSkillType", skillGetSkillType, METH_VARARGS},
      {"GetSkillConditionDescriptionCount",
       skillGetSkillConditionDescriptionCount, METH_VARARGS},
      {"GetSkillConditionDescription", skillGetSkillConditionDescription,
       METH_VARARGS},
      {"GetSkillAffectDescriptionCount", skillGetSkillAffectDescriptionCount,
       METH_VARARGS},
      {"GetSkillAffectDescription", skillGetSkillAffectDescription,
       METH_VARARGS},
      {"GetSkillCoolTime", skillGetSkillCoolTime, METH_VARARGS},
      {"GetSkillNeedSP", skillGetSkillNeedSP, METH_VARARGS},
      {"GetSkillContinuationSP", skillGetSkillContinuationSP, METH_VARARGS},
      {"GetSkillMaxLevel", skillGetSkillMaxLevel, METH_VARARGS},
      {"GetSkillLevelUpPoint", skillGetSkillLevelUpPoint, METH_VARARGS},
      {"GetSkillLevelLimit", skillGetSkillLevelLimit, METH_VARARGS},
      {"IsSkillRequirement", skillIsSkillRequirement, METH_VARARGS},
      {"GetSkillRequirementData", skillGetSkillRequirementData, METH_VARARGS},
      {"GetSkillRequireStatCount", skillGetSkillRequireStatCount, METH_VARARGS},
      {"GetSkillRequireStatData", skillGetSkillRequireStatData, METH_VARARGS},
      {"CanLevelUpSkill", skillCanLevelUpSkill, METH_VARARGS},
      {"IsLevelUpSkill", skillIsLevelUpSkill, METH_VARARGS},
      {"CheckRequirementSueccess", skillCheckRequirementSueccess, METH_VARARGS},
      {"GetNeedCharacterLevel", skillGetNeedCharacterLevel, METH_VARARGS},
      {"IsToggleSkill", skillIsToggleSkill, METH_VARARGS},
      {"IsUseHPSkill", skillIsUseHPSkill, METH_VARARGS},
      {"IsStandingSkill", skillIsStandingSkill, METH_VARARGS},
      {"CanUseSkill", skillCanUseSkill, METH_VARARGS},
      {"GetIconName", skillGetIconName, METH_VARARGS},
      {"GetIconImage", skillGetIconImage, METH_VARARGS},
      {"GetIconImageNew", skillGetIconImageNew, METH_VARARGS},
      {"GetIconInstance", skillGetIconInstance, METH_VARARGS},
      {"GetIconInstanceNew", skillGetIconInstanceNew, METH_VARARGS},
      {"DeleteIconInstance", skillDeleteIconInstance, METH_VARARGS},
      {"GetGradeData", skillGetGradeData, METH_VARARGS},

      {"GetNewAffectDataCount", skillGetNewAffectDataCount, METH_VARARGS},
      {"GetNewAffectData", skillGetNewAffectData, METH_VARARGS},
      {"GetDuration", skillGetDuration, METH_VARARGS},

      {"TEST", skillTEST, METH_VARARGS},

      {NULL, NULL, NULL},
  };

  PyObject *poModule = Py_InitModule("skill", s_methods);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_NONE",
                          CPythonSkill::SKILL_TYPE_NONE);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_ACTIVE",
                          CPythonSkill::SKILL_TYPE_ACTIVE);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_SUPPORT",
                          CPythonSkill::SKILL_TYPE_SUPPORT);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_GUILD",
                          CPythonSkill::SKILL_TYPE_GUILD);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_HORSE",
                          CPythonSkill::SKILL_TYPE_HORSE);
  PyModule_AddIntConstant(poModule, "SKILL_TYPE_MAX_NUM",
                          CPythonSkill::SKILL_TYPE_MAX_NUM);

  PyModule_AddIntConstant(poModule, "SKILL_GRADE_COUNT",
                          CPythonSkill::SKILL_GRADE_COUNT);
  PyModule_AddIntConstant(poModule, "SKILL_GRADE_STEP_COUNT",
                          CPythonSkill::SKILL_GRADE_STEP_COUNT);
  PyModule_AddIntConstant(poModule, "SKILL_GRADEGAP",
                          CPythonSkill::SKILL_GRADEGAP);
  PyModule_AddIntConstant(poModule, "SKILL_EFFECT_COUNT",
                          CPythonSkill::SKILL_EFFECT_COUNT);
}
