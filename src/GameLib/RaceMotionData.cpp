#include "StdAfx.h"
#include "EffectLib/EffectManager.h"
#include "FlyingObjectManager.h"
#include "RaceMotionData.h"

CDynamicPool<CRaceMotionData> CRaceMotionData::ms_kPool;

void CRaceMotionData::CreateSystem(UINT uCapacity) {
  ms_kPool.Create(uCapacity);
}

void CRaceMotionData::DestroySystem() { ms_kPool.Clear(); }

CRaceMotionData *CRaceMotionData::New() { return ms_kPool.Alloc(); }

void CRaceMotionData::Delete(CRaceMotionData *pkData) {
  pkData->Destroy();

  ms_kPool.Free(pkData);
}

void CRaceMotionData::SetName(UINT eName) {
  m_eName = eName;

  switch (m_eName) {
  case NAME_NONE:
    SetType(TYPE_NONE);
    break;
  case NAME_WAIT:
  case NAME_INTRO_WAIT:
  case NAME_STOP:
    SetType(TYPE_WAIT);
    break;
  case NAME_WALK:
  case NAME_RUN:
    SetType(TYPE_MOVE);
    break;
  case NAME_DAMAGE:
  case NAME_DAMAGE_BACK:
    SetType(TYPE_DAMAGE);
    break;
  case NAME_DAMAGE_FLYING:
  case NAME_DAMAGE_FLYING_BACK:
    SetType(TYPE_KNOCKDOWN);
    break;
  case NAME_STAND_UP:
  case NAME_STAND_UP_BACK:
    SetType(TYPE_STANDUP);
    break;
  case NAME_SPAWN:
  case NAME_CHANGE_WEAPON:
  case NAME_INTRO_SELECTED:
  case NAME_INTRO_NOT_SELECTED:
  case NAME_SPECIAL_1:
  case NAME_SPECIAL_2:
  case NAME_SPECIAL_3:
  case NAME_SPECIAL_4:
  case NAME_SPECIAL_5:
  case NAME_SPECIAL_6:
  case NAME_CLAP:
  case NAME_DANCE_1:
  case NAME_DANCE_2:
  case NAME_DANCE_3:
  case NAME_DANCE_4:
  case NAME_DANCE_5:
  case NAME_DANCE_6:
  case NAME_CONGRATULATION:
  case NAME_FORGIVE:
  case NAME_ANGRY:
  case NAME_ATTRACTIVE:
  case NAME_SAD:
  case NAME_SHY:
  case NAME_CHEERUP:
  case NAME_BANTER:
  case NAME_JOY:
  case NAME_CHEERS_1:
  case NAME_CHEERS_2:
  case NAME_KISS_WITH_WARRIOR:
  case NAME_KISS_WITH_ASSASSIN:
  case NAME_KISS_WITH_SURA:
  case NAME_KISS_WITH_SHAMAN:
  case NAME_FRENCH_KISS_WITH_WARRIOR:
  case NAME_FRENCH_KISS_WITH_ASSASSIN:
  case NAME_FRENCH_KISS_WITH_SURA:
  case NAME_FRENCH_KISS_WITH_SHAMAN:
  case NAME_SLAP_HIT_WITH_WARRIOR:
  case NAME_SLAP_HIT_WITH_ASSASSIN:
  case NAME_SLAP_HIT_WITH_SURA:
  case NAME_SLAP_HIT_WITH_SHAMAN:
  case NAME_SLAP_HURT_WITH_WARRIOR:
  case NAME_SLAP_HURT_WITH_ASSASSIN:
  case NAME_SLAP_HURT_WITH_SURA:
  case NAME_SLAP_HURT_WITH_SHAMAN:
  case NAME_DIG:
    SetType(TYPE_EVENT);
    break;
  case NAME_DEAD:
  case NAME_DEAD_BACK:
    SetType(TYPE_DIE);
    break;
  case NAME_NORMAL_ATTACK:
    SetType(TYPE_ATTACK);
    break;
  case NAME_COMBO_ATTACK_1:
  case NAME_COMBO_ATTACK_2:
  case NAME_COMBO_ATTACK_3:
  case NAME_COMBO_ATTACK_4:
  case NAME_COMBO_ATTACK_5:
  case NAME_COMBO_ATTACK_6:
  case NAME_COMBO_ATTACK_7:
  case NAME_COMBO_ATTACK_8:
    SetType(TYPE_COMBO);
    break;
  case NAME_FISHING_THROW:
  case NAME_FISHING_WAIT:
  case NAME_FISHING_REACT:
  case NAME_FISHING_CATCH:
  case NAME_FISHING_FAIL:
  case NAME_FISHING_STOP:
    SetType(TYPE_FISHING);
    break;
  default:
    if (eName >= NAME_SKILL && eName <= NAME_SKILL + SKILL_NUM)
      SetType(TYPE_SKILL);
    else
      TraceError("CRaceMotionData::SetName - UNKNOWN NAME %d", eName);
    break;
  }
}
void CRaceMotionData::SetType(UINT eType) {
  m_eType = eType;

  switch (m_eType) {
  case TYPE_ATTACK:
  case TYPE_COMBO:
  case TYPE_SKILL:
    m_isLock = TRUE;
    break;
  default:
    m_isLock = FALSE;
    break;
  }
}

