#pragma once

#include "EffectLib/EffectManager.h"
#include "FlyingObjectManager.h"

namespace NMotionEvent {
typedef struct SMotionEventData {
  int iType;
  DWORD dwFrame;
  float fStartingTime;
  float fDurationTime;

  SMotionEventData() : dwFrame(0), fStartingTime(0.0f), fDurationTime(0.0f) {}
  virtual ~SMotionEventData() {}

  virtual void Save(FILE *File, int iTabs) = 0;
  virtual bool Load(CTextFileLoader &rTextFileLoader) { return true; }
} TMotionEventData;

// Screen Waving
typedef struct SMotionEventDataScreenWaving : public SMotionEventData {
  int iPower;
  int iAffectingRange;

  SMotionEventDataScreenWaving() {}
  virtual ~SMotionEventDataScreenWaving() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "DuringTime           %f\n", fDurationTime);
    PrintfTabs(File, iTabs, "Power                %d\n", iPower);
    PrintfTabs(File, iTabs, "AffectingRange       %d\n", iAffectingRange);
  }
  bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenFloat("duringtime", &fDurationTime))
      return false;
    if (!rTextFileLoader.GetTokenInteger("power", &iPower))
      return false;
    if (!rTextFileLoader.GetTokenInteger("affectingrange", &iAffectingRange))
      iAffectingRange = 0;

    return true;
  }
} TMotionEventDataScreenWaving;

// Screen Flashing
typedef struct SMotionEventDataScreenFlashing : public SMotionEventData {
  D3DXCOLOR FlashingColor;

  SMotionEventDataScreenFlashing() {}
  virtual ~SMotionEventDataScreenFlashing() {}

  void Save(FILE *File, int iTabs) {}
  bool Load(CTextFileLoader &rTextFileLoader) { return true; }
} TMotionEventDataScreenFlashing;

// Effect
typedef struct SMotionEventDataEffect : public SMotionEventData {
  BOOL isAttaching;
  BOOL isFollowing;
  BOOL isIndependent;
  std::string strAttachingBoneName;
  D3DXVECTOR3 v3EffectPosition;

  DWORD dwEffectIndex;
  std::string strEffectFileName;

  SMotionEventDataEffect() {}
  virtual ~SMotionEventDataEffect() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "IndependentFlag      %d\n", isIndependent);
    PrintfTabs(File, iTabs, "AttachingEnable      %d\n", isAttaching);
    PrintfTabs(File, iTabs, "AttachingBoneName    \"%s\"\n",
               strAttachingBoneName.c_str());
    PrintfTabs(File, iTabs, "FollowingEnable      %d\n", isFollowing);
    PrintfTabs(File, iTabs, "EffectFileName       \"%s\"\n",
               strEffectFileName.c_str());
    PrintfTabs(File, iTabs, "EffectPosition       %f %f %f\n",
               v3EffectPosition.x, v3EffectPosition.y, v3EffectPosition.z);
  }
  virtual bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenBoolean("independentflag", &isIndependent))
      isIndependent = FALSE;
    if (!rTextFileLoader.GetTokenBoolean("attachingenable", &isAttaching))
      return false;
    if (!rTextFileLoader.GetTokenString("attachingbonename",
                                        &strAttachingBoneName))
      return false;
    if (!rTextFileLoader.GetTokenString("effectfilename", &strEffectFileName))
      return false;
    if (!rTextFileLoader.GetTokenPosition("effectposition", &v3EffectPosition))
      return false;
    if (!rTextFileLoader.GetTokenBoolean("followingenable", &isFollowing)) {
      isFollowing = FALSE;
    }
    dwEffectIndex =
        GetCaseCRC32(strEffectFileName.c_str(), strEffectFileName.length());
    CEffectManager::Instance().RegisterEffect(strEffectFileName.c_str());

    return true;
  }
} TMotionEventDataEffect;

// Effect To Target
typedef struct SMotionEventDataEffectToTarget : public SMotionEventData {
  DWORD dwEffectIndex;

  std::string strEffectFileName;
  D3DXVECTOR3 v3EffectPosition;
  BOOL isFollowing;
  BOOL isFishingEffect;

  SMotionEventDataEffectToTarget() {}
  virtual ~SMotionEventDataEffectToTarget() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "EffectFileName       \"%s\"\n",
               strEffectFileName.c_str());
    PrintfTabs(File, iTabs, "EffectPosition       %f %f %f\n",
               v3EffectPosition.x, v3EffectPosition.y, v3EffectPosition.z);
    PrintfTabs(File, iTabs, "FollowingEnable      %d\n", isFollowing);
    PrintfTabs(File, iTabs, "FishingEffectFlag    %d\n", isFishingEffect);
  }
  virtual bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenString("effectfilename", &strEffectFileName))
      return false;
    if (!rTextFileLoader.GetTokenPosition("effectposition", &v3EffectPosition))
      return false;
    if (!rTextFileLoader.GetTokenBoolean("followingenable", &isFollowing)) {
      isFollowing = FALSE;
    }
    if (!rTextFileLoader.GetTokenBoolean("fishingeffectflag",
                                         &isFishingEffect)) {
      isFishingEffect = FALSE;
    }
    dwEffectIndex =
        GetCaseCRC32(strEffectFileName.c_str(), strEffectFileName.length());
    // #ifndef _DEBUG
    CEffectManager::Instance().RegisterEffect(strEffectFileName.c_str());
    // #endif

    return true;
  }
} TMotionEventDataEffectToTarget;