UINT CRaceMotionData::GetType() const { return m_eType; }

bool CRaceMotionData::IsLock() const { return m_isLock ? true : false; }

int CRaceMotionData::GetLoopCount() const { return m_iLoopCount; }

float CRaceMotionData::GetMotionDuration() { return m_fMotionDuration; }

void CRaceMotionData::SetMotionDuration(float fDuration) {
  m_fMotionDuration = fDuration;
}

// Combo
BOOL CRaceMotionData::IsComboInputTimeData() const { return m_isComboMotion; }

float CRaceMotionData::GetComboInputStartTime() const {
  assert(m_isComboMotion);
  return m_ComboInputData.fInputStartTime;
}

float CRaceMotionData::GetNextComboTime() const {
  assert(m_isComboMotion);
  return m_ComboInputData.fNextComboTime;
}

float CRaceMotionData::GetComboInputEndTime() const {
  assert(m_isComboMotion);
  return m_ComboInputData.fInputEndTime;
}

// Attacking
BOOL CRaceMotionData::isAttackingMotion() const { return m_isAttackingMotion; }

const NRaceData::TMotionAttackData *
CRaceMotionData::GetMotionAttackDataPointer() const {
  return &m_MotionAttackData;
}

const NRaceData::TMotionAttackData &
CRaceMotionData::GetMotionAttackDataReference() const {
  assert(m_isAttackingMotion);
  return m_MotionAttackData;
}

BOOL CRaceMotionData::HasSplashMotionEvent() const { return m_hasSplashEvent; }

// Skill
BOOL CRaceMotionData::IsCancelEnableSkill() const {
  return m_bCancelEnableSkill;
}

// Loop
BOOL CRaceMotionData::IsLoopMotion() const { return m_isLoopMotion; }

float CRaceMotionData::GetLoopStartTime() const { return m_fLoopStartTime; }

float CRaceMotionData::GetLoopEndTime() const { return m_fLoopEndTime; }

// Motion Event Data
DWORD CRaceMotionData::GetMotionEventDataCount() const {
  return m_MotionEventDataVector.size();
}

BOOL CRaceMotionData::GetMotionEventDataPointer(
    BYTE byIndex, const CRaceMotionData::TMotionEventData **c_ppData) const {
  if (byIndex >= m_MotionEventDataVector.size())
    return FALSE;

  *c_ppData = m_MotionEventDataVector[byIndex];

  return TRUE;
}

BOOL CRaceMotionData::GetMotionAttackingEventDataPointer(
    BYTE byIndex,
    const CRaceMotionData::TMotionAttackingEventData **c_ppData) const {
  if (byIndex >= m_MotionEventDataVector.size())
    return FALSE;

  const CRaceMotionData::TMotionEventData *pData =
      m_MotionEventDataVector[byIndex];
  const CRaceMotionData::TMotionAttackingEventData *pAttackingEvent =
      (const CRaceMotionData::TMotionAttackingEventData *)pData;

  if (MOTION_EVENT_TYPE_SPECIAL_ATTACKING == pAttackingEvent->iType)
    return FALSE;

  *c_ppData = pAttackingEvent;

  return TRUE;
}

int CRaceMotionData::GetEventType(DWORD dwIndex) const {
  if (dwIndex >= m_MotionEventDataVector.size())
    return MOTION_EVENT_TYPE_NONE;

  return m_MotionEventDataVector[dwIndex]->iType;
}

float CRaceMotionData::GetEventStartTime(DWORD dwIndex) const {
  if (dwIndex >= m_MotionEventDataVector.size())
    return 0.0f;

  return m_MotionEventDataVector[dwIndex]->fStartingTime;
}

const NSound::TSoundInstanceVector *
CRaceMotionData::GetSoundInstanceVectorPointer() const {
  return &m_SoundInstanceVector;
}

void CRaceMotionData::SetAccumulationPosition(const TPixelPosition &c_rPos) {
  m_accumulationPosition = c_rPos;
  m_isAccumulationMotion = TRUE;
}

bool CRaceMotionData::LoadMotionData(const char *c_szFileName) {
  const float c_fFrameTime = 1.0f / g_fGameFPS;

  CTextFileLoader *pkTextFileLoader = CTextFileLoader::Cache(c_szFileName);
  if (!pkTextFileLoader)
    return false;

  CTextFileLoader &rkTextFileLoader = *pkTextFileLoader;

  if (rkTextFileLoader.IsEmpty())
    return false;

  rkTextFileLoader.SetTop();

  if (!rkTextFileLoader.GetTokenString("motionfilename", &m_strMotionFileName))
    return false;

  if (!rkTextFileLoader.GetTokenFloat("motionduration", &m_fMotionDuration))
    return false;

  CTokenVector *pTokenVector;

  if (rkTextFileLoader.GetTokenVector("accumulation", &pTokenVector)) {
    if (pTokenVector->size() != 3) {
      TraceError("CRaceMotioNData::LoadMotionData : syntax error on "
                 "accumulation, vector size %d",
                 pTokenVector->size());
      return false;
    }

    TPixelPosition pos(atof(pTokenVector->at(0).c_str()),
                       atof(pTokenVector->at(1).c_str()),
                       atof(pTokenVector->at(2).c_str()));

    SetAccumulationPosition(pos);
  }

  std::string strNodeName;
  for (DWORD i = 0; i < rkTextFileLoader.GetChildNodeCount(); ++i) {
    CTextFileLoader::CGotoChild GotoChild(&rkTextFileLoader, i);

    rkTextFileLoader.GetCurrentNodeName(&strNodeName);

    if (0 == strNodeName.compare("comboinputdata")) {
      m_isComboMotion = TRUE;

      if (!rkTextFileLoader.GetTokenFloat("preinputtime",
                                          &m_ComboInputData.fInputStartTime))
        return false;
      if (!rkTextFileLoader.GetTokenFloat("directinputtime",
                                          &m_ComboInputData.fNextComboTime))
        return false;
      if (!rkTextFileLoader.GetTokenFloat("inputlimittime",
                                          &m_ComboInputData.fInputEndTime))
        return false;
    } else if (0 == strNodeName.compare("attackingdata")) {
      m_isAttackingMotion = TRUE;

      if (!NRaceData::LoadMotionAttackData(rkTextFileLoader,
                                           &m_MotionAttackData))
        return false;
    } else if (0 == strNodeName.compare("loopdata")) {
      m_isLoopMotion = TRUE;
      if (!rkTextFileLoader.GetTokenInteger("motionloopcount", &m_iLoopCount)) {
        m_iLoopCount = -1;
      }
      if (!rkTextFileLoader.GetTokenInteger("loopcancelenable",
                                            &m_bCancelEnableSkill)) {
        m_bCancelEnableSkill = FALSE;
      }
      if (!rkTextFileLoader.GetTokenFloat("loopstarttime", &m_fLoopStartTime))
        return false;
      if (!rkTextFileLoader.GetTokenFloat("loopendtime", &m_fLoopEndTime))
        return false;
    } else if (0 == strNodeName.compare("motioneventdata")) {
      DWORD dwMotionEventDataCount;

      if (!rkTextFileLoader.GetTokenDoubleWord("motioneventdatacount",
                                               &dwMotionEventDataCount))
        continue;

      stl_wipe(m_MotionEventDataVector);

      m_MotionEventDataVector.resize(dwMotionEventDataCount, NULL);

      for (DWORD j = 0; j < m_MotionEventDataVector.size(); ++j) {
        if (!rkTextFileLoader.SetChildNode("event", j))
          return false;

        int iType;
        if (!rkTextFileLoader.GetTokenInteger("motioneventtype", &iType))
          return false;

        TMotionEventData *pData = NULL;
        switch (iType) {
        case MOTION_EVENT_TYPE_FLY:
          pData = new TMotionFlyEventData;
          break;
        case MOTION_EVENT_TYPE_EFFECT:
          pData = new TMotionEffectEventData;
          break;
        case MOTION_EVENT_TYPE_SCREEN_WAVING:
          pData = new TScreenWavingEventData;
          break;
        case MOTION_EVENT_TYPE_SPECIAL_ATTACKING:
          pData = new TMotionAttackingEventData;
          m_hasSplashEvent = TRUE;
          break;
        case MOTION_EVENT_TYPE_SOUND:
          pData = new TMotionSoundEventData;
          break;
        case MOTION_EVENT_TYPE_CHARACTER_SHOW:
          pData = new TMotionCharacterShowEventData;
          break;
        case MOTION_EVENT_TYPE_CHARACTER_HIDE:
          pData = new TMotionCharacterHideEventData;
          break;
        case MOTION_EVENT_TYPE_WARP:
          pData = new TMotionWarpEventData;
          break;
        case MOTION_EVENT_TYPE_EFFECT_TO_TARGET:
          pData = new TMotionEffectToTargetEventData;
          break;
        default:
          assert(!" CRaceMotionData::LoadMotionData - Strange Event Type");
          return false;
          break;
        }
        m_MotionEventDataVector[j] = pData;
        m_MotionEventDataVector[j]->Load(rkTextFileLoader);
        m_MotionEventDataVector[j]->iType = iType;
        if (!rkTextFileLoader.GetTokenFloat(
                "startingtime", &m_MotionEventDataVector[j]->fStartingTime))
          return false;

        m_MotionEventDataVector[j]->dwFrame =
            (m_MotionEventDataVector[j]->fStartingTime / c_fFrameTime);

        rkTextFileLoader.SetParentNode();
      }
    }
  }

  std::string strSoundFileNameTemp = c_szFileName;
  strSoundFileNameTemp = CFileNameHelper::NoExtension(strSoundFileNameTemp);
  strSoundFileNameTemp += ".mss";

  if (strSoundFileNameTemp.length() > 13) {
    const char *c_szHeader = &strSoundFileNameTemp[13];

    m_strSoundScriptDataFileName = "sound/";
    m_strSoundScriptDataFileName += c_szHeader;

    LoadSoundScriptData(m_strSoundScriptDataFileName.c_str());
  }

  return true;
}