// Fly
typedef struct SMotionEventDataFly : public SMotionEventData {
  BOOL isAttaching;
  std::string strAttachingBoneName;
  D3DXVECTOR3 v3FlyPosition;

  DWORD dwFlyIndex;
  std::string strFlyFileName;

  SMotionEventDataFly() {}
  virtual ~SMotionEventDataFly() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "AttachingEnable      %d\n", isAttaching);
    PrintfTabs(File, iTabs, "AttachingBoneName    \"%s\"\n",
               strAttachingBoneName.c_str());
    PrintfTabs(File, iTabs, "FlyFileName       \"%s\"\n",
               strFlyFileName.c_str());
    PrintfTabs(File, iTabs, "FlyPosition       %f %f %f\n", v3FlyPosition.x,
               v3FlyPosition.y, v3FlyPosition.z);
  }
  bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenBoolean("attachingenable", &isAttaching))
      return false;
    if (!rTextFileLoader.GetTokenString("attachingbonename",
                                        &strAttachingBoneName))
      return false;
    if (!rTextFileLoader.GetTokenString("flyfilename", &strFlyFileName))
      return false;
    if (!rTextFileLoader.GetTokenPosition("flyposition", &v3FlyPosition))
      return false;
    dwFlyIndex = GetCaseCRC32(strFlyFileName.c_str(), strFlyFileName.length());

    // #ifndef _DEBUG
    // Register Fly
    CFlyingManager::Instance().RegisterFlyingData(strFlyFileName.c_str());
    // #endif

    return true;
  }
} TMotionEventDataFly;

// Attacking
typedef struct SMotionEventDataAttack : public SMotionEventData {
  NRaceData::TCollisionData CollisionData;
  NRaceData::TAttackData AttackData;
  BOOL isEnableHitProcess;

  SMotionEventDataAttack() {}
  virtual ~SMotionEventDataAttack() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "DuringTime           %f\n", fDurationTime);
    PrintfTabs(File, iTabs, "EnableHitProcess     %d\n", isEnableHitProcess);
    PrintfTabs(File, iTabs, "\n");

    NRaceData::SaveAttackData(File, iTabs, AttackData);
    NRaceData::SaveCollisionData(File, iTabs, CollisionData);
  }
  bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenFloat("duringtime", &fDurationTime))
      return false;

    if (!rTextFileLoader.GetTokenBoolean("enablehitprocess",
                                         &isEnableHitProcess)) {
      isEnableHitProcess = TRUE;
    }

    if (!NRaceData::LoadAttackData(rTextFileLoader, &AttackData))
      return false;

    if (!NRaceData::LoadCollisionData(rTextFileLoader, &CollisionData))
      return false;

    return true;
  }
} TMotionEventDataAttacking;

// Sound
typedef struct SMotionEventDataSound : public SMotionEventData {
  std::string strSoundFileName; // Direct Sound Node

  SMotionEventDataSound() {}
  virtual ~SMotionEventDataSound() {}

  void Save(FILE *File, int iTabs) {
    PrintfTabs(File, iTabs, "\n");
    PrintfTabs(File, iTabs, "SoundFileName        \"%s\"\n",
               strSoundFileName.c_str());
  }
  bool Load(CTextFileLoader &rTextFileLoader) {
    if (!rTextFileLoader.GetTokenString("soundfilename", &strSoundFileName))
      return false;

    return true;
  }
} TMotionEventDataSound;

// Character Show
typedef struct SMotionEventDataCharacterShow : public SMotionEventData {
  SMotionEventDataCharacterShow() {}
  virtual ~SMotionEventDataCharacterShow() {}

  void Save(FILE *File, int iTabs) {}
  void Load() {}
} TMotionEventDataCharacterShow;

// Character Hide
typedef struct SMotionEventDataCharacterHide : public SMotionEventData {
  SMotionEventDataCharacterHide() {}
  virtual ~SMotionEventDataCharacterHide() {}

  void Save(FILE *File, int iTabs) {}
  void Load() {}
} TMotionEventDataCharacterHide;

// Warp
typedef struct SMotionEventDataWarp : public SMotionEventData {
  SMotionEventDataWarp() {}
  virtual ~SMotionEventDataWarp() {}

  void Save(FILE *File, int iTabs) {}
  void Load() {}
} TMotionWarpEventData;
}; // namespace NMotionEvent