bool CRaceMotionData::LoadSoundScriptData(const char *c_szFileName) {
  NSound::TSoundDataVector SoundDataVector;
  if (!LoadSoundInformationPiece(c_szFileName, SoundDataVector)) {
    return false;
  }

  DataToInstance(SoundDataVector, &m_SoundInstanceVector);
  return true;
}

const char *CRaceMotionData::GetMotionFileName() const {
  return m_strMotionFileName.c_str();
}

const char *CRaceMotionData::GetSoundScriptFileName() const {
  return m_strSoundScriptDataFileName.c_str();
}

void CRaceMotionData::Initialize() {
  m_iLoopCount = 0;
  m_fMotionDuration = 0.0f;
  m_accumulationPosition.x = 0.0f;
  m_accumulationPosition.y = 0.0f;
  m_accumulationPosition.z = 0.0f;
  m_fLoopStartTime = 0.0f;
  m_fLoopEndTime = 0.0f;

  m_isAccumulationMotion = FALSE;
  m_isComboMotion = FALSE;
  m_isLoopMotion = FALSE;
  m_isAttackingMotion = FALSE;
  m_bCancelEnableSkill = FALSE;

  m_hasSplashEvent = FALSE;

  m_isLock = FALSE;

  m_eType = TYPE_NONE;
  m_eName = NAME_NONE;

  m_MotionEventDataVector.clear();
  m_SoundInstanceVector.clear();
}

void CRaceMotionData::Destroy() {
  stl_wipe(m_MotionEventDataVector);
  Initialize();
}

CRaceMotionData::CRaceMotionData() { Initialize(); }

CRaceMotionData::~CRaceMotionData() { Destroy(); }